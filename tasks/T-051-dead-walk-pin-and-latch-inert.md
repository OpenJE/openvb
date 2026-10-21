---
status: in-progress
assignee: UltraMoon `01a15cb7` (dispatched by coordinator at e662a42; pre-work authorized, final gate held for T-034)
depends: T-034 (lockstop readings) for the digest side, T-011/T-048 (walk + latch, ported)
---

# T-051 — Pin the dead round-walk branch: mode/rearm constants + T-048 latch inert in shipped-reachable configuration

## PRE-WORK LANDED (2026-10-21, UltraMoon 01a15cb7) — final GATE held for T-034 per dispatch ruling

Deliverables 1-3 verified this run (PathManager 117>=74, all observed counts; sim-driver-verify 13
checks PASS incl. 3 new; run-all-tests 10 pass 0 fail as REGRESSION check on the pre-GATE exe):

- **Suite**: new T-051 block — default manager is free-run (`RoundMode()==0`); a LATCHED node is
  still DISPATCHED under free-run, proven behaviorally (manual latch is cleared by maintenance
  in BOTH branches; the value being 1 again after the call can ONLY come from the T-048 raiser
  inside a pump that RAN — a live walk would have skipped and advanced instead); two consecutive
  free-run Ticks advance no `accMs`, unlink nothing, rotate nothing. +3 asserts (117 observed
  vs 114 pre-row; table floor 74 — floor move deferred to the GATE commit, editing
  devenv/scripts here is outside this row's grant).
  **Self-correction logged**: my first version asserted the manual latch SURVIVES the free-run
  Tick — red on the first run, because maintenance runs regardless of branch. The rewritten
  assert is stronger (dispatch-happens proof, not absence proof). This is exactly why pre-work
  runs its suites even before the formal gate.
- **Arming audit PRINTED**: `SetRoundMode` non-comment sites — sim-driver.cpp: **0** (new verify
  check asserts it every run); production: definition (hpp:101, cpp:294) + the pre-existing
  shipped mirror of 0x5a957f inside the dead gate (arms to 0 = release, unreachable, pre-T-051,
  unchanged); tests: 7 `SetRoundMode(1)` arming sites, EVERY one now annotated
  `TEST-ONLY arming (dead in shipped, 10e/045a7ec; T-051 annotation)` — 4 pre-existing from
  T-011, 2 from T-048, **0 added by T-051** (the new block uses the default free-run).
- **sim-driver**: one stderr trace line naming the branch TAKEN, read from the live
  `RoundMode()` (not a constant): `combat branch: free-run (mode0x4_==0) - the only
  shipped-live path (10e)...`; if anything ever arms the dead walk, the line prints
  `round-walk ... SOMETHING ARMED IT - T-051 audit expects never` and sim-driver-verify's new
  guard FAILS. Verify additions: arming grep (want 0), trace line check, round-walk guard.
- **Docs**: `docs/f3/CombatTurnDispatch.md` walk-branch section now opens with the pinned
  DEAD-IN-SHIPPED banner citing 10e/`045a7ec`, naming T-050's Branch 2 as the strong prior and
  stating the observation outranks the banner.

**Remaining for DONE**: the quoted GATE (fresh `msvc-build` exe + run-all-tests + this row's stamp,
paired with T-034's capture per the like-for-like ruling) — held, not skipped.

## Established, do not re-derive (T-010 10e, `discoveries/2025-10-21-t010-10e-manager-producers.md`)
- `cls_0x5a9330` +0x4 (mode) and +0x14 (rearm latch) have NO nonzero producer anywhere in the shipped
  image; `meth_0x5a9330` (sole +4 writer) is called exactly once (`0x5a957f`) behind `[+14]!=0`.
- Consequence: the manager's round-walk branch (the `+4 != 0` path at 0x5a951d onward) is DEAD CODE
  in the shipped demo; only the free-run queue loop (0x5a9525) ever runs.
- The port mirrors all constants already (`mode0x4_=0`, no rearm). T-048's +0x319 store landed and is
  exercised ONLY through the walk branch — reachable in tests, unreachable in shipped-config. This is
  exactly the cited-and-absent discipline of T-019's never-fires ledger, not a deviation.

## Task
1. CombatManager suite asserts: fresh manager => `Tick` takes the free-run branch; no port path
   reaches `MidTurn()` skip-advance without an explicit `SetRoundMode()` test call that SAYS SO in the
   test name/comment (the shipped image has no such caller; the sim-driver must NOT add one).
2. docs/f3/CombatTurnDispatch.md gets a "dead in shipped" banner line at the walk-branch section
   citing the 10e doc — so later lanes stop treating rotation depth as a live shipped question
   (T-050's Branch 2 is now the LIKELY branch; keep it gated on the observation, but the
   re-entry question should be re-scoped in 10e's light: the walk being dead means nested
   `meth_0x5a94a0` observation cannot rotate anything on the shipped side either).
3. sim-driver-verify: confirm the driver reaches combat through JoinCombat/flush only (free-run),
   and print the branch taken in its trace (one line, not a new field).

## Acceptance
- New asserts quoted from a run (suite floors move or a new floor is written down);
- no invented arming path anywhere in the diff (grep the row's evidence: zero new callers of
  `SetRoundMode` outside tests);
- gate 10/0 + msvc-build if headers touched, stamp from THIS run.

## Boundary
This row does NOT resolve the T-050 re-entry question — it records that the shipped demo's mode
path is dead, which makes Branch 2 (writer-study reopen, no probe) the strong prior. If T-034's
capture ever shows a live second `meth_0x5a94a0` entry anyway, that observation overrides this row's
framing, not the other way around.
