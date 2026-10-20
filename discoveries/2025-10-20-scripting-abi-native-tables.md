# 2025-10-20 — Scripting ABI: three native tables (209 functions), AMX load path, Protagonist/GameParty, quest stubs dead

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Method: IDA decompile + raw table walks (recipe: `{u32 namePtr,u32 fn}` pairs until {0,0},
strings via latin1 NUL-slice; base tables `0x6FFAF8`, `0x6F3750`, `0x7004C0`).
Continues ge94 doc §12c (Condition → `conditions.amx`) and gameworld-interfaces-effects-classid
§1.1 (srv-intf slot +0x24 = CreateScript).

## 1. Script VM architecture (JE side)

`JE::World::CreateScript(vft+0x24 of 0x66856C)` → `new JE::Script_Small (0xE0)` ctor 0x5A7240
(vft **0x6A539C**, 12 slots; arg → `+0x04` owner-id + byte `+0xD4`; SSO name `+0x08..`;
AMX core struct `+0x26..+0xBD` (0x98B); blob ptr `+0xD0`; map head `+0xD8`+count `+0xDC`).

`Script_Small::Load_5a8880(file, mode=1)` (= vft slot 1):
1. `FileStreamOpen(file)` + reads **52-byte header**, whole-image read into `mbr_0xD0` blob
   (error strings: "attempt to load NULL/unknown script", "error reading header").
2. header flag byte bit `0x10` → debugger: `sub_5CE320(&amx, Script_Small::Debugger)`,
   `mbr_0x24=1`; bit `0x2` → logger hook `sub_5CE320(&amx, 0x5A5860)`, `mbr_0x25=1`.
3. `sub_5CC810(&amx, blob)` = **amx_Init** (fail: "was unable to initialize the AMX.").
4. native registrations (`sub_5CCDE0` = **amx_Register**, `{table,-1}`):
   `0x7004C0` (float lib) → `0x6FFAF8` `small_script_functions` (engine core) →
   `0x6F3750` (F3 game lib). fail ⇒ "Unable to locate a matching function for a Native in: %s".
5. global registrations (`sub_5CCD00` = **amx_AllowGlob/DefineGlob**):
   tag `0x464C534D` "myself" ← `mbr_0x4` (owner entity id — ctor arg),
   tag `0x54415453` ← `dword_70C724 = (mbr_0x24!=0)` (debug flag),
   tag `0x45555146` ← `&mbr_0xC0` (script **function-queue** handle — backs
   QueueFunction/ClearFunctionQueue natives),
   (logger mode only) tag `0x4C425446` ← `&mbr_0xD4`.
Condition::RunScript's "myself" fetch = `sub_5CCCC0` (**amx_GetGlObal**) with tag "myself".

`Script_Small` vft 0x6A539C raw: [0]0x5A78E0 [1]0x5A8880 Load [2]0x5A72F0 (used via
`(*(this->vft+2))(this)` at Load entry — reset/unload) [3]0x5A4A80 [4]0x5A4390
[5]0x5A5A90 **Run/Execute** (Condition eval calls it at +0x14… via caller wrapper)
[6]0x5A58C0 [7]0x5A43C0 [8]0x5A4450 [9]0x5A44B0 [10]0x5A5600 [11]0x5A73C0. *(slot bodies
beyond Load not yet decoded — queue item.)*

## 2. `Protagonist()` native — the class-10 flag origin (answers ext-classid open item)

`F3::Protagonist_45a960(myselfId, ...)`:
```
v2 = GetWorldServerInstance → factory-intf (GW+0x398)
v3 = (*[v2vft + 104])()                 ; byte +104 = 0x6684FC entry 0x470010 → get World+0xF4
v4 = RTTI_cast(v3, Party → GameParty)   ; "Protagonist() could not find GameParty. Returning INVALID_ID."
v6 = *(GameParty + 0x14)                ; raw 45a9c7: mov eax,[eax+0x14] — PROTAGONIST ENTITY PTR
if (!v6) → "Protagonist() Protagonist is NULL. Returning INVALID_ID."
return *(v6 + 0x1DC)                    ; its engine netId (Entity+0x1DC, 476 dec) — NOT the ptr
```
⇒ **World+0xF4 (via factory slot +104) leads to GameParty*** (the jeworld-layout doc's
"+0xF0..0xF4 pad" reading → corrected: +0xF4 holds/leads to the Party singleton).
⇒ **GameParty+0x14 = protagonist entity pointer** (written at 0x46CD0E/0x46CD56 — GameWorld TU
party-init; scripts observe it via this native returning `Entity+0x1DC` netId).

