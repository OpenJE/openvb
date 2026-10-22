# F3 shipped native tables - PE dump, arity-gate claims, port audit

Generated 2026-10-22 by `tools/native-table-dump.py`
(sha256 `b1d871d6c839483b...`). Raw tool output is embedded verbatim in section 7;
nothing in the tables below is hand-transcribed.

| field | value |
|---|---|
| exe | `/nix/store/a1j412798wh979ig7xp3bp9aji4zfpz0-f3demo-1.0/share/f3demo/F3.exe` |
| sha256 | `4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87` |
| size | 3194880 bytes |
| image base | 0x400000 |

**Scope: findings only.** `src/F3/ScriptNatives.{cpp,hpp}` and
`modules/openje/tests/je/ScriptNatives` are reserved by TrueRaven (R-USERRE-B natives
lane). This document edits nothing there; every divergence below is reported, not acted on.

## 1. Why dump the image instead of querying the database

Shipped's authoritative native-name -> function mapping exists **only** as three
`{name-ptr, fn-ptr}` arrays that `Script_Small::Load` (0x5a8880) hands to `amx_Register`
(`sub_5CCDE0`) in this order (per `discoveries/2025-10-20-scripting-abi-native-tables.md` S1):

```
0x5a8ada  sub_5CCDE0(&amx, &off_7004C0,             -1)   float lib    @0x7004C0
0x5a8ae7  sub_5CCDE0(&amx, &small_script_functions,  -1)   engine core  @0x6FFAF8
0x5a8afe  sub_5CCDE0(&amx, &off_6F3750,              -1)   F3 game lib  @0x6F3750
```

No type database lists these arrays (f3_method_index on 0x45b900/0x45b980 returns nothing),
so aliasing is invisible to a database query. Walking the image is what proves four names
share one `return 1` body. Each array ends at a `{0, 0}` pair and shipped stores **no count**,
so every count here comes from walking to the terminator.

Reproduce:

```bash
python3 tools/native-table-dump.py "$OPENVB_ORIG_EXE"                 # all three tables
python3 tools/native-table-dump.py "$OPENVB_ORIG_EXE" --selftest       # gates, exit != 0 on fail
python3 tools/native-table-dump.py "$OPENVB_ORIG_EXE" 6F3750 \
        --audit src/F3/ScriptNatives.cpp                               # port audit
```

## 2. The section-header trap (why counts were re-derived, not trusted)

Section fields are `VirtualSize, VirtualAddress, SizeOfRawData, PointerToRawData` at header
+8/+12/+16/+20. Swapping them, or using `SizeOfRawData` as the VA span, produces output that
*looks* structured while being garbage. The tool therefore refuses to print unless known
`(name -> fn VA)` pairs resolve (`EXPECTED_FN`), and separates three failure modes:

- nothing resolves -> VA decode wrong (field order / image base);
- a known name resolves but pairs to the wrong fn -> pairing off, dump untrusted;
- no anchored name in scope -> dump unverified (e.g. single-table dump of an unanchored lib).

Tested against a deliberately swapped-field section table: the gate fires. Tested with a vftable
VA (0x6A539C) passed as a native table: also fires.

## 3. Counts verified by walking to the terminator

| table | VA | entries (walked) | distinct fn | {0,0} terminator | discovery-note claim |
|---|---|---|---|---|---|
| float lib (off_7004C0) | `0x7004c0` | 9 | 9 | yes | 9 -> VERIFIED |
| engine core (small_script_functions) | `0x6ffaf8` | 155 | 155 | yes | 155 -> VERIFIED |
| F3 game lib (off_6F3750) | `0x6f3750` | 45 | 42 | yes | 45 -> VERIFIED |
| **total** | | **209** | | | 209 claimed |

All three counts the discovery note claims are confirmed independently (9+155+45=209), each
table terminating on its own `{0,0}` pair.

- **42 distinct fn values for 45 names** in the F3 lib - the alias signature, and the only
  aliasing in all three tables: `0x420990` = `b8 01 00 00 00 c3` = `mov eax,1; ret` <-
  `OpenQuest, CloseQuest, AddQuestEntry, AddJournalEntry` (idx 41-44). Quest/journal scripting
  is **dead** in the shipped demo.
- Engine core: 155 names / 155 distinct fn. Float lib: 9 / 9. No aliasing on either side.
- **No name and no fn value appears in two tables**, so the float -> engine -> F3 registration
  order never shadows anything. A port that registers one flat table cannot introduce a
  shadowing bug, because shipped has none to reproduce.

## 4. The arity gate is a second, independent claim per native

Gated natives all open with the same shape (decoded from bytes, not pattern-guessed):

```
push <imm32 name-ptr>   ; the name the ARITY ERROR prints
push <imm8 cells>       ; expected parameter cells
push <reg> / push <reg> ; params block + amx
call 0x59af40            ; the shared arity gate (port: ScriptNativeCheckArity)
```

Note the pointer pushed is **not** the table's name pointer - it is a separate .rdata string,
so shipped can (and does) complain under a different name than it registers. Coverage:

| table | gated | ungated |
|---|---|---|
| float lib (off_7004C0) | 0/9 | float, floatstr, floatmul, floatdiv, floatadd, floatsub, floatfract, floatround, floatcmp |
| engine core (small_script_functions) | 139/155 | TransferItemToParty, A_SendMessage, A_SendMessageToTeam, SendMessage, SendMessageToTeam, MessageArgCount, SendMessageToSquad, A_SendMessageToSquad, Year, Month, Day, Hour, Minute, Second, GetTimeStamp, Debug |
| F3 game lib (off_6F3750) | 41/45 | OpenQuest, CloseQuest, AddQuestEntry, AddJournalEntry |

The float lib has no arity gate at all (engine-side arity), and the F3 lib's only ungated
entries are the four dead quest/journal aliases - which is *why* the port records `cells = -1`
for them. The 16 ungated engine-core entries include the whole time family and the
`SendMessage` group; porting those needs the no-gate behavior reproduced, not an invented gate.

## 5. Audit of the port's 36-name table against shipped (verbatim in 7.3)

| check | result |
|---|---|
| port rows parsed from `s_gameCreatureNatives` | 36 |
| names matching shipped exactly | **36/36** |
| port-only names (invented) | **0** |
| case/spelling drift | **0** |
| port EA comment != shipped fn VA | **0** |
| duplicate names in the port table | 0 |
| port `cells` != shipped gate cells | **0** (32 gated rows checked) |
| port rows with `cells = -1` | 4, exactly the 4 shipped ungated rows |
| shipped F3 names not yet in the port | **9** |

