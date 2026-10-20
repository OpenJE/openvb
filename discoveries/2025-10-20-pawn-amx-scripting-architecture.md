# 2025-10-20 — SCRIPTING IDENTIFIED: Pawn/AMX VM + full native API (session 29) ★

Target: ORIGINAL_REFERENCE. Read-only, native IDA + raw dumps. This is the big one the
combat/script threads were circling: **the script engine is a Stock Pawn AMX VM** and its
entire game-facing API is extractable static data.

## 1. Script_Small = AMX exec wrapper (0xE0 bytes) — `Load_0x5a8880` anatomy

```
FileStreamOpen(scriptName) ; resource-system file, any ext (script resources)
read 52-byte header → size; malloc(size) → +0xD0 (bytecode image)
memset interp struct this+0x26 (0x98 bytes = AMX)
header flags: 0x10 → debugging hook (JE::Script_Small::Debugger registered via amx_SetCallback)
              0x02 → alt callback sub_5A5860
amx_Init(sub_5CC810) — fail msg "was unable to initialize the AMX."
amx_SetCallback(...,"printf" magic 0x46... , &this+0xD4) ; also 1179407181/1413567571 ids
amx_Register tables (sub_5CCDE0):
   1) &off_7004C0            — PAWN FLOAT LIBRARY (float*, 9 natives)
   2) &small_script_functions — ★ THE GAME API (45 natives)
   3) failure ⇒ "Unable to locate a matching function for a Native in: %s"
Call entry: vft+16 → amx_Exec(sub_5CCA80) "call by name"; vft+20 → get global slot
```
Engine-naming note: "Script_Small/Script_Big" = *small* (32-bit cells) vs *big* Pawn cell
models — this is literally SourcePawn-family nomenclature; the reimpl's openje docs already
had `Scripting` as a subsystem singleton (4 bytes, dtor/ctor only).

## 2. THE GAME API — `small_script_functions` @0x6F3750 (45 natives, full dump)

| native | fn | | native | fn |
|---|---|---|---|---|
| A_Attack | 0x459eb0 | | GetPerkRank | 0x45ab50 |
| A_JustAttack | 0x459f70 | | GetLevel | 0x45abe0 |
| StartDialogue | 0x45a040 | | GetPrimaryAttribute | 0x45ac70 |
| DisplayStringRef | 0x45a410 | | GetDerivedAttribute | 0x45ad00 |
| HideInterfaces | 0x45a100 | | GetBaseSkill | 0x45ad90 |
| SetMoveMode | 0x45a140 | | GetTotalSkill | 0x45ae20 |
| EquipItem | 0x45a580 | | GetRace | 0x45aeb0 |
| UnequipItem | 0x45a6f0 | | GetGender | 0x45af40 |
| LockDoor | 0x45a1b0 | | GetEthnicity | 0x45afd0 |
| UnlockDoor | 0x45a240 | | GetSubRace | 0x45b070 |
| AddCNPCToParty | 0x45a2d0 | | GetMaxHP | 0x45b110 |
| A_ReloadWeapon | 0x45a380 | | GetCurrentHP | 0x45b1b0 |
| Protagonist | 0x45a960 | | SetHP | 0x45b2e0 |
| GetWeaponCombo | 0x45a9f0 | | AddHP | 0x45b390 |
| SetWeaponCombo | 0x45aa40 | | GetWeaponRange | 0x45b620 |
| GetSquadAttitude | 0x45aa90 | | GetItemType | 0x45b240 |
| GetTeamAttitude | 0x45aaf0 | | GetUsableObjectType | 0x45b440 |
| IsLocked | 0x45b4e0 | | IsItemEquipped | 0x45b750 |
| GetLockDC | 0x45b570 | | PlayMusic | 0x45b9c0 |
| OpenTask | 0x45b900 | | CloseTask | 0x45b940 |
| AddEntry | 0x45b980 | | OpenQuest/CloseQuest/AddQuestEntry/AddJournalEntry | all → 0x420990 (stub aliasing!) |

