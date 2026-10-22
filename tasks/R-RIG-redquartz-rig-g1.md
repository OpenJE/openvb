---
status: verdict delivered — seat CONVERTED to RIG KEEPER (QuickNova, post-185e393): device-gated test
  support for slice-2 + item-5 sink-proofs; board entry point = tools/rig-g1-run.sh (committed,
  keeper-verified rc=124 + mid-run capture on the warm rig). Release only when the queue says idle.
assignee: RedQuartz (2nd successor; dispatch from QuickNova 23:31, seat re-issued verbatim)
depends: T-067 (DXVK stack recipe), T-056 (Xorg-dummy :96 recipe), M-001 (the G1 question)
---

# R-RIG — Rig rebuild + G1 (write-new-rig only)

## Mission (three steps, per dispatch)
1. Rebuild the known-good rendering rig: Xorg :96 (dummy driver + vrefresh trick),
   private win32 wine prefix, DXVK x32 THREE dlls (d3d8/d3d9/**dxgi**), the TWO
   HKCU\Software\Wine\DllOverrides REG lines (env overrides silently fail for d3d9 on wine-11),
   ICD = nvidia **32-bit** json via VK_DRIVER_FILES (hardware, NOT lavapipe — lvp32 OOM'd).
2. G1: run the gated reimpl exe (`build/F3.exe`, HEAD `ece5726` era, imports d3d8.dll for real —
   objdump-verified this sitting) with `OPENVB_RENDER=1` + `OPENVB_RENDER_TRACE=<file>`.
3. Report from the trace: **did Direct3DCreate8 EVER return non-NULL on this host?**
   Both answers are gold. Honest-negative re-shapes the renderer plan before slice 2 burns days.

## Constraints
Write-new-rig only: no IDB, no bulk PE sweeps, no `src/` edits (BrightYak owns it), no `git clean`.

## Salvage log (SageBear + predecessor leftovers, treated as signal, NOT cleaned)
| artifact | mtime | verdict |
|---|---|---|
| `/tmp/t056-xorg.conf` | 15:25 | KNOWN-GOOD, with Monitor HorizSync/VertRefresh + Modelines — this is the vrefresh-trick config. My rig copies it verbatim. |
| `/tmp/xorg-dummy.conf` | 21:29 | INCOMPLETE config written by a predecessor: **no Monitor/Modeline section** → publishes vrefresh=0 modes → FindMode wall re-armed. NOT used. (SageBear's likely first blocker.) |
| `/tmp/prefix-health.log` | 21:17 | `nodrv_CreateWindow ... no driver could be loaded` — a wine GUI launch died for want of any X display. Consistent with :96 never coming back up after the xorg2 server expired. |
| `/tmp/t056-xorg2.log` | 23:05 | Xorg :96 second-generation server, "terminated successfully" at 23:31-ish — that is SageBear's death edge. Socket gone (`/tmp/.X11-unix` = X0 only). |
| `/tmp/t056-wineprefix` | 21:24 | SALVAGED IN FULL: 3 DXVK dlls in system32, md5 MATCHES the verified `nix` store set (d3d8 `76c44ce4…`, d3d9 `cf8a8df6…`, dxgi `a107ae82…` — T-067 §"recipe" dlls) + BOTH `DllOverrides` lines (d3d8=n, d3d9=n, written 21:29). Copied to my private `/tmp/r-rig-redquartz/prefix`. |
| `/tmp/dxvk-logs` | 18:03 | empty dir (HappyEagle era); mine is `/tmp/r-rig-redquartz/dxvk-logs`. |
| winedbg :31338/:31437 + their wineservers | running | MANAGED PAIRS — untouched per hygiene contract. |

## The rebuilt rig (exact paths)
- Rig dir: `/tmp/r-rig-redquartz/` — `xorg.conf` (verbatim t056 copy), `prefix/` (cp -a of t056 prefix),
  `data/` (demo-root symlink stage, same shape as `f3-reimpl-run-data`), `dxvk-logs/`, `g1-trace.log`.
- Xorg: `/nix/store/qwbmq81rc42zf9qgfim64jf4b4fnqwgz-xorg-server-21.1.24/bin/Xorg :96 -config … -noreset +extension GLX +extension RANDR` (log `/tmp/r-rig-redquartz/xorg.log`).
- Wine: `/nix/store/hf94anvjgxy56ddx1arxrswpv2zwbzvi-wine-11.0/bin/wine` (the running store, wine-11.0).
- DXVK x32 source (hash-verified): `/nix/store/ap3294dglm9jmj3vly0mnsmlgdnw1yjf-dxvk-i686-w64-mingw32-2.7.1/bin/{d3d8,d3d9,dxgi}.dll`.
- ICD: `VK_DRIVER_FILES=/run/opengl-driver-32/share/vulkan/icd.d/nvidia_icd.json` (library_path = nvidia-x11 595.71.05 **lib32** — hardware, NOT lvp).
- NO `LIBGL_ALWAYS_SOFTWARE`/`GALLIUM_DRIVER=softpipe` (caused the 32-bit LLVM OOM once DXVK owns GL).
- `WAYLAND_DISPLAY=wayland-1` stays INHERITED (t056 hybrid lesson: pure-X11 kills Direct3DCreate8 outright on this host).

## G1 repro line (the one true invocation)
```sh
cd /tmp/r-rig-redquartz/data && \
WINEPREFIX=/tmp/r-rig-redquartz/prefix WINEARCH=win32 \
WINEDLLOVERRIDES='mscoree,mshtml=' WINEDEBUG='-all,+err' \
DISPLAY=:96 VK_DRIVER_FILES=/run/opengl-driver-32/share/vulkan/icd.d/nvidia_icd.json \
DXVK_LOG_LEVEL=info DXVK_LOG_PATH='Z:\tmp\r-rig-redquartz\dxvk-logs' \
OPENVB_RENDER=1 OPENVB_RENDER_TRACE='Z:\tmp\r-rig-redquartz\g1-trace.log' \
timeout 120 /nix/store/hf94anvjgxy56ddx1arxrswpv2zwbzvi-wine-11.0/bin/wine \
  /mnt/shanes-ssd-1tb/code/shane/openvb/build/F3.exe
```
(Trace path is a WINDOWS path — the exe's fopen sees wine's namespace; `Z:\tmp` = host `/tmp`.)

## Verdict slots (fill at first trace)
- Direct3DCreate8 at PHASE1 (WinMain entry, earliest port-controlled point — `src/main.cpp:40`): **NON-NULL — "Direct3DCreate8 SUCCEEDED pre-Startup (VA/ORDER was the wall)"**
- GetAdapterDisplayMode hr/format: **hr=0x00000000, format=0x00000016 (D3DFMT_X8R8G8B8)**
- CreateDevice hr / device alive: **OK** (`CreateDevice OK; magenta clear+present armed per frame`), DXVK swapchain 800x580, VK_PRESENT_MODE_IMMEDIATE
- presented frames: **run1 5,400+ @ T+72s; run2 13,800 @ T+120s; rc=124 (alive at reap), ZERO fatal/err lines** — verdict replicated across two independent boots

## CAPTURE PROOF (the first port-rendered frame, in bytes)
`DISPLAY=:96 import -window root /tmp/g1-frame.png` mid-run → 800x600 PNG, histogram:
**464,000 px (255,0,255) magenta** (= 800x580, exactly the DXVK-reported swapchain client area) + 16,000 px black root margin.

## G1 VERDICT (2026-10-22, RedQuartz)
**POSITIVE — Direct3DCreate8 CAN and DOES return non-NULL on this host**, given the full rig stack:
Xorg :96 dummy-with-Modelines (vrefresh 67.20 live-verified via xrandr) + DXVK x32 three-dll set
(md5-matched: d3d8 `76c44ce4…`, d3d9 `cf8a8df6…`, dxgi `a107ae82…`) + BOTH registry DllOverrides
lines + nvidia **lib32** ICD via VK_DRIVER_FILES + softpipe env ABSENT + WAYLAND_DISPLAY inherited.
The historic pre-init NULLs were rig-state artifacts, not host incapacity. Slice-2 / R-FINISH device
gates / the sitting: UNBLOCKED on the positive branch. The repro line above is the board's rig; runner
script saved at `/tmp/r-rig-redquartz/g1-run.sh` (writes `g1-rc.txt` + `g1-trace2.log`).

## Status of rig at handoff
Xorg :96 LEFT RUNNING (pid in `/tmp/r-rig-redquartz/xorg.pid`, log `xorg.log`) as the warm-rig asset the
board asked for; managed winedbg pairs 31338/31437 and their wineservers untouched (environ-gated check:
their WINEPREFIXes are the .devenv/state ones, mine is /tmp/r-rig-redquartz/prefix).

## Relay contract
Report to QuickNova at FIRST trace output with the exact repro line + which of the two gold answers it is.
**FULFILLED**: first-trace relay sent (POSITIVE); exit-code + capture-proof relay sent after run2 reap (rc=124).

## KEEPER ADDENDUM (post-cfeb8df, commits c815809 + dcbb28e)
- **Cold bootstrap PROVEN** (5th replication): bare wineboot + fresh Xorg :97 built entirely by
  `tools/rig-g1-run.sh` -> full positive verdict chain, rc=124, magenta capture. Two self-corrections
  banked with it: wine-11 ships msvcr70/msvcp70 natively (the "bare prefix lacks them" warning was
  FALSE; the .devenv seed stays as harmless insurance), and **keeper law = harness item 59**: a black
  capture can coexist with a healthy device (intro-movie occlusion) — TRACE = device truth, CAPTURE =
  wall truth, trust in that order.
- **Runner hardened** (dcbb28e): item-59 CAPTURE GATE — SCREENSHOT mode polls the root histogram until
  magenta-dominant (CAPTURE_TRIES/CAPTURE_GAP), and when it can't reach magenta it WARNs instead of
  manufacturing a pass (negative path live-fired against pre-slice-1 F3-trace.exe). Plus the
  **RUN-IDENTITY LEDGER** (`$RIG_DIR/g1-runs.log`: UTC + git HEAD + exe sha256 per window) — earned the
  same night, when build/F3.exe VANISHED mid-window (a BrightYak cmake.toml reconfigure) and returned
  168s later under a NEW sha. Verdicts must name the bytes they ran.
- Windows served from this seat so far: G1 x2 (warm) + keeper verify + cold x2 + gate-negative x2 +
  fresh-exe re-check — all ledgered; current rig exe `894e53a5…` at HEAD e2c1bac-era.
- Paper note (relayed, not chased): QuickNova's 23:47 feed announced an "R-KEEPER standing row" commit;
  no such row exists in PARK/tasks/README on this branch. The contract's durable homes are the PARK
  R-FINISH row ("warm rig available for item 5's sink-proof"), harness-items item 59, this file, and
  the runner header. If a standing row is wanted, keeper's suggested text: "R-KEEPER — the rig is a
  board service; entry point tools/rig-g1-run.sh; check $RIG_DIR/g1-runs.log for window identity;
  item 59 governs captures."
- Queue stance: item-5 sink-proof window first, slice-2 acceptance proper second. Keeper standing by.

## SLICE-2 ACCEPTANCE WINDOW (pre-armed per item 61: unique named TRACE2)
Scope note from PARK:54 (M-001-S2 row): slice 2's stated acceptance = "magenta frame on that config"
— the config being exactly what tools/rig-g1-run.sh bootstraps — plus the row's "needs the user"
clause, i.e., a human eye on the artifact, and the row itself is UNCLAIMED product work (not the
keeper's to claim; src/ stays lane territory).
Window protocol when the slice-2 build lands (or to formally accept the CURRENT exe if the coordinator
rules the bar already met):
  1. `TIMEOUT_S=120 SCREENSHOT=1 CAPTURE_AT=8 TRACE2=slice2-acceptance.log tools/rig-g1-run.sh`
     (item 61 honored: named lane-window file; no shared filenames).
  2. Acceptance bundle = ledger line (UTC+HEAD+exe sha256) + trace head (PHASE1/PHASE2/CreateDevice)
     + cadence tail + rc=124 + the histogram-gated g1-frame.png COPIED OUT of $RIG_DIR before any
     next window overwrites it (keeper contract), handed to the coordinator for banking beside
     artifacts/frames/ (binary commits stay the coordinator's pen, per the 185e393 precedent).
  3. Human-eyes clause: the banked PNG + the claim-ceiling wording from item 5 go in the acceptance
     relay so the user sees EXACTLY what is proven (device clear on hardware Vulkan via DXVK) and
     what is not (nothing renders beyond slice-1's paint until model/texture work lands).
  4. If slice-2's real product adds geometry/color beyond the clear, the acceptance gains a PIXEL-
     TARGET step: keeper crops the PNG region the lane names and reports the value — wall truth,
     read at a specified place, still gated by item 59 ordering.

## ITEM-5 WINDOW RECIPE (pre-armed; item 5 = JE/ActionAnim.cpp:44-47 stub-call restore, proven on frames)
PROOF OPTIONS (QuickNova's framing, both valid — SwiftOwl picks):
  (a) CADENCE-BASED — no src/ change: sink restore is visible as trace-line ordering against T-085's
      bypass-family shape; the run below is the whole protocol.
      SEMANTICS LAW (SwiftOwl 01a16198, verbatim at their request): a cadence run shows ABSENCE OF
      DISTURBANCE, not PRESENCE OF ARRIVAL. This is why (a)'s ceiling is 'runs clean on frames' and
      why item5b stays parked: spending (b) before the arm is reachable would make a claim citable
      past its evidence.
  (b) ONE CITED PROBE LINE — device lane adds a probe-conditional line at the sink call in the GATED
      build only (un-gated exe untouched, item-57 cited, gates green — BrightYak's ten-minute verb).
      Window shape then: the trace interleaves `[render] probe: <sink marker>` lines with cadence lines;
      the ledger line's sha256 names exactly the exe that carried the marker — marker IN trace = sink
      reached in that binary, which is strictly stronger than ordering inference and survives re-quotability.
  Either way the keeper adds nothing to src/; (b) is the device lane's gift, requested not assumed.

### ITEM-5 WINDOW VERDICT (executed + concurred, SwiftOwl 01a16198)
Two overlapping windows on IDENTICAL bytes — ledger: 2026-10-22T04:12:40Z (SwiftOwl) + T04:13:38Z (keeper),
head 01e88e3, exe sha256 3b8c45a5b1ca1dd1cb87…399dc5. Both: PHASE1 SUCCEEDED -> CreateDevice OK ->
cadence ~11,400 presents (≈38fps) -> rc=124 alive-at-reap -> T+8 capture magenta-dominant first try
(post-occlusion, item 59 honored). **PROVEN (option (a), stated exactly)**: the exe carrying the restored
ActionAnim START (openje 791710b: sink call @0x5a92a1-shape, [ent+0x288] clear @0x5a92a8, deadline math
@0x5a92d7) runs the device for the FULL window with zero disturbance. **NOT CLAIMED**: sink DEVICE-arm
reach — unreachable from any port-reachable state (+0x160 never set; gates 1+2 are the shipped-shaped
terminators). NOBODY may cite these windows as "sink reached"; the honest ceiling is "restore runs clean
on frames" until the anim-model era. Option (b) parked as named successor **item5b** (SwiftOwl's request
to QuickNova: don't spend the probe-line verb before the arm is reachable). Window etiquette learned:
TRACE2 names must be unique per concurrent window — the two writers interleaved into one file (harmless
here, identical cadence families; a different-bytes overlap would NOT be).

When SwiftOwl's ActionAnim restoration lands in openje + parent:
1. `msvc-build` (fresh exe; item-60 fix makes cmake.toml edits actually regenerate — first build where
   that matters for a sink-proof).
2. Run the named entry point with a window-identity trace + gated capture:
   `TIMEOUT_S=300 SCREENSHOT=1 CAPTURE_AT=8 TRACE2=item5-sinkproof.log tools/rig-g1-run.sh`
   — the histogram-poll guarantees any kept g1-frame.png is POST-occlusion (item 59); rc=124 + trace
   cadence ("presented frame N") is the device truth; the ledger line names the exact exe sha256 that
   proved the completion on frames.
3. Timing rationale for the 300 s budget: shipped-side ActionAnim inserts land mid-first-scripted-round
   (T-056 funnel hits well before 120 s under clean stacks, T-067 12-min proof); DXVK removes the bink
   wall for both exes, so the window is about SCRIPT time, not survival. Adjust per SwiftOwl's observed
   instant; keeper suggestion only.
4. If the sink-proof wants targeted PIXEL reads (not root capture): the magenta clear is the ONLY thing
   slice-1 paints — anim effects render through the same device but the port has no sprite pipeline yet;
   the frame-proof for item 5 is therefore trace-cadence + exe-identity + (if a debug verb exists in the
   restored path) a Probe() line. Keeper will NOT invent new src/ hooks — that's the device lane's call.
