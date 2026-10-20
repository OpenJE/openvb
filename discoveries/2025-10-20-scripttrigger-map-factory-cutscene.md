# 2025-10-20 — ScriptTrigger anatomy, map trigger factory (ESTR/ETTR/EBTR), cutscene dispatcher (CLIENT-side)

Target: ORIGINAL_REFERENCE. Read-only. Follows entity-script-binding + scripting-abi docs.
⚠ This doc CORRECTS entity-script-binding §4's blanket "demo never executes scripts":
**trigger scripts DO run in client mode** (the cutscene/trigger dispatcher below is gated on
`flag == 0`, i.e. the plain client path). Only `JE::Entity::RunScript` (entity scripts) and
EFDT/Condition effect scripts are server-throw.

## 1. `JE::Map` trigger factory — primary vft 0x699FA4 **slot 7 (byte +28)** = `CreateTrigger(type)`

```
type 0 → JE::TransitionTrigger      0x50 ctor 0x56E3C0   (ETTR chunk)
type 1 → JE::ScriptTrigger          0x50 ctor 0x5B2600   (ESTR chunk) ⭐
type 2 → JE::EffectTreeTrigger      0x88 ctor 0x5AAD50
type 3 → JE::EffectTreeCircleTrigger  0x8C ctor 0x5B22B0 (ring-shape family: radius in own fields)
type 4 → JE::EffectTreeSemiCircleTrigger 0x90 ctor 0x5B20D0
type 5 → JE::EffectTreeRingTrigger  0x90 ctor 0x5B1FA0
type 6 → JE::BuildingTrigger        0x38 ctor 0x5B1C30   (EBTR chunk — spec says fixed 19B)
default → "Error creating trigger. Unknown type %d.\n"
post:  obj+0x28 = (*factoryIntf + 68)()   ; = 0x46FF20 = ++World+0x88 | 0x40000000, 29-bit
       obj+0x2C = map->mbr_0xB4           ; back-ref word per map
```
⇒ **trigger ids = the 0x40000000-tagged allocator** (gameworld doc §10.1 space table now has a
confirmed consumer). The `myself & 0xE0000000` guard in time natives (ABI doc §4) excludes
these from entity resolution. *(inferred: EffectTree* triggers pair with the EFDT trees of
entities at map level — shapes for area effects.)*

## 2. `JE::ScriptTrigger` (0x50) layout + methods

vft 0x6AA644 (9 raw entries; DB-index): slot0 0x5B26B0 dtor-tail, slot2 **0x5B26D0 ReadFrom**,
slot3 0x5B23F0 / slot4 0x5B2460 (un-decoded), slot5 **0x5B24D0 Update-dispatch**.

| off | field | evidence |
|---|---|---|
| +0x00 | `JE::Trigger` base (0x25) | ctor 0x5B18A0 |
| +0x28 | trigger id (0x40-tagged) | factory §1 |
| +0x2C | map back-ref word | factory §1 |
| +0x30 | **script name** std::string (buf@+0x34, size@+0x44, cap@+0x48) | ReadFrom sub_49C8D0(&+0x30) |
| +0x4C | own `JE::Script_Small*` | ReadFrom CreateScript store |

`ReadFrom_5b26d0(res)` (chunk-driven):
1. chunk tag MUST be **`ESTR`** (u32 0x52545345; via Common::Chunk header read) else fail;
2. name = length-prefixed string → +0x30 (matches community ESTR: `{u8 len, gap, name}`);
3. **u32 varCount**, then varCount × `{string, u32}` → local map (name→initial value);
4. if name non-empty: `Script_Small* = srvView->vft[+0x24](0)` (CreateScript; **owner id = 0**
   — triggers have no "myself" entity; the extra userpurge arg is not consumed by 0x544B80);
5. `script->vft[+4](name)` Load (fail ⇒ dtor+null + return 0);
6. if varCount>0: `script->vft[+0xC](&varsMap)` = **Script_Small::SetGlobals(map)** —
   same slot as entity attach path (entity-script-binding §1); the AMX `STAT`-style init loop
   (0x5A4A80 body pending — mechanical).
Note: community ESTR tail `[1,1]` = engine's `{varCount=1? …}` serialization quirk — trust OUR
layout (step 2–3) over the editor doc when they differ *(discrepancy recorded)*.

## 3. `ScriptTrigger::Update-dispatch_0x5B24D0` — the cutscene state machine (CLIENT path)

Gate: runs only when `(serverFlag & 1) == 0` — i.e. **in the demo/client**. Requires
`[subsystem[0]->int_0x18 + 0x70E4] != 0` (the cls_0x5529e0 gate object, +28900=0x70E4 —
"cutscene subsystem present/enabled" byte adjacent to the pause fields 0x70E5/70E6/0x70E8
from entity-script-binding §3).

