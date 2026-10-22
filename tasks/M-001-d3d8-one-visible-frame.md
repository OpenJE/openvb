---
status: slice-1 LANDED except the visible frame (device slot + guards + msvc link green 2026-10-21; visible-frame attempt measured on DXVK: Direct3DCreate8 NULL + 32-bit LLVM OOM - boot-lane question outstanding, see observations)
priority: P2 slice / P4 whole
assignee: KeenStorm (slice 1; board mirror via relay per coordinator-only law)
type: milestone
---

> CLAIM NOTE (KeenStorm, 2026-10-21): pulling slice 1 with the NEW input nobody had when the
> harness-limit verdict was written: T-067 proved tonight that this host runs d3d8 via **DXVK +
> 32-bit Vulkan ICD** for the shipped demo (wined3d/GL was never the only backend - the earlier
> "GL context impossible" probe result is wined3d-specific and stands ONLY for wined3d). Slice-1
> step 1 smoke first (real msvc leg, prerelease paths per bbf7346), then the renderer slot with
> the row's probe posture (opt-in, SEH-guard, file trace). Acceptance attempted against
> DXVK+lavapipe; if CreateDevice still faults, the row's harness-limit sentence restands,
> re-measured not inherited.

> CORRECTION TO MY OWN CLAIM NOTE (same day, gen-7 applied to me within the hour): the note's
> premise that the installed msvc-build payload "predates the bbf7346 wiring" is WRONG. I grepped
> `bin/msvc-build` - which is a two-line LAUNCHER delegating to a separate `-msvc-build-script`
> store path that DOES call `msvc-sdk-prerelease.bat` in both cmake passes. "0 matches in the
> launcher" is not "the wiring is stale"; the payload my session runs is current. The check I
> should have done (and the wrapper's own text proves) is grep the FILE THAT EXECUTES. This is
> the coordinator's b2a0633 trap replayed by me, minutes later, in the very row about it -
> which is how the register earns the name standing order.

# M-001 — D3D8: get one visible frame (slice 1 of 4). NOT "port the renderer".

## Why this is on the board now, and why it is sliced

Today nothing we build can be **seen**. `Display.cpp` is `RegisterClassA` / `CreateWindowExA` /
`ShowWindow` and nothing else; `cmake.toml` links `user32 gdi32 winmm shell32` — **no d3d8**. So the
whole team verifies by log lines and lockstep traces while five open questions are genuinely
eye-verifiable:

| open item | how a visible frame changes it |
|---|---|
| hit-reaction anim ids (33/34 vs 37/38/39/40) | watch a victim get punched; the anim *is* the observable |
| `SelectHitVeg` `.veg` table + `+0xAD`/`+0xAC` flags | blood vs ricochet vs electric is visible per damage type |
| skins / fonts / `Window+0x90` (T-007) | B cannot bind a texture without a device; their lane is stalled on this |
| model bbox multi-cell footprints | needs `Gfx_ModelInstance` at all |
| the auto-move / door dance | a moving sprite is the acceptance test |

**But the full renderer is not a task, it is a program.** `Gfx_Engine` is a 2212-byte object with
**140 methods** and 81 members; there are eight `Gfx_RS_*` render-style variants (fog / height-fog /
skin combinations), `Gfx_ParticleSystem` alone is 1828 bytes / 146 members, plus `Gfx_ModelInstance`,
`Gfx_SkinMesh`, cameras, light, streaming buffers, and a `granny2.dll` model format on top. Anyone
who opens this as "implement DirectX 8" will burn a week and land nothing viewable.

So: **slice 1 now, slices 2–4 later, and only when a lane is actually gated on them.**

## Slice 1 — device + clear + present (the only part that is cheap)

1. **Probe first, 10 minutes, in the build image:** does the MSVC 2002 image have `d3d8.h` and
   `d3d8.lib`? (VS.NET 2002 shipped DX8 headers in some installs; the coordinator's docker probe
   from the host could not exec bash in that image, so this answer must come from inside it.)
   - Present → add `d3d8` to `windows.private-link-libraries` and go on.
   - Absent → **stop and report**; options are vendoring the SDK headers or a `LoadLibrary`+vtable
     shim, and that is a decision, not a detour.
2. `Direct3DCreate8(D3D_SDK_VERSION)` → `CreateDevice` on the existing `HWND`, windowed,
   `D3DPRESENT_PARAMETERS` with the demo's adapter format where knowable and a sane fallback where
   not. `Reset`/lost-device path is **out of scope** for slice 1 — one create, one clear, one present.
3. Clear to a distinctive non-black colour (magenta is the honest choice: it cannot be confused with
   an uninitialised black buffer) and `Present()` once per frame from the existing
   `GameStateLoop`/subsystem tick, **after** the world pump, so frame ordering matches the original's
   message-loop-then-tick shape rather than being invented.
4. Wire it as a **subsystem slot**, not as free-floating calls in `F3.cpp`, so `GameStateLoop` stays
   the single frame driver.

Runtime feasibility is already established, not assumed: wine 11 in `.devenv/state/f3-reimpl-wineprefix`
ships `d3d8.dll` (+ `d3d8thk.dll`), and the demo's `granny2.dll` / `binkw32.dll` are already staged
next to our exe in the data dir.

### Premise correction (implementer-B) — my escalated "vendoring decision" was wrong

I told the human `d3d8.lib` is **absent** from the MSVC 2002 image and asked them to choose between
vendoring and a `LoadLibrary` shim. It is **not absent**: it lives at
`C:\Program Files\Microsoft Visual Studio .NET 2002\Vc7\PlatformSDK\lib\prerelease\d3d8.lib`
(with `d3dx8.lib`; headers in `...\Include\prerelease\d3d8.h`). What is actually true is that
`vcvars32.bat` does not put either `prerelease` directory on INCLUDE or LIB. So:

1. The `LNK1181` I blamed on a missing library came from a **stale installed `msvc-build` payload** —
   three of five `*-msvc-build` store dirs contain zero occurrences of `prerelease`. That is the same
   staleness class that handed me a bogus `app-boot` pass this morning; twice in one day I reasoned
   about a toolchain instead of checking which payload actually ran.
2. The remaining question is **"append two paths that already exist in the image"** — a changefile edit
   in the coordinator's lane, not a licensing or vendoring decision. The escalation overstated it and
   the correction stands in its place.

### CLOSED by measurement: `tools/render-device-probe.sh` — a visible frame is not obtainable on this host

ZenRaven automated the three cases (commit `16a3c8a`), verdict on this machine:

| case | result |
|---|---|
| no `DISPLAY` | **exit 1** — `nodrv_CreateWindow` |
| `Xvfb` + render device OFF | **124 alive**, trace written |
| `Xvfb` + render device ON | **exit 1** — `Failed to find a suitable pixel format` / `Failed to get a GL context` |

So the `LIBGL_ALWAYS_SOFTWARE=1` hope is dead *and* the virtual-display route is dead: **even on a virtual X
server with software GL explicitly requested, wined3d cannot obtain a context here.** Slice 1's *visual*
acceptance is therefore a **harness limit, not our code** — which is the sentence I asked for and now have,
mechanically re-checkable on any host by running the script. Design note worth copying: the probe prints
**`UNKNOWN` rather than `FAIL`** when the exe lacks the `OPENVB_RENDER` opt-in, because "not built here" and
"broken" are different answers.

Remaining honest scoping for slice 1: acceptance needs a host where wined3d gets a context (real driver, or a
GLES/other wine build). On *this* host the verifiable part of slice 1 is "device-creation path runs and its
failure is a logged no-device state, not a crash" — which the probe already demonstrates.

### The finding that really gates slice 1: no GPU driver in this harness

Measured and bisected in a clean worktree with B's file as the only variable: `Direct3DCreate8`
succeeds, `GetAdapterDisplayMode` succeeds, and **`CreateDevice` faults** (app exit 40, not 124),
consistent with this host's `libEGL warning: pci id for fd 41: 10de:2204, driver (null)` lines — wine's
d3d8 backend wants a GL context and the sandbox has no driver. **A visible magenta frame is not
verifiable here as configured**, even after the two-path fix.

One cheap experiment precedes any conclusion: mesa's software rasteriser is in the store
(`/nix/store/ag2gbvb2x4v0907rxp4lcwcf426g54f3-mesa-26.1.8/lib/dri/swrast_dri.so`), so
`LIBGL_ALWAYS_SOFTWARE=1` may give wined3d a context and let `CreateDevice` succeed. If that fails,
slice 1's acceptance belongs on a machine with a driver — and that is a fact about our harness, not a
property of our code, and must be recorded as such rather than as a failure.

### Probe posture worth keeping
- Rendering is **opt-in** (`OPENVB_RENDER=1`) with an **SEH guard around `CreateDevice`**, so a fault
  becomes a logged no-device state instead of a crash; default-off keeps `app-boot` honest.
- `JE::LogDebugString` -> `OutputDebugString` is **invisible under wine without a debugger**, and
  **`fprintf(stderr, ...)` crashes `F3.exe`**, which links as a GUI-subsystem image where the CRT
  leaves stderr NULL. Hence the file-based trace channel (`OPENVB_RENDER_TRACE=<native path>`). These
  are project-wide diagnostics facts, not render-specific trivia.

### Acceptance (slice 1)
- `msvc-build` exit 0 with `d3d8` linked; a magenta window appears under `f3-reimpl-run-data`.
- A single `Present()` per frame: no flicker, no double-buffer confusion, and the world pump still
  runs 9/9 green with the new slot wired in (movement/attack tests unaffected).
- `app-boot` still returns 124 (alive), and `--no-window`/headless CI does not hang waiting on a
  device that cannot be created — guard the create and log the failure instead.
- **No** model, texture, GUI, or sprite code in the same commit. Slice 1 lands alone or it is not
  slice 1.

## Slices 2–4 (deliberately not scoped here)
2. Textures + the `Gfx_Engine` `+0xDC`/`+0xC4` hooks B needs for T-007 skin binding → *unblocks GUI*,
   still no models.
3. Sprites/2D draw of one `.veg` or one tile → *first real picture*, and the acceptance test for the
   reaction-anim and damage-type work.
4. `granny2` model loading + `Gfx_ModelInstance` + `Gfx_SkinMesh` → *the expensive one*; do not start
   until footprints or camera work are actually a lane goal.

## References
- `src/F3/Display.cpp` (window-only today), `cmake.toml:57` (link list), `src/F3.cpp:517` (`GameStateLoop`)
- `docs/f3/StartupAndSubsystemTable.md` (where a render slot belongs)
- Gfx class census from the object DB: `Gfx_Engine` 2212B/140 methods; `Gfx_ParticleSystem` 1828B;
  eight `Gfx_RS_*` variants; `Gfx_ModelInstance` 202B
- `discoveries/2025-10-20-session16-updater-map-children-modelinstance.md` (model-instance ticks)

## Notes (coordinator)
My recommendation, stated as a position you can overrule: **do slice 1 soon and small; defer the
renderer.** Slice 1 pays for itself because it converts five currently-blind verification problems
into observable ones and it is the only thing standing between B and finishing T-007. Slices 3–4 pay
for themselves only when something is gated on them, and nothing is yet. If slice 1's probe says the
image has no DX8 headers, that is a decision point for the human, not a work-around to invent.


## Slice-1 observations (KeenStorm, 2026-10-21, landed code + measurements)

- **Step 1 CLOSED affirmatively, live:** `[sdk-prerelease] d3d8 SDK on INCLUDE+LIB` prints in the
  real msvc leg; throwaway smoke compiled d3d8.h and linked d3d8.lib (20,992-byte exe). No
  vendoring, no shim. The bbf7346 wiring is CURRENT in my session's executing payload - my claim
  note's "stale payload" premise was my own gen-7 replay (grepped the launcher, not the delegated
  script) and is corrected in place above.
