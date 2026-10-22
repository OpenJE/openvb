#!/usr/bin/env bash
# tools/rig-g1-run.sh — the board's WARM-RIG runner (rig keeper: RedQuartz, seat since 185e393-era)
#
# What this is: the committed form of the named repro line that produced the G1 verdict
# (tasks/R-RIG-redquartz-rig-g1.md, discoveries/2025-10-21-t056-xorg-dummy-six-rows.md,
# discoveries/2026-10-21-t067-dxvk-rung2-depth-harvest.md). Any lane needing a REAL D3D8
# device (slice-2 frame acceptance, R-FINISH device gates, item-5 sink-proofs) runs THIS,
# so nobody re-invents the prefix. Idempotent: warm parts are reused, missing parts built.
#
# The five non-obvious keys (each was somebody's wall — do not "simplify" them away):
#   1. Xorg :96 config MUST carry the Monitor HorizSync/VertRefresh + Modeline sections, or
#      RandR publishes vrefresh=0 modes. (/tmp/xorg-dummy.conf-style configs = dead end.)
#   2. DXVK x32 needs THREE dlls — d3d8 imports Direct3DCreate9 from d3d9; d3d8 alone = AV
#      at d3d8+0x1305a. dxgi rides along (T-067 verified set).
#   3. Overrides MUST be registry (HKCU\Software\Wine\DllOverrides), NOT WINEDLLOVERRIDES env
#      — wine-11 silently ignores env for d3d9 ("get_load_order got hardcoded default").
#   4. ICD = the nvidia 32-bit json (VK_DRIVER_FILES). NOT lavapipe (lvp32 OOMs); and do NOT
#      set LIBGL_ALWAYS_SOFTWARE/GALLIUM softpipe once DXVK owns GL (32-bit LLVM OOM storm).
#   5. WAYLAND_DISPLAY stays inherited; DISPLAY=:96 stays for RandR metadata (hybrid stack).
#
# COLD BOOTSTRAP IS PROVEN (keeper run 2026-10-22: bare wineboot prefix + fresh Xorg :97 ->
# Direct3DCreate8 non-NULL, CreateDevice OK, 3300+ presents, rc=124, magenta capture). Two
# errata this proof corrected:
#   * msvcr70/msvcp70 are NOT missing on a bare prefix — wine-11 ships them natively
#     (lib/wine/i386-windows/*.dll, byte-identical to the t056 template's copies, which were
#     therefore never a manual install). The .devenv/state/rig-crt-seed/ pair below is harmless
#     redundancy insurance against a wine that ever stops shipping them.
#   * A mid-run root capture can legitimately be BLACK while the device is healthy — early in
#     boot the intro movie occludes the magenta clear (cold run: black @ T+45s, magenta @ T+8s).
#     The TRACE lines are device truth; the capture is wall truth. Trust both only in that order.
#
# Usage:  tools/rig-g1-run.sh                     # gated run of build/F3.exe, 120s window
#         TIMEOUT_S=45 SCREENSHOT=1 tools/rig-g1-run.sh   # short window + mid-run capture
# Env knobs: RIG_DIR (default /tmp/r-rig-redquartz), EXE (default $PWD/build/F3.exe),
#   TIMEOUT_S (120), DISPLAY_NUM (96), CAPTURE_AT (20, with SCREENSHOT=1), TRACE2 (g1-trace.log).
# Exit code mirrors `timeout`: 124 = ALIVE at reap (the good answer), 1 = exited on its own,
# 40 = hard fault. Written to $RIG_DIR/g1-rc.txt with the trace at $RIG_DIR/<TRACE2>.

set -uo pipefail

RIG_DIR="${RIG_DIR:-/tmp/r-rig-redquartz}"
EXE="${EXE:-$PWD/build/F3.exe}"
TIMEOUT_S="${TIMEOUT_S:-120}"
DISPLAY_NUM="${DISPLAY_NUM:-96}"
ICD="${VK_DRIVER_FILES:-/run/opengl-driver-32/share/vulkan/icd.d/nvidia_icd.json}"
DEMO_ROOT="${OPENVB_F3DEMO_ROOT:-}"
PREFIX_TEMPLATE="${PREFIX_TEMPLATE:-/tmp/t056-wineprefix}"   # proven set, if this host still has it
FAIL=0
say(){ printf 'rig: %s\n' "$*"; }
die(){ printf 'rig FAIL: %s\n' "$*" >&2; FAIL=1; }

first_of(){ ls -d "$@" 2>/dev/null | head -1; }

