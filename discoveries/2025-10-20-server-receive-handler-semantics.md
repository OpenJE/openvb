# 2025-10-20 — Server receive-handler semantics; Entity+0x15C tag READERS found (session 11)

Target: ORIGINAL_REFERENCE. Read-only. Decodes the 23 handler blocks of
`F3::GameServer::virt_meth_0x46dd70` (from the full decompile; tables from session 10 §5).
**Closes the oldest open queue item** ("F3-side readers of the +0x15C classId tag — DB sees
none; possibly write-only").

## 1. id → meaning table (confidence per evidence shape)

| id | semantics | evidence |
|----|-----------|----------|
| 11 | touch/wake entity (`Entity::meth_0x54c440`) + build msg-type 0 → attach | 0x46ddcc |
| 58 | explicit passthrough to JE control dispatcher + sub_46CF70 | 0x46eaac |
| 72, 74, 75, 76, 77, 80 | **NO-OP in the shipped demo**: read 2 int fields, resolve 2 entities, discard (blocks 0x46dfdc / 0x46e201 / 0x46e010) | shape identical, no side-effect call at all |
| 73 | **Initiate dialogue**: gate `!client.loginState(55bdb0)` + actor&target exist → `new 0x24; GameActionInitiateDialogue::ctor_0x404b40`, store target ptr at action+0x64-ish (`*v26->gap19 = v24`), flag byte set, then `Entity::AddAction` (0x550720) | 0x46df38 |
| 78, 79 | **Operate/open target — dispatches ON `*(target + 348)` = Entity+0x15C classId**: tag==3 → RTTI-cast GameDoor → `new 0x28 GameActionOperateDoor`; tag==4 \|\| tag==1 → `new 0x24 GameActionOperateContainer`; attach via AddAction | 0x46e044(78), 0x46e114? — see note | 
| 81 | **Talk/GetInfo script**: entity→`JE::Entity::GetScript_54aef0`; if script && script->vft[+16]("GetInfo")==1 → script->vft[+24]("GetInfo", arg) | 0x46e235 |
| 82 | **Pick up item**: build action obj via msg-factory type 3 → `GameActionPickUp::meth_0x406a30(action, target)` → AddAction | 0x46e2e5 |
| 83 | **Give item**: actor+target+int → `new 0x30; GameActionGive::ctor_0x4052a0` + `meth_0x4052d0(...)` → AddAction | 0x46e34d |
| 84 | creature poke (RTTI GameCreature → cls_0x409da0::meth) + **server responds with id 84 packet** | 0x46e4dc |
| 89 | state toggle: msg-factory type 11, fields (+32 = int, +28 = (int==2)+2) → AddAction | 0x46e827 |
| 90 | **Reload**: RTTI GameWeapon + RTTI GameAmmo (+int) → GameActionReload construction path | 0x46e567 |
| 91 | **Attack**: payload 3×int + bool + optional 3 (sub_45E560 reads) → actor/target entities → `GameActionAttack` (v19) build + AddAction | 0x46de30 |
| 95 | **Inventory move**: 4 ints; inventory container at `entity+360` (cls_0x46a8e0), `meth_0x55fea0(inv, item)`; on fail → GameServer::meth_0x46d110 broadcast, else msg-type 9 {item, slot, 30, 1} | 0x46e69c |
| 96 | **Inventory drop/remove**: mirror of 95 (msg-type 10) → meth_0x46d260 broadcast | 0x46e788 |
| 100 | **Weapon query → server reply**: RTTI GameWeapon → cls_0x4134a0::meth; server sends **id 100** packet (LABEL_70 → GameServer::meth_0x5a0a00 send) | 0x46e3f2 |
| 101 | same with int param (weapon set-mode?) → server replies **id 101** | 0x46e460 |
| 106/107 | read 0x104-byte string + 2 ints → discard (demo stubs) | 0x46e8b5 |
| 110 | **WEATHER: toggle `demo_rain.veg`** — VEG instance cached at GameServer+0x70F0, attached instance at +0x70F4 (sub_573270/sub_573430 = VEG attach/detach on current map entity) | 0x46e8de |
| 111 | **WEATHER: `demo_snow.veg`** (+0x70F8/+0x70FC) | 0x46e97e |
| 112 | **WEATHER: `demo_wind.veg`** (+0x7100/+0x7104) | 0x46ea1c |

Notes:
- `Entity::AddAction` = `meth_0x550720` (IDA labels it under net::critical_section — mislabel;
  `this` is always the actor Entity* after vb-adjust). This IS the queue seam the reimpl's
  "world singleton queue" models. ✓
- `sub_401130` = **third interface of the GameWorld singleton** (raw: `global + *(vbtable at
  GameWorld+0x10 → 0x388) + 8`, i.e. the JE::World-side vb surface, distinct from
  GetWorldServerInstance's +4). `(*(obj->vft + 20))(type)` = make typed message (ids seen:
  0, 3=pickup-arg, 9, 10, 11). So the message pool lives on the engine JE::World interface,
  not a separate factory class.

## 2. ★ Entity+0x15C tag: readers CONFIRMED (ids 78/79 handlers)

`*(target + 348)` where 348 = 0x15C is the tag slot; the handler compares 3 (door) vs
4/1 (container/creature) to pick the GameActionOperate* class — i.e. the "classId tag" from
full-class-geometry §3 is the **engine-side fast type dispatch for Use/Open requests**. The
RTTI casts follow the tag (tag is a hint; RTTI is the trust-check). Not debug-only after all.
Also updates the write-only hypothesis: dead. (DB missed these readers because the access is
`*(v + 348)` through the Entity* — no named member; the f3 DB usage list was incomplete here.)

## 3. Demo-relevant consequence list for the reimpl

- The client command surface 72..112 is **mostly inert** in the demo build (six ids are
  literal no-ops, two are stub reads). The live set: 73 dialogue, 78/79 operate, 81 talk,
  82 pick up, 83 give, 90 reload, 91 attack, 95/96 inventory, 100/101 weapon queries,
  110/111/112 weather toggles.
- Weather: the demo's rain/snow/wind ARE driven by these server-side packets —
  `GameServer+0x70F0..0x7104` = six VEG instance slots — nice observable for run-compare
  (visible effect from a packet!).
- Action object sizes now pinned by ctor sites: InitiateDialogue 0x24, OperateDoor 0x28,
  Give 0x30 (ctor 0x4052a0), PickUp via factory-3 (see §1 note).

## 4. Queue update

1. `sub_401130` factory class + typed-message pool (types 0/3/9/10/11) — next natural decode.
2. Per-sender callers (client UI → sender EA) still open (which action builds which id).
3. Older open items: clip-name table, damage Apply body remainder, JE::World layout,
   GENT f1..f6 consumers.
4. ✅ CLOSED: Entity+0x15C readers (this doc §2).
