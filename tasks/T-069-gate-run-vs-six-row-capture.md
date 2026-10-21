---
status: in-progress
assignee: CalmRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — gate run, T-051 pairing auto-resolved by T-056 capture existence at d283ce3)
type: gate run + evidence pairing (build/test lane; no IDB writes)
depends: T-051 pre-work (LANDED: PathManager 117>=74, sim-driver-verify 13/13), T-056 six-row capture
---

# T-069 — Execute T-051's held GATE against the LIVE six-row capture

## Objective
Pair T-051's dead-walk pins against HappyEagle's REAL shipped numbers (T-056 six-row table). The pins
assert the manager round-walk branch is DEAD in shipped (mode +0x4 / rearm +0x14 have no nonzero
producer; T-048's +0x319 latch raiser lives inside that walk). R2 (frame-0 clock leap 0s→1s) and R3
(deque count 0→1→2→3 at funnel 0x550720) are the live datums the pins must be consistent with.

## Test to run (precise scope)
Pins claim UNREACHABLE-BY-DEFAULT: (a) the `mode!=0` round-walk branch / `MidTurn()` skip-advance,
(b) the T-048 acted-latch (+0x319) raising on the shipped path. They do NOT claim the world clock is
static or that entity action queues never grow. So: does the capture exercise (a)/(b), or is it fully
explained by the free-run path + world pump? If any pin is contradicted, THE PIN CHANGES and this row
says why, citing the capture.

## Acceptance
run-all-tests 9/9+ with quoted counts; msvc-build rc0 if headers move; TEST-ONLY arming audit re-run
(0 new SetRoundMode callers in sim-driver); sim-driver branch-trace prints free-run; evidence pasted
here + ledger; submodule-first clean, parent second.
