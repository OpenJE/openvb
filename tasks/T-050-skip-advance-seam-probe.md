---
status: open (deferred — GATED)
assignee: none by design — owner resolves only after T-034/R-batch reports re-entrancy either way
depends: T-048 (store landed, af1da87), T-011 Q1 (mechanism closed), T-034 (the observation gates this row)
---

# T-050 — Skip-advance seam probe: make `0x5a955d -> 0x5a9572` constructible in tests, in the shape the observation dictates

## Consolidation provenance (this row is the merge of a 2-minute double-filing)
Filed twice concurrently after the T-048 ruling: UltraMoon's `276c0f8` (row: skip-advance seam
probe, two branches) and the coordinator's `855d0f6` (row: advance-once test seam). The
cross-resolve (`5438cec` + `afc091a`) removed BOTH files while one board row survived — this file
restores the single canonical row, keeping UltraMoon's branch structure and the ruling provenance.
The number T-050 is taken by this row (item-28 namespace rule).

## Why it is not executable now (the T-048 finding, do not re-derive)
The manager's skip-advance (`0x5a955d` test -> `0x5a9572` iterator advance) is unconstructible in
the port as it stands: maintenance-clear-before-test is unconditional (0x549094 clears every
state-5 latch before the walk tests it), the only latch-surviving window is the
`SetRoundMode(0)`-and-return at `0x5a9575` — reachable only after entering the skip branch, whose
gate `+14` has no nonzero writer in the DB (T-011 Q2, open). In shipped, the window opens through
**re-entrancy** (+8 at `0x5a94a3`, manager-owned — DB-closed: cls_0x5a9330 `mbr_0x8` usages
0x5a94a3/ab/552) — and the lockstep side has NOT yet observed the nested call running. Writing the
seam before that observation would freeze the shape of an unseen mechanism (T-042's cost class).
T-048's interim canonical test stands meanwhile: stable head with the store present, and
rotation-without-reentry fails loudly (falsification executed, `af1da87`).

## Task — ONLY after the gate clears; pick the branch the observation supports
- **Branch 1 (re-entry observed):** add an explicit test seam in `CombatManager` (friend
  declaration or narrow internal accessor — test-visible, ZERO production-behavior change) so a
  test can drive a nested `Tick` (or force `maintaining_`/`endIter_`), set `actedLatch0x319`, and
  assert the iterator advances the head **exactly once** per the shipped window and the successor
  dispatches on the next outer call. Cited mirror: skip 0x5a955d, deadline clear 0x5a9567, advance
  0x5a9572, head-wrap release 0x5a958f.
- **Branch 2 (re-entry falsified or unreachable in the demo's lifetime):** this row converts to a
  writer-study reopen for `+14` (and the `0x5a9575` window) — build NO probe; land the verdict in
  docs/f3/CombatTurnDispatch.md and retire the probe requirement with that citation.

## Gate to START
T-034's routed observation (or the T-031 parity digest showing rotation in the original) must
answer: is `meth_0x5a94a0` re-entered from inside a dispatch, and how deep?

## Boundary
Hard: the seam must be inert in production — any urge to make it also SET/CLEAR `maintaining_`
outside the nested-call experiment is a behavior edit and belongs to a different row. If re-entry
never happens, the stable-head test remains the canonical shape forever, and a later "rotation
fix" is the failure mode this row exists to keep honest.
