# 2025-10-20 — Command record → packet issue: the client command enum (session 12)

Target: ORIGINAL_REFERENCE. Read-only. Completes the request path:
cursor/command-record → `sub_436b40` dispatcher → session-10 senders → session-11 server
handlers. Complements (and was partly anticipated by) the reimpl's
`gcre-versions-gwam-queueanimevent-table.md` §3.

## 1. `sub_436b40` is NOT a flush loop — it's `IssueCommand(record)` from the cursor layer

Structure (raw, 0x436b40..0x436f78):

```
esi = *(0x70BD90 + 0x2C)              ; the GameClient* (same slot SetAttackMode RTTI-casts
                                      ;   Client→GameClient — client lives at area-map-icons-
                                      ;   object +0x2C)
ebp = [esi+0xE0]                      ; actor entity id (the selected/PC entity)
ebx = [esi+0xDC]                      ; target entity id (cursor selection)
edi = Entity::Lookup(ebp); ebx = Entity::Lookup(ebx)     ; via 0x54be10
ecx = record arg (thiscall)
type = (record[+0x30] == 1) ? record[+0x34] : record[+0x30]   ; wrapper indirection
if (type-2 > 0xE) invalid;
jump table 0x436f78[0xF] → per-type windows; EVERY case guarded by (edi && ebx) non-null,
then: push record-field, push actorId(ebp), mov ecx,esi(GameClient), call <sender>
```

Jump table entries → senders (windows paired with `call` sites):

| command type | window | sender | packet id | server-side semantics (session 11) |
|---|---|---|---|---|
| 2 | 0x436f4f | (bounds/exit path) | — | — |
| 3 | 0x436bb2 | 0x45f4f0 | 72 | demo no-op |
| 4 | 0x436bd4 | 0x45f5d0 | 73 | initiate dialogue; PLUS side-path building a string (push 0x655178 default text; `0x587a60(client, {0x7d0,0x80,0x80,0x80}, ...)`) — dialogue UI params (2000/128/128/128 — box size?) *(inferred)* |
| 5,6,7,8 | 0x436c45/51/73/95 | 0x45f6b0/790/870/950 | 74,75,76,77 | demo no-ops |
| 9 | 0x436eb7 | 0x45fa30 | 78 | operate — door/container by Entity+0x15C |
| 10 | 0x436eed | 0x45fb10 | 79 | operate variant (open vs use?) |
| 11 | 0x436f04 | 0x45fbf0 | 80 | demo no-op |
| 12 | 0x436f1b | 0x45fcd0 | 81 | talk → script `GetInfo` |
| 13 | 0x436f32 | 0x45fdb0 | 82 | pick up item |
| 14 | 0x436f48 | 0x45fe90 | 83 | give item |
| 15 | 0x436c67 | →0x436d6f 0x45f370 | **91** | **attack**, a5 = NULL-ish triple absent path |
| 16 | 0x436e25 | →0x436e1b 0x45f370 + 0x436e47 0x45ef70 | **91 + gate** | **attack with payload triple** (`push 9; push 0; ...` = a5 record of 3 ints — the attack params: dir/height? matches GWAM sub-mode reading from reimpl doc) then menu-gate `0x45ef70` (RTTI CSMenu check — cancel if dialogue menu active?) |

Key structural facts:
- The **command record** (caller-owned, ecx) is ≥0x38 bytes: fields of interest +0x30
  (opcode/type), +0x34 (sub-type when opcode==1). Rest of fields feed `push [esp+0x14]`
  variants (per-type extra ints).
- Command types ≈ the cursor verbs; ids 3..11 mostly dead in this demo build — the LIVE
  verbs are: 4 dialogue, 9/10 operate, 12 talk, 13 pickup, 14 give, 15/16 attack.
- Reload(90), inventory 95/96, weapon 100/101, weather 110..112 senders have OTHER callers
  (UI panels): reload sender 0x460100 ← `cls_0x41a2b0::meth_0x41c790`; id-100 ←
  `cls_0x41a2b0::SetAttackMode_0x41c040` (weapon attack-mode change — matches the GWAM
  record doc); id-84 ← same TU (creature stance?). `cls_0x41a2b0` is the inventory/actions
  UI class (returns GameInventory from members, holds game_action_equip_0x88 = its subject
  action). 95/96 have `j_` thunks (0x4606f0/0x460700) = **virtual slots** — called via the
  GameClient vftable (GUI polymorphic calls).

## 2. Verified end-to-end chains (three examples, all addresses)

```
attack:  cursor record(+0x30=15) → sub_436b40 → 0x45f370 (id 91, 3+3 ints) →
         server 0x46de30 → GameActionAttack built → Entity::AddAction(0x550720) on actor
operate: record type 9 → 0x45fa30 (id 78, 2 ints) → server 0x46e044 → tag=*(target+348):
         3→GameActionOperateDoor 0x28 / 4|1→OperateContainer 0x24 → AddAction
weapon-mode: cls_0x41a2b0::SetAttackMode (UI) → 0x45ff80 (id 100, weapon entity id) →
         server 0x46e3f2 → RTTI GameWeapon → mode meth → server SENDS id 100 back (echo)
```

## 3. For the reimpl

- Your `QueueAnimEvent` roadmap item is safe to build on command-type 15/16 → id 91 as the
  only live combat entry; the "9" pushed in the type-16 path mirrors GWAM attack-mode
  record value 9 seen in effects (GameEffectDamage +0xA8=9) — probable same enum space.
- The cursor selection state to mirror: `client+0xDC = targetId`, `client+0xE0 = actorId`
  (GameClient offsets in the RTTI-cast client object) — this is where "who acts on what"
  lives between frames.
- No-ops 3..11 (except 4,9,10) can be wired as accept-and-ignore to match the original
  1:1 — do not invent behavior for them.

## 4. Cursor dispatch layer (decoded same session — closes old queue item 1)

`GameCursorManager::virt_meth_0x437d80` = **vftable slot[7] (+0x1C)** of an 11-slot table
@0x6550B4 (.rdata ptr; only ref). Command-object layout (a2):

```
+0x00 kind          (must == 2 to dispatch)
+0x04 flags.bit0    (force/confirm modifier)
+0x08 cmdClass      1 = item-use command, 2 = world/entity command
+0x0C recPtr        → the command record consumed by sub_436b40
+0x10 param2        (record extra)
+0x1C(+0x20?) subMode  0 = normal, 2 = confirmed/shift variant
```

Branches (guard: `client+0x110` must be non-null — the PC entity exists):
- class 2, subMode 0 → **`IssueCommand` = sub_436b40** (§1) — the normal click-commit
- class 2, subMode 2 → flags&1 ? sub_437030 : meth_0x436fc0 (confirm/force issuers)
- class 1, subMode 0/2 → `JE::Inventory::virt_meth_0x436230(itemSlotA, itemSlotB)` —
  **item commands bypass the packet enum** (inventory-internal ops: move/equip locally)

So the shipped input model: cursor builds a command object → this virtual routes it →
world-commands go to the packet enum, item-ops to the inventory manager.

## 5. Queue update

1. ~~Where command records are created~~ CLOSED §4 (cursor command object + routing).
   The two force/confirm issuers (sub_437030, meth_0x436fc0) still to be sampled.
2. cls_0x41a2b0 (inventory/actions UI) full member map — rich vein: it drives reload,
   weapon mode, and probably 95/96 via the vft thunks.
3. Older: clip-name table, damage Apply math, JE::World layout, msg-factory types 0/3/9/10/11.
