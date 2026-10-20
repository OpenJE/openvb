# 2025-10-20 — GameActionType enum + CreateGameAction factory + GameClient receive table (session 31)

Target: ORIGINAL_REFERENCE. Read-only, native IDA + raw tables. Delivers the action-system
master registry and the client-side wire table — the pieces the reimpl's action layer and
GameNetwork lockstep consume. Also: death-branch hunt narrowed.

## 1. `F3::GameWorld::CreateGameAction_0x46f290` — the action factory (enum values raw-derived)

`GameActionType` enum (bounds 0..0xB; 6/7 fall through to `JE::World::CreateAction`):

| type | class | new size | ctor |
|------|-------|----------|------|
| 0 | GameActionDie | 0x2C | 0x404e50 |
| 1 | GameActionIdle | 0x24 | 0x4055f0 |
| 2 | GameActionMoveTo | 0x48 | 0x4056f0 |
| 3 | GameActionPickUp | 0x30 | 0x406e70 |
| 4 | GameActionWander | 0x44 | 0x46f140 |
| 5 | GameActionWait | 0x24 | 0x407ed0 |
| 6,7 | (engine JE::World::CreateAction) | — | — |
| 8 | GameActionFloatingSpeechText | 0x34 | 0x405240 |
| 9 | GameActionEquip | 0x30 | 0x405060 |
| 10 | GameActionUnequip | 0x28 | 0x407af0 |
| 11 | GameActionSetActiveWeapon | 0x44 | 0x4075d0 |

⚠ NOT here (direct-`new` sites, session 21/12): Attack 0xC8 (3 creators), Give 0x30,
OperateDoor 0x28, OperateContainer (ctor 0x406a10?), InitiateDialogue 0x24, Reload 0x3C,
UseSkill — the factory covers AMBIENT/AI actions; command actions are constructed at their
issue sites. The enum values match the reimpl's existing MoveTo wiring (they used the same
numbering for ActionMoveTo in openje ✓ cross-validated).

## 2. `F3::GameClient::recv_packet_461a10` — the client game-packet table

Pre-checks ids 41/42 (log "Received GAME_ID_ITEM_EQUIP/UNEQUIP" — NOTE: logged pre-switch at
41/42, actual switch cases are 89/96 → those two log labels are mis-numbered or legacy), then
gate `client-vft[+204]` ("should process?") and switch on packet+0x30:

| id | handler |
|----|---------|
| 50 | ENGINE_ID_ITEM_REMOVE: inventory-of(entity)+8 slot vft+4(remove item) |
| 84,100,101 | shared tail → `client-vft[+120]` interface-refresh (after case-specific: '`'=96 & '\\' =92 do inventory add/remove w/ find-inventory `entity+360` cls_0x46a8e0, `inv->vft[+56](item,count,1)` add / `vft[+60]` remove) |
| 85 | **GAME_ID_ENTITY_DAMAGE** (log-confirmed name): resolve target → if GameEntity: itoa(amount) → `GameClient::meth_0x587a70` (floating text/overlay) — damage NOTIFIES the client visually |
| 86 | resolve → `*(GE+0x24)=val0; *(GE+0x28)=val1` (dword-pair write ≈ cur+? — companion of 85) |
| 87 | resolve → `*(entity+0x08)=val` |
| 88 | **ENTITY_SETCURRENTHP** → `0x4629c0` (the clamped SetCurHP ✓ same chokepoint both sides) |
| 89 | (with 84/100/101 tail) |
| 92/96 | inventory remove/add paths (see above) |
| 97/98/99 | GameJournal: `sub_46C7A0/46C840/46C6C0(id, str)` (journal quest/entry add-close variants) |
| 102/103 | GAME_ID_WEAPON_SET_PRIMARY/SECONDARY_AMMO (log-confirmed) → weapon 0x412c10/0x412c70 |
| 104 | **INTERFACE_HIDE** → sub_45E6C0 |
| 105 | string packet → sub_461960 (music? "PlayMusic"-reply) |
| 108 | container-open: `client+160 → +52` = operate-container UI object; populates (str, id); `client+264=1` |
| 109 | debug overlay print (text + color bytes) |
| default | `JE::Client::virt_meth_0x58c660` (engine packets) |

