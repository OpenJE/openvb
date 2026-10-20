# Session 23 — combat join/leave mechanism (Entity vft +0x30 = JoinCombat) — encounter entry is the attack action

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
> ⚠ FRAMING UPDATE (session 25): the mode flag `[0x70BD90]+4 bit0` IS SET during demo
> play (connect handshake, 0x58a6d6) — the server-gated paths described here as "inert /
> client-skip" are LIVE in the shipped single-player (loopback host). Mechanism details stand.
Extends `2025-10-20-cutscene-controller-world-subsystem-roster.md` §7 (their Combat identity
round) with the join/leave machinery. No overlap: they mapped the scheduler object; this maps
who enters/leaves it and the state-5 semantics that resolve a lattice oddity both lines noted.

## 1. `JE::Entity::JoinCombat @0x5490b0` = Entity-vb vft slot +0x30 (idx12)

```
JoinCombat(entity):
  server-flag gate (throws "Invalid attempt to retrieve world server instance." in client)
  combat = (*(*worldServer(0x66856C)+32))(worldServer)     ; idx8 getter → JE::Combat (World+0x8C)
  Combat::AddToList_5a9810(this=combat, entity):
      if entity not in combat->list(+28): insert
      entity.ChangeState(5)                                 ; VALIDATED 4→5 (lattice §8 of ses.12)
      id = worldServer vft[+8](...); this->vft[+12](this,id) ; notify/self refresh
```
Callers of JoinCombat — ONLY attack actions:
- `GameActionAttack::OnCreate_0x4011d0` @0x401229 (on the action's owner),
- `GameActionAttack::meth_0x403320` @0x4033d4/0x40359b (sub-attack series; likely owner +
  per-target joins).
⇒ **There is no proactive encounter trigger in engine code: an entity enters the round system
exactly when it begins an attack action.** The "combat manager" list = set of entities with
pending/ongoing attack actions.

## 2. Leave paths

- `JE::Entity::meth_0x54fff0` (Entity detach/cleanup — the dtor-chain helper seen in session 8)
  @0x55003d calls `Combat::Remove_5a93c0(entity)` — detach on removal.
- `Combat::Flush_5a9440` (session 12's caller list) → all members ChangeState(5→4).
- Conditional `if state==5 → ChangeState(4)` sites @0x5a9410-0x5a941a and @0x551c8b — the
  "drop out of rounds" transition used by those paths.
No "round timer" member found on Combat (0x24 = list+scheduler only) — per-round AP economy is
driven by the entity-side AP regen tick (session 22) + action durations, not a combat clock.

## 3. State-5 semantics RESOLVED the lattice oddity (both lines hit it)

5 has TWO producers with the same numeric: (a) `GameEntity::ctor` raw-SetMode(5) =
"pre-registration sentinel" — never validated (Create's 0→1 check reads a DIFFERENT base: the
Create guard sees the post-ctor-zeroed slot via the interface base, see §4 caveat), and
(b) `JoinCombat` validated 4→5 = "in rounds". The pump's `if state==5: skip ServerUpdate`
(session 14) therefore pauses GENERIC ticking for combatants — they're driven by the action
pipeline instead (attack steps call ServerUpdate-relevant logic via AddAction/queues). After
Flush/leave (5→4), the entity returns to ambient ticking. This matches every observed site —
the alternative reading ("5 = spawned, never ticked") broke the OnUpdate/AP-regen behavior of
combatants, which demonstrably runs (session 22 sits in Entity::Update@+0x60 — still called
for state-5 entities by the ACTION path, not the world loop).

## 4. Caveat kept honest

The Create-guard vs ctor-5 base question (session 8 §4 "netId vs state adjacent dwords") still
wants ONE debugger read at spawn time: expected value at `[interface+1E0]` just after
`operator new + GameEntity::ctor` is 5, and just after `Entity::Create()` is 1 — if instead
Create sees 5 the two fields use different bases as session 8 §4 suspected. Everything in §1-§3
is independent of that resolution.

## 5. Reimpl action

- Model "combat" as a SET, not a phase: `combatants: Set<Entity>` + `JoinCombat()` invoked from
  attack-action create/sub-attack only; leave on detach + flush paths; pump skips generic tick
  while in the set (their pump needs the combatant branch to call update via the action path).
- The worldServer `+32` getter and `+8` notify mirror are single-liners in their GameWorld port.
