---
status: in-progress
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
- Direct3DCreate8 at PHASE1 (WinMain entry, earliest port-controlled point — `src/main.cpp:40`): TBD
- GetAdapterDisplayMode hr/format: TBD
- CreateDevice hr / device alive: TBD
- presented frames: TBD

## Relay contract
Report to QuickNova at FIRST trace output with the exact repro line + which of the two gold answers it is.
