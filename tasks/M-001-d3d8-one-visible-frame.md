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


### Hardware-ICD round (same evening, after the coordinator's item-53 correction)

VK_DRIVER_FILES/VK_ICD_FILENAMES = the banked 32-bit nvidia ICD (graphics-drivers-32bit/
nvidia_icd.json, library_path libGLX_nvidia.so.0 lib32, api 1.4.329; /dev/nvidia0/-ctl/-uvm
all PRESENT, kernel 595.71.05 open module): DXVK armed (d3d8+dxgi native, HKCU overrides) ->
run on private :98 -> **clean `Direct3DCreate8 returned NULL` at 124-alive, no hang, no crash,
no DXVK stderr at all** (the earlier hang was a wineserver-state artifact; recovery recipe below).
So on THIS shell, DXVK's d3d8 proxy fails at/ before vkCreateInstance against BOTH backends
(lvp32: LLVM OOM; nv32: silent NULL) while HappyEagle's demo boots crash-free 12 minutes
hardware-backed in ITS prefix. The delta is in his prefix/env, not our code or the ICD family -
relay question sharpened with the exact two failure signatures.

SHARED-STATE HONOR NOTE: the experiments transiently broke f3-reimpl-wineprefix boot (rc 53,
then a cold-start race) - recovered with the canonical `wineserver -k; wineboot -u`, re-verified
124 + full battery re-run after clean revert (dlls removed, DllOverrides deleted). When a row
touches a shared CI prefix, the exit gate is proving the UN-GATED boot again, not assuming it.


### Three-dll round (the coordinator's fix applied; HappyEagle's d3d9=native diagnosis confirmed - and the wall MOVED)

