---
status: open
assignee: unassigned (pick up after T-009; implementer-B may take it after T-007)
depends: T-010a (behind-sector constants, for the reaction anim side of a landed hit)
---

# T-011 — Combat: wire the turn queue's per-turn dispatch (and join-on-attack)

## Objective

`F3::CombatManager` is a faithful *structural* port of 0x5A94A0 (circular turn list, re-entry
latch `+8`, `accMs` `+10`, iterator `+C`, maintenance pass with TurnPrep semantics) but its
**per-turn dispatch is deliberately unimplemented** — it was left as documented pseudocode rather
than invented. This task closes that, plus the join path, using only raw-verified mechanics.

## What is already established (do not re-derive)

- **Join is not a phase, it is a set, and only attacks join it.** `JE::Entity::JoinCombat_0x5490b0`
  (Entity-vb vft slot +0x30) is called *only* from `GameActionAttack::OnCreate_0x4011d0` @0x401229
  and `meth_0x403320` @0x4033d4/0x40359b. It does `Combat::AddToList_5a9810` then
  `ChangeState(5)`. There is no proactive encounter trigger.
- **State 5 has two producers with the same number:** ctor raw-`SetMode(5)` (pre-registration
  sentinel) and validated `4→5` from JoinCombat (= "in rounds"). The world pump skips generic
  ticking for 5; combatants are driven through the action pipeline instead.
- **Leave paths:** `Entity` detach `0x54fff0` @0x55003d → `Combat::Remove_5a93c0`;
  `Combat::Flush_5a9440` → all members `ChangeState(5→4)`; conditional 5→4 at 0x5a9410-0x5a941a
  and 0x551c8b.
- **No round timer exists on Combat** (`0x24` = list + scheduler only). The AP economy is
  entity-side (creature `OnUpdate` AP regen) plus action durations. So do not build a round clock.
- Turn protocol fields already exist on `SpawnedEntity`: `frameDt0x300`, `actedLatch0x319`,
  `turnDeadline0x29C`, `noTurn0x1E4`, `busy0x314` (cited to the 0x5A94A0 decompile).
- Gate interlock is live: `World::CombatTick` runs before `ProcessJobs` (0x545E0D/0x545E18) and the
  step-5 entity loop is skipped when `CombatMidTurn()` (0x545E23).

## Steps

1. Reserve only what you touch: `src/F3/CombatManager.{hpp,cpp}`,
   `src/F3/GameActionAttack.{hpp,cpp}`, `src/F3/MapEntitySpawn.cpp`, and your test file.
2. Wire **join** into `GameActionAttack::Start` (the OnCreate analogue) rather than into the
   constructor, and make it go through the manager so the list and the state change stay together.
   Follow the legal-transition table (4→5 only; 5→4/6/2 out). An entity that cannot join must not
   silently stay in rounds — decide and document which side fails.
3. Implement per-turn dispatch in `CombatManager::Tick` from the 0x5A94A0 lines, not from
   intuition: iterator walk, the `+0x319` latch check (an entity that already acted gets its
   `+0x29C` deadline cleared and is skipped this frame), the `+0x1E4 == 0 || +0x314 != 0` gate,
   deadline arming from `accMs`, and the entity-side turn update it dispatches to.
4. **Time source — now settled (canonical §13), and it is NOT a wall clock.** GW+0x40 has exactly
   two writers: the World ctor @0x548b23 (=6) and `InitClock` @0x546b5b/0x546c8f
   (`atol("Seconds Per Round")`). Nothing writes it continuously. The consumer @0x5a95ac adds
   `1000 * GW+0x40` to the manager's `+0x10` accumulator **once per dispatched queue event**, so
   it is a **round-based virtual clock**: N processed items ⇒ N × SecondsPerRound × 1000 ms.
   Second domain: when the owner byte `+0x319` is set, the event carries an explicit time stored at
   `+0x29C` and the round clock is not consulted at all.
   ⇒ Hard rule for your port: never mix `+0x10` (round-clock ms) with real-ms action deadlines
   unless `+0x319 == 0`. Keep the manager clock-source-agnostic (it takes `clockSeconds` in), but
   feed it per **dispatched event**, not per wall-ms frame. Do not resurrect the old `IWorldServer`
   slot-0 reading — that was a
   decompiler artifact and C-23 is closed: `World+0x70` is a double `worldSimSeconds`, and the
   pump's seconds-carry is the authoritative advance in the reimpl. If the *original's* cadence
   value still matters, that is a research question (T-010), not a license to invent.
5. Interaction with the FIFO: a combatant in state 5 must not be double-ticked (generic pump +
   turn dispatch). The existing `CombatMidTurn()` gate is the seam — extend it, don't bypass it.
6. Tests: extend the PathManager suite (it already covers grid+A\*+waypoint+clock+combat protocol)
   with: join-on-attack puts exactly one entity in the list and flips 4→5; a second attack does not
   duplicate; flush returns everyone to 4; the latch skip actually skips; deadline arming is a
   pure function of the injected clock (deterministic — no wall-clock reads).

## Acceptance

- `run-all-tests` 9/9 with the new cases named in the summary; `msvc-build` exit 0; `app-boot` 124.
- Every new mechanic line in `CombatManager.cpp` cites an EA. **Any line that would invent a rule
  (round timers, initiative ordering, an AP spend) is out of scope here** — if you believe one is
  needed, post the question instead of coding it.
- No behavior change for non-combat entities: the step-5 loop still ticks identically when the
  turn list is empty (assert this in a test).

## References

- `discoveries/2025-10-20-session23-combat-join-leave-state5.md` (join/leave + state-5 semantics)
- `discoveries/2025-10-20-session12-serverupdate-ondeath-flip.md` (pump dispatch, 6→7)
- `discoveries/2025-10-20-session8-entity-lifecycle-state-die.md` §1 (legal transition table)
- `src/F3/CombatManager.hpp` (layout + the C-23 caveat note to replace)
