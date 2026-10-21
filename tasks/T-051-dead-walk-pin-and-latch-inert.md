---
status: done
assignee: UltraMoon `01a15cb7` (pre-work, dispatched at e662a42) → CalmRaven (GATE RUN executed + paired vs the T-056 capture, T-069 dispatch `d283ce3`)
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

## GATE RUN EXECUTED (T-069, CalmRaven, 2026-10-21) — evidence from THIS run

A header moved (the `actedLatch0x319` comment in `src/F3/MapEntitySpawn.hpp`), so `msvc-build` was required
and run: **exit 0**, `Built target F3`, no `error C`, no `LNK`. Pairing target = HappyEagle's T-056 live
six-row table (`discoveries/2025-10-21-t056-xorg-dummy-six-rows.md`, run H) — the capture that auto-resolved
this row's pairing decision.

- **`run-all-tests` => 10 pass, 0 fail** on the NEW payload (`exe 2026-10-21 17:32:19, 57,344 B`):
  cls_0x4d8d70 11≥11 · **PathManager 117≥117** · ResourceSystem 132≥111 · StringSystem 27≥20 ·
  ActionMoveTo 13≥13 · Scripting 6≥6 · GUILoading 197≥116 · SpawnMove 88≥80 · CombatMath 43≥43 ·
  `app-boot` exit **124** (alive past 6 s).
- **Stamp**: `2026-10-21T21:32:26Z`, HEAD `b029da0` / openje `653fa79`, table 9 rows hash 483599247,
  compiler `i686-w64-mingw32-g++ (GCC) 15.3.0`, dirty 3 files / 1 build-affecting.
- **`sim-driver-verify` PASS** (22 checks) incl. the three T-051 guards: `SetRoundMode` call sites in
  sim-driver = **0**; branch trace names **free-run**; walk-arming guard **ok (no round-walk trace;**
  free-run only)**.
- **Arming audit re-run**: 0 new arming sites; all 7 `SetRoundMode(1)` suite sites carry the
  `TEST-ONLY walk arming (dead in shipped, 10e/045a7ec; T-051 annotation)` comment (:437/476/507/509/595/621).
- **Pairing verdict vs the live datums**: R2 (`0s→1s` frame-0 clock leap) and R3 (deque count
  `0→1→2→3` at funnel `0x550720`) are **both free-run phenomena** — R2 is the WORLD clock at
  `[[0x70BD98]+0x3C]` while the manager's round accumulator `accMs_` stays 0 in free-run
  (`fresh.AccMs() == 0u`, asserted twice); R3 is the `+0x304` action DEQUE, not the `+0x2E0`
  animated-event queue the raiser's second shipped branch reads. Neither exercises the dead branch ⇒
  **pins HOLD; no reclassification**. Rotation depth / re-entrancy stays **BLOCKED-ON-RUNG-2**
  (bink-upload SIGSEGV at 2.5–4 min precedes interactive combat; `F3::Attack` never fired),
  explicitly not a null result about the seam.
- **One pin wording changed, with the capture cited**: see the SCOPE CORRECTION bullet above — the
  raiser is shipped-live, only the skip-advance consumer is walk-only. Doc/`MapEntitySpawn.hpp`
  comments updated to match; no statement, digest field, or assertion changed (assert counts unmoved
  is the proof this was prose-only).
- Constraints honored from the table: no combat-round claim beyond the scripted funnel, no `>4`
  queue-depth claim, R1 used only as STATE-at-instant.

## Established, do not re-derive (T-010 10e, `discoveries/2025-10-21-t010-10e-manager-producers.md`)
- `cls_0x5a9330` +0x4 (mode) and +0x14 (rearm latch) have NO nonzero producer anywhere in the shipped
  image; `meth_0x5a9330` (sole +4 writer) is called exactly once (`0x5a957f`) behind `[+14]!=0`.
- Consequence: the manager's round-walk branch (the `+4 != 0` path at 0x5a951d onward) is DEAD CODE
  in the shipped demo; only the free-run queue loop (0x5a9525) ever runs.
- The port mirrors all constants already (`mode0x4_=0`, no rearm). **T-069 SCOPE CORRECTION — this row's
  next sentence was wrong as written.** It claimed T-048's +0x319 store "is exercised ONLY through the walk
  branch — reachable in tests, unreachable in shipped-config." It is not: `PumpEntity`'s raiser
  (`src/F3/MapEntitySpawn.cpp:476`) is reached by BOTH pump entrances — the world step-5 loop
  (`TickOneEntity`, `combatantDispatch=0`) and the manager dispatch (`TickEntityServerUpdate`, `=1`) — and the
  store is not gated on that flag, so the RAISER is shipped-LIVE on the free-run path. This row's OWN T-051
  block proves it (green in the gate run, `modules/openje/tests/je/PathManager/PathManager.cpp:672-691`): a
  node latched BEFORE a `Tick` with NO `SetRoundMode` is found latched AGAIN after it, and since
  maintenance/TurnPrep zeroes the latch in BOTH branches, only a raiser inside a pump that RAN can restore it.
  What IS walk-only and dead in shipped — correctly pinned — is the CONSUMER: the skip-advance
  `0x5a955d -> 0x5a9572`, which needs a latch that survives its own call's maintenance clear (re-entrancy only).
  So the walk stays head-stable for the re-entrancy reason, NOT for lack of a store. `mode +4` / `rearm +14`
  still have no nonzero producer (10e/`045a7ec`) and nothing in the T-056 capture bears on that. Cited-and-absent
  discipline stands, now scoped to the right half. Full pairing + gate numbers:
  `discoveries/2025-10-21-t069-gate-run-ledger.md`.

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
