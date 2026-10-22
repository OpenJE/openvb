# F3 Startup & Subsystem Slot Table

[Return to `F3`](/docs/f3.md)

Reverse-engineering notes from the original `F3.exe` (database `artifacts/F3.exe.i64`).

## The slot table (original .data 0x6FF6C8 .. 0x6FF70C)

Eight interleaved `{factory, object}` pairs, stride 8 (renamed in the IDB as
`F3_subsystem_slot_table_0x6ff6c8`, end marker `F3_gamestate_loop_array_end_0x6ff70c`):

| slot | factory (original) | reimplementation initializer | constructed object |
|------|--------------------|------------------------------|--------------------|
| 0 | 0x5B3270 | `InitializeMemorySystem` | `JE::MemorySystem` |
| 1 | 0x45E110 | `InitializeResources` (inferred) | `JE::Resources` |
| 2 | 0x45E1C0 | `InitializeScripting` (inferred) | `JE::Scripting` |
| 3 | 0x45E330 | `InitializeStrings` | `JE::Strings` (4-byte object: vftptr only) |
| 4 | 0x45E170 | `InitializeNetwork` (inferred) | `JE::Network` |
| 5 | 0x45E270 | `InitializeModelCache` | `JE::ModelCache` singleton |
| 6 | 0x45E2D0 | `InitializeGameAnimationCache` (inferred) | `F3::GameAnimationCache` |
| 7 | 0x45E210 | `InitializeGameWorld` | `F3::GameWorld` (0x39C bytes) |

Factory addresses for slots 1/2/4/6 are read verbatim from the .data dump; the class mapping is
inferred and each is confirmed by matching its ctor when its subsystem is reconstructed.

## The drivers (uniform virtual-interface contract)

- `F3_Startup_ConstructSubsystems_0x5ACD60`: for each pair — `if (factory) { obj = factory();
  store obj into the object slot (0x5ACD7B); call obj->vft[1] (0x5ACD82); }`
- `F3_GameStateLoop_TickSubsystems_0x5ACDE0`: per frame — for object slots `0x6FF6CC..0x6FF708`
  step 8: `call obj->vft[3]` (0x5ACDEA, unguarded).
- `F3_Shutdown_TerminateSubsystems_0x5ACDA0`: reverse walk — `call obj->vft[2]` (0x5ACDB8), then
  `obj->vft[0](obj, 1)` scalar deleting destructor (0x5ACDC6), clear the slot (0x5ACDC8).

=> every subsystem object shares a vtable prefix:
`vft[0]` = scalar deleting destructor, `vft[1]` = Initialize, `vft[2]` = Terminate, `vft[3]` = Tick.
The reimplementation models this as `JE::Subsystem`
(`modules/openje/src/JE/Subsystem.hpp/.cpp`); all eight initializers return subclasses of it.

Observed per-class slot targets so far: `JE::MemorySystem` vft[1]=0x5B31D0, vft[2]=0x5B3870,
vft[3]=0x5B3DB0; `JE::World` vft[3]=0x545D60 (server tick; drives the path steerer, see
[`GamePathManager`](/docs/f3/GamePathManager.md)).

## The GameWorld subsystem cascade (JE::World::Initialize_0x546690)

`World::Initialize` (GameWorld vft[1], run by the generic loop after all 8 constructors) is the
composition root of the game:

1. `vft[4]` -> `JE::World::InitInterfaces_0x5460D0` via the secondary-base table at `*(world+8)+8`
   (result -> `world+0x3C`) - GUI interface factory table (dialogue/console/menus).
2. Parses **globals.ini** into the world-owned parameter tree at `world+0x90` and **locals.ini**
   into `world+0xB8` (both ftype 800 in Scripts.grp; `LoadIni_0x5A9AB0` wrapper; the engine tree
   at 0x70C2C0 only carries sys.ini).
