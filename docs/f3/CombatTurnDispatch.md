# Combat turn dispatch and join (0x5A94A0 / 0x5490B0 / 0x5A9810 / 0x5A9440)

Durable notes for `F3::CombatManager` (JE `cls_0x5a9330`, world+0x8C, DB size 36). Landed by T-011
(2026-10-21, sessionId 01a15be7). Every line below cites the EA it was read from; the two things the
port does NOT do are called out as absences rather than filled in.

## Layout, from the DB (not inferred)

`mbr_0x4` (size 4) · `mbr_0x8` (1) · `mbr_0xc` (4) · `mbr_0x10` (4) · `mbr_0x14` (1) · `mbr_0x1c` (4)
· `mbr_0x20` (4). Node = 3 dwords, entity at node+8 (read as `v4[2]` / `*(node+8)` at 0x5a94c2/0x5a955a).

## Manager+0x04 is a MODE, not a pause — the polarity is the opposite of the old label

0x5a951d `if (!this->mbr_0x4)`:

* **+4 == 0 → free-run.** Walk the whole list and give EVERY node a plain
  `F3::ServerUpdate(entity, dt)` (0x5a9525-0x5a954f), then clear +8 and return. Nothing is paused;
  everybody acts every frame. This is the branch the app runs in today.
* **+4 != 0 → the round walk.** AT MOST ONE entity per call: latch-skip → quantum → gate → dispatch.

So a field named "paused" reads backwards, which is why it is `mode0x4_` in the port. **Who raises
+4 to nonzero is NOT established** — the DB records writes only at 0x5a936e (`= 1`, inside
`meth_0x5a9330`'s `param_1 == 1` path) and 0x5a93ad (`= param_1`), and the only caller of
`meth_0x5a9330` found by xref is 0x5a957f, which passes **0**. The port therefore starts in
free-run mode and nothing in it raises the mode on its own. `+14` likewise has two zero-stores
(0x5a9371/0x5a93b0) and one read (0x5a9575) — no nonzero writer at all. Inventing a round timer, an
initiative order or an AP spend to make those fields do something is out of scope by the row's own
rule; they are open questions, not TODOs with a shape I guessed.

`meth_0x5a9330(mode)` also walks the GLOBAL entity list `byte_70BE0C` calling each entity's
`meth_0x548ee0` / `meth_0x548f30` behind a per-entity flag (0x5a9354 / 0x5a9393). Those hooks are
**not ported**, so `SetRoundMode()` stores +4 and clears +14 and nothing more.

## The round walk, line by line (0x5a9514-0x5a95de)

> **DEAD IN SHIPPED — pinned (T-010 10e, closed `045a7ec`; suite + driver asserts landed by T-051).**
> Everything below is the `+4 != 0` branch, and `+4` (mode) plus the `+14` (rearm) that gates its sole
> writer have **NO nonzero producer anywhere in the image** — `meth_0x5a9330` is called exactly once
> (`0x5a957f`) behind `[this+14]!=0`. The shipped demo only ever runs the free-run loop (`0x5a9525`).
> The port mirrors the branch for completeness; it is reachable in TESTS only, via explicit
> `SetRoundMode(1)` calls, each annotated as test-only, and `sim-driver-verify` audits that the
> driver never arms it. Consequence for the rotation questions: even a nested `meth_0x5a94a0`
> re-entry cannot rotate anything in the shipped configuration — **T-050's Branch 2 (writer-study
> reopen, no probe) is the strong prior**. The observation outranks this banner: if a T-034 capture
> ever shows a live nonzero `+4`/`+14` producer, THIS line is what gets rewritten.

