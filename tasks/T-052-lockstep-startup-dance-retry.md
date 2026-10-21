---
status: in-progress
assignee: HappyEagle `01a15f82`-assigned (T-052 retry; coordinator LoudDragon)
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
