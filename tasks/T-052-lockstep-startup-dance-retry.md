---
status: done (branch b - second blocker NAMED with static+live samples; six rows stay NOT READ behind the named wall)
assignee: HappyEagle `01a15f82`-assigned (T-052 retry; coordinator LoudDragon) - closed this sitting
depends: T-034 (the miss verdict + harness v3 at /tmp/t034-run.py), T-031 (values pairing waits on this)
---

# T-052 — Fix the instrumented-boot startup dance, then retry the six reads

## Established (T-034 live verdict, `5717bd9`) - do not re-derive
- Attach WORKS (5/5 cycles, MZ@0x400000). Port collision irrelevant (31437 workaround clean). xdotool installed but moot with no mapped window.
- The blocker is the winedbg `--no-start` + gdb 17.2 STARTUP-STOP DANCE: instrumented boot never consumes wine's `DbgBreakPoint`; main thread parks at `ntdll+0xe229` (4 identical samples), TICK probe `0x56af50` NEVER fires => engine never completes a frame. Plain `continue` on this pair blocks forever; async path dead on the stub.
- The prior "alive at menu" observation was made WITHOUT a debugger attached. Uninstrumented F3.exe boots fine; instrumented does not. That delta is the bug to fix.

## Task
1. Retry the harness with the DOUBLE-RESUME shape: `winedbg --gdb --port N F3.exe` WITHOUT `--no-start`; after attach: continue -> expect ntdll startup stop -> continue again -> THEN arm breakpoints and judge "never fired".
2. If R1 (`0x48ccf6`) fires at menu after (1): drive movie->menu->New Game (xdotool, windowactivate --sync on the real window id) and collect the six rows per T-034's original table spec (R1/R0/R2/R3/R4/R4b; cold reads forbidden).
3. If window STILL never maps after a clean double-resume: stop debugger-side effort and escalate as a wined3d/d3d8 ENVIRONMENT row instead (candidates: `LIBGL_ALWAYS_SOFTWARE=1` for the wineserver env, or Xvfb per render-device-probe case 3) - that is a devenv/user-harness decision, route to the coordinator, do not edit devenv/** from this row.

## Acceptance
- Either the six-row table WITH values (address/instant/breakpoint/value per T-034), or a named mechanism for the second blocker (startup-dance-fixed-but-wined3d-stuck, with samples), so the retry terminates the row in one sitting either way.
- R0 loud-flag: if the live `[vft+0x18]` disagrees with T-045's static `0x544a90`, say so loudly.
- Harness v3 -> re-land under `tools/` (the /tmp files do not survive a reboot - banked THIS note as a loss risk); keep RedXenon's uncommitted `tools/t034-capture.py` corrections in the same landing pass or supersede them explicitly.

## Boundary
One gdb per stub; ImHex 31337 never evicted; kill loops must gate on /proc/<pid>/environ WINEPREFIX (comm-grep is never an owner test - T-034 incident 320383). The row must not conclude "shipped never pumps" from "our harness can't run it" - R1's "menus roll dice" non-reproduction is already flagged as instrumented-boot-specific, not shipped behavior.

## VERDICT 2025-10-21 (HappyEagle, this sitting) - full report: `discoveries/2025-10-21-t052-startup-dance-retry.md`
1. **DANCE FIXED (shape that produced the result: manual `winedbg --gdb --port 31440 F3.exe` WITHOUT `--no-start`, stdin = held-writer fifo, private cloned prefix).** wine-11 auto-spawns a frontend gdb already attached at the ntdll startup stop; the dance is **N resumes (one per thread-start `DbgUiRemoteBreakin` SIGTRAP), not 2** - that is what made T-034's `continue` "block forever". Driver re-landed: `tools/t052-drive.py`; `tools/t034-capture.py` rewritten (v4) documenting the shape; RedXenon's +0x314/+0x310 corrections kept.
2. **Progress proof:** six bps armed on a live instrumented session; **TICK probe 0x56af50 HIT** (engine pumps frames), RNG state seeded (`[0x707C60]=0x6AD906E5`), **real 800x600 `F3` window mapped** (Xvfb+softpipe; also on :0 instrumented). "Shipped never pumps" is FALSIFIED as a general claim; `0x552bb5`/`0x48ccf6` still never execute because the boot dies one step earlier.
3. **TERMINAL MECHANISM (named, branch b):** `JE::Gfx_D3D::meth_0x502870` (FindMode behind `Graphics::Initialize` `0x5818c4`) rejects any adapter mode with **`RefreshRate < 60`** (gate `0x50292b`); wined3d-11's X11 backend publishes `RefreshRate=0` in EVERY mode on every reachable display (live read at the pre-fatal stop: `{1024x768, refresh=0, fmt=23}` x2) - Xvfb keeps RandR `vrefresh=0` server-side (`xrandr --newmode` 67.2 Hz not picked up; `ForceVideoModeList` ignored by the GL backend), so `JE::FatalError("Cannot find requested mode.")` at `0x5818d0` is unavoidable. `:0` (Xwayland+glvnd "driver (null)") fails EARLIER (`get_pixel_formats` 100% spin; reproduced on a PRIVATE prefix -> zombie-stub cross-talk exonerated for the spin). `LIBGL_ALWAYS_SOFTWARE=1` fixes nothing on `:0` (converts spin into `glGetTexImage` sleep) but is REQUIRED on Xvfb (with `GALLIUM_DRIVER=softpipe`) to get the adapter up at all. `renderer=vulkan` is worse for d3d8: the shipped enumeration itself fails the legacy caps (`[Enum] Device doesn't support minimum of 2 simultaneous textures!` -> `No Compatible D3D Adapters found!`).
4. **Six rows: R0-R4b = NOT READ** (behind the named wall; no cold padding; R0 loud-flag: no live contradiction of T-045 - pump never reached; `WorldTickPump` assumption still unfalsified-not-confirmed).
5. **Env-decision routed to LoudDragon (not executed here, no devenv edits):** (1) DXVK native `d3d8=n` (host Vulkan works; DXVK = real refresh + legacy caps) - best odds; (2) Xorg `-dummy` (server-computed vrefresh RandR modes); (3) real GPU X session. PLUS independent repair: managed `ida-orig-gdb` stub (46779/46781) is CLOSE-WAIT-wedged at ~35-40% CPU while `devenv processes list` says ready - restart is controller-lane.
6. **Downstream:** T-031 pairing stays blocked on the env decision (values NOT killable-by-effort); T-050 stays GATED (this retry says nothing about re-entrancy either way - engine never reached gameplay orders); T-051 gate resolves the same way. Distinction per row: **"harness can't run past mode validation"**, not "shipped never pumps".
