# Session 8 — Entity lifecycle state machine + GameActionDie (death flow closed)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Closes session 7 §6 item "Entity+0x1E0 == 7 state writer"; everything below evidence-addressed.

## 1. `Entity::ChangeState` @0x5507f0 — the state machine at Entity+0x1E0

Byte/dword state `S = *(Entity+0x1E0)`. `JE::Entity::meth_0x5507f0(this, newState)` validates
every transition and throws `Common::Exception("Invalid state change in Entity::ChangeState().")`
on violation (string @0x550820 etc.):

| current S | legal newState |
|---|---|
| 0 (invalid/none) | only 1 or 3 |
| 1 or 3 | only 4 |
| 2 | only 8 |
| 4 | only 5, 6, 2 |
| 5 | only 4, 6, 2 |
| 6 | only 7, 2 |
| 7 (dead) | only 2 |

After the check: write S=newState, then `vft[27]()` (Entity-vb slot +0x6C predicate — *(inferred:
"is visible/in-world"*) → `meth_0x54ef30` (register/update world lists) else `meth_0x54bf00`
(unregister).
Related: **`Entity::SetMode` @0x550750** writes the same slot **without validation** — this is why
`GameEntity::ctor_0x4678a0` @0x46797e can stamp `5` at construction: state 5 = "created/instantiated,
pre-spawn" mode tag (see transition table: 5→4/6/2 = becomes active / destroyed).

## 2. Located lifecycle drivers (all through ChangeState)

| fn | name (by strings) | script hook → transition |
|---|---|---|
| 0x551880 | **`Entity::Create(ArgList)`** — throws `"Create() method may only be called on invalid entities."` unless S==0; logs `"+++ Entity::Create( %s ) begin/end +++"` | assigns **Entity+0x1DC = worldServer-vft[+36](idx9) reserved id**, `Entity+0x168 = vft[+176].create()` linked object (`link+4 = id`); optional name copy (`meth_0x54a460` + StringAssign to Entity+0x18C); **if no name → model "Player.amx"** + name-string compare (`cls_0x50db20::meth_0x41e4b0`) — the art-name (EEN2 token) binding point; else branch sets default bbox floats @Entity+0x3A4..0x3B8; `vft[+220]()`; server-mode: `Entity+0x16C = vft[+180]()`; **final ChangeState(1)** |
| 0x550970 | activate/on-create wrapper | RunScript **"OnCreate"** → ChangeState(4) |
| 0x550a90 | load wrapper | RunScript **"OnLoad"** → ChangeState(4) |
| 0x550a50 | destroy wrapper | RunScript **"OnDestroy"** → ChangeState(8) |
| 0x551b60 | **`Entity::Destroy()`** — throws `"Destroy() method may only be called on valid entities."` when S==0 | ChangeState(2) — (seen in action: emptied ammo stack destroyed @0x413c44, session 6 §4) |
| 0x551ca0 | **`Entity::Kill()`** *(server-gated, flag&1)* | ChangeState(6) — sole pre-death state |
| 0x5a9440 | `JE::Combat::meth` — encounter list flush | each entity → ChangeState(4) |
| 0x551b36/0x551b8e/0x551c8b | Create/Destroy internals | — |

## 3. DEATH FLOW (complete chain, was: session 7 §10)

```
damage makes IsDead true                    IsDead = (state==7 || GameEntity+0x24<=0)  [0x467a10, session 7 §3]
  → GameEffectDamage::Apply queues GameActionDie on victim      (session 7 §3.10)
GameActionDie::Execute (Entity-vb vft slot4 @0x648694 → 0x404e90):
  1. JE::ActionDie::virt_0x55fb30 → victim.Kill() = ChangeState(6)   [SERVER FLAG GATED @0x551cb0 —
     in client-mode demo the death STATE CHANGE IS A NO-OP; consistent with EFTR inertness]
  2. clear action flag mbr_0x4_bool; RTTI owner→GameEntity
  3. death animation via GameEntity::QueueAnimEvent_4673d0 per the die "category" dword:
     switch(gap19 /*+0x19*/):
       0: (mbr_0x20==4 → anim 45) else anim 46
       1: anim 44
       2,3,5: anim 47 queued, then (mbr_0x20==1 → 48; ==3 → 51; else 52)
       default: anim ((byte2(mbr_0x24)!=0)+17 → 17/18) + anim 43
     → return 0; duration stored at mbr_0x20+1 (action end-time = a2 + queue duration)
```
**QueueAnimEvent ids 43..52 are the DEATH animation set** (not "busy-locked" placeholders):
43 generic-combo, 44 category-1, 45/46 category-0 (by weapon/ammo class ==4), 47+48/51/52
categories 2/3/5, 17/18 default-fallback pair. Correction for
`2025-10-20-gcre-versions-gwam-queueanimevent-table.md` §3 naming.

The 7 (dead) state is reached via the engine's own combat manager (only legal predecessor is 6;
`JE::Combat`/server tick flips 6→7 when the action completes — F3.exe has no direct
`mov [..+1E0], 7`; only the cmp in `virt_meth_0x549030` @0x54902f). In the shipped single-player
demo the state machine effectively never leaves the ctor-set 5 *(inferred from server gating)* —
which is why the client-safe death visuals (anim queue) are driven explicitly by
GameActionDie::Execute instead of by the state change.

