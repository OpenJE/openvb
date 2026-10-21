# 2025-10-21 — T-052 startup-dance retry (HappyEagle): dance FIXED and proven; six rows still NOT READ — the terminal blocker is the shipped FindMode `RefreshRate>=60` gate vs wined3d-11 RandR modes that always carry `RefreshRate=0`

Row: T-052 (retry of T-034's live half). Coordinator: LoudDragon `01a15f82`.
Verdict branch: **the row's alternative acceptance — second blocker NAMED, with static +
live samples.** The startup-dance hypothesis is dead: instrumented boot now runs (real
`F3` window maps, engine hits its own per-frame TICK). What remains is a hard
wined3d-11/X11-GLX environment wall at display-mode validation. The "shipped never
pumps" reading is **falsified**; "harness can't run it (past the mode gate)" is the
accurate statement.

[DISCOVERY-BATCH 1 ready]

## The six-row table (final T-052 status)

| # | read | address / field | breakpoint | instant | VALUE |
|---|---|---|---|---|---|
| R0 | World vftable slot 6 target | `[$ebx+0x18]` at `$ebx` = World vftptr | `0x552bb5` | first world pump | **NOT READ** — engine never reaches world load; `0x552bb5` armed, died at `JE::Graphics::Initialize` mode check first. `[0x70BD98]` read **0x00000000** while parked at the pre-fatal stop (no world exists at that point — consistent, not a contradiction). T-045 static (`0x544a90`) stays **unfalsified, unconfirmed**. No loud-flag either way. |
| R1 | `F3::start_time` LCG | dword `[0x707C60]`, gate byte `[0x707C64]`, draw `$eax` | `0x48ccf6` | first return of `sub_48CCC0` | **NOT READ** (the bp fired zero times before the fatal). Boot-phase ADJACENT datum at the parked pre-fatal stop — **NOT an R1 row value**: `[0x707C60]=0x6AD906E5` (seeded, state moved), `[0x707C64]=0x01`. The seed happens during boot init; the first draw at `0x48ccf6` had not occurred yet. |
| R2 | clock behind `world+0x3C` | `[[0x70BD98]+4+0x3C]` → `[Greg+0x34]` | carriers = R1/R0 | same instants | **NOT READ** — both carrier bps unfired; `[0x70BD98]=0` makes the chain undefined at the phase reached. |
| R3 | queue depth under load | deque `Entity+0x304`: `+0x308` blocks, `+0x30C` cap, `+0x310` **HEAD INDEX**, `+0x314` **LIVE COUNT** (RedXenon 9bbdc9e — kept, not re-derived) | `0x550720` | mid combat round | **NOT READ** — no world, no entities, zero funnel traffic (bp armed, never hit). |
| R4 | owner back-link pre/post | `[action+0x14]` around the funnel store `0x550737` | `0x550720`/`0x550741`/`0x459f5d` | one real enqueue | **NOT READ** — same blocker; pre-call garbage-vs-zero question stays open. |
| R4b | action class vftptr per slot | `[*slot]`, slots `head..head+count` | same instants as R3 | same instant as R4 | **NOT READ** — no live action object ever existed. |

No cold-read padding taken (row forbids). Nothing in this table is an estimate.

## 1. The startup dance — shape that produced the result, and what it actually is

Per the assignment: **manual `winedbg --gdb --port 31440 F3.exe` WITHOUT `--no-start`** in
the f3demo root, against a **private cloned prefix** (`/tmp/t052-wineprefix`, `cp -a` of
`.devenv/state/f3demo-wineprefix`). The managed devenv 31437 stub was NEVER attached:
it is not LISTENING at all (CLOSE-WAIT socket to a dead peer 55156, `winedbg.exe` 46779
spinning ~35-40% CPU with its parked inferior 46781 — `devenv processes list` still calls
it `ready`). Left untouched, coordinator lane — see §5.

Discovered shape (this is why every T-034 cycle hallucinated a dead stub):

1. wine-11 `winedbg --gdb <exe>` (no `--no-start`) spawns the inferior **and its own
   frontend gdb** (`gdb -x <prefix>/.../Temp/gdbXXXX.tmp`, child of `winedbg.exe`), already
   TCP-connected to 127.0.0.1:31440 and parked at ntdll `process_rtl_work_item+102`
   (the wine startup stop). No LISTEN remains after that connect.
2. If that frontend's stdin EOFs (e.g. `nohup` with `/dev/null`), the frontend quits
   `[answered Y; input not from terminal]`, **detaches**, and the inferior runs
   debugger-LESS. Fix: a fifo with a held writer (`sleep max > fifo`) as stdin; drive the
   frontend by writing gdb commands into the fifo, parse its log.
3. The dance is **N resumes, not 2**: every wine-created thread posts a
   `DbgUiRemoteBreakin -> DbgBreakPoint` SIGTRAP that ALL-STOPs the debuggee at the gdb
   prompt (`create thread D @7BF40740` + `Thread N received signal SIGTRAP` in the log).
   Until the loader is done (~10-15 threads at this boot phase), each resume yields one
   more stop. A single/double-continue harness therefore spends the entire deadline
   parked at the prompt while appearing to "hang on continue". This is the T-034
   "plain continue blocks forever" observation, explained and fixed.
4. SIGINT to the frontend gdb reliably stops the running debuggee (contradicting T-034's
   "async is dead" only for the RSP-`interrupt`-packet form — host-signal interrupt of the
   frontend works), and queued `break *<ea>` lines arm at the resulting stop.

Working driver re-landed: **`tools/t052-drive.py`** (consumes thread-start stops, services
all six read bps + two Graphics-probe bps, dumps raw stop chunks + register/memory reads
to the values log). `tools/t034-capture.py` is rewritten as v4 documenting this shape
(both supersede `/tmp/t034-run.py` / `/tmp/t052-drive*.py`).

Evidence the dance is fixed — under the debugger, session J (`DISPLAY=:0`):
```
Breakpoint 6, 0x0056af50 in ?? ()          <- TICK probe (JE_TickMilliseconds) FIRED
0x707c60: 0x6ad906e5   0x707c64: 0x01      <- engine RNG subsystem alive & seeded
```
plus a real 800x600 `"F3"` X window in sessions J/N/O/P (Xvfb sessions). The engine's own
frame loop runs. What kills the boot now is §2.

## 2. The terminal blocker, NAMED with static + dynamic samples

`JE::Graphics::Initialize_581810` builds its mode request (adapter 0, w=**800**, h=**600** —
dword dump at the parked stop: `[gfx+0x3c]=0x320, [gfx+0x40]=0x258`, refresh-requirement
`[gfx+0x44]=0`, bpp-flag `[gfx+0x48]=0`) and asks `Gfx_Engine vft+40` →
`JE::Gfx_D3D::meth_0x502870` (singleton at `0x7082F8`) to find a matching
`EnumAdapterModes` entry. Its filter (decompile, decisive lines):

```c
if (mode->width != param_4 || mode->height != param_5) continue;      // 800x600
fmt = mode->format;              // v15[4] = +0x10
if (bpp_flag == 0) { if (fmt!=23 && fmt!=24 && fmt!=25 && fmt!=26 && fmt!=30) continue; }
else             { if (fmt!=21 && fmt!=22) continue; }
/* gate @0x50292b: */
reject = (req_refresh ? mode->refresh < req_refresh : mode->refresh < 60);
if (reject) continue;             // 0x50292b — REFRESH < 60 Hz => REJECT
```

What wined3d-11 (GL backend, wine-11.0) actually hands it — read **live from the debuggee
at the pre-fatal stop** (both Xvfb 800x600 and 1024x768 screens):

```
mode[0] = { w=1024, h=768, RefreshRate=0x00000000, ..., format=0x17(23=R5G6B5) }
mode[1] = { w=1024, h=768, RefreshRate=0x00000000, ..., format=0x16(22=A1R5G5B5) }
```

**`RefreshRate` is 0 in every mode wined3d's X11 backend exposes on any software/virtual X
server, and the shipped filter requires `>= 60` — no X-side configuration can satisfy it:**

* Xvfb's RandR `ModeInfo->vrefresh` server field is 0; `xrandr --newmode` (verified:
  `f3_800x600_60`, client-computed 67.20 Hz) does not populate it — live mode list re-read
  after the switch still showed `RefreshRate=0`, and still reported the pre-switch
  1024x768 geometry (session N).