```
state = (*factoryIntf + 152)()   ; 0x470150 → JE::World 0x544CB0: cutscene state getter
                                  ;   (calls World+0x154-cursor object vft+140; also
                                  ;    stores into World+0x104 region — refine later)
id    = (*factoryIntf + 156)()   ; 0x470170 → 0x544CC0-ish sibling (cutscene identifier)
switch state:
  0 (none)      → runScript("OnUpdate",      args = {tickParam})        ; 1 arg
  1 (starting)  → runScript("OnCutsceneStart", args = {tickParam, id})  ; 2 args
  2 (running)   → runScript("OnCutscene",     args = {tickParam, id})
  3 (ending)    → runScript("OnCutsceneEnd",  args = {tickParam, id})
runScript = meth_0x5B23D0: `if (+0x4C) return script->vft[+0x14](name,out,&argc,argv)`
```
⇒ **the four OnCutscene*/OnUpdate names are THE client-visible scripting surface of the
demo** (any shipped `.amx` with those exports runs per trigger). `StartCutscene` native
(0x5C9520) presumably drives state 0→1 (sets gate 0x70E8 owner + flips the World+0x154
context state) — decode next.

## 4. `JE::Map` vft 0x699FA4 slot map (30 slots; F3 overrides located)

Slot 7 = CreateTrigger (§1). **F3 overrides: slot 1 = 0x46C900, slot 6 = 0x46CB30**
(GameMap TU — F3-specific map behavior; likely "on-map-load" + entity-list hooks given
neighbours in MapEntitySpawn work; bodies are the next cheap decompile).
Other engine slots (for openje naming): 0 0x57B880 (dtor family), 2..5 tiny stubs
(0x56C470/480/490/4C0), 8 0x56E850, 10 0x56E970, 11 0x56DED0, 12 0x56DF50, 13 0x56DFE0,
14 0x56C770, 15 0x56C9D0, 16 0x573B00, 17 0x575050, 18 0x573BF0, 19 0x573F30, 20 0x575A40,
21 0x56CCF0, 22 0x56CD70, 23 0x5742B0, 24 0x575180, 25 0x576940, 26 0x576C30, 27 0x578FB0,
28/29 0x56CF30/56CF50 (no-ops). `Map::meth_0x56E9E0` (map OnUpdate script, entity-script-binding
§4) sits between slots 10/11 — Map-level script host fields live in the same TU.

## 5. Questions for the reimpl agent
1. ~~Do the shipped maps contain ESTR chunks?~~ **ANSWERED BY DISCOVERY (local scan, §5a)**:
   `data/Scripts.grp` (5.8 MB!) ships **~180 OnUpdate-bearing script modules** (compiled AMX
   with debug symbol sections). So ESTR triggers + client-side scripts are NOT dead — this
   is the live quest/dialogue driver surface. Map-by-map ESTR presence still worth one line.
2. Your EME2 `record.flag` — is it adjacent to a nested ESTR? If correlated → the
   trigger↔entity link.
3. (replaces prior Q3) Scripts.grp contents (verified §5a): `[Globals]` INI-like block with
   quest globals (`02_AldcrownQuest`, `00_Know_May`…), skill-difficulty constants
   (TRIVIAL=10…EPIC=130) — the AMX debug sections make all script/global names extractable.
   Your resource layer should be able to list them (GRP type = "STR"? — you own that table).

## 5a. Scripts.grp evidence (raw, same session)
- file: `data/Scripts.grp` 5,832,974 B; header `{02, 01, 79, d403...}` (GRP versioning —
  resource-system side).
- `grep -c 'OnUpdate|Callback_OnUpdate'` = **180**; strings include per-module debug name
  tables (function/var names), `[Globals]` default-value blocks, and game config
  (SKILLDIFF*, quest vars with `NN_` prefix ordering = quest pipeline stages).
- ⇒ every `OnCutscene*`/`OnUpdate` export name in §3 is real, and `Callback_OnUpdate` shows
  up as a distinct export (secondary pump hook — decode which script owns it when
  lockstepping a trigger).

## 6. Queue (this author)
1. StartCutscene/Stop/IsIn bodies → gate-field writes (0x5C9520/5B0/610).
2. Script_Small slot 3 `0x5A4A80` (SetGlobals map init) + slot 0/2/4 bodies.
3. `cls_0x5529e0` full geometry (0x70E4..0x70E8 neighborhood + size) and its owner class.
4. F3::Map overrides 0x46C900/0x46CB30 bodies.
5. World+0x154 cutscene-context object class (cursor-chain from gameworld §12a — now it has
   a name-candidate: cutscene controller).