- **Slot landed:** src/F3/RenderD3D8.{hpp,cpp} + F3.cpp three touchpoints (InitOnce(hwnd) after
  ShowWindow - parameter, not the F3.hpp namespace-static g_window [decompiler-shape internal
  linkage; LNK2019 caught the reach-across-TU attempt]; Tick after the 8-slot walk beside the
  digest hook, NOT a 9th table entry; Shutdown before window destroy). cmake.toml + manual
  CMakeLists sync (board precedent 22ad779; regeneration-safe: toml is the truth).
- **2002-era header truths (live-caught compile errors, not folklore):** CreateDevice wants a
  NON-const D3DPRESENT_PARAMETERS* (C2664); d3d8's field is `FullScreen_PresentationInterval`
  (C2039 - the short name is d3d9); SetRenderTarget is two-arg (C2660). Recorded so the next
  lane does not rediscover them.
- **Gates:** msvc rc0 + d3d8 linked; run-all 10/10 (app-boot 124 with the new exe - un-gated path
  is a strict no-op); cppcheck 0; gate-integrity rc0.
- **Visible-frame attempt, measured (revert-clean):** DXVK 2.7.1 i686 d3d8+dxgi into
  f3-reimpl-wineprefix + HKCU DllOverrides + lvp_icd.i686.json (library_arch 32, loader present).
  Our guarded path executed EXACTLY as designed: window created (hwnd 0x0005006C logged),
  **Direct3DCreate8 returned NULL** while stderr showed `LLVM ERROR: out of memory` (the 32-bit
  lavapipe OOM HappyEagle named for softpipe) - NO crash, app kept booting until killed. The
  row's harness-limit sentence therefore AMENDS, not restands: wined3d dead (proven), DXVK
  d3d8 REACHES our guard, the remaining blocker is whatever made the demo's 32-bit DXVK boot
  survive Vulkan on this host - a boot-lane question with a known-good prefix to compare against,
  not a code defect. Prefix fully reverted (dlls + registry), :96 untouched.