## 3. Full native tables (verbatim — the reimpl's scripting contract)

### 3.1 float lib @0x7004C0 (9) — fixed-point float ABI
`float floatstr floatmul floatdiv floatadd floatsub floatfract floatround floatcmp`
(natives 0x5CE5F0..0x5CE7C0). Scripts pass floats as tagged cells; this family is the
arithmetic bridge — matches Obsidian/Fallout "float.inc" convention *(inferred)*.

### 3.2 engine core `small_script_functions` @0x6FFAF8 (155 entries, all names+EAs verified)
Full dump in this session's raw output — grouped:
- **Actions (mirror of CreateGameAction family, factory-intf +40)**: `A_Attack A_JustAttack
  A_ReloadWeapon A_MoveToEntity A_MoveToPosition A_Wander A_Kill A_Wait A_PlayAnim A_PickupItem
  A_SendMessage A_SendMessageToTeam A_SendMessageToSquad A_FloatingSpeechText`
- entities: `SpawnEntity DestroyEntity ActivateEntity DeactivateEntity FindEntity MatchEntity
  GetObject FindObjects DestroyItem GiveNewItem TransferItem[From/ToParty] DropItem PickupItem
  EnableDialogue DisableDialogue StartCutscene StopCutscene IsInCutscene`
- teams/squads: `FindTeamMembers GetTeamMember GetTeam AddToTeam RemoveFromTeam GetTeamRank
  SetTeamRank SetTeamAttitude CreateTeam FindSquadMembers GetSquadMEmber(typo,sic) GetSquadID
  GetSquadRank SetSquadRank AddToSquad RemoveFromSquad CreateSquad SetSquadAttitude`
- spatial/LOS: `Distance PointDistance GetPosition SetPosition GetFacing SetFacing
  SetFacingTarget SetFacingPoint GetDirectionToEntity GetDirectionToPoint GetLOS GetLOSPoint
  IsUnderFOW IsEntityInArea GetMapID`
- **nav/path**: `GetNavPointCount GetNavPointLinks GetNavPointPosition GetNavPointOrientation
  GetWayPointCount GetWayPointPosition GetWayPointOrientation GetNearestPath`
- **time family (Year 0x5C5040 .. SecondsPassed 0x5C4F30)** — see §4
- vars: `GetGlobal SetGlobal IncGlobal DecGlobal GetValue SetValue IncValue DecValue Myself`
- messages: `MessageArgCount MessageGetArg SendMessage[ToTeam/ToSquad]`
- dialogue build: `DialogueCall RegisterLinkToOtherDialogue RegisterNodeText RegisterReply
  SetNodeTextFunction SetCurrentNodeName WasLastNodeName IsInDialogue PCSpeaker
  SetTokenStrref SetTokenInt`
- world FX: `MapEffectSetActive MapSoundSetActive MapEffectResetElapsedTime AttachVEG
  RemoveVEG GetVEGID AttachSound RemoveSound GetCameraFog SetCameraFog ResetCameraFog
  GetHeightFog SetHeightFog ResetHeightFog PlayMusic FloatingSpeechText FloatingDamageText`
- meta: `Sample RandomNumber RandomNumberFloat Debug Breakpoint ClearActions QueueFunction
  ClearFunctionQueue IsIdle IsDead IsInParty IsInCombat HasItem PartyHasItem GetType
  EditToken GetAreaRestStatus SetAreaRestStatus RecordDeath HasDied SetModel
  SetEncyclopediaData SetWorldMapArea{Visible,Explorable,Explored} StartStore JoinParty
  LeaveParty TeleportParty`