Nothing in the lane is invented, and the arity column is byte-correct on every row.

### 5.1 The 9 unported shipped F3 natives

| idx | name | fn | cells | error-name |
|---|---|---|---|---|
| 0 | `A_Attack` | `0x459eb0` | 2 | `Attack` |
| 1 | `A_JustAttack` | `0x459f70` | 2 | `Attack` |
| 2 | `StartDialogue` | `0x45a040` | 2 | `StartDialogue` |
| 3 | `DisplayStringRef` | `0x45a410` | 4 | `DisplayStringRef` |
| 4 | `HideInterfaces` | `0x45a100` | 0 | `HideInterfaces` |
| 5 | `SetMoveMode` | `0x45a140` | 3 | `SetMoveMode` |
| 11 | `A_ReloadWeapon` | `0x45a380` | 1 | `ReloadWeapon` |
| 15 | `GetSquadAttitude` | `0x45aa90` | 2 | `GetSquadAttitude` |
| 16 | `GetTeamAttitude` | `0x45aaf0` | 2 | `GetTeamAttitude` |

`A_Attack` and `A_JustAttack` are near-twins and should be read side by side. Byte-compared:

- **First 22 bytes are byte-identical, displacement included.** They are exactly 0xC0 apart and
  the gate lives 0xC0 further away in the other direction, so `push 0x65D7C8` ("`Attack`"),
  `push 2`, `push edi`, `push esi`, `call 0x59af40` is the *same byte string* in both. So
  **arity errors for both natives print "Attack"**, and both expect 2 cells.
- Past the gate they stay structurally identical: both call `0x59aef0` (a second resolve), then
  `0x59ad20` (the error sink) with only the message immediate differing -
  `Attack()`/`JustAttack()` variants of "was unable to find the attacking entity."
  (0x65D794 / 0x65D804) and of "was unable to find the target entity."
  (0x65D764 / 0x65D7D0).
- Note the asymmetry shipped leaves in place: the *entity* messages are name-specific
  ("Attack() ...", "JustAttack() ...") but the *arity* message is not (both "Attack").

Two natives, one shared arity-error string; not one native with a flag.

## 6. New divergences found by the walk (action: report only, lane owner decides)

### 6.1 Shipped prints a DIFFERENT name in its arity error than it registers (20 natives)

F3 game lib (4):

| table name | error prints | cells | string VA |
|---|---|---|---|
| `A_Attack` | `Attack` | 2 | `0x65d7c8` |
| `A_JustAttack` | `Attack` | 2 | `0x65d7c8` |
| `UnequipItem` | `EquipItem` | 3 | `0x65dbf4` |
| `A_ReloadWeapon` | `ReloadWeapon` | 1 | `0x65da30` |

Engine core (16):

| table name | error prints | cells | string VA |
|---|---|---|---|
| `A_MoveToEntity` | `MoveToEntity` | 4 | `0x6b60f8` |
| `A_MoveToPosition` | `MoveToPosition` | 4 | `0x6b618c` |
| `A_Kill` | `Kill` | 1 | `0x659f00` |
| `A_Wait` | `Wait` | 2 | `0x6b62b0` |
| `A_PlayAnim` | `PlayAnim` | 5 | `0x6b62ec` |
| `DecGlobal` | `IncGlobal` | 1 | `0x6ad81c` |
| `GetSquadMEmber` | `GetSquadMember` | 2 | `0x6b4b80` |
| `GetSquadID` | `GetSquad` | 1 | `0x6b4bc4` |
| `GetSquadRank` | `GetSquad` | 1 | `0x6b4bc4` |
| `GetNavPointCount` | `GetNavpointCount` | 0 | `0x6b4058` |
| `FloatingSpeechText` | `AddFloatingSpeechText` | 6 | `0x6b0188` |
| `A_FloatingSpeechText` | `A_AddFloatingSpeechText` | 6 | `0x6b01e4` |
| `FloatingDamageText` | `AddFloatingDamageText` | 5 | `0x6b0130` |
| `SetWorldMapAreaVisible` | `WorldMapSetAreaVisible` | 3 | `0x6af730` |
| `SetWorldMapAreaExplorable` | `WorldMapSetAreaExplorable` | 3 | `0x6af78c` |
| `SetWorldMapAreaExplored` | `WorldMapSetAreaExplored` | 3 | `0x6af7ec` |

Byte-level proof of the three sharings:

- `EquipItem` (0x45a580) and `UnequipItem` (0x45a6f0) both reach the gate at **+0x32** with the
  same two immediates: `68 f4 db 65 00` = 0x65DBF4 = `"EquipItem"`, then `6a 03`. So UnequipItem
  is confirmed at byte level, not just by name reading.
- `IncGlobal` (0x5c8200) and `DecGlobal` (0x5c8360) push the **same** immediate
  `68 1c d8 6a 00` = 0x6AD81C = `"IncGlobal"`. So `DecGlobal(1,2,3)` complains about
  **IncGlobal**. Copy-paste, shipped, live.
- `GetSquadID` (0x5c6750) and `GetSquadRank` (0x5c67c0) push `68 c4 4b 6b 00` = 0x6B4BC4 =
  `"GetSquad"` - a name that is in **no** table. Their arity error cites a function scripts
  can never call.

Why this matters for the port: the lane already found and faithfully kept one of these
(`UnequipItem` reports `"EquipItem"`, src/F3/ScriptNatives.cpp:1019, marked `SHIPPED: wrong
name`). That is the correct treatment - a port that "fixes" the name is a silent divergence
in the error channel, and any script or test that greps the error text would then pass on the
port and fail on shipped. The remaining 19 are mostly not yet ported, so the rule to carry
forward is: **the registration name and the arity-error name are two separate fields to port,
not one.**

### 6.2 A shipped typo is in the table name, not the error message

`GetSquadMEmber` (idx, fn 0x5c6710) - capital E - is what scripts must actually call, while its
arity error prints the correctly spelled `"GetSquadMember"` (0x6B4B80). The discovery note marks
it `(typo, sic)`; this confirms the typo is the *wired* name. Porting the clean spelling would
break every shipped script that calls it.

### 6.3 Cross-table claims already verified

