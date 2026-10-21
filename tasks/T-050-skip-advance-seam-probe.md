---
status: open
assignee: owner: "after T-034/R-batch reports" — no one may start this row before the lockstep side has reported whether
  re-entrancy occurs at all (T-011 routed question / T-034 remaining observation). Filed by UltraMoon 01a15cb7 at
  ZenNova 01a15cb6's ruling after T-048 was accepted (af1da87).
depends: T-048 (store landed; this row probes the consumer window it created), T-034/R-batch (re-entrancy observation, routed)
---

# T-050 — Skip-advance seam probe: give the walk a constructible surviving-latch window, ONLY after the lockstep side has seen re-entrancy run

## Why this row exists (the finding it must not lose)

T-048 landed the `actedLatch0x319` raiser (`PumpEntity`, cites `0x54c833`), but established a structural
fact first (UltraMoon dispatch question, RULLED by the coordinator before commit af1da87):

**The skip-advance path (`0x5a955d` test -> `0x5a9572` iterator advance) is unconstructible in the
current port.** Maintenance runs unconditionally before the walk's test inside every call and clears all
state-5 latches; the only window where a latch can survive to the test is the `endIter_`
`SetRoundMode(0)` return at `0x5a9575` that leaves `maintaining_` set — but entering that window already
requires having taken the skip branch once (chicken-and-egg), and `+14` (`endIter_`) has no nonzero
writer anywhere in the DB (`0x5a9371`/`0x5a93b0` store 0 only; T-011 verified). In the shipped binary the
window opens through **re-entrancy**: `+8` is the re-entry latch (`0x5a94a3` tests it), and the manager's
`meth_0x5a94a0` entered from inside a node's own dispatch skips maintenance, sees the pump's just-raised
latch, and advances. Whether that nested entry actually occurs is the routed T-011/T-034 lockstep
question — it has NOT been observed running yet.

T-048 therefore pinned the failure mode that IS decidable statically: rotation-without-re-entry fails
loudly (stable-head asserts, PathManager T-048 block). This row is the follow-on that makes the advance
itself testable — but ONLY in the shape the observation dictates.

## Task (gated; do not start before the dependency reports)

1. Read the T-034/R-batch re-entrancy report when it lands (discoveries/ or docs/f3/CombatTurnDispatch.md).
2. **If re-entrancy is OBSERVED**: design the minimal test seam — most likely a read-only or
   test-only way to place the manager in the `maintaining_ == true` state (e.g. expose `bool
   MaintainingForTest() const`, or a `SetEndIterForTest()` matching the exact nested-call shape the
   observation showed) — mirroring the observed nesting path, and add the PathManager/SpawnMove case:
   latched combatant at the iter position + nested entry => iterator advances exactly once per call,
   deadline `+0x29C` zeroed (`0x5a9567`), round clock NOT advanced on the skip (`v9` stays 0, `0x5a95ac`
   writes the accumulator unconditionally), `turnOrder=` stable.
3. **If re-entrancy is NOT observed / falsified**: the shipped walk advances by some OTHER writer/window
   we have not found — the row becomes a discovery request, not a seam. Do not build the probe in that
   case; reopen the writer study instead.

## Hard boundary

- No CombatManager behavioral edits; any seam must be inert outside tests (this is what froze the design:
  the seam must match the observed nesting shape, and an unobserved shape would freeze a guess).
- No invented rotation: if the probe would let a test make the walk rotate WITHOUT the observed shipped
  path, the probe is the bug.
- The digest (`turnOrder=`, `st=`) is the arbiter of any parity claim; this row's tests may only assert
  mechanism, never "matches shipped", until the lockstep observation says so.

## Acceptance (to be completed once unblocked)

- Cite the observations/report filename that unblocked the row (discoveries/ or docs/f3/), with the EA
  of the nested call site if re-entrancy was seen (`0x5a94a3` is the +8 test; the CALLER of the nested
  entry is what the observation owes us);
- seam is inert (grep: no production caller), advance-once/deadline-zero/accumulator-zero asserts green,
  existing T-048 stable-head block stays green;
- run-all-tests + (if any header touched) msvc-build quoted from the row's run.
