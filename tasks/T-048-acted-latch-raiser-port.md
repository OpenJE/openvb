---
status: open
assignee: unassigned (port work; needs T-011's dispatch path and the event-queue fields)
depends: T-011 (walk + latch consumer, ported), T-041+raiser study (shipped writer closed)
---

# T-048 — Port the +0x319 raiser: our pump never marks "more work pending", so the walk is permanently head-stable

## Established, do not re-derive
- Shipped `Entity+0x319` writer set is CLOSED (docs/f3/CombatTurnDispatch.md §"+0x319 raiser"):
  the entity's own update `Entity::virt_meth_0x54c570` sets it at `0x54c833` (an animated-event item
  did not finish this frame: remaining time < `event+0xC`) or `0x54c864` (event list exhausted while
  dt budget at `+0x179` is still nonzero); TurnPrep `0x549094` clears it for state-5 nodes each
  maintenance pass; `0x5a955d` (the combat manager) tests it and 0x5a9572 advances the iterator.
- Our port HAS the consumer (`CombatManager.cpp` round walk reads `actedLatch0x319`) and the clear
  (TurnPrep mirror), but NOTHING writes the latch: `SpawnedEntity::actedLatch0x319` has only the
  ctor zero and the digest read. Consequence: the skip-advance path is dead in the port.

## Task
Raise `actedLatch0x319` in the port's entity-update path under the shipped two conditions. Honest
scope decision first: our port's event-queue analog (Entity+0x2E0 deque + the `+0x179` dt budget) —
check what exists in `src/F3` (MapEntitySpawn/SimDigest fields) and either model the two conditions
where the analog exists, or land the latch write for the ONE condition the port can express (e.g. an
action that defers because its duration exceeds the frame budget) with the other condition marked
unexpressible. Do NOT invent a rotation side effect in CombatManager — the manager only READS the
latch, as shipped.

## Acceptance
- Cited port sites for the store (mirror `0x54c833`/`0x54c864` semantics, each store's condition
  cited to the shipped EA);
- SpawnMove/PathManager suite gains a case: an entity whose update defers work shows the latch set,
  the walk advances its iterator past it exactly once per shipped rule, and a non-deferring entity
  keeps the stable head (the existing no-invented-rotation test must stay green);
- `turnOrder=`/digest lines unchanged for runs where nothing defers (falsification: the new test
  fails if the store is removed);
- msvc-build + run-all-tests + app-boot quoted from the row's run; digest fields documented in
  docs/f3/SimDigest.md if the `acted` bit starts moving.

## Boundary
Whether `meth_0x5a94a0` is RE-ENTERED from inside a dispatch remains a shipped open question
(T-011, routed; lockstep observation). This row must not assume re-entrancy: make the store
correct, and let the digest tell us later whether the original rotates more than our single
skip-per-call. A rotation appearing WITHOUT both the store and re-entry is the failure this row's
test exists to catch.