| EA | what it does | port |
|---|---|---|
| 0x5a94a3-0x5a9511 | maintenance runs only when +8 == 0; state-5 nodes get TurnPrep `vft+96` (0x549090: `+0x300 = dt`, **clears +0x319** at 0x549094), any other state UNLINKS its node; then the iterator is refreshed | already ported, unchanged |
| 0x5a9517 | `v9 = 0` initialised BEFORE the branch | matters — see the reset below |
| 0x5a955d | `if (*(entity + 793))` — the +0x319 acted latch | `e->actedLatch0x319` |
| 0x5a9567 | latch set ⇒ **`*(entity + 668) = 0`**: the +0x29C deadline is cleared | the explicit-time domain: the round clock is not consulted for a skipped event |
| 0x5a9572 | advance the iterator | the ONLY iterator advance in the function |
| 0x5a9575-0x5a957f | `if (+14)` ⇒ `SetRoundMode(0)` and return — and this is the one return that does **not** clear +8 | so the world's step-5 loop stays suppressed after a mode switch |
| 0x5a95aa | `v9 = 1000 * (world server read) + this->mbr_0x10` — the quantum | `Tick(dtMs, secondsPerRound)`, added **once per dispatched event** |
| 0x5a95ac | `this->mbr_0x10 = v9` — unconditional after the branch | **the skipped-actor path therefore writes accMs = 0**: skipping an actor that already acted RESETS the round clock. Mirrored, not "fixed" |
| 0x5a95c1 | gate `*(entity+484) == 0 \|\| (v = *(entity+788)) != 0` — +0x1E4 and **T-039's +0x314 count** | `inMap0x1E4 == 0 \|\| QueuedActionCount() != 0` |
| 0x5a95ce | `*(entity+668) = accMs / 1000` | deadline armed in SECONDS from the round accumulator |
| 0x5a95de | `F3::ServerUpdate(entity, dt + accMs)` | `F3::TickEntityServerUpdate(e, dtMs + accMs)` — the one place the two clock domains meet |

The consequence for a port that reads +0x314 as "busy" (it is not — see
`docs/f3/EntityActionQueueSeam.md`): a queued order **forces** the turn even when +0x1E4 says
**T-041 closed the +0x1E4 half of that predicate**: it is the map-membership flag (`inMap0x1E4`,
writer `Entity::SetInMap_0x551ba0`, driven by the `ENGINE_ID_ENTITY_ADDTOMAP`/`REMOVEFROMMAP`
packet pair) — "not placed in a map, or has an order queued". Evidence and the full writer set:
`tasks/T-041-entity-flag0x1e4-semantics.md` Result section.
otherwise. That is a dispatch-behaviour difference, not a naming nicety.

## Why the walk does not rotate in the port yet, and why that is not "unfinished"

The iterator advances only at 0x5a9572, behind a +0x319 that has to survive this call — but the same
call's maintenance pass clears +0x319 at 0x549094 *before* 0x5a955d tests it. So inside one
non-re-entrant call the head actor is always dispatched, never skipped. The advance is therefore a
property of **re-entrancy**: +8 is the re-entry latch (that is what 0x5a94a3 tests), and the pump
raises +0x319 for an entity whose action deferred (`0x54c833`/`0x54c864` in
`Entity::virt_meth_0x54c570`). A second `meth_0x5a94a0` entered from inside a dispatch skips
maintenance, sees the latch, and moves on.

Our port's action pump does not raise +0x319 (nothing in `src/F3` writes it), so the walk is stable
at the head. **I did not add a round-robin to make it move**: a rotation that cites no store changes
WHO acts, which is precisely the fact `turnOrder=` in the digest exists to compare. The test in the
PathManager suite asserts the stable head so that if someone adds a rotation without the store, it
fails loudly instead of looking like progress. Open question, routed: which shipped writer raises
+0x319 on the combat path, and is the re-entrant `meth_0x5a94a0` reachable from a node's own
ServerUpdate (the +8 test says the authors expected it).

**+0x319 raiser, deverified from the decompilation (coordinator, this wave).** The DB usage list is
`0x549094` (clear, TurnPrep), `0x54c833`/`0x54c864` (the two stores), `0x54c83a` (load paired with
the stores), `0x5515ff` (ctor zero) — plus the manager's test at `0x5a955d`, which reaches the byte
by raw offset. The stores are in **`Entity::virt_meth_0x54c570`**, which
decompiles as the entity's own per-frame update: it walks the SECOND deque (`+0x2E0`, head `+0x2EC`,
count `+0x2F0` — the animated-event queue per T-039's structural note, distinct from the `+0x304`
action deque), and:
- `0x54c833` sets the latch when an event's remaining time falls below `event+0xC` — the event
did not finish this frame;
- `0x54c864` sets it when `+0x2E0.flag` is clear, the event list is EXHAUSTED, and the dt budget at
`+0x179` is still nonzero — i.e. "the pump stopped early".
Both branches also touch the sibling byte at `+0x2E0+0x34` (`0x54c85d/0x54c86d`). `+0x319` is
therefore the **entity's per-frame "more work pending" latch raised by its own update**, consumed by
the manager test at `0x5a955d` and cleared by TurnPrep only for SPAWNED (state-5) nodes.