## 4. Net-id / state slot clarification (matters for the reimpl — FLAG FOR DEBUGGER CHECK)

Two lifecycle values live adjacent in the Entity body, both written/read through
slightly different register bases, and the DB's field names conflate them. Concrete sites:
- **State**: read `cmp dword[iface+1E0h], 7` @0x54902f (`Entity::IsDead` helper 0x549030 —
  IDA types the same slot as `Entity::mbr_0x1E0` via the DynamicObject `gap1AC[52]`).
  Written only by ChangeState @0x550945 / SetMode @0x550759 at that slot.
- **Engine netId**: written `Entity::Create` @0x55190c — decompiler text says `mbr_0x1DC`;
  Create's own precondition `if (a1->mbr_0x1e0)` @0x5518b5 reads STATE, so the two are the
  SAME slot numbers under one base only if the decomp's member offsets carry a −4 bias.
  Most-consistent single reading (matches Reload's `netId = Entity-vb-displaced dword idx 480`):
  **state @ Entity+0x1E0, engine-netId @ Entity+0x1DC, both interface-based**; the `Create`
  `mbr_0x1DC` label is then the −4-biased spelling of the value the Apply sync uses
  (`[victim-vb+0x1E0]`, i.e. Entity+0x1DC). Reimpl should pin these two with one debugger read
  after spawning (Mainmenu has 3 EME2 entities — trivial to inspect).
- F3-side `GameEntity+0x28` (ctor world-server counter read, session 5) remains a **third**
  distinct id-ish slot — already masked to 29 bits; unchanged conclusion.
- Extra corroboration for the −4-biased netId read: `ApplyEquipEffects_46ba30` @0x46bad4
  `mov eax,[edx+edi+1E0h]` (base+idx = Entity−4 form) → passes it as the effect's entity param —
  same slot as `Entity::Create`'s write @0x55190c. Consistent: **engine netId = Entity+0x1DC**.

## 5. Donation to the reimpl agent

- If you keep flattened entities: add `state: u8 @Entity+0x1E0` and `netIdEngine: u32 @Entity+0x1DC`
  as DISTINCT fields; the ctor stamp is 5, and legal transitions are §1's table. In client-mode
  parity mode you may freeze at 5 — but GameActionDie's visuals (anim 43..52) DO run in the demo,
  so wire the die-action anim queue even without the state machine.
- Death anim table (category → QueueAnimEvent id):
  `{0:{46, 45 if sub==4}, 1:{44}, 2:{47,(48|51|52)}, 3:{47,(48|51|52)}, 5:{47,(48|51|52)},
    default:{17|18, 43}}` — category byte = die.mbr_0x19 (F3 code writes die+0x1C=DT at queue
  time — see session 7 §3.10; mapping 0x19↔0x1C differs by interface base; verify with your
  own struct once GameActionDie is ported).
- `Entity::Create` model fallback **"Player.amx"** @0x551a37 when the entity has no art name —
  another client-visible default you may need for unnamed spawns.

## 6. Addendum (same session, later turn) — effect FIRING sites found; pump question closed

`JE::Entity::meth_0x5506c0` (per-effect evaluate-on-entity, session 7 §1) has exactly 6 callers:
| firing site | context | effect source |
|---|---|---|
| `GameActionAttack::meth_0x403770` @0x403906 | **attack resolution loop** over the queued damage list (consumes what `CalculateAndQueueDamage_0x404060` queued, session 6 §5!) | `factory vft[+40](1)` — a **fresh runtime GameEffectDamage per hit record**, NOT from the EFTR trees |
| `GameProjectile::DropPayload_0x412090` @0x412120 | projectile impact payload | projectile's payload effect |
| `GameActionEquip::ApplyOnCreateEffects_466fe0` @0x4670d0 | entity activation | **EFTR tree index 3** (`meth_0x463960(entity, 3)` = tree-by-index accessor) |
| `GameActionEquip::ApplyEquipEffects_46ba30` @0x46bb69, 0x46bc58 | equip/unequip (two passes) | **EFTR tree index 0** (raw: `push 0; call 0x463960` @0x46ba50) |
| `JE::EffectTreeTrigger::virt_0x5aae40` @0x5aaf60 | engine trigger node (unreachable from F3 factory, session 7 §4) | children |

Runtime damage-effect wiring @0x403851-0x403906 (per hit record R in the attack queue):
`eff = factory(1)`; `setEntity(eff, victim)` (0x55a360); `setParam(1=weapon?, 2=attackerEntity,
0=victim, 3=0)` (0x55a3e0 = indexed param setter); `0x55a420(eff,1)`; direct writes
`eff+0x9C = R.damage`, `+0xA0 = R.dtype`, `+0xA4 = R.field`, `+0xA8 = R.voice`,
`+0xAC/+0xAD = last-strike/last-record bytes (v11[172/173])`; then evaluate on victim.
**⇒ the complete combat loop**: ActionAttack computes damage (session 6 §5) → queue →
meth_0x403770 builds GameEffectDamage per record → 0x5506c0 → tree walk → vft+40 slot10 →
Apply (HP clamp + anims + VFX + OnDamage + GameActionDie) — all of it server-flag gated
@0x4037b5, confirming demo-client inertness end-to-end.

`ApplyOnCreateEffects` rules: OnCreate effects must resolve to target-type Myself/Subject
(`eff->vft[+0x10]()` returns 0..3; 0/3 = rejected with debug msg, 1/2 = evaluated on the entity
itself). Effect **prototypes from the tree are re-instantiated via the factory + `vft[+8](node)`
copy** — file trees are templates, not live objects (matches session 6b's trampoline-object model).

Also: **`meth_0x463960(entity, i)` = the GameEntity+0x38 5-tree container accessor by index i**
(tree[3]=OnCreate confirmed; the 12-byte header per tree = RbTree head+sentinel, session 4's
array finally has an API).

## 7. Still open (candidates for next session)

- `GameEntity+0x18` consumers (long-standing).
- vft[+36] (0x544a70, id reserve on 0x66856C) and vft[+176]/[+220]/[+180] bodies — engine
  dynamic-object attach; decode when the reimpl needs exact creation parity.
- Where effect-tree *roots* per entity get iterated on the pump side — **CLOSED** (see §6).
- EFTR tree index map: **0 = equip (ApplyEquipEffects), 3 = OnCreate (ApplyOnCreateEffects)**;
  remaining roots 1, 2, 4 unidentified; the projectile payload path uses the projectile's own
  effect object, not an entity tree.
- Who flips state 6→7 (server-only; no static writer in binary besides ChangeState stores —
  probably `JE::Combat` after duration expiry; low priority for single-player parity).
