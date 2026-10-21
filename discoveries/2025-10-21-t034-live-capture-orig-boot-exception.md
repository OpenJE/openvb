# 2025-10-21 — T-034 live capture (NiceHawk session 01a15cb7): zero of six rows reached — the instrumented original never passes wine startup; main thread parked at `ntdll!_DbgBreakPoint`

Row: T-034 remaining half — R1/R3/R4b live values from the ORIGINAL `F3.exe` (feeds T-031 parity oracle).
Verdict up front: **R0–R4b = NOT READ.** One live datum was captured (a boot-phase program counter), and it is
evidence about WHY, not a substitute for any row. Per the row's discipline: no guessed reads, no cold-entity
padding. Target, ports, recipe per the NiceHawk handover: `winedbg --gdb --port 31437 --no-start F3.exe` in
`.devenv/state/f3demo-wineprefix`, single host gdb 17.2, ImHex left undisturbed on 31437 (never evicted),
reimpl stub on 31338 (not mine). IDA EA == target VA confirmed again (image at `0x400000`, `x/4bx 0x400000`
→ `MZ`, in prior sessions).

## The six-row table

| # | read | address / field | breakpoint | instant requested | VALUE |
|---|---|---|---|---|---|
| R0 | World vftable slot 6 target | `[World vftptr]` at `$ebx`; datum = `[$ebx+0x18]` | `0x552bb5` | first pump of a new game | **NOT READ** — bp never fired; pump never ran (no world exists at the phase the target reached; see §3) |
| R1 | `F3::start_time` LCG state | dword `[0x707C60]` (reseed gate byte `[0x707C64]`; drawn value in `$eax`) | `0x48ccf6` (ret of `sub_48CCC0`) | first return of `sub_48CCC0` | **NOT READ** — bp never fired, zero RNG draws observed in ~13 min of running target, including through the whole blank-screen phase — contradicts the handover's "menus draw, menus roll dice" for THIS boot path |
| R2 | clock behind `world+0x3C` | `[[0x70BD98]+4+0x3C]` → dword `[Gregorian+0x34]` | same instants as R1/R0 | R1 instant + frame 1 | **NOT READ** — both carrier bps unfired. `[0x70BD98]` was never sampled while stopped; reading it blind at the boot stop is NOT what the row asked for and is not reported |
| R3 | queue depth under load | deque `Entity+0x304`: blocks`[+0x308]`, cap`[+0x30C]`, head`[+0x310]`, **LIVE COUNT `[+0x314]`** | funnel entry `0x550720` (`$ecx`=entity) | mid combat round, after script-native `F3::Attack` | **NOT READ — unreachable-by-tooling this session** (§2/§4): no window, no menu, no world, hence no entities and zero funnel traffic |
| R4 | owner back-link pre/post | `[action+0x14]` before `0x550720` (arg at `[esp+4]`; re-read at `0x550741` after the `0x550737` store) | `0x459f5d` (F3::Attack call site), `0x550720`/`0x550741` | immediately around one real enqueue | **NOT READ — same blocker as R3.** Pre-call garbage-vs-zero question remains open |
| R4b | action class vftptr per slot | `[*slot]` for slots `head..head+count` of the `+0x304` deque, mapped via `artifacts/F3.json` | same instants as R3/R4 | same instant as R4 | **NOT READ — same blocker.** No live action object ever existed to classify |

R0 loud-flag check: **no live contradiction to T-045's static answer** (slot 6 = `Update_0x544a90`) and no live
confirmation either — `0x552bb5` never executed, so the `WorldTickPump.md` assumption stays unfalsified.

## 1. The one live datum (why-list material, not a row value)

Four interrupt-samples of the running target, 15 s apart, all identical:

```
pc = 0x7bf3e229  →  ntdll.dll (wine-11.0 i386) + 0xe229  =  _DbgBreakPoint@0   (`int3 ; ret`)
```

Thread census (`/proc/<F3.exe>/task`): main thread `R`, ~800 s of CPU (≈100 % sustained); all 12 worker threads
`S` at `futex_do_wait` with ZERO ticks. The process is not loading, not animating a movie, not pumping:
only the main thread burns CPU while the game code below it never reaches any instrumented address.

## 2. What was run (all cycles, honestly)

| cycle | attach | design | result |
|---|---|---|---|
| 1 (07:18Z) | OK | sync `continue` + 5 sw bps | 0 bp hits in 5 min, no window |
| 2 (07:25Z) | OK | `continue_for` slice sampler | **harness defect exposed: plain `continue` against this stub never returns** — the previous owners' "stub died" reports were partly this, not the stub dying |
| 3 (07:31Z) | OK | async `continue &` + interrupt-poll | probe proved winedbg+gdb17.2: `continue &` accepted, `interrupt` no-ops, no `wait` cmd, `stepi` refused — async sampling path is dead on this stub |
| 4 (07:35Z) | OK | sync bps incl. **TICK probe `0x56af50`** (JE_TickMilliseconds = per-frame heartbeat) | 12 min: TICK never fired ⇒ engine main loop NEVER completed a frame; kill-INT arm worked (v3 patch) → the four `0x7bf3e229` samples above |
| 5 (07:56Z) | attach accepted but gdb saw target **already running** ("Cannot execute this command while the target is running") — no-start lifecycle anomaly reproduced; timeout-killed | — |

