---
status: done
assignee: UltraMoon 01a15cb7 (build lane, dispatched by ZenNova 01a15cb6 after scope verification)
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

## Acceptance evidence (2026-10-21T05:31:54Z gate, UltraMoon 01a15cb7)

**Store — `src/F3/MapEntitySpawn.cpp` `PumpEntity`.** After the FIFO loop, one conditional store:
the taken slot (`actedSlot`) still occupied after its branch ran => `e->actedLatch0x319 = 1`.
- condition (a) = the `0x54c833` analog exactly as the dispatch defined it: `acted=1` and the slot NOT
  dropped (`mv->finished` false, `at->Update != 1`, `w->Expired` false, `die->Update != 1`);
- the slot-DROP path does NOT clear (shipped leaves the latch as the earlier branch decided; the only
  zeroing writers are TurnPrep `0x549094` — ported at `CombatManager.cpp:113` — and the ctor `0x5515ff`);
- condition (b) `0x54c864` is marked **UNEXPRESSIBLE in the port** in the store's own comment, with the
  shipped EAs cited (`0x54c864`, budget at `Entity+0x179`, sibling stores `0x54c85d/0x54c86d` NOT
  replicated): the port has NO +0x179 field (JE::Mover carries only a zeroed gap array, read by nothing)
  and the pump consumes no per-tick budget — exactly one action slot per entity per tick, so "list
  exhausted with budget left" has no state to construct. No budget was invented (T-042's lesson cited
  in the comment as the reason why).

**Tests — openje `PathManager` suite (+11 asserts, 103 -> 114, floor 74):** defer raises (0x54c833
analog); drop does not clear; TurnPrep zeroes what the pump raised; completes-in-one-tick never raises
(the nothing-defers digest-stable shape); stable head WITH the store live — a deferring combatant is
dispatched AGAIN (`LastDispatched()==&d1` twice) and the successor's latch stays 0; and one digest
frame line proves the byte prints: `,st=4,0,1` for the latched entity. The existing no-rotation pin
(clear-then-check dispatch of b1) and the `,st=5,0,0,q=4,` triple-shape assert (T-039) both stay green.

**Falsification was EXECUTED, not asserted:** the store condition was neutralized (`&& 0`) and the
suite re-run — 5 T-048-named asserts went red (`processed-and-not-finished raises`, `slot drop does
NOT clear`, `raise fires under manager dispatch`, `digest prop defers and is latched`, `digest line
names the latched entity ... st=4,0,1`), PathManager FAIL exit 1; the stable-head asserts did NOT
fail (they do not depend on the store — by design). Store restored verbatim (backup+cmp) before the
gate run below.

**"advances its iterator past it exactly once" — landed as the stable-head pin, with reason:** the
skip-advance at `0x5a955d -> 0x5a9572` needs a latch that survives THIS call's maintenance clear, and
maintenance runs before the test unconditionally; the only port-visible window is the `endIter_`
SetRoundMode(0) return that leaves `maintaining_` set (`0x5a9575`, no nonzero writer for +14 per
T-011) — entering that window itself requires a surviving latch, so without re-entrancy the advance
is unreachable **by construction**, which is the shipped-consistent behavior the row's boundary
demands ("make the store correct, let the digest tell us later"). The test therefore pins the
failure the acceptance names: a rotation appearing WITHOUT re-entrancy would break
`LastDispatched()==&d1` on the second call.

**Gate, this run:** `tools/gate-integrity.sh` rc 0 (payload `x95aqfk70mrsqicri2x6spbf38cl63ha`,
9 rows = tree, hash `2945679485`); `msvc-build` exit 0 (no header touched by the store, but the row
required it) printing `toolchain refreshed from registry (ghcr.io/openje/msvc2002:latest)`, F3.exe
2026-10-21 01:31:53 -0400; `run-all-tests` rc 0 => **10 pass, 0 fail** (PathManager 114>=74,
SpawnMove 88>=80, all floors met), app-boot alive past 6s on the fresh exe; `cppcheck-openvb` rc 0.
`docs/f3/SimDigest.md`: the `st=` third value documented as now-live (T-048 section of the edit).
