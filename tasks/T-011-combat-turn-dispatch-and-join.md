---
status: in-progress
assignee: HappyKnight — dispatched 2026-10-21 as PRIMARY feature work (sequenced ahead of T-031)
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
- **The attack object's series is a vector, not a counter** (FastArrow, T-010c follow-up; fixes
  session-21's IDA-skewed layout): `+0x7C begin / +0x80 finish / +0x84 storage-end`, ctor-init via
  `sub_401690(ecx=&this+0x7C)`, plus a **second parallel vector** `+0x88/+0x8C/+0x90` whose role is
  per-sub-attack timing (hitDelay / the gap-0x65 machinery) and whose element layout is **not
  pinned**. Elements are records; the uncalled-shot hit location is computed by a selector and
  written to `record+0x20` @0x404989, then reaches `CalculateDamage` as `param_5`. So for a
  called-shot attack the part comes from `this+0x78`, and otherwise from the per-volley record —
  T-011 must not assume the action carries one scalar part for the whole series.
- Turn protocol fields already exist on `SpawnedEntity`: `frameDt0x300`, `actedLatch0x319`,
  `turnDeadline0x29C`, `flag0x1E4` (T-039: DB size 4, meaning OPEN — do not rename it after a guess),
  and the queue depth, which is **derived**: `SpawnedEntity::QueuedActionCount()`. **T-039 settled step
  3's second term:** `+0x314` is the action deque's ELEMENT COUNT (`++` at 0x54ea71 via the funnel push,
  `--` at 0x54b637/0x54b672 on erase, `virt_meth_0x4084c0` returns `+0x314 == 0`), so
  `+0x1E4 == 0 || +0x314 != 0` at 0x5a95c1 dispatches when the entity **has queued actions** — NOT "is
  busy". Mirror that test; cite the writer EAs, not the reader EAs.
- Gate interlock is live: `World::CombatTick` runs before `ProcessJobs` (0x545E0D/0x545E18) and the
  step-5 entity loop is skipped when `CombatMidTurn()` (0x545E23).

## Landed (2026-10-21, HappyKnight 01a15be7) — steps 2-6, with two things NOT done on purpose

Read from the original and ported line by line; the durable write-up is **`docs/f3/CombatTurnDispatch.md`**.

* **Join** is in `GameActionAttack::Start` (the `OnCreate_0x4011d0` @0x401229 analogue, not the ctor) and
  goes through `CombatManager::JoinCombat` = 0x5490b0 → 0x5a9810: dedupe-walk (0x5a9826-0x5a9841, which is
  what makes a second attack neither duplicate NOR re-state), insert, then `ChangeState(5)` (0x5a987b).
  If the state store is refused the node is REMOVED again — a listed entity that never entered rounds is
  the half-join the atomicity exists to prevent. `Flush()` mirrors 0x5a9440 (unlink, `--count`,
  ChangeState(4) at 0x5a9485) and the dtor calls it, per 0x5a9a10.
* **Dispatch** is the whole of 0x5A94A0, including the two things that read like bugs and are not:
  the skipped-actor path writes **accMs = 0** (0x5a9517 + 0x5a95ac), and the `SetRoundMode(0)` return at
  0x5a957f is the ONLY exit that leaves +8 set. Quantum is `1000 * secondsPerRound` **per dispatched
  event** (0x5a95aa); the old `accMs += TotalSeconds()*1000` per frame is retired, and `GameWorld::CombatTick`
  now feeds `m_secondsPerRound` (world+0x40) instead of the clock. `sim-driver` was corrected the same way —
  passing elapsed seconds there was the retired C-23 placeholder wearing new clothes.
* **Gate** mirrors 0x5a95c1 as `flag0x1E4 == 0 || QueuedActionCount() != 0`, i.e. **a queued order forces
  the turn** (T-039). `+0x1E4`'s MEANING is not adopted — it is named for its offset, and the question is
  T-041's.
* **Interlock** extended, not bypassed: one `PumpEntity(e, ms, combatantDispatch)` body, because the
  state-5 skip at 0x545e8e belongs to the WORLD loop and the combat dispatch must be able to pump the
  same entity. A second copy of that function would drift and the drift would read as combat behaviour.
* **Tests** (PathManager, 103 asserts, floor 74): the six the row asked for, plus the ones that only make
  sense after reading the lines — clear-then-check order (0x5a94fd before 0x5a955d), "1 ms and 999 ms
  frames give the SAME round clock", and **non-combat entities byte-identical with an empty list AND with a
  list holding someone else** (the second control is the one that can actually catch a leak).

### NOT done, and why — three routed questions, no invented code

1. **The walk does not rotate in the port.** The only iterator advance is 0x5a9572, behind a +0x319 that
   this call's own TurnPrep cleared at 0x549094 — so rotation is a property of RE-ENTRANCY (+8 is the
   re-entry latch, 0x5a94a3) plus the pump raising +0x319 (0x54c833/0x54c864). Nothing in `src/F3`
   writes `actedLatch0x319`. Adding a round-robin would change WHO acts — the one fact `turnOrder=` exists
   to compare — so a test pins the stable head and fails loudly if a rotation appears without the store.
   Question: which shipped writer raises +0x319 on the combat path, and is `meth_0x5a94a0` re-entered
   from a node's own ServerUpdate?
2. **Who sets manager+0x04 nonzero, and who sets +14 nonzero.** The DB has no nonzero writer for either,
   and the only xref'd caller of `meth_0x5a9330` passes 0. Until that is read, the round-walk branch is
   reachable only from tests that call `SetRoundMode()` and say so.
3. **`meth_0x401dd0(manager+0x18, 1)` at 0x5a9868 and the `vft+27 → meth_0x54ef30 / meth_0x54bf00` tail
   of ChangeState (0x55094b-0x550960)**: the deferred GUI/net refresh. Cited and absent, same rule the
   enqueue seam follows — a flag nobody reads would be a false model of a subsystem not reconstructed.

One link fact recorded because it cost two red suites first: `GameActionAttack.cpp` is compiled into the
ActionMoveTo and SpawnMove rows WITHOUT `CombatManager.cpp`, so the join is reached through a function
pointer stored in `MapEntitySpawn.cpp` (harness item 22 — run the suites you are not editing).

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

   **TIME BASE: SETTLED (T-010d closed by HappyWolf - no re-verify caveat remains).** `GW+0x40` is
   **immutable config**, and the key name proves my label: `JE::World::virt_meth_0x546b50` (world
   config load) defaults it to 6 @0x546b5b, selects INI section **"World"**, reads the key
   **"Seconds Per Round"** @0x546c35 and `atol`s it into `GW+0x40` @0x546c8f - the same pass that
   loads the Year/Month/Day/Hour/Minute/Second block at `GW+0x14..0x28`. Writer census is closed:
   **{that config init, net world-unpack @0x547e9e} only**, and the one suspected write-back channel
   (the `GW+0x3C` clock object's `vft+88` AddSeconds) is **dead** - no path to +0x40 exists.
   So arm deadlines as a **per-round quantum** (`1000 × SecondsPerRound` added to the manager's
   `+0x10` once per dispatched queue event), never as wall-clock ms and never as a continuously
   advancing accumulator.
   - Separate domain, still yours to honour: when the owner byte `+0x319` is set the event carries an
     explicit time stored at `+0x29C` and the round quantum is not consulted at all.
   - **Never** mix `+0x10` (round-clock ms) with real-ms action deadlines unless `+0x319 == 0`.
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