**Consequence for the port — an unregistered deviation:** our `SpawnedEntity` has
`actedLatch0x319` with NO writer (only the ctor zero and the digest read), so the shipped raise
never happens and the walk is permanently head-stable. The port's `OnServerUpdate` path must set the
latch under the shipped two conditions above; that is port work, not a shipped question, and the
shipped writer set IS now closed. The only remaining shipped-side question is whether the manager's
`meth_0x5a94a0` is also re-entered from inside a dispatch (the +8 test suggests the authors expected
it) — that is a lockstep observation, not more statics.

**+8 reentry semantics, decompile-confirmed this wave (`0x5a94a0` full read):** `mbr_0x8` is set at
`0x5a94ab` only inside the `if (!mbr_0x8)` block, and is cleared by `0x5a9551` — which is the
SHARED epilogue reached by every exit except the `SetRoundMode(0)` return at `0x5a9586`. So
normal dispatches RELEASE the guard before returning (the port's `maintaining_` matches), and the
single documented path that leaves +8 set is exactly the mode-switch return (ported at
`CombatManager.cpp:180`). A nested `meth_0x5a94a0` from inside a dispatch therefore skips
maintenance (no TurnPrep, no unlink, no latch-clear) and proceeds to test the head's latch —
which the inner entity update may have raised at `0x54c833` — and ADVANCE the iterator without the
non-reentrant sequence ever allowing it. That is the full rotation mechanism minus one fact: the
re-entering call site. Whether the entity's dispatch (`F3::ServerUpdate` at `0x5a95de`, via
`vft+0` of the world-server getter chain at `0x5a9596`) re-enters the manager is the routed
lockstep observation; every other link is now cited.

**Owner of +8: MANAGER, confirmed two ways after a coordinator retraction scare (same wave).** I
sent a message "correcting" +8 to an ENTITY byte; that correction was wrong and I reverted it from
the doc the same hour. The base is settled by the DB, not by decompiler prose: `cls_0x5a9330` (size
36, the manager) registers `meth_0x5a94a0` as its own method, and `mbr_0x8` (size 1) has its usage
list closed at exactly `0x5a94a3` test / `0x5a94ab` set / `0x5a9552` clear. Entity cannot own it —
the whole walk indexes `this+8/12/16/20/28/32`, which are the manager's node/cursor/count fields.
So the port's `maintaining_` models the shipped field, and T-048 must NOT invent an entity-side
reentry byte. The mechanism story is unchanged: the nested `meth_0x5a94a0` skips maintenance,
sees the latch the entity's own update raised, and advances.

**FINAL GATE RESULT (T-010 10e, `045a7ec`): the walk branch is DEAD CODE in the shipped demo.**
`meth_0x5a9330` — the only writer of `+0x4` — is called exactly once in `.text` (`0x5a957f`), and that
call is gated by `[this+14]!=0`, which has NO nonzero producer anywhere (three encoding families of
byte/dword stores + lea producers, each self-tested against known positives). `+4` is therefore
permanently 0 on every reachable path: the consumer test at `0x5a951d` always takes the free-run
queue loop, the skip/advance machinery (0x5a955d/0x5a9567/0x5a9572) and the entity-list rearm sweep
(join/leave helpers `cls_0x548ee0`/`cls_0x548f30`, otherwise uncalled) never execute. Rotation is
thus not "unobserved" — it is UNREACHABLE in the shipping configuration. The mechanism analysis
above stays valid as the description of the dormant branch (it is still what a live arming door
would do, and what our tests exercise through explicit `SetRoundMode`); the digest/parity question
collapses to: does the ORIGINAL ever print a second `meth_0x5a94a0` entry at all — and if it
rotates anyway, this finding is falsified and gets retracted from the evidence, not argued.