# ── 0. locate the store pieces ────────────────────────────────────────────────
XORG=$(first_of /nix/store/*-xorg-server-*/bin/Xorg)
DUMMY_DRV=$(first_of /nix/store/*-xf86-video-dummy-*/lib/xorg/modules/drivers)
XORG_SRV=$(first_of /nix/store/*-xorg-server-*/lib/xorg/modules)
WINE=$(command -v wine 2>/dev/null || first_of /nix/store/*-wine-*/bin/wine)
DXVK_BIN=$(first_of /nix/store/*-dxvk-i686-w64-mingw32-*/bin)
[ -n "$XORG" ]      || die "no xorg-server in /nix/store (user-scoped `nix build nixpkgs#xorg.xorgserver`)"
[ -n "$DUMMY_DRV" ] || die "no xf86-video-dummy (nix build nixpkgs#xf86-video-dummy)"
[ -n "$WINE" ]      || die "no wine on PATH or in store"
[ -n "$DXVK_BIN" ]  || die "no dxvk x32 (nix build nixpkgs#dxvk; the i686 output)"
[ -f "$ICD" ]       || die "no 32-bit nvidia ICD at $ICD (NOT lavapipe — lvp32 OOM'd)"
[ -f "$EXE" ]       || die "no exe at $EXE (msvc-build first)"
[ "$FAIL" = 0 ] || { say "missing pieces above — fix and re-run (recipe: this header)"; exit 2; }
mkdir -p "$RIG_DIR/dxvk-logs"

# ── 1. Xorg :$DISPLAY_NUM (config embedded for durability — vrefresh sections are the trick) ─
cat > "$RIG_DIR/xorg.conf" <<EOF
Section "Files"
    ModulePath  "$XORG_SRV"
    ModulePath  "$DUMMY_DRV"
EndSection
Section "Module"
    Load  "glx"
    Load  "dri"
    Load  "randr"
EndSection
Section "Device"
    Identifier  "dummy0"
    Driver      "dummy"
    VideoRam    65536
EndSection
Section "Monitor"
    Identifier  "mon0"
    HorizSync   30-90
    VertRefresh 59-75
    Modeline "800x600"  40.00 800 840 880 960  600 603 607 620 +hsync +vsync
    Modeline "1024x768" 65.00 1024 1048 1184 1344  768 771 777 806 -hsync -vsync
    Option "Enable" "true"
EndSection
Section "Screen"
    Identifier  "scr0"
    Device      "dummy0"
    Monitor     "mon0"
    DefaultDepth 24
    Subsection "Display"
        Depth       24
        Modes       "800x600" "1024x768"
    EndSubsection
EndSection
Section "ServerFlags"
    Option "DontZap" "true"
EndSection
EOF
if [ -S "/tmp/.X11-unix/X${DISPLAY_NUM}" ]; then
    say ":$DISPLAY_NUM already warm (socket present)"
else
    say "starting Xorg :$DISPLAY_NUM"
    nohup "$XORG" ":$DISPLAY_NUM" -config "$RIG_DIR/xorg.conf" -noreset \
        +extension GLX +extension RANDR > "$RIG_DIR/xorg.log" 2>&1 &
    echo $! > "$RIG_DIR/xorg.pid"
    sleep 3
    [ -S "/tmp/.X11-unix/X${DISPLAY_NUM}" ] || { tail -5 "$RIG_DIR/xorg.log"; die ":$DISPLAY_NUM never came up"; exit 2; }
fi

# ── 2. private win32 prefix (template first; full bootstrap otherwise) ───────
if [ ! -f "$RIG_DIR/prefix/system.reg" ]; then
    if [ -d "$PREFIX_TEMPLATE" ]; then
        say "cloning proven template $PREFIX_TEMPLATE"
        cp -a "$PREFIX_TEMPLATE" "$RIG_DIR/prefix"
    else
        say "bootstrapping fresh win32 prefix (wineboot)"
        mkdir -p "$RIG_DIR/prefix"
        WINEPREFIX="$RIG_DIR/prefix" WINEARCH=win32 "$WINE" wineboot -u > "$RIG_DIR/wineboot.log" 2>&1 || die "wineboot failed, see $RIG_DIR/wineboot.log"
    fi
    S32="$RIG_DIR/prefix/drive_c/windows/system32"
    say "copying the THREE dxvk x32 dlls (d3d8+d3d9+dxgi — d3d9 is NOT optional)"
    for d in d3d8 d3d9 dxgi; do cp -f "$DXVK_BIN/$d.dll" "$S32/"; done
    say "writing the TWO registry DllOverrides lines (env overrides silently fail for d3d9)"
    for d in d3d8 d3d9; do
        WINEPREFIX="$RIG_DIR/prefix" "$WINE" reg add 'HKCU\Software\Wine\DllOverrides' \
            /v "$d" /t REG_SZ /d n /f >> "$RIG_DIR/reg.log" 2>&1 || die "reg add $d failed"
    done
    if [ ! -f "$S32/msvcr70.dll" ] || [ ! -f "$S32/msvcp70.dll" ]; then
        CRT_SEED_DEFAULT="$(cd "$(dirname "$0")/.." 2>/dev/null && pwd)/.devenv/state/rig-crt-seed"
        CRT_SEED="${CRT_SEED:-$CRT_SEED_DEFAULT}"
        for c in msvcr70 msvcp70; do
            if [ -f "$CRT_SEED/$c.dll" ]; then
                cp -f "$CRT_SEED/$c.dll" "$S32/" && say "seeded $c.dll from $CRT_SEED"
            else
                say "WARN: prefix lacks $c.dll and no seed at $CRT_SEED/$c.dll — the exe imports it;
        stage from the VC7.0 redist or reuse the t056 template (PREFIX_TEMPLATE=<dir>)."
            fi
        done
    fi