### 3.3 F3 game lib @0x6F3750 (45)
`A_Attack A_JustAttack StartDialogue DisplayStringRef HideInterfaces SetMoveMode EquipItem
UnequipItem LockDoor UnlockDoor AddCNPCToParty A_ReloadWeapon Protagonist GetWeaponCombo
SetWeaponCombo GetSquadAttitude GetTeamAttitude GetPerkRank GetLevel GetPrimaryAttribute
GetDerivedAttribute GetBaseSkill GetTotalSkill GetRace GetGender GetEthnicity GetSubRace
GetMaxHP GetCurrentHP GetWeaponRange GetItemType GetUsableObjectType SetHP AddHP IsLocked
IsItemEquipped GetLockDC PlayMusic OpenTask CloseTask AddEntry OpenQuest CloseQuest
AddQuestEntry AddJournalEntry`
- `GetMaxHP_45b110/GetCurrentHP_45b1b0` — third independent confirmation:
  GameEntity `+0x20 = MaxHealth, +0x24 = CurHealth` (gameworld doc §3).
- **`OpenQuest CloseQuest AddQuestEntry AddJournalEntry` are ALL the same stub
  `JE::GUI::Window::virt_meth_0x420990 = return 1`** — quest/journal scripting is DEAD in the
  shipped demo. Reimpl: accept-and-true. (`AddEntry/OpenTask/CloseTask` are real: 0x45B980/
  0x45B900/0x45B940.)

## 4. Time natives read the clock stack (cross-doc pin)

`Year(param_1)`: amx_GetGlObal "myself" → `(*factory+116)`(=0x470330? NO — byte +116 =
0x668508 entry **0x4700D0 = get World+0x100**) → calendar OBJECT; `(myself & 0xE0000000)`
space-tag test: ids NOT in a registered space → return `calendar->vft[+20]` (world clock
Year). Else `entity = 0x54BE10(myself)`; `t = *(entity + 0x29C)`; builds a SECOND calendar via
`factory + 0x10` (0x46FDA0→0x544D90 alloc?), `cal2.vft[0](cal2, base)`,
`cal2.vft[+88](cal2, t)` (set time), `return cal2.vft[+20]()` (GetYear), free(cal2).
⇒ **per-entity birth/time value at Entity+0x29C** — same family as gameworld §12d
entity+0x28 = world-clock sample (two recorded times: GE+0x28 coarse, Entity+0x29C precise?)
*(inferred link)*. Calendar objects = the JE::Gregorian class (their session-12 doc): World
accessors +116 (0x100 field) = world calendar ptr; +0x3C (0x470070) = evaluate clock also
calendar — consistent dual use.
Remaining: Month/Day/.../SecondsPassed (0x5C4490..0x5C4F30) = same shape, vft[+20]→+24→…
slot increments — decode bodies when the reimpl needs exact calendar epoch semantics.

## 5. Questions to the reimpl agent
1. `DialogueCall`+`RegisterNodeText/Reply` family (0x5C3C30/0x5C3980/0x5C4060) defines the
   whole dialogue-tree build API — do you want those decoded next (GameDialogueWindow
   contract), or is the AMX interpreter itself (`sub_5CC810` = full Pawn VM) out of scope?
2. The `myself` registration proves scripts bind per-entity with owner id at
   Script_Small+0x04 — matches your entity-script slot story? (docs/f3/GameScripting.md is
   otherwise empty of evidence.)
3. Flag origin for class-10 (§2): if your lockstep ever runs server-mode, bp 0x45A960 +
   0x46FB20 and diff the flag against GameParty+0x14 (protagonist ptr) — we predict
   protagonist path ⇒ flag=1.

## 6. Queue
1. Script_Small remaining vft bodies (0x5A4A80/0x5A4390/0x5A5A90 Run/…) — arg marshalling.
2. `sub_5CCDE0/5CCCC0/5CCD00/5CC810/5CE320` = confirm Pawn SDK names (amx_Register/GetGlob/
   DefineGlob/amx_Init/SetDebugHook) — mostly mechanical.
3. Team/squad natives → GameParty/GameSquad storage maps (+0xF4 party pin already landed).
4. `0x4700D0` (World+0x100) writer — who installs the world calendar.