## 3. Read of the evidence (mechanism unresolved, stated as such)

The instrumented original stops — before ANY game code we can see — with the main thread at wine's own
`DbgBreakPoint`. Candidate mechanisms, in the order I'd test them:

1. **The wine startup stop is never consumed.** winedbg `--no-start` + gdb: the spawned process should break
   in at startup (ntdll `DbgBreakPoint`) and the FIRST debugger response must be a resume. A batch
   `execute("continue")` issued while gdb still considers the target "not running / running" raced exactly
   that event; after SIGINT-unblock, gdb re-reported the SAME stop address forever — consistent with the
   startup exception being re-delivered rather than passed through. This is a harness/protocol dance defect.
2. A genuine first-chance exception during `d3d8`/wined3d device init (`libEGL warning: pci id 10de:2204,
   driver (null)`; `egl: failed to create dri2 screen` — no GL for wined3d on this X), routed to the debugger
   as a breakpoint-style stop. The render-device-probe already documented device creation as
   environment-dependent on this stack.
3. Less likely: a real shipped-code `__debugbreak` guard during init (would show game EAs in the chain —
   `bt` was NOT capturable, every attempt at it happened while gdb considered the target running).

What is NOT the blocker: the ImHex port collision (31437 workaround attached cleanly every cycle), missing
xdotool (installed — but with no mapped window there is nothing to click; the only wine window ever seen was
the `WineD3D fake window`), and any attach failure (attach worked in all five cycles; `MZ` readable at
`0x400000`).

## 4. Not reached, and why (the miss list)

- **R1, R2** — not reached: `0x48ccf6` never hit in ~13 min instrumented runtime. The menu-phase dice claim
  from the handover did not reproduce on the gdb-instrumented boot; a prior external observation of "alive at
  menu" (mesh note, 07:4x) was made WITHOUT a gdb stub attached — which is precisely the delta to chase.
- **R3, R4, R4b** — not reached, **unreachable-by-tooling this session**: needs a live entity mid-round; the
  instrumented target never created a world (`0x552bb5` unfired), never pumped a frame (TICK probe unfired),
  never mapped a window, never enqueued anything (funnel unfired). A cold entity read would be a substitute
  the row forbids — none taken.
- **R0** — not reached live; statics (T-045) stand unfalsified by this session (the pump literally never ran).

## 5. Next steps for whoever resumes (post-HOLD)

1. Fix the startup-stop dance FIRST, off the critical row: uninstrumented `wine F3.exe` → attach gdb to the
   running-free inferior via a second winedbg? (no — single stub per process) — instead `winedbg --gdb
   --port N F3.exe` **without** `--no-start`, and have the script issue a double-resume right after attach
   (continue → expect the ntdll startup stop → continue again) before arming judgment into any "never fired".
2. If R1 fires after (1), everything above collapses to a harness fix and the row finishes in one sitting;
   if the window still never maps, escalate as a wined3d/d3d8 environment row (candidate:
   `LIBGL_ALWAYS_SOFTWARE=1` for wineserver, or Xvfb per the render-device-probe's case 3).
3. Keep `tools/t034-capture.py` untouched (RedXenon's uncommitted corrections live there); this session's
   working harness (v3, interrupt-tolerant, with TICK probe + R4 pre/post + R4b deque walk) is at
   `/tmp/t034-run.py` + logs `/tmp/t034-run.log`, `/tmp/t034-exception.log` — resurrect or re-land under
   `tools/` at the row holder's choice.

## 6. Incidents and boundaries

- **Incident (disclosed, accepted by coordinator):** during cycle cleanup, `pgrep -x F3.exe` matched across
  wineprefixes and SIGKILLed the reimpl inferior pid 320383 under the 31338 stub (stub 320381 itself left
  exactly as found, LISTEN, no writes). Durable rule now self-enforced: any kill loop must gate on
  `/proc/<pid>/environ WINEPREFIX` (or cwd); comm-grep is never an owner test. No reimpl state beyond the
  inferior process touched; correct repair on resume = `devenv processes restart ida-reimpl-gdb`.
- ImHex pid 12337 on 31337 never touched, never evicted. No IDB writes this session; no `devenv/**` edits;
  no commits (discoveries/ left uncommitted per instruction).
- Clock note for T-031: R1 is per-boot by construction (LCG seeded from `time()` on first call); every
  capture instant in this doc carries wall-clock UTC anyway — nothing was capturable.