Float library @0x7004C0: `float floatstr floatmul floatdiv floatadd floatsub floatfract
floatround floatcmp` — stock Pawn float.inc natives.

⚠ Note: the four quest/journal natives share ONE implementation 0x420990 — either an
aliasing table bug or deliberate; check semantics before porting.

## 3. Immediate resolutions this unlocks

- **HP location FINAL**: `AddHP` (0x45b390) → resolve entity-id arg (sub_59AEF0 helpers are
  the standard "check params / resolve ref" pair) → RTTI GameEntity →
  `meth_0x46ee10(*(Entity-view+0x1E0 /*cur HP*/) + delta)`. ⇒ **Entity+0x1E0 = current HP,
  +0x1E4 = max HP** — naming sessions 16 §2 wire fields 8/9 and closing the
  "victim handler" thread from the script side (scripts mutate HP via these natives;
  combat effects' Apply tail presumably ends in the same 0x46ee10 or ChangeState — one
  decompile away).
- **`A_Attack`/`A_JustAttack`** (0x459eb0/0x459f70) = scripted combat entry — the "who
  fills damage outside the attack path" answer: these natives build/queue GameActionAttack
  (verify bodies when porting quests/combat scripts).
- `Protagonist` native (0x45a960) — likely the class-10/GamePlayer promotion path asked
  about in sessions 9/12 (script marks PC — check whether it's the flag≠0 spawn source!).
- Native param convention: (amx*, params*) with `sub_59AF40(fnname,type-checks)` prologue —
  openje's Scripting surface can standardize on it.

## 4. Method-slot map (Script_Small vft 0x6A539C, 12 slots)

```
[0]0x5a78e0 [1]Load 0x5a8880 [2]0x5a72f0(reset?) [3]ExportVars 0x5a4a80
[4]CallByName 0x5a4390 (=amx_Exec wrapper) [5]0x5a5a90 [6]0x5a58c0 [7]0x5a43c0
[8]0x5a4450 [9]0x5a44b0 [10]0x5a5600 [11]0x5a73c0   (+dtor 0x5a75f0, ctor 0x5a7240)
```
GetScript (session 28) uses [1]bind [3]export [4]call — matches.

## 5. NATIVE BODIES — first two decoded immediately (closes two threads)

**`Protagonist()` (0x45a960)** → world 3rd-intf vft[+104] → RTTI `Party`→**`GameParty`** →
`protagonist = *(party+0x14)` (entry ptr; null ⇒ "Protagonist is NULL. Returning INVALID_ID")
→ returns `*(entry + 476)` = **protagonist entity's netId**. ⇒ GameParty owns the PC
reference at +0x14; the class-10 story completes: whatever code sets party+0x14 (party/join
flow) is the *gameplay* player origin. (docs/f3/GameParty.md can now be titled: size +0x18+,
field +0x14 = protagonist entry with netId at entry+0x1DC.)

**`Attack(actorId,targetId)` (0x459eb0)** → resolve both refs → `new GameActionAttack(0xC8)`
→ ctor_0x403e40 → `meth_0x403fa0(action, target)` → **`Entity::AddAction(actor, action)`**.
Identical shape to server id-91 (0x46de30) and to the OnServerUpdate self-continuation —
⇒ **3 creation sites for GameActionAttack now pinned** (packet, native, continuation);
scripts and netcode are the same code path, so the reimpl needs one Attack-action factory.

## 6. Queue

1. Who writes `GameParty+0x14` (protagonist assignment) — likely join flow (session 9 case-0x43).
2. 0x420990 quest-journal stub disambiguation.
3. Remaining 42 natives: bodies on demand (all share the sub_59AF40/59AEF0 param-check prologue
   — cheap to batch when the reimpl ports a specific script).
4. Everything previously queued stands.