**Process note for the next reader of this page (and for the base-claim rule, harness item 31's
third instance):** my "confirmed two ways" was ONE way plus a guess. The DB evidence
(class registration + member usage closure) is decisive; the second "confirmation" I cited —
`F3::ServerUpdate(this: *(this->mbr_0xc + 8), ...)` at 0x5a953b — proves nothing about the base of
the guard byte: `mbr_0xc` is the walk cursor and `+8` there is that NODE's entity pointer, not
`entity+8`. Do not reuse that argument; the claim stands on the DB alone, which is why it survived
being asserted in three directions inside one hour.

## Join, flush, and the order that makes them safe

* `Entity::JoinCombat` **0x5490b0**: throws `"Invalid attempt to retrieve world server instance."`
  (0x5490d7) unless the AreaMapIcon guard bit is set; resolves the manager at 0x5490f4
  (`(*(world[1].vft + 32))(world+1)`), then `AddToList`. Xrefs: **only** 0x401229
  (`GameActionAttack::OnCreate`) and 0x4033d4/0x40359b (`GameActionAttack::meth_0x403320`) — join is
  an attack property, not a phase. Ported into `GameActionAttack::Start`, not the ctor.
* `Combat::AddToList` **0x5a9810**: walks the list first and **returns on a match** — no insert AND
  no state store — so a second attack can neither duplicate a node nor re-state its actor. On a miss
  it inserts, calls `cls_0x401b00::meth_0x401dd0(manager+0x18, 1)` (the deferred GUI refresh,
  **cited and absent** in the port like the enqueue seam's `meth_0x54ef30`), then
  `meth_0x5507f0(entity, 5)` = ChangeState(5) at 0x5a987b.
* `Combat::Flush` **0x5a9440**: per node — unlink, free, `--count`, then ChangeState(**4**) at
  0x5a9485. Reached from 0x5a95f0 and the destructor 0x5a9a10, so the port's `~CombatManager()`
  flushes too, and additionally unregisters itself: a stack manager going out of scope must not be
  the pointer the join seam resolves to.
* `Entity::ChangeState` **0x5507f0**: legal exits `0→{1,3}`, `1,3→4`, `2→8`, `4→{5,6,2}`,
  `5→{4,6,2}`, `6→{7,2}`, `7→2`; an illegal pair THROWS
  `"Invalid state change in Entity::ChangeState."` (seven sites, 0x550820…0x550934). The port logs
  the same string and stores nothing, and `JoinCombat` **removes the node it just inserted** when the
  store is refused — a node in the list behind an entity that never entered rounds is the half-join
  the atomicity exists to prevent. After a legal store the original calls `vft[+27]` then
  `meth_0x54ef30`/`meth_0x54bf00` (cited and absent, as above).

## Port-only seam, because of a link fact

`GameActionAttack.cpp` is compiled into the ActionMoveTo and SpawnMove suites, whose source lists do
**not** include `CombatManager.cpp`. A direct `manager->JoinCombat( e )` there is an
`undefined reference` in two unrelated suites — measured, both went red. So the join is reached
through a function pointer stored in `MapEntitySpawn.cpp` (the one game-layer TU in every such
suite), installed by `CombatManager`'s constructor, and `JoinCombatThroughSeam()` returns 0 when no
manager exists. A null seam is **logged** at the call site
(`"GameActionAttack::Start: no combat manager - actor did not enter rounds"`), never reported as a
join. That is harness item 22's rule paying rent inside a build graph.

## Double-tick

The world's step-5 loop skips state-5 entities at 0x545e8e, which is the mechanism that keeps a
combatant from being pumped twice; `CombatMidTurn()` (+8) already mirrors the outer gate at
0x545e23. The port therefore did NOT bypass the interlock — `PumpEntity( e, ms, combatantDispatch )`
is one body with one flag, because a second copy of the pump would drift, and the drift would read
as combat behaviour. `TickWorldForEntity` keeps its old state-5 skip, so the suites that were written
against it are unchanged.