3. Factory slots, results stored on the world object:
   | slot | factory | object | size | world field | ctor |
   |---|---|---|---|---|---|
   | [7]  | CreatePathManager_0x46f9e0 | F3::GamePathManager | 0x10 | +0xF0  | 0x45BA40 (RE'd this session) |
   | [8]  | CreateParty_0x46f820   | F3::GameParty       | 0x18 | +0xF4  | 0x46CD00 |
   | [9]  | CreateJournal_0x46f880 | F3::GameJournal     | 0x1C | +0xF8  | 0x46C410 |
   | [10] | CreateGameEncyclopedia_0x46f8e0 | JE::Encyclopedia->GameEncyclopedia vft 0x668430 | 0x10 | +0xFC | 0x58F2F0 |
   | [11] | CreateGameStoreManager_0x46f930 | JE::StoreManager->GameStoreManager vft 0x668448 | 0x10 | +0x100 | 0x5A1FB0 |
   | [12] | CreateGameWorldMap_0x46f980     | JE::WorldMap->GameWorldMap vft 0x668480         | 0x80 | +0x104 | 0x599F90 |
4. `[World] Year/Month` read from the tree -> game calendar (`JE::GameTime`-ish, deferred).
5. Engine `World::CreateEntity` dispatch -> `F3::CreateGameEntityByClass_0x46f530` (secondary
   GameWorld vft 0x6684AC): the 11-class entity factory (sizes/ctors in docs/DEFERRED-NOTES.md).

All six factory slots + the two ini parses are now WIRED in the reimplementation
(`src/F3/GameWorld.cpp` `GameWorld::Initialize`, values stored per original offsets; class ctors
themselves remain traced stubs until their own RE passes). Boot-verified: 47k trace events, zero
faults.

## Main flow (`F3::Main` 0x5ACE00)

```
Startup()                       // construct + Initialize all slots
RegisterCommand("quit", Quit)   // into the static tree at 0x70C668
while (ProcessMessagesAndUpdateTime())   // 0x56AF60; false on WM_QUIT (msg == 18)
    GameStateLoop()                    // Tick every subsystem
Shutdown()                      // Terminate + delete in reverse order
```

## Command registry (`JE::cls_0x4d8d70` @ 0x70C668)

MSVC4 `_Tree` control block (0x38 bytes; count at +0x08; header/sentinel node pointer at +0x04,
aliased `byte_70C66C`). Case-insensitive (`_stricmp`) `std::string` -> handler map. Nodes are
0x30 bytes from `buynode_0x59EC40`:

```
+0x00 _Left  +0x04 _Parent  +0x08 _Right
+0x10 key std::string {inline buffer|heap ptr, +0x20 size, +0x24 capacity}
+0x28 value (handler fn ptr)   +0x2C _Color   +0x2D _Isnil
```

Entry points: `cls_0x4d8d70_Meth_LowerBound_0x545710`, `Meth_InsertOrGetCore_0x59f3e0` +
`Meth_RbInsertAtBottom_0x59f200` (rotations at 0x583930/0x4C8E50), wrapper
`Meth_InsertOrGet_0x59f540` (returns `node+0x28`), lookup `Meth_FindValue_0x5ace40`,
caller `F3_RegisterCommand_0x59f5d0` (duplicate command ->
`FatalError("Command %s already registered.")`). Static ctor via `GLOBAL__I` 0x643F60,
destructor registered with `_atexit` thunk 0x646B90 -> `meth_0x59f4d0`.

Reconstructed in `modules/openje/src/JE/cls_0x4d8d70.{hpp,cpp}`; the reconstruction performs plain
BST insertion without the red-black fixup (documented deviation: lookup results and value slots are
identical, only internal shape differs). Unit test:
`modules/openje/tests/je/cls_0x4d8d70/` (built with i686-wingw g++, run under wine — all assertions
pass, including SSO/heap key paths and case-insensitive find).

## Strings on the wire (`cls_0x50db20` tempString shape)

MSVC4 `basic_string` at object offset +4: `{ inline buffer (16B) | heap pointer @+4, size @+0x14,
capacity @+0x18 }` (evidence: `cls_0x50db20_StringAssign_0x401bd0` and the lower-bound walk).
`RegisterCommand` initializes capacity 15 (SSO) then assigns; frees the heap buffer only when
capacity >= 0x10 (0x59F6BD).

[Return to `F3`](/docs/f3.md)

---

## T-079 batch-4 annotation (2026-10-22) — ST1-ST3

Per `discoveries/2025-10-21-t072-doc-errata-sweep.md` §4.

- **ST1 — label currency**: `JE::cls_0x4d8d70` is minted **`JE::StringTree`** (T-033 batch-3 `_Tree`
  family proof CONFIRMED; T-059's amendment left the mint standing). Keep item-43's caution where
  this page's name appears in pad contexts: the 2,062-pad "owner" reading was RETRACTED — 1,958 pads
  target ONE function, `0x4B2DD0`, the app-wide std::string SSO dtor wearing a stale
  `JE::StringTree::meth_*` live name (split-comment queue).
- **ST2 — class attribution REFUTED, layout claims survive**: the "`cls_0x50db20` tempString shape =
  JE::String" aggregate is batch-1 REFUTED — re-checked at apply against the frozen export's member
  table: `cls_0x50db20` embeds `.?AVG3D_Water@@` (180B) at +0x20 and `.?AVVFX_Parameter@@` at +0x1c
  (size 577, 109 members), which no string class is. T-057 re-confirmed both single-embedders land
  on it. The MSVC4 STRING LAYOUT mechanics cited on the page (0x401bd0 assign shape, SSO cap 15,
  free-at-`>=0x10`) are independently evidenced and stand — as layout knowledge, not as this
  class's identity.
- **ST3 — superseded cascade item**: "[World] Year/Month -> JE::GameTime-ish, deferred" is now
  specific: `JE::Gregorian` (vft ladder 0x69166C, size 0x38) lives at `World+0x3C`, fields read by
  `0x546b50` into `+0x14..+0x28`/`+0x40` as Initialize's FIRST statement (`0x5466b0` -> primary slot
  5), and the clock ctor seeds via a 24-byte `rep movsl` block copy (`0x54497d`, `ecx=6`). The
  `?: 2253` fallback was proven a port bug and FIXED under T-042 (absent-key = ctor `0,1,1,0,0,0`;
  authored-zero reachable; StringSystem suite asserts the order) — ported and tested, no longer deferred.