⇒ Server send-sides (session 11) ↔ client recv match: {72..84 action broadcasts? forwarded},
85/86/88 HP-damage trio, 92/96 inventory, 97-99 journal, 102/103 ammo, 104 UI, 108 container.

## 3. Death branch — RESOLVED (same session, GameActionDie::OnStart 0x404e90)

`GameActionDie` (from `JE::ActionDie` base) on server-start:
```
base::OnStart; action.repeat=false; RTTI entity→GameEntity (else done)
entity hooks: 0x54BD90 (movement/anim cancel), 0x548F90 (engine die/disable)
switch (die-cause this+0x19):
  0: sub(+0x20)==4 → QueueAnimEvent(45) else (46)
  1: 44 ;  2|3: 47 (+variant) ;  5: +0x20==1→48, ==3→51, else 52
  default: QueueAnimEvent(17|18 by +0x25) + QueueAnimEvent(43)
duration = now + QueueAnimEvent-return (timeout branch support)
```
⇒ **CONFIRMED (hypothesis from sessions 8/13/24): QueueAnimEvent cases 43..52 = the DEATH
anim family, and the sticky GE+0x9C lock they set = the DYING lock** — dead entities refuse
further anim queueing until destruction.

Remaining death-thread hop: WHO creates the Die action (callers of world-vb `vft+20`
CreateGameAction with type 0). Candidates: effect-Apply tail or entity Update when cur≤0 —
SetCurHP (0x4629c0) itself doesn't check. Static: vft+20 call-site census next round;
runtime: one breakpoint on CreateGameAction names it instantly (reimpl offer stands).

## 4. Queue update

1. CreateGameAction callers census (last death hop).
2. Death/attack anim handler bodies: sub_464940 (43..52), sub_463c80 (53..104) → per-id
   corpse & attack anim semantics.
3. GameClient+0xA0 container-UI owner; +0x108 flag.
4. Standing: reimpl runtime trio (prefix dump, +0x2C/0x34 breakpoints, CreateGameAction
   log); native bodies on demand; quest-stub 0x420990.

## 5. SAME-SESSION ADDENDA (round 2 of this doc)

a) **CORRECTION to sessions 11/12**: the "typed message factory" I called
   `world3rdIntf vft[+20](type)` IS `CreateGameAction` (thunk 0x46FDB0 = slot 5 of the
   0x668494 vb table, verified 0x6684a8). So "msg types 0/3/9/10/11" = GameActionType
   {…0=Die, 3=PickUp, 9=Equip, 10=Unequip, 11=SetActiveWeapon} — every server handler I
   described as "building a typed message" is building an ACTION and AddAction'ing it
   (case-82 pickup type-3 is the smoking gun). The 0x34-byte heap objects near those sites
   are per-action param blobs (or FloatText 0x34 — see (c)).

b) **Apply HP mechanics pinned raw** (0x45c1a3..): victim curHP = `[[eff+0x48]+0x24]`;
   Apply computes `cur - dmg`, calls the SetCurHP chokepoint 0x4629c0 (clamps to max),
   then (gated on attacker/existence) does the floater-angle math and sends id 85
   (GAME_ID_ENTITY_DAMAGE). No ≤0 check anywhere in Apply's remaining body (checked
   through 0x45c684).

c) **Death-trigger final scope**: not Apply, not SetCurHP, not client recv-88 → the
   Die-action creator is engine-side (JE::Entity state machinery — candidates: UpdateActive
   states per reimpl doc / ChangeState path). Cheapest close: reimpl breaks on
   `CreateGameAction_virt_0x46fdb0` and logs (type, ret-addr) — one hit names it.
   Static follow-up if needed: vft-slot-5 call-site scan restricted to [obj] derefs of the
   0x668494 view (lower yield).