* `HKCU\Software\Wine\Direct3D ForceVideoModeList="800x600x32,800x600x16"` — no effect on
  the GL backend's list (session O: identical modes, identical fatal at `0x5818d0`).
* Format membership was never the problem on Xvfb (`23 ∈ {23,24,25,26,30}` passes), and
  resolution matched in session F — the ONLY rejection term that ever fired is `0x50292b`.

Consequence: `JE::FatalError_0x48cc40("Cannot find requested mode.\n")` → the
"Fatal Error / Cannot find requested mode." MessageBox (screenshot-captured), engine
parked in the modal pump. R0-R4b can never be reached on this stack.

## 3. The `:0` and override mechanisms, measured (so nobody re-walks them)

| shape | result | samples |
|---|---|---|
| `DISPLAY=:0` (Xwayland), debugger, shared prefix, no overrides (session B) | main thread **100% spin** in wine `opengl32!get_pixel_formats` <- `wglChoosePixelFormat` <- `wined3d_adapter_gl_init`; 3 identical bt samples ~30-60s apart; glvnd `pci id 10de:2204, driver (null)` + `egl: failed to create dri2 screen` spam | T-034's parked-ntdll + "continue blocks" is THIS spin, plus the all-stop prompt-parking of §1 |
| `DISPLAY=:0`, debugger, **private prefix** (session I) | **same spin reproduced** — the devenv zombie stub is NOT the cause of the spin (zombie is still its own problem, §5) | bt `#0 get_pixel_formats` |
| `:0` + `LIBGL_ALWAYS_SOFTWARE=1` (session C) | adapter init advances past GL selection, then **dead-sleep** (0 CPU) in `wined3d!match_broken_arb_fog -> opengl32!glGetTexImage` — a GLX round-trip Xwayland never answers | 2 samples, same frame, utime frozen |
| `:0` uninstrumented (session H) | **100% CPU, same `get_pixel_formats`-class stall** — T-034's "uninstrumented boots fine" did NOT reproduce on today's host; the uninstrumented boot fails identically (no window, no menu). Environment drift since the older observation, almost certainly the compositor/GLX-vendor situation | utime 5955->8944/30s, fake-window-only |
| Xvfb 800x600x24 + `LIBGL_ALWAYS_SOFTWARE=1` + `+iglx` (sessions F uninstr / J-N instr.) | **wined3d ADAPTER WORKS** (llvmpipe/softpipe: GLSL 4.60, caps read, quirks applied, real `F3` window, engine `[Enum]` capability probe PASSES) → reaches §2 mode wall | mode-list reads §2 |
| Xvfb + `GALLIUM_DRIVER=softpipe` | same as above (llvmpipe's `glGetTexImage` dead-sleep avoided; softpipe completes adapter init) | sessions N-O |
| `HKCU\Software\Wine\Direct3D renderer=vulkan` (session P, Xvfb) | **WORSE for d3d8**: F3's own capability enumeration fails — shipped trace strings: `[Enum] Device doesn't support minimum of 2 simultaneous textures!`, `... TEXGEN!`, `... required TextureOp!`, `No Compatible D3D Adapters found!` (vkwined3d lacks legacy fixed-function caps; never reaches `0x581810`) | log tail `021c:0220: output debug string` |

So the row's candidate list was half right: **Xvfb is a real step forward (it is the only
thing that ever gets the device up), `LIBGL_ALWAYS_SOFTWARE=1` is required WITH Xvfb but
actively harmful on `:0`** (it converts the spinner into a sleeper), and the named
terminal mechanism is the refresh gate, not "window never maps".

## 4. What this means downstream (the sharp distinction)

* **NOT "shipped never pumps."** The shipped engine's frame heartbeat (`0x56af50`) fires
  live under the debugger, its RNG state is seeded, its window maps, its own d3d8
  capability probe and mode validation run and the latter fails on a wined3d artifact
  (`RefreshRate=0`) that no Windows d3d8 runtime ever returned. The shipped path through
  combat/world pumping remains **untested**, not disproven.
* **The six values stay unreachable-by-environment** (not by tooling this time): the
  gate is display-mode metadata, before any world entity exists. R3/R4/R4b mid-combat,
  R0/R2 first-pump, R1 first-draw all sit behind it.
* **T-031**: continue with the synthetic shipped side; value pairing needs the devenv
  decision below. No value can be salvaged by better driving (xdotool is moot — the
  engine never paints a menu, it dies at mode validation; the `Fatal Error` dialog is
  all that is on screen).
* **T-050**: stays GATED. This retry produced neither re-entrancy confirmation nor
  falsification (never reached gameplay orders). Do not read "harness can't run it" as
  evidence about the skip-advance seam.
* **T-051** pairing gate: same routing as T-031 — the capture decision is now a pure
  devenv/environment decision with a named mechanism and a candidate list.

## 5. Devenv decision routed to the coordinator (not executed from this row)

Ranked candidates to get a mode list with `RefreshRate >= 60` + 32-bit formats:

1. **DXVK as native `d3d8`** (`WINEDLLOVERRIDES='d3d8=n'` + DXVK's `d3d8.dll`/`dxgi.dll`
   in the prefix). Host Vulkan works (compositor runs on nvidia vulkan); DXVK implements
   the legacy d3d8 fixed-function caps AND synthesizes 60 Hz mode lists. Needs: dxvk in
   the pinned nixpkgs (couldn't verify offline — the lock rev 404s against direct
   fetchTarball; use the devenv pinned source), and a process/env change in `devenv/**`
   = coordinator lane. **Best odds.**
2. **Xorg `-dummy` driver** instead of Xvfb (dummy/xf86-video-modesetting computes
   vrefresh server-side into RandR ModeInfo — likely to clear the 60Hz gate with plain
   wined3d-GL; needs xorg-server + dummy driver in devenv).
3. A real X session on the GPU (nvidia GLX vendor + KMS modelines) — heaviest, host
   session is currently Wayland/wlroots; also fixes the `:0` glvnd-null problem.
4. **Independent of the above, the managed original stub needs a restart**:
   `ida-orig-gdb` (pid 46779/46781) is CLOSE-WAIT-wedged, burns 35-40% CPU continuously,
   its inferior is parked-sleeping, and `devenv processes list` still reports `ready`
   (the owner-verified probe matches the port-holder, not liveness — the limitation
   recorded in T-034's env status). Repair = `devenv processes restart ida-orig-gdb`
   (controller-owned, T-034 handover wording). Anyone re-using the SHARED
   `f3demo-wineprefix` under a debugger should not trust that stub being "up".
   (Also note my B-vs-I comparison: the zombie did NOT cause the GL spin, but it DOES
   hold a busy shared wineserver 46078.)

## 6. Boundary/incident log for this sitting

* ImHex 31337 never touched. Reimpl stub/inferior 31338/46775/46777 never touched.
  The devenv-owned 31437 pair never killed or restarted — flagged for coordinator (§5.4).
* Every kill in this session was owner-gated: exact pids captured at spawn +
  `/proc/<pid>/environ` `WINEPREFIX=` equality checks; one `pgrep -f` self-match
  (killed my own shell, rc=143) recorded as the lesson — even `-f` patterns self-match,
  prefer recorded pids with environ gates. One cleanup loop initially missed a stale
  inferior (session L) whose port-squat made a fresh stub bind-conflict — reaped with
  the same gate afterwards.
* IDA original MCP: read-only (decompile/xrefs/strings only), zero writes. No
  `devenv/**` edits; no registry/file writes outside `/tmp/t052-*` scratch + reserved
  paths. The private prefix `/tmp/t052-wineprefix` and logs
  (`/tmp/t052-w*.log`, `/tmp/t052-values*.log`, screenshots `/tmp/t052-*.png`) are left
  in place for reproduction until the next reboot.
* Reservations: `tasks/T-052-*`, `tools/t034-capture.py`, this discoveries file.
* Harness re-landed under `tools/` per acceptance (`t034-capture.py` v4 + `t052-drive.py`;
  /tmp v3 explicitly superseded in both headers). RedXenon's
  `+0x314 LIVE-COUNT / +0x310 HEAD` correction is carried in both files.