fi

# ── 3. data stage (same shape as f3-reimpl-run-data) ──────────────────────────
if [ -n "$DEMO_ROOT" ]; then
    mkdir -p "$RIG_DIR/data"
    find "$RIG_DIR/data" -mindepth 1 -maxdepth 1 -type l -delete
    for entry in "$DEMO_ROOT"/*; do ln -sf "$entry" "$RIG_DIR/data/$(basename "$entry")"; done
    for d in granny2.dll binkw32.dll; do [ -e "$(dirname "$EXE")/$d" ] || ln -sf "$DEMO_ROOT/$d" "$(dirname "$EXE")/$d" 2>/dev/null; done
    RUN_CWD="$RIG_DIR/data"
else
    say "WARN: OPENVB_F3DEMO_ROOT unset — running from rig dir without staged data; boot may FatalError AFTER PHASE1"
    RUN_CWD="$RIG_DIR"
fi

# ── 4. the named run (G1 repro) ───────────────────────────────────────────────
TRACE2="${TRACE2:-g1-trace.log}"
# KEEPER LEDGER: the board's exe is a moving target (lanes rebuild build/F3.exe constantly —
# one literally vanished mid-window at 23:55). Every rig window records the HEAD + exe hash it
# ran, so a verdict can never be quoted against bytes it didn't see.
{
    printf 'run-window %s | head=%s | exe=%s | sha256=%s\n' \
        "$(date -u +%FT%TZ)" \
        "$(git -C "$(dirname "$0")/.." rev-parse --short HEAD 2>/dev/null || echo NO-GIT)" \
        "$EXE" \
        "$(sha256sum "$EXE" 2>/dev/null | cut -d' ' -f1 || echo MISSING)"
} >> "$RIG_DIR/g1-runs.log"
WIN_TRACE="Z:$(echo "$RIG_DIR/$TRACE2" | sed 's#/#\\#g')"
say "run: exe=$EXE timeout=${TIMEOUT_S}s trace=$RIG_DIR/$TRACE2 rc->g1-rc.txt"
( cd "$RUN_CWD" && \
  WINEPREFIX="$RIG_DIR/prefix" WINEARCH=win32 \
  WINEDLLOVERRIDES='mscoree,mshtml=' WINEDEBUG='-all,+err' \
  DISPLAY=":${DISPLAY_NUM}" VK_DRIVER_FILES="$ICD" \
  DXVK_LOG_LEVEL=info DXVK_LOG_PATH="Z:$(echo "$RIG_DIR/dxvk-logs" | sed 's#/#\\#g')" \
  OPENVB_RENDER=1 OPENVB_RENDER_TRACE="$WIN_TRACE" \
  timeout "$TIMEOUT_S" "$WINE" "$EXE" ) > "$RIG_DIR/g1-stderr.log" 2>&1 &
RUNPID=$!

if [ "${SCREENSHOT:-0}" = 1 ]; then
    sleep "${CAPTURE_AT:-20}"
    if command -v import >/dev/null 2>&1; then
        # KEEPER LAW GATE (item 59): a black root capture can coexist with a HEALTHY device —
        # the intro movie occludes the magenta clear early in boot (cold run: black@T+45s,
        # magenta@T+8s). Poll the capture until the dominant pixel is magenta (wall truth) or
        # tries run out — a kept g1-frame.png is then guaranteed post-occlusion, never a false FAIL.
        tries=0
        CAPTURE_TRIES_N="${CAPTURE_TRIES:-6}"
        until DISPLAY=":${DISPLAY_NUM}" import -window root "$RIG_DIR/g1-frame.png" 2>/dev/null \
              && convert "$RIG_DIR/g1-frame.png" -format %c histogram:info:- 2>/dev/null | sort -rn | head -1 | grep -q '(255,0,255)' \
              || [ "$tries" -ge "$CAPTURE_TRIES_N" ]; do
            tries=$((tries+1)); sleep "${CAPTURE_GAP:-5}"
        done
        if [ "$tries" = 0 ]; then
            say "captured $RIG_DIR/g1-frame.png at T+${CAPTURE_AT:-20}s (magenta-dominant, first try)"
        elif [ "$tries" -ge "$CAPTURE_TRIES_N" ] && ! convert "$RIG_DIR/g1-frame.png" -format %c histogram:info:- 2>/dev/null | sort -rn | head -1 | grep -q '(255,0,255)'; then
            say "WARN: kept capture is NOT magenta-dominant after $tries tries — device trace is still the truth; wall may be mid-intro (keeper law, item 59)"
        else
            say "captured $RIG_DIR/g1-frame.png after $tries re-tries (intro occlusion cleared)"
        fi
    else
        say "no ImageMagick `import` on PATH — skipping capture"
    fi
fi

wait $RUNPID
RC=$?
echo $RC > "$RIG_DIR/g1-rc.txt"
say "exit code $RC (124=alive at reap — good; 1=exited; 40=faulted)"
tail -3 "$RIG_DIR/$TRACE2" 2>/dev/null
exit $RC