The port's `cells` column matches shipped's gate immediates on all 32 gated rows, so no
arity value in the lane needs re-deriving. The `PlayMusic`/`GetWeaponCombo`/`SetWeaponCombo`
stub claims were likewise not contradicted by the image.

## 7. Raw output (verbatim)

### 7.1 `--selftest`

```text
selftest PASS
  exe sha256 pinned: 4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87
  0x6f3750 F3 game lib (off_6F3750): 45 entries, 42 distinct fn, terminator=True
  0x6ffaf8 engine core (small_script_functions): 155 entries, 155 distinct fn, terminator=True
  0x7004c0 float lib (off_7004C0): 9 entries, 9 distinct fn, terminator=True
  F3 quest/journal stub 0x420990 <- AddJournalEntry, AddQuestEntry, CloseQuest, OpenQuest
  gate names resolved: 209
```

### 7.2 all three tables

```text
# native-table-dump
exe          : /nix/store/a1j412798wh979ig7xp3bp9aji4zfpz0-f3demo-1.0/share/f3demo/F3.exe
sha256       : 4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87
pinned sha256: 4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87
file size    : 3194880 bytes
image base   : 0x400000
sections     : .text@0x401000/0x245c52, .rdata@0x647000/0xa9790, .data@0x6f1000/0x200e0, .data1@0x712000/0x8e0, .rsrc@0x713000/0x2728

## F3 game lib (off_6F3750) @ 0x6f3750
entries=45  terminator_{0,0}_found=True  distinct_fns=42
  idx  name                    name_va       fn_va  cells  gate-name            first_bytes
   0  A_Attack                 65fa80      459eb0      2  Attack               83 ec 08 56 8b 74 24 10   <<< gate error-name != table name
   1  A_JustAttack             65fa70      459f70      2  Attack               83 ec 08 56 8b 74 24 10   <<< gate error-name != table name
   2  StartDialogue            65d8ac      45a040      2  StartDialogue        83 ec 08 56 8b 74 24 10
   3  DisplayStringRef         65db04      45a410      4  DisplayStringRef     83 ec 24 a1 c0 62 70 00
   4  HideInterfaces           65d8bc      45a100      0  HideInterfaces       8b 44 24 08 8b 4c 24 04
   5  SetMoveMode              65d8cc      45a140      3  SetMoveMode          51 56 8b 74 24 10 57 8b
   6  EquipItem                65dbf4      45a580      3  EquipItem            81 ec a4 00 00 00 a1 c0
   7  UnequipItem              65fa64      45a6f0      3  EquipItem            81 ec c0 00 00 00 a1 c0   <<< gate error-name != table name
   8  LockDoor                 65d928      45a1b0      1  LockDoor             51 56 8b 74 24 0c 57 8b
   9  UnlockDoor               65d984      45a240      1  UnlockDoor           51 56 8b 74 24 0c 57 8b
  10  AddCNPCToParty           65d9f0      45a2d0      1  AddCNPCToParty       51 56 8b 74 24 10 57 8b
  11  A_ReloadWeapon           65fa54      45a380      1  ReloadWeapon         51 56 8b 74 24 0c 57 8b   <<< gate error-name != table name
  12  Protagonist              65e198      45a960      0  Protagonist          8b 44 24 08 56 8b 74 24
  13  GetWeaponCombo           65e1e0      45a9f0      1  GetWeaponCombo       51 56 8b 74 24 0c 57 8b
  14  SetWeaponCombo           65e22c      45aa40      2  SetWeaponCombo       51 56 8b 74 24 0c 57 8b
  15  GetSquadAttitude         65e23c      45aa90      2  GetSquadAttitude     8b 44 24 04 56 8b 74 24
  16  GetTeamAttitude          65e250      45aaf0      2  GetTeamAttitude      8b 44 24 04 56 8b 74 24
  17  GetPerkRank              65e2d8      45ab50      2  GetPerkRank          51 56 8b 74 24 10 57 8b
  18  GetLevel                 65e358      45abe0      1  GetLevel             51 56 8b 74 24 0c 57 8b
  19  GetPrimaryAttribute      65e3ec      45ac70      2  GetPrimaryAttribute  51 56 8b 74 24 10 57 8b
  20  GetDerivedAttribute      65e484      45ad00      2  GetDerivedAttribute  51 56 8b 74 24 10 57 8b
  21  GetBaseSkill             65e510      45ad90      2  GetBaseSkill         51 56 8b 74 24 10 57 8b
  22  GetTotalSkill            65e598      45ae20      2  GetTotalSkill        51 56 8b 74 24 10 57 8b
  23  GetRace                  65e614      45aeb0      1  GetRace              51 56 8b 74 24 0c 57 8b
  24  GetGender                65e68c      45af40      1  GetGender            51 56 8b 74 24 0c 57 8b
  25  GetEthnicity             65e710      45afd0      1  GetEthnicity         51 56 8b 74 24 0c 57 8b
  26  GetSubRace               65e794      45b070      1  GetSubRace           51 56 8b 74 24 0c 57 8b
  27  GetMaxHP                 65e80c      45b110      1  GetMaxHP             51 56 8b 74 24 0c 57 8b
  28  GetCurrentHP             65e88c      45b1b0      1  GetCurrentHP         51 56 8b 74 24 0c 57 8b
  29  GetWeaponRange           65ebfc      45b620      1  GetWeaponRange       51 53 8b 5c 24 0c 56 8b
  30  GetItemType              65e8f8      45b240      1  GetItemType          51 56 8b 74 24 0c 57 8b
  31  GetUsableObjectType      65ea58      45b440      1  GetUsableObjectType  51 56 8b 74 24 0c 57 8b
  32  SetHP                    65e96c      45b2e0      2  SetHP                51 56 8b 74 24 10 57 8b
  33  AddHP                    65e9dc      45b390      2  AddHP                51 56 8b 74 24 10 57 8b
  34  IsLocked                 65eabc      45b4e0      1  IsLocked             51 56 8b 74 24 0c 57 8b
  35  IsItemEquipped           65ecf0      45b750      2  IsItemEquipped       81 ec bc 00 00 00 a1 c0
  36  GetLockDC                65eb38      45b570      1  GetLockDC            51 56 8b 74 24 10 57 8b
  37  PlayMusic                65f5d8      45b9c0      1  PlayMusic            8b 44 24 08 56 8b 74 24
  38  OpenTask                 65f134      45b900      2  OpenTask             8b 44 24 04 56 8b 74 24
  39  CloseTask                65f140      45b940      2  CloseTask            8b 44 24 04 56 8b 74 24
  40  AddEntry                 65f14c      45b980      2  AddEntry             8b 44 24 04 56 8b 74 24
  41  OpenQuest                65fa48      420990      -  -                    b8 01 00 00 00 c3 90 90
  42  CloseQuest               65fa3c      420990      -  -                    b8 01 00 00 00 c3 90 90
  43  AddQuestEntry            65fa2c      420990      -  -                    b8 01 00 00 00 c3 90 90
  44  AddJournalEntry          65fa1c      420990      -  -                    b8 01 00 00 00 c3 90 90
  alias groups (1 fn claimed by >1 name):
    0x420990 <- OpenQuest, CloseQuest, AddQuestEntry, AddJournalEntry

## engine core (small_script_functions) @ 0x6ffaf8
entries=155  terminator_{0,0}_found=True  distinct_fns=155
  idx  name                            name_va       fn_va  cells  gate-name                    first_bytes
    0  Sample                           6adacc      5ca820      3  Sample                       81 ec 88 00 00 00 a1 c0
    1  ClearActions                     6adabc      5c8a00      1  ClearActions                 51 56 8b 74 24 0c 57 8b
    2  DestroyItem                      6adab0      5c9650      2  DestroyItem                  81 ec 28 01 00 00 a1 c0
    3  GiveNewItem                      6adaa4      5c8a60      3  GiveNewItem                  81 ec 0c 01 00 00 a1 c0
    4  A_MoveToEntity                   6ada94      5c8b70      4  MoveToEntity                 83 ec 08 56 8b 74 24 10   <<< gate error-name != table name
    5  A_MoveToPosition                 6ada80      5c8c50      4  MoveToPosition               83 ec 10 56 8b 74 24 1c   <<< gate error-name != table name
    6  TransferItem                     6ada70      5c9780      4  TransferItem                 81 ec 2c 01 00 00 a1 c0
    7  TransferItemFromParty            6ada58      5c9ff0      3  TransferItemFromParty        81 ec 40 01 00 00 a1 c0
    8  TransferItemToParty              6ada44      5ca1e0      -  -                            81 ec 3c 01 00 00 a1 c0
    9  DropItem                         6ada38      5c9930      2  DropItem                     81 ec 40 01 00 00 a1 c0
   10  A_Wander                         6ada2c      5c8d40      4  A_Wander                     83 ec 1c 56 8b 74 24 24
   11  SpawnEntity                      6ada20      5ca650      3  SpawnEntity                  81 ec 28 02 00 00 a1 c0
   12  DestroyEntity                    6ada10      5ca5f0      1  DestroyEntity                51 56 8b 74 24 0c 57 8b
   13  A_Kill                           6ada08      5c8e70      1  Kill                         51 56 8b 74 24 0c 57 8b   <<< gate error-name != table name
   14  A_Wait                           6ada00      5c8f00      2  Wait                         51 56 8b 74 24 0c 57 8b   <<< gate error-name != table name
   15  A_PlayAnim                       6ad9f4      5c8f90      5  PlayAnim                     51 56 8b 74 24 10 57 8b   <<< gate error-name != table name
   16  MapEffectSetActive               6ad9e0      5c9ab0      3  MapEffectSetActive           81 ec 34 01 00 00 a1 c0
   17  MapSoundSetActive                6ad9cc      5c9c50      3  MapSoundSetActive            81 ec 34 01 00 00 a1 c0
   18  MapEffectResetElapsedTime        6ad9b0      5c9df0      2  MapEffectResetElapsedTime    81 ec 34 01 00 00 a1 c0
   19  JoinParty                        6ad9a4      5c9030      1  JoinParty                    51 56 8b 74 24 10 57 8b
   20  LeaveParty                       6ad998      5c90d0      1  LeaveParty                   51 56 8b 74 24 10 57 8b
   21  TeleportParty                    6ad988      5ca3a0      2  TeleportParty                81 ec 18 01 00 00 a1 c0
   22  ActivateEntity                   6ad978      5c9180      1  ActivateEntity               51 56 8b 74 24 0c 57 8b
   23  DeactivateEntity                 6ad964      5c9200      1  DeactivateEntity             51 56 8b 74 24 0c 57 8b
   24  EnableDialogue                   6ad954      5c9280      1  EnableDialogue               51 56 8b 74 24 0c 57 8b
   25  DisableDialogue                  6ad944      5c9300      1  DisableDialogue              51 56 8b 74 24 0c 57 8b
   26  PickupItem                       6ad938      5c9380      2  PickupItem                   83 ec 08 56 8b 74 24 10
   27  A_PickupItem                     6ad928      5c9460      2  A_PickupItem                 83 ec 08 56 8b 74 24 10
   28  QueueFunction                    6ad918      5ca4f0      2  QueueFunction                81 ec 28 01 00 00 a1 c0
   29  ClearFunctionQueue               6ad904      5c9f80      0  ClearFunctionQueue           51 8b 44 24 0c 56 8b 74
   30  StartCutscene                    6ad8f4      5c9520      1  StartCutscene                51 56 8b 74 24 0c 57 8b
   31  StopCutscene                     6ad8e4      5c95b0      0  StopCutscene                 8b 44 24 08 8b 4c 24 04
   32  IsInCutscene                     6ad8d4      5c9610      0  IsInCutscene                 8b 44 24 08 8b 4c 24 04
   33  Distance                         664b9c      5c6970      2  Distance                     83 ec 2c 56 8b 74 24 34
   34  FindEntity                       6ad8c8      5c7f10      1  FindEntity                   81 ec 0c 01 00 00 a1 c0
   35  GetPosition                      6ad8bc      5c6b30      2  GetPosition                  83 ec 10 56 8b 74 24 18
   36  SetPosition                      6ad8b0      5c6bd0      2  SetPosition                  83 ec 10 56 8b 74 24 18
   37  GetFacing                        6ad8a4      5c6c90      1  GetFacing                    51 56 8b 74 24 0c 57 8b
   38  SetFacing                        6ad898      5c6cf0      2  SetFacing                    83 ec 10 56 8b 74 24 1c
   39  SetFacingTarget                  6ad888      5c6d80      2  SetFacingTarget              83 ec 20 56 8b 74 24 28
   40  SetFacingPoint                   6ad878      5c6eb0      2  SetFacingPoint               83 ec 28 56 8b 74 24 30
   41  GetDirectionToEntity             6ad860      5c6ff0      2  GetDirectionToEntity         83 ec 14 56 8b 74 24 1c
   42  GetDirectionToPoint              6ad84c      5c70e0      2  GetDirectionToPoint          83 ec 1c 56 8b 74 24 24
   43  IsIdle                           6ad844      5c71f0      1  IsIdle                       51 56 8b 74 24 10 57 8b
   44  IsDead                           6ad83c      5c7250      1  IsDead                       51 56 8b 74 24 10 57 8b
   45  IsInParty                        6ad830      5c72c0      1  IsInParty                    51 56 8b 74 24 10 57 8b
   46  Myself                           6ad828      5c7340      0  Myself                       51 8b 44 24 0c 56 8b 74
   47  GetGlobal                        69d1a4      5c8000      1  GetGlobal                    81 ec 08 01 00 00 a1 c0
   48  SetGlobal                        69d198      5c8120      2  SetGlobal                    81 ec 08 01 00 00 a1 c0
   49  IncGlobal                        6ad81c      5c8200      1  IncGlobal                    81 ec 08 01 00 00 a1 c0
   50  DecGlobal                        6ad810      5c8360      1  IncGlobal                    81 ec 08 01 00 00 a1 c0   <<< gate error-name != table name
   51  GetValue                         6ad804      5c7390      2  GetValue                     51 56 8b 74 24 10 57 8b
   52  SetValue                         6ad7f8      5c7400      3  SetValue                     51 56 8b 74 24 10 57 8b
   53  IncValue                         6ad7ec      5c7480      2  IncValue                     51 56 8b 74 24 0c 57 8b
   54  DecValue                         6ad7e0      5c7500      2  DecValue                     51 56 8b 74 24 0c 57 8b
   55  HasItem                          6ad7d8      5c84c0      2  HasItem                      81 ec 28 01 00 00 a1 c0
   56  PartyHasItem                     6ad7c8      5c88a0      1  PartyHasItem                 81 ec 38 01 00 00 a1 c0
   57  FindObjects                      6ad7bc      5c7580      3  FindObjects                  83 ec 1c 53 8b 5c 24 28
   58  MatchEntity                      6ad7b0      5c6a50      2  MatchEntity                  81 ec 10 01 00 00 a1 c0
   59  GetObject                        6ad7a4      5c7700      2  GetObject                    8b 44 24 04 56 8b 74 24
   60  GetLOS                           6ad79c      5c7740      2  GetLOS                       83 ec 08 56 8b 74 24 10
   61  GetLOSPoint                      6ad790      5c77e0      2  GetLOSPoint                  83 ec 10 56 8b 74 24 18
   62  SetModel                         6ad784      5c78a0      2  SetModel                     83 ec 08 53 8b 5c 24 14
   63  GetAreaRestStatus                6ad770      5c7980      0  GetAreaRestStatus            51 8b 44 24 0c 56 8b 74
   64  SetAreaRestStatus                6ad75c      5c7a50      1  SetAreaRestStatus            51 56 8b 74 24 0c 57 8b
   65  IsEntityInArea                   6ad74c      5c7e00      2  IsEntityInArea               81 ec 0c 01 00 00 a1 c0
   66  GetMapID                         6ad740      5c7ba0      1  GetMapID                     51 56 8b 74 24 0c 57 8b
   67  RecordDeath                      6ad734      5c85c0      1  RecordDeath                  83 ec 28 a1 c0 62 70 00
   68  HasDied                          6ad72c      5c8750      1  HasDied                      55 8b ec 83 e4 f8 81 ec
   69  IsUnderFOW                       6ad720      5c7d10      1  IsUnderFOW                   83 ec 18 56 8b 74 24 20
   70  IsInCombat                       6ad714      5c7b30      1  IsInCombat                   51 56 8b 74 24 0c 57 8b
   71  GetType                          6ad70c      5c7c00      1  GetType                      51 56 8b 74 24 0c 57 8b
   72  EditToken                        69cb74      5c7c60      2  EditToken                    81 ec 04 01 00 00 a1 c0
   73  FindTeamMembers                  6ad6fc      5c6390      2  FindTeamMembers              83 ec 08 53 8b 5c 24 10
   74  GetTeamMember                    6ad6ec      5c6420      2  GetTeamMember                8b 44 24 04 56 8b 74 24
   75  GetTeam                          6ad6e4      5c6460      1  GetTeam                      51 56 8b 74 24 0c 57 8b
   76  AddToTeam                        6ad6d8      5c64d0      2  AddToTeam                    8b 44 24 04 56 8b 74 24
   77  RemoveFromTeam                   6ad6c8      5c6510      1  RemoveFromTeam               8b 44 24 04 56 8b 74 24
   78  GetTeamRank                      6ad6bc      5c6550      1  GetTeamRank                  51 56 8b 74 24 0c 57 8b
   79  SetTeamRank                      6ad6b0      5c65c0      2  SetTeamRank                  8b 44 24 04 56 8b 74 24
   80  SetTeamAttitude                  6ad6a0      5c6600      3  SetTeamAttitude              8b 44 24 04 56 8b 74 24
   81  CreateTeam                       6ad694      5c6650      0  CreateTeam                   8b 44 24 08 8b 4c 24 04
   82  FindSquadMembers                 6ad680      5c6680      2  FindSquadMembers             83 ec 08 53 8b 5c 24 10
   83  GetSquadMEmber                   6ad670      5c6710      2  GetSquadMember               8b 44 24 04 56 8b 74 24   <<< gate error-name != table name
   84  GetSquadID                       6ad664      5c6750      1  GetSquad                     51 56 8b 74 24 0c 57 8b   <<< gate error-name != table name
   85  GetSquadRank                     6ad654      5c67c0      1  GetSquad                     51 56 8b 74 24 0c 57 8b   <<< gate error-name != table name
   86  CreateSquad                      6ad648      5c6830      0  CreateSquad                  8b 44 24 08 8b 4c 24 04
   87  RemoveFromSquad                  6ad638      5c6860      1  RemoveFromSquad              8b 44 24 04 56 8b 74 24
   88  SetSquadRank                     6ad628      5c68a0      2  SetSquadRank                 8b 44 24 04 56 8b 74 24
   89  AddToSquad                       6ad61c      5c68e0      2  AddToSquad                   8b 44 24 04 56 8b 74 24
   90  SetSquadAttitude                 6ad608      5c6920      3  SetSquadAttitude             8b 44 24 04 56 8b 74 24
   91  GetNavPointCount                 6ad5f4      5c5aa0      0  GetNavpointCount             51 8b 44 24 0c 56 8b 74   <<< gate error-name != table name
   92  GetNavPointLinks                 6ad5e0      5c5b50      2  GetNavPointLinks             83 ec 18 53 8b 5c 24 20
   93  GetNavPointPosition              6ad5cc      5c5c40      2  GetNavPointPosition          83 ec 10 53 8b 5c 24 18
   94  GetNavPointOrientation           6ad5b4      5c5d30      1  GetNavPointOrientation       83 ec 10 56 8b 74 24 18
   95  GetWayPointCount                 6ad5a0      5c5fc0      1  GetWayPointCount             81 ec 24 01 00 00 a1 c0
   96  GetWayPointPosition              6ad58c      5c60e0      3  GetWayPointPosition          81 ec 30 01 00 00 a1 c0
   97  GetWayPointOrientation           6ad574      5c6240      2  GetWayPointOrientation       81 ec 30 01 00 00 a1 c0
   98  GetNearestPath                   6ad564      5c5e20      2  GetNearestPath               83 ec 24 53 8b 5c 24 2c
   99  A_SendMessage                    6ad554      5c5120      -  -                            83 ec 14 55 8b 6c 24 20
  100  A_SendMessageToTeam              6ad540      5c52b0      -  -                            83 ec 10 55 8b 6c 24 1c
  101  SendMessage                      6ad534      5c5410      -  -                            83 ec 10 53 8b 5c 24 1c
  102  SendMessageToTeam                6ad520      5c57c0      -  -                            83 ec 0c 55 8b 6c 24 18
  103  MessageArgCount                  6ad510      5c5540      -  -                            51 56 8b 74 24 0c 8d 44
  104  MessageGetArg                    6ad500      5c55c0      1  MessageGetArg                51 56 8b 74 24 0c 57 8b
  105  SendMessageToSquad               6ad4ec      5c5930      -  -                            83 ec 10 55 8b 6c 24 1c
  106  A_SendMessageToSquad             6ad4d4      5c5660      -  -                            83 ec 10 55 8b 6c 24 1c
  107  Year                             6918e4      5c5040      -  -                            51 56 8b 74 24 0c 8d 44
  108  Month                            6918dc      5c4490      -  -                            51 56 8b 74 24 0c 8d 44
  109  Day                              6918d8      5c4570      -  -                            51 56 8b 74 24 0c 8d 44
  110  Hour                             6918d0      5c4650      -  -                            51 56 8b 74 24 0c 8d 44
  111  Minute                           6918c8      5c4730      -  -                            51 56 8b 74 24 0c 8d 44
  112  Second                           6918c0      5c4810      -  -                            51 55 8b 6c 24 0c 8d 44
  113  GetTimeStamp                     6ad4c4      5c4910      -  -                            8b 0d 90 bd 70 00 83 ec
  114  YearsPassed                      6ad4b8      5c49e0      1  YearsPassed                  83 ec 08 55 8b 6c 24 14
  115  MonthsPassed                     6ad4a8      5c4af0      1  MonthsPassed                 83 ec 08 55 8b 6c 24 14
  116  DaysPassed                       6ad49c      5c4c00      1  DaysPassed                   83 ec 08 55 8b 6c 24 14
  117  HoursPassed                      6ad490      5c4d10      1  HoursPassed                  83 ec 08 55 8b 6c 24 14
  118  MinutesPassed                    6ad480      5c4e20      1  MinutesPassed                83 ec 08 55 8b 6c 24 14
  119  SecondsPassed                    6ad470      5c4f30      1  SecondsPassed                83 ec 08 55 8b 6c 24 14
  120  DialogueCall                     6ad460      5c3c30      2  DialogueCall                 81 ec 48 02 00 00 a1 c0
  121  RegisterLinkToOtherDialogue      6ad444      5c3f20      2  RegisterLinkToOtherDialogue  81 ec 44 02 00 00 a1 c0
  122  RegisterNodeText                 6ad430      5c3980      1  RegisterNodeText             8b 44 24 04 56 8b 74 24
  123  RegisterReply                    6ad420      5c4060      4  RegisterReply                81 ec bc 00 00 00 a1 c0
  124  SetNodeTextFunction              6ad40c      5c41b0      1  SetNodeTextFunction          83 ec 60 a1 c0 62 70 00
  125  SetCurrentNodeName               6ad3f8      5c42a0      1  SetCurrentNodeName           81 ec 24 01 00 00 a1 c0
  126  WasLastNodeName                  6ad3e8      5c4390      1  WasLastNodeName              81 ec 24 01 00 00 a1 c0
  127  IsInDialogue                     6ad3d8      5c39d0      1  IsInDialogue                 51 56 8b 74 24 0c 57 8b
  128  PCSpeaker                        6ad3cc      5c3a50      0  PCSpeaker                    51 8b 44 24 0c 56 8b 74
  129  SetTokenStrref                   6ad3bc      5c3ae0      2  SetTokenStrref               83 ec 48 a1 c0 62 70 00
  130  SetTokenInt                      6ad3b0      5c3b80      2  SetTokenInt                  83 ec 68 a1 c0 62 70 00
  131  RandomNumber                     6ad3a0      5c3870      2  RandomNumber                 8b 44 24 04 56 8b 74 24
  132  RandomNumberFloat                6ad38c      5c38b0      2  RandomNumberFloat            83 ec 0c 56 8b 74 24 14
  133  Debug                            69cb18      5c3710      -  -                            56 8b 74 24 0c 57 8b 7c
  134  PointDistance                    6ad37c      5c3780      2  PointDistance                83 ec 18 56 8b 74 24 20
  135  Breakpoint                       6ad370      5c33a0      0  Breakpoint                   8b 44 24 08 83 ec 08 56
  136  AttachVEG                        6ad364      5c29b0      2  AttachVEG                    81 ec 0c 01 00 00 a1 c0
  137  RemoveVEG                        6ad358      5c2a80      2  RemoveVEG                    51 56 8b 74 24 10 57 8b
  138  GetCameraFog                     6ad348      5c2bb0      5  GetCameraFog                 83 ec 24 56 8b 74 24 2c
  139  SetCameraFog                     6ad338      5c2d20      5  SetCameraFog                 83 ec 1c 56 8b 74 24 28
  140  ResetCameraFog                   6ad328      5c2eb0      0  ResetCameraFog               51 8b 44 24 0c 56 8b 74
  141  GetHeightFog                     6ad318      5c2f40      5  GetHeightFog                 83 ec 24 56 8b 74 24 2c
  142  SetHeightFog                     6ad308      5c30b0      5  SetHeightFog                 83 ec 20 56 8b 74 24 2c
  143  ResetHeightFog                   6ad2f8      5c3260      0  ResetHeightFog               51 8b 44 24 0c 56 8b 74
  144  GetVEGID                         6ad2ec      5c2af0      2  GetVEGID                     81 ec 0c 01 00 00 a1 c0
  145  AttachSound                      6ad2e0      5c2860      3  AttachSound                  81 ec 0c 01 00 00 a1 c0
  146  RemoveSound                      6ad2d4      5c2940      2  RemoveSound                  51 56 8b 74 24 10 57 8b
  147  FloatingSpeechText               6ad2c0      5c25e0      6  AddFloatingSpeechText        83 ec 28 a1 c0 62 70 00   <<< gate error-name != table name
  148  A_FloatingSpeechText             6ad2a8      5c2710      6  A_AddFloatingSpeechText      83 ec 28 a1 c0 62 70 00   <<< gate error-name != table name
  149  FloatingDamageText               6ad294      5c2500      5  AddFloatingDamageText        81 ec 88 00 00 00 a1 c0   <<< gate error-name != table name
  150  SetEncyclopediaData              6ad280      5c2450      3  SetEncyclopediaData          81 ec 08 01 00 00 a1 c0
  151  SetWorldMapAreaVisible           6ad268      5c2240      3  WorldMapSetAreaVisible       81 ec 08 01 00 00 a1 c0   <<< gate error-name != table name
  152  SetWorldMapAreaExplorable        6ad24c      5c22f0      3  WorldMapSetAreaExplorable    81 ec 08 01 00 00 a1 c0   <<< gate error-name != table name
  153  SetWorldMapAreaExplored          6ad234      5c23a0      3  WorldMapSetAreaExplored      81 ec 08 01 00 00 a1 c0   <<< gate error-name != table name
  154  StartStore                       6ad228      5c21a0      1  StartStore                   51 53 8b 5c 24 10 56 8b

## float lib (off_7004C0) @ 0x7004c0
entries=9  terminator_{0,0}_found=True  distinct_fns=9
  idx  name           name_va       fn_va  first_bytes
  0  float           673958      5ce5f0  8b 44 24 08 db 40 04 d9
  1  floatstr        6b7988      5ce610  8b 44 24 08 8b 48 04 8b
  2  floatmul        6b797c      5ce650  8b 44 24 08 8b 48 08 51
  3  floatdiv        6b7970      5ce680  8b 44 24 08 8b 48 04 51
  4  floatadd        6b7964      5ce6b0  8b 44 24 08 8b 48 08 51
  5  floatsub        6b7958      5ce6e0  8b 44 24 08 8b 48 04 51
  6  floatfract      6b794c      5ce710  8b 44 24 08 8b 48 04 51
  7  floatround      6b7940      5ce740  8b 44 24 08 8b 48 04 51
  8  floatcmp        6b7934      5ce7c0  8b 44 24 08 8b 48 04 51

```

