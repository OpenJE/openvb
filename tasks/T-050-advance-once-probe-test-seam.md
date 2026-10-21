---
status: open (deferred)
assignee: none — OWNER UNRESOLVED until the shipped re-entrancy question lands
depends: T-048 (store landed, af1da87), T-011 Q1 (mechanism), T-034 R-batch (the observation this row waits for)
---

# T-050 — Literal advance-once probe: needs a test seam into `maintaining_`/`endIter_` in CombatManager

## Why it does not exist yet (ruling recorded, 2026-10-21, coordinator + UltraMoon)
T-048's dispatch asked for "the walk advances its iterator past the deferring entity exactly once
per shipped rule". That is NOT constructible in the port without re-entrancy: maintenance-clear is
unconditional (0x549094 fires before the 0x5a955d test in every non-reentrant call), and the only
latch-surviving window is the `SetRoundMode(0)`-and-return path at 0x5a9575 — reachable only after
already entering the skip branch, whose gate `+14` has no nonzero writer (T-011 Q2, open).
T-048 landed the correct substitute instead: the stable-head test stays green with the store present
and rotation-without-reentry fails loudly. Falsification executed (5 named asserts red under the
neutralized store).

## Task (ONLY after the dependency clears)
Add an explicit test seam in `CombatManager` (friend declaration or a narrow internal accessor —
NOT a behavior change) so a test can: enter `Tick`, force `maintaining_` (or drive a real nested
`Tick` from inside a dispatched entity's update once the port models one), set `actedLatch0x319`,
and assert the iterator advances the head exactly once and the successor is dispatched on the next
outer call. The shipped mirror is cited: skip branch 0x5a955d-0x5a9591, clear 0x5a9567, advance
0x5a9572.

## Gate to START this row
The lockstep observation (T-034 routed questions, or the parity digest showing rotation in the
original) must first answer: does `meth_0x5a94a0` actually get re-entered from inside a dispatch,
and how deep? Building the probe before that freezes a mechanism shape we have not yet watched
running — the exact cost class of the T-042 lesson.

## Boundary
The seam touches CombatManager internals — test-visible only, zero shipped-mirroring change; any
temptation to make the seam also CLEAR or SET `maintaining_` outside the nested-call experiment is
a behavior edit and belongs in a different row. If the observation says re-entry never happens in
the demo's lifetime, this row is retired with that verdict recorded, and the stable-head test
remains the canonical shape.