Armed state that ran: dxvk 2.7.1 x32 **three dlls** (naqr5g64...-dxvk-2.7.1-bin/x32 - sha1-IDENTICAL
to the store path I'd used: b0e02516.../06b0d2df.../761bcefb...) + HKCU DllOverrides {d3d8=n, d3d9=n}
(dxgi deliberately WITHOUT a reg line, matching the working prefix exactly) + nv32 ICD + Xvfb :98.
Signature TRANSITIONED, decisively:

| state | Direct3DCreate8 outcome | evidence |
|---|---|---|
| d3d9 absent (builtin wins bridge QI) | silent clean NULL, zero stderr | HappyEagle's diagnosis, reproduced exactly |
| d3d9=native present (his fix) | **`std::bad_alloc` + wine assertion at fixed site (+0x1579 across runs: F4F1E579/F504D579/ED3FB579)** - DXVK PROCEEDS into real init, ALLOCATES, dies in the 32-bit address space | /tmp/render-A.log |

So the coordinator's two-command ask is answered by his own grep AND mine: the third dll was in and
it moved the failure from "never loaded" to "loaded and OOMed" - the SAME allocation wall that killed
lvp32's LLVM, now on the hardware path. Remaining variables that could still differ from the 12-min
crash-free demo boot, cheapest first: (1) **init ORDER** - our gated InitOnce runs AFTER the engine's
resource/subsystem init (window then render, per the slot placement the row dictated); the shipped
boot creates its device in the Gfx slot much earlier in a much emptier VA - if DXVK's slab reservations
need the address space our 600MB-asset-loaded 32-bit process no longer has, bad_alloc is exactly this.
Testable WITHOUT the prefix: move the gated Init earlier in our startup, or shrink pre-init allocations.
(2) his wine build vs hf94-wine-11 (unverifiable from my shell). Display class (Xvfb vs Xorg-dummy) is
RULED OUT as the wall cause (bad_alloc precedes any surface work; second Xorg couldn't start rootless -
noted, not needed).

LAW-55 executed on exit: all three dlls removed, both reg lines deleted, wineserver reset + wineboot -u,
**un-gated boot RE-PROVED 124** after revert.


### Phase-split round (876814f) - three theories died by bytes

1. **PHASE1-at-Main had never run**: the window is created in **WinMain (main.cpp:38), BEFORE
   F3::Main()** - the row's 'single frame driver' phrase means TICKS. PreInit moved to the true
   earliest port-controlled point. Evidence chain: Probe() line ORDERING (probe logged AFTER
   PHASE2 lines) proved non-execution; after the move, "PHASE1 enter, gate OK" lands first.
2. **VA/init-order hypothesis KILLED**: Direct3DCreate8 returns NULL at WinMain entry on a
   nearly-empty 32-bit process ("NULL at PRE-init too"). The allocation-timing story died with
   its own instrument. The earlier bad_alloc was OLD-PREFIX state (fresh prefix behaves
   differently; reproduce on clean state before naming theories - law).
3. **Display-class hypothesis TESTED-and-eliminated**: the host runs a LIVE sway compositor
   (/run/user/1000/wayland-1); our own Xwayland :99 gave real RandR monitor geometry
   (1258x698) - still silent NULL. No root Xorg needed, and no flip.
4. **Wine-build space exhausted**: only wine-11.0 exists on this host (both store paths report
   it); "w12" in HE's note resolves to nothing installable here.

FULL replicated-difference list, all clean: 3 dlls sha-identical to the working prefix,
{d3d8=n, d3d9=n} HKCU (dxgi deliberately unreg'd), nv32 ICD verbatim, loader LD path, display
class, init order, wine build. REMAINING CANDIDATE: HE's prefix CONTENTS (non-DllOverrides
state), or the humbler reading - **nobody on this host may ever have had d3d8
Direct3DCreate8 succeed**: the demo's 12 crash-free minutes live in the bink/GDI intro era,
which never asks for a D3D8 device (his own boot logs put the gameplay request behind input
injection). Slice 1's guard would then be the FIRST code on this host to ask the question
honestly, and its NULL is the first real answer. One-command test on his side: run our gated
build/F3.exe under the known-good prefix (gated path touches no demo data).

Citation sweep: RenderD3D8 8/8, main.cpp guard, T-026 digest helpers - my files 0 uncited;
checker truth: CITE regex requires EXACTLY SIX hex digits (0x1305a was invisible; 0x01305A
counts). Gates re-greened on the final exe: 10/10 incl un-gated app-boot 124, cppcheck 0,
integrity rc0; prefix reverted law-55-clean before commit.


## VISIBLE FRAME ACHIEVED (2026-10-22, harness lane — not the assignee; status line left for the row owner)

The row's remaining premise — "nobody on this host may ever have had d3d8 Direct3DCreate8 succeed" —
is FALSE, and the reason it looked true is a prefix-state lie, not a host limit. Full writeup in
docs/notes/harness-items.md item 68; the operative facts:

1. The prefix's system32 d3d8/d3d9/dxgi dlls were **Wine's own builtins** (md5 == `${pkgs.wine}/lib/wine/
   i386-windows/*.dll`), not DXVK, and DllOverrides was empty. So every "DXVK-armed" measurement since the
   2026-10-21 prefix recreation actually ran wined3d — the NULL and the bad_alloc signatures were the
   builtin's, not DXVK's.
2. Arming is now declared + hash-verified: `env.OPENVB_DXVK_X32 = ${pkgs.dxvk.bin}/x32` and
   `scripts.f3-wine-dxvk {status|arm|disarm}` (ARMED/INERT/IMPOSTOR/CLEAN/PARTIAL). `f3-reimpl-run-data`
   and `-trace` print the verdict on every boot. Caveat for the row: the pinned flake's dxvk 2.7.1 is NOT
   byte-identical to the set T-067 recorded (d3d8 7f991638… vs 76c44ce4…) — same version, newer revision.
3. Probe (i686-mingw, armed prefix, Xvfb): Create8 non-NULL on the REAL device (`nvd3dum.dll /
   NVIDIA RTX A3000 Laptop GPU`, DXVK banner 2.7.1); GetAdapterDisplayMode hr=0; **CreateDevice hr=0,
   Clear hr=0, Present hr=0** in our port's exact param shape (swvp + DISCARD/COPY, no auto-depth-stencil).
   One combination DID fail and is worth keeping: `flip + EnableAutoDepthStencil + INTERVAL_IMMEDIATE`
   -> 0x8876086c (D3DERR_INVALIDCALL), so the port's INTERVAL_DEFAULT choice is load-bearing, not cosmetic.
4. Engine itself, gated (`OPENVB_RENDER=1`, F3-trace.exe, staged data, :97): render-trace
   `CreateDevice OK; magenta clear+present armed per frame` … `presented frame 3300 (magenta)`, and the
   WALL agrees: root capture histogram = 233758 px #FF00FF magenta over 552674 px black. Slice-1's
   acceptance ("a magenta window appears under f3-reimpl-run-data") is MET on this host.
5. Prefix left ARMED (that is the state that works); gate re-run in that state: run-all-tests 12 pass /
   0 fail, app-boot 124, rc=0 — the un-gated path is unaffected. `f3-wine-dxvk disarm` returns the
   law-55 clean state in one command if a lane wants it.

NEXT for the row (not done here, it is the assignee's call): a committed capture artifact + the slice-2
frame-validation windows now become reachable, and the 32-bit-address-space/bad_alloc line can be
re-tested against REAL DXVK instead of the builtin that was standing in for it.