### 7.3 `--audit` against `src/F3/ScriptNatives.cpp`

```text
# native-table-dump
exe          : /nix/store/a1j412798wh979ig7xp3bp9aji4zfpz0-f3demo-1.0/share/f3demo/F3.exe
sha256       : 4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87
pinned sha256: 4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87
file size    : 3194880 bytes
image base   : 0x400000
sections     : .text@0x401000/0x245c52, .rdata@0x647000/0xa9790, .data@0x6f1000/0x200e0, .data1@0x712000/0x8e0, .rsrc@0x713000/0x2728

## F3 game lib (off_6F3750) @ 0x6f3750
entries=45  terminator_{0,0}_found=True  distinct_fns=42
  idx  name                    name_va       fn_va  cells  gate-name            first_bytes
   0  A_Attack                 65fa80      459eb0      2  Attack               83 ec 08 56 8b 74 24 10   <<< gate error-name != table name
   1  A_JustAttack             65fa70      459f70      2  Attack               83 ec 08 56 8b 74 24 10   <<< gate error-name != table name
   2  StartDialogue            65d8ac      45a040      2  StartDialogue        83 ec 08 56 8b 74 24 10
   3  DisplayStringRef         65db04      45a410      4  DisplayStringRef     83 ec 24 a1 c0 62 70 00
   4  HideInterfaces           65d8bc      45a100      0  HideInterfaces       8b 44 24 08 8b 4c 24 04
   5  SetMoveMode              65d8cc      45a140      3  SetMoveMode          51 56 8b 74 24 10 57 8b
   6  EquipItem                65dbf4      45a580      3  EquipItem            81 ec a4 00 00 00 a1 c0
   7  UnequipItem              65fa64      45a6f0      3  EquipItem            81 ec c0 00 00 00 a1 c0   <<< gate error-name != table name
   8  LockDoor                 65d928      45a1b0      1  LockDoor             51 56 8b 74 24 0c 57 8b
   9  UnlockDoor               65d984      45a240      1  UnlockDoor           51 56 8b 74 24 0c 57 8b
  10  AddCNPCToParty           65d9f0      45a2d0      1  AddCNPCToParty       51 56 8b 74 24 10 57 8b
  11  A_ReloadWeapon           65fa54      45a380      1  ReloadWeapon         51 56 8b 74 24 0c 57 8b   <<< gate error-name != table name
  12  Protagonist              65e198      45a960      0  Protagonist          8b 44 24 08 56 8b 74 24
  13  GetWeaponCombo           65e1e0      45a9f0      1  GetWeaponCombo       51 56 8b 74 24 0c 57 8b
  14  SetWeaponCombo           65e22c      45aa40      2  SetWeaponCombo       51 56 8b 74 24 0c 57 8b
  15  GetSquadAttitude         65e23c      45aa90      2  GetSquadAttitude     8b 44 24 04 56 8b 74 24
  16  GetTeamAttitude          65e250      45aaf0      2  GetTeamAttitude      8b 44 24 04 56 8b 74 24
  17  GetPerkRank              65e2d8      45ab50      2  GetPerkRank          51 56 8b 74 24 10 57 8b
  18  GetLevel                 65e358      45abe0      1  GetLevel             51 56 8b 74 24 0c 57 8b
  19  GetPrimaryAttribute      65e3ec      45ac70      2  GetPrimaryAttribute  51 56 8b 74 24 10 57 8b
  20  GetDerivedAttribute      65e484      45ad00      2  GetDerivedAttribute  51 56 8b 74 24 10 57 8b
  21  GetBaseSkill             65e510      45ad90      2  GetBaseSkill         51 56 8b 74 24 10 57 8b
  22  GetTotalSkill            65e598      45ae20      2  GetTotalSkill        51 56 8b 74 24 10 57 8b
  23  GetRace                  65e614      45aeb0      1  GetRace              51 56 8b 74 24 0c 57 8b
  24  GetGender                65e68c      45af40      1  GetGender            51 56 8b 74 24 0c 57 8b
  25  GetEthnicity             65e710      45afd0      1  GetEthnicity         51 56 8b 74 24 0c 57 8b
  26  GetSubRace               65e794      45b070      1  GetSubRace           51 56 8b 74 24 0c 57 8b
  27  GetMaxHP                 65e80c      45b110      1  GetMaxHP             51 56 8b 74 24 0c 57 8b
  28  GetCurrentHP             65e88c      45b1b0      1  GetCurrentHP         51 56 8b 74 24 0c 57 8b
  29  GetWeaponRange           65ebfc      45b620      1  GetWeaponRange       51 53 8b 5c 24 0c 56 8b
  30  GetItemType              65e8f8      45b240      1  GetItemType          51 56 8b 74 24 0c 57 8b
  31  GetUsableObjectType      65ea58      45b440      1  GetUsableObjectType  51 56 8b 74 24 0c 57 8b
  32  SetHP                    65e96c      45b2e0      2  SetHP                51 56 8b 74 24 10 57 8b
  33  AddHP                    65e9dc      45b390      2  AddHP                51 56 8b 74 24 10 57 8b
  34  IsLocked                 65eabc      45b4e0      1  IsLocked             51 56 8b 74 24 0c 57 8b
  35  IsItemEquipped           65ecf0      45b750      2  IsItemEquipped       81 ec bc 00 00 00 a1 c0
  36  GetLockDC                65eb38      45b570      1  GetLockDC            51 56 8b 74 24 10 57 8b
  37  PlayMusic                65f5d8      45b9c0      1  PlayMusic            8b 44 24 08 56 8b 74 24
  38  OpenTask                 65f134      45b900      2  OpenTask             8b 44 24 04 56 8b 74 24
  39  CloseTask                65f140      45b940      2  CloseTask            8b 44 24 04 56 8b 74 24
  40  AddEntry                 65f14c      45b980      2  AddEntry             8b 44 24 04 56 8b 74 24
  41  OpenQuest                65fa48      420990      -  -                    b8 01 00 00 00 c3 90 90
  42  CloseQuest               65fa3c      420990      -  -                    b8 01 00 00 00 c3 90 90
  43  AddQuestEntry            65fa2c      420990      -  -                    b8 01 00 00 00 c3 90 90
  44  AddJournalEntry          65fa1c      420990      -  -                    b8 01 00 00 00 c3 90 90
  alias groups (1 fn claimed by >1 name):
    0x420990 <- OpenQuest, CloseQuest, AddQuestEntry, AddJournalEntry

## audit: port (ScriptNatives.cpp) vs F3 game lib (off_6F3750)
port (ScriptNatives.cpp) rows=36  F3 game lib (off_6F3750) entries=45  exact name matches=36  F3 game lib (off_6F3750) distinct fn=42

### port (ScriptNatives.cpp)-only names - registered by the port, absent in shipped: 0
  (none)

### F3 game lib (off_6F3750)-only names - shipped registers it, the port (ScriptNatives.cpp) table does not: 9
  - A_Attack  (fn 0x459eb0, idx 0, bytes 83 ec 08 56 8b 74 24 10)
  - A_JustAttack  (fn 0x459f70, idx 1, bytes 83 ec 08 56 8b 74 24 10)
  - A_ReloadWeapon  (fn 0x45a380, idx 11, bytes 51 56 8b 74 24 0c 57 8b)
  - DisplayStringRef  (fn 0x45a410, idx 3, bytes 83 ec 24 a1 c0 62 70 00)
  - GetSquadAttitude  (fn 0x45aa90, idx 15, bytes 8b 44 24 04 56 8b 74 24)
  - GetTeamAttitude  (fn 0x45aaf0, idx 16, bytes 8b 44 24 04 56 8b 74 24)
  - HideInterfaces  (fn 0x45a100, idx 4, bytes 8b 44 24 08 8b 4c 24 04)
  - SetMoveMode  (fn 0x45a140, idx 5, bytes 51 56 8b 74 24 10 57 8b)
  - StartDialogue  (fn 0x45a040, idx 2, bytes 83 ec 08 56 8b 74 24 10)

### case/spelling drift: 0
  (none)

### port (ScriptNatives.cpp) EA comment disagrees with the shipped table's fn value: 0
  (none)

### duplicate names in the port (ScriptNatives.cpp) table: 0
  (none)
### duplicate names in the F3 game lib (off_6F3750) table: 0
  (none)

### port (ScriptNatives.cpp) names sharing one implementation symbol: 1
  - ScriptNative_QuestJournalAlias <- AddJournalEntry, AddQuestEntry, CloseQuest, OpenQuest  [shipped fns: 0x420990]

### port (ScriptNatives.cpp) rows with cells < 0 (arity claim is 'body has no gate'): 4
  - AddJournalEntry
  - AddQuestEntry
  - CloseQuest
  - OpenQuest

### shipped arity-gate claims vs port (ScriptNatives.cpp) cells column: 0 mismatches
  (none)

### shipped natives whose arity-ERROR NAME differs from their table name: 4
  - table 'A_Attack' -> gate prints 'Attack' (fn 0x459eb0)
  - table 'A_JustAttack' -> gate prints 'Attack' (fn 0x459f70)
  - table 'A_ReloadWeapon' -> gate prints 'ReloadWeapon' (fn 0x45a380)
  - table 'UnequipItem' -> gate prints 'EquipItem' (fn 0x45a6f0)
### shipped natives with NO arity gate at all: 4
  - AddJournalEntry
  - AddQuestEntry
  - CloseQuest
  - OpenQuest

```
