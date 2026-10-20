# GUI loading recon — `.int` interface files (T-003, read-only)

Author: implementer-B (2026-10-20). Target: **ORIGINAL_REFERENCE** (`ida_original_f3`,
`artifacts/F3.exe.i64`, exe `$OPENVB_ORIG_EXE`) + the shipping data files. No IDB edits, no
source edits. `0x…` = original F3.exe VA; unverified claims marked *(inferred)*.

## 0. Headline: the demo has a complete data-driven GUI loader

```
F3::GameClient::virt_meth_0x45e8e0                       (subsystem Initialize body)
 -> F3::GameInterfaces::ctor_0x4398e0                    (constructs all interface objects, 0xb3d)
    -> <per-interface ctor>, e.g. cls_0x440990::ctor_0x440990 @0x4399bf   ("CMainmenuInterface")
       -> JE::String::Assign("Mainmenu.INT")             0x440a35
       -> JE::CLoadMapInterface::LoadInterface_0x495b50  0x440a4b   <-- the .int loader
       -> JE::String::Assign("MainmenuWin")              0x440a9a
       -> JE::CLoadMapInterface::FindWidget_0x495740     0x440ab5   (result cached at obj+4)
```

`LoadInterface_0x495b50` + `FindWidget_0x495740` are shared base helpers (0x495xxx range) that
every interface ctor calls; widgets are addressed **by name** from F3 code.

## 1. `Skin::LoadDefault_0x523490` — task premise corrected

It does **not** demand `.int` files; it parses a **key→value parameter block** into a skin-default
record. Caller `JE::GUI::Skin::meth_0x522ac0` loops a record list and calls
`LoadDefault(record+0x28)` at 0x522ae8; reached via Skin vft `0x66f2dc` slot1 `0x522b10` /
slot2 `0x522c30`. Decompiler fails (confirmed) — decoded from disassembly. Accessors:
`sub_433A20(key)` find-entry, `sub_433930(entry,key)` verify-match, then `atol`/assign.

| key | EAs | stored | default when key absent |
|-----|-----|--------|-------------------------|
| `flags` | 0x5234a7–0x523568 | record+0x00 (atol) | 0 |
| `texture` | 0x52356b–0x5236d7 | record+0x04 (string) | skips the demand |
| `color` | 0x5236fb–0x523835 | record+0x08 | `0xFF808080` @0x523700 |
| `width` | 0x523835–0x523931 | record+0x0C (atol) | 8, else `parent[+0x24]→[+0x1C]` @0x523917 |
| `height` | 0x523938–0x523a13 | record+0x10 (atol) | 8, else `parent[+0x24]→[+0x20]` @0x523a08 |
| `rate` | 0x523a1a–0x523ad8 | record+0x14 (atol) | 0 |

`color` = 4 components split on `",\n"` (`asc_677A40`, tokenizer `sub_61A7D1` @0x5237cb/0x5237e2),
each `atol`, packed @0x523806–0x523832, and **stored only if exactly 4 parts** (`cmp esi,4`
@0x5237d7/@0x5237fc). Resource touchpoints: `Demand_0x49b750(global_resource_system, name, 1)`
@**0x523665** (name = record+0x28 string; SSO pointer select 0x523649–0x52365c) with
`Release_0x498eb0` @**0x5236ce**; plus three vtable calls on global `byte_707CE4`:
`vft[+0xDC](name)` @0x52363b (probe — nonzero skips the demand), `vft[+0xC4](name)` @0x5236b4
(index; `<0` forces 0 @0x5236c5), `vft[+0xD8](name)` @0x5236e7. Class unknown → **G1**.

## 2. `.int` header, proven from the files

RHT (`resource.rht` v1.1, 7044 entries): ftype **1600 = 25 entries, all in `Interface.grp`**
(453 lumps, vMajor=2 vMinor=1). Loader-side evidence for each field below; the table is a direct
walk of all 25 lumps (u32 magic, u32 version, u32 nameLen+name, u8 flag, u32 nRoot, then the root
record: u32 type, 16-byte rect, u32 nameLen+name).

| off | field | code evidence |
|-----|-------|---------------|
| +0 | u32 magic `'GUI '` = 0x20495547 | read 0x495bbd, cmp 541676871 @0x495bca |
| +4 | u32 version `V1.0`..`V1.3` → internal `a2`=1..4 | switch @0x495be8 (0x302e3156/0x312e3156/0x322e3156/0x332e3156) |
| — | magic absent → `FileStreamSeek_0x498800(s,0,0)`, `a2=0` (legacy) | 0x495c30 |
| +8 | u32 len + bytes (own filename) → obj+4 | `sub_50B880` @0x50b880, called @0x495c3a |
| +n | u8 flag → obj+32 ; u32 nRoot | both reads @0x495c78 |
| +n | per root: u32 type → factory `0x4b2910` → `vft[+28](stream,a2)` → add | 0x495cae / 0x495cca / 0x495cd6 / 0x495ce4 |

| file (lump) | ver a2 | flag | root type | root widget | rect | nRoot |
|---|---|---|---|---|---|---|
| Debug (78) | V1.2 3 | 0 | 0 Window | `DebugWin` | 130,140,583,171 | 1 |
| EscapeMenu (79) | V1.3 4 | 0 | 0 | `EscapeMenuWin` | 344,160,440,256 | 1 |
| Mainmenu (80) | V1.3 4 | 0 | 0 | `MainmenuWin` | 0,0,1024,768 | 1 |
| CharacterGeneration (88) | V1.3 4 | 0 | 0 | `winCharGen` | 0,0,800,600 | 1 |
| DebugTools (104) | V1.2 3 | 0 | 0 | `DebugToolsWin` | 228,18,424,450 | 1 |
| MessageBox (116) | V1.3 4 | 0 | 0 | `winMessageBox` | 0,0,386,162 | 1 |
| CharGen_NAS (124) | V1.3 4 | 0 | 0 | `winNAS` | 8,36,539,128 | 1 |
| Inventory (139) | V1.3 4 | 0 | 0 | `winInventory` | 0,0,96,96 | 1 |
| AreaMap (140) | V1.2 3 | 0 | 0 | `winAreaMap` | 0,0,1024,614 | 1 |
| Actionbar (149) | V1.3 4 | 0 | 0 | `winActionbar` | 0,0,96,96 | 1 |
| Dialog (155) | V1.2 3 | 0 | 0 | `SpeakerWin` | 152,140,532,260 | **2** |
| Feedback (156) | V1.2 3 | 0 | 0 | `FeedbackWin` | 130,140,581,164 | 1 |
| PipBoy (162) | V1.3 4 | 0 | **2 Picture** | `picPipBoy` | 0,-1,1024,609 | 1 |
| Loading (167) | **V1.1 2** | 0 | 0 | `LoadingWin` | 232,184,783,476 | 1 |
| MiniMap (178) | V1.3 4 | 0 | 0 | `winMiniMap` | 10,620,290,752 | 1 |
| AreaMapNote (179) | V1.3 4 | 0 | 0 | `winAreaMapNote` | 100,100,294,214 | 1 |
| AddNote (191) | V1.3 4 | 0 | 0 | `AddNoteWin` | 28,120,344,264 | 1 |
| Options (192) | V1.3 4 | 0 | 0 | `winOptions` | 0,0,96,96 | 1 |
| Options_Game (199) | V1.3 4 | 0 | 0 | `Window` | 0,0,96,96 | 1 |
| Options_Video (200) | V1.3 4 | 0 | 0 | `Window` | 0,0,96,96 | 1 |
| Options_Multiplayer (201) | V1.3 4 | 0 | 0 | `Window` | 0,0,96,96 | 1 |
| Options_Hotkeys (202) | V1.3 4 | 0 | 0 | `Window` | 0,0,96,96 | 1 |
| Options_Load (203) | V1.3 4 | 0 | 0 | `Window` | 0,0,96,96 | 1 |
| Container (249) | V1.3 4 | 0 | 0 | `Main` | 0,0,256,176 | 1 |
| EditToken (451) | V1.3 4 | 0 | 0 | `Main` | 0,0,298,166 | 1 |

All 25 are flag=0; versions 1.1/1.2/1.3 all shipped → `a2` gates extra fields (below), so a
reader must honour 2/3/4. Only `Dialog` has 2 roots, only `PipBoy` roots at a Picture.

**Widget record** (`Window::ReadFrom_0x4b10f0`; the sibling `ReadFrom_0x4b0e10` has an identical
field order using `fread`+`sub_50B7F0` instead of `FileStreamRead`+`sub_50B880`):

```
u32 type                                       (parent's loop)
16 bytes rect -> Base+0x08, Base+4=1           [Base::ReadFrom_0x4b2850 @0x4b2864]
str name      -> Window+0x1C                   @0x4b116d
u8 u8 u8      -> +0x38 +0x39 +0x3A             @0x4b117f..0x4b118f
str           -> +0x40 ; u32 -> +0x5C
a2>=2: u8 +0x60 ; u32 +0x64 (+ Base::slot2(this,+0x64))
a2< 2: if high byte(+0x5C)!=0xFF -> +0x64=1    @0x4b11b0
16 bytes      -> +0x74                         @0x4b11c6
a2>=3: str +0xCB8, u32 +0xCD4, str +0xCD8, u32 +0xCF4, str +0xCF8   @0x4b1285
9 x skin sub-record, inline stride 86 from +0x90, each via its vft+28  @0x4b1297/0x4b12af
child-link table at +0xCAC, 28-byte entries {str,u32 next}, -1 terminates  @0x4b12f2
u32 nChildren; nChildren x { u32 type, record(vft+28) }, added via Base::slot3  @0x4b13a0/0x4b13bc
```
NOT `GUI::Skin`: those 9 inline records read through `vft+28` but Skin's vft `0x66f2dc` has 4
slots (max +0x0C) → different class, 86-byte inline array. **G2**.

## 3. Factory + per-class read slots

`GUI_CreateFragment_0x4b2910(type)` (stdcall, 0x15d) is the only factory; called from the `.int`
loader (0x495cca), `ReadFrom_0x4b0e10` (0x4b10a5) and `ReadFrom_0x4b10f0` (0x4b139b).

| type | class | alloc | ctor |
|---|---|---|---|
| 0 | `GUI::Window` | 0xD1C | `ctor_0x4b0810(this,0)` |
| 1 | `GUI::Button` | 0xD78 | `ctor_0x4a6ed0(this,1)` |
| 2 | `GUI::Picture` | 0xD2C | `ctor_0x4aa8b0(this,2)` |
| 3 | `GUI::Label` | 0xE64 | `ctor_0x4a90e0(this,3)` |
| 4 | `GUI::Edit` | 0x103C | `ctor_0x4c51b0(this,4)` |
| other | returns NULL → loader aborts, returns 0 | | |

`Fragment::ctor_0x4b24a0` is not in the factory: its only xref is a data ref @0x4b08aa inside
`Window::ctor_0x4b0810` (sub-object). Read slots (from `artifacts/F3.json` vftables):

| class | vft (len) | slot7 +0x1C (used by `.int` loader) | slot8 +0x20 (child loop variant) |
|---|---|---|---|
| `GUI::Base` | 0x66e41c (11) | 0x4b2850 (rect only) | 0x4b2800 |
| `GUI::Fragment` | 0x66ea1c (17) | 0x4b20a0 | 0x4b1e50 |
| `GUI::Window` | 0x66e4d4 (75) | 0x4b10f0 | 0x4b0e10 |
| `GUI::Picture` | 0x66da34 (80) | 0x4ab190 | 0x4ab160 |
| `GUI::Label` | 0x66d43c (106) | 0x4aa640 | 0x4aa3e0 |
| `GUI::Button` | 0x66c9bc (76) | 0x4a6b60 | 0x4a6a10 |

`Fragment::slot7 0x4b20a0` and `slot8 0x4b1e50` each call `Demand_0x49b750` (@0x4b21dd, @0x4b1f88)
— that is where a widget's picture/font resources are demanded. `GUI::Edit` vft 0x670cac (106):
slot7/8 unresolved → **G3**.

## 4. English.stf → widget text

`EngineStringSystem::Get_0x56c260` (vft[1]) is the binding surface:
`StringSystem::Get_0x4c5a30(this,out,inOut)` @0x56c26e then `Format_0x56be30(this,inOut)`
@0x56c276. `Format` decompile fails (confirmed; SEH prologue 0x56be30, 0x118-byte frame). From
disassembly: `String::CopyAssign_0x401b20` snapshot @0x56be89, then a scan for `'<'`(0x3C)
@0x56befd to the matching `'>'`(0x3E) @0x56bf1c, payload extracted via `meth_0x458ef0` @0x56bf40,
loop bound `jbe` @0x56bee8 — i.e. **inline `<tag>` markup processing on the string**, not printf;
no argument substitution inside Format. Tag vocabulary unresolved → **G4**.
Consumers of `Get_0x56c260`: `sub_40EC90` (0x40ee34,0x40efa7),
`JE::CLoadMapInterface::meth_0x4580c0` (0x4581e2 — the 0x64a-byte F3 text setter),
`sub_56C280` (0x56c290 wrapper), `JE::Client::meth_0x58aad0` (0x58ab1d),
`JE::DialogueWindow::virt_meth_0x595010` (0x59532f,0x5957b8). F3 then mutates widgets by name
through their vftables — e.g. `vft[+360]` (colour, seen with literals `-16777088`/`1082163328`)
and `vft[+232]` (enable/grey) around the label blocks at 0x45065b–0x4509bd.

Interface string constant → owning ctor (2 data xrefs each, verified):
`Mainmenu.INT` 0x658190→`cls_0x440990::ctor_0x440990` (vft `0x65816c`) · `Actionbar.INT`
0x650ae0→`cls_0x41a2b0::ctor_0x41a2b0` · `Inventory.INT` 0x657020→`cls_0x43cd50::ctor_0x43cd50` ·
`PipBoy.INT` 0x65bef0→`cls_0x455e40::ctor_0x455e40` · `Dialog.INT` 0x655918→
`F3::GameDialogueInterface::ctor_0x438080` · `MessageBox.INT` 0x658684→`cls_0x441420` ·
`LoadMap.INT` 0x6578dc→`JE::CLoadMapInterface::ctor_0x440050` · `Feedback.INT` 0x654354→
`cls_0x434f70` · `CharGen_NAS.INT` 0x651be4→`cls_0x426720` · `Options_Game.INT` 0x65a27c→
`cls_0x4468c0` · `Options_Hotkeys.INT` 0x65a758→`cls_0x448c40` · `Options_Load.INT` 0x65abac→
`cls_0x4491f0` · `Options_Video.INT` 0x65b21c→`cls_0x4496a0` · `Debug.INT` 0x692f4c→
`cls_0x558740`. Remaining 10 (AreaMap, AreaMapNote, Container, DebugTools, EditToken, EscapeMenu,
Loading, Options, CharacterGeneration, MiniMap): identical recipe, one `xrefs_to` each.

## 5. Things that are NOT the loader (avoid dead ends)
- `GUI::Manager::ctor_0x4b5640` is installed by **static init @0x641665** (no code xref) — same
  pattern as the EngineStringSystem global @0x643565.
- `GUI::Manager::meth_0x4b4920` (from `JE::Graphics::meth_0x581490` @0x581499 and a dtor) is a
  **reset**: frees interface/fragment lists, sets +0x60=1024, +0x64=768, +0x68/+0x6C=1.0f, then
  assigns a literal at 0x64D7D8 into Manager+0x70 (that VA did not resolve to a clean string
  through the section map) → **G5**.
- `.int` files are opened with `FileStreamOpen_0x49b9f0(name,1)` @0x495b9a — **not** `Demand`;
  only skin/fragment textures/fonts use `Demand`.
- `JE::GUI::Interface::meth_0x450230` (0x1413) is per-frame widget *refresh* (labels
  `lblAddNote`/`lblZoomIn`…), not loading; useful later for the update loop.

## 6. Gaps — questions for implementer-A

- **G1** Who is `byte_707CE4` and what are `vft[+0xD8]`/`[+0xDC]`/`[+0xC4]` (texture-name probe /
  index / remove)? Blocks skin texture binding.
- **G2** Class of the 9 inline skin sub-records at Window+0x90 (stride 86, read via vft+28).
- **G3** `GUI::Edit` slot7/slot8 readers; does a type-4 record add fields to §2?
- **G4** Full `<tag>` vocabulary of `Format_0x56be30` (chain after 0x56bf40) — decides what our
  string layer must support before any text renders.
- **G5** Content of the Manager+0x70 literal at 0x64D7D8.
- **G6** `sub_50B7F0` vs `sub_50B880`: confirm both are u32-len+bytes, and why one variant reads
  through libc `fread` (editor/plain-file path?) — affects which stream API we implement first.
- **G7** Naming: `LoadInterface_0x495b50`/`FindWidget_0x495740` are shared base helpers but the IDB
  labels them `JE::CLoadMapInterface::*` (where the vftable landed). What do we call the owner
  class before implementing?
- **G8** Propose the first three implementation tasks: (a) `.int` header+window/label/picture
  record reader producing a widget tree with name lookup (test: Mainmenu → `MainmenuWin` +
  child count), (b) `EngineStringSystem::Format` tag decode + `Get` wiring into Label text,
  (c) skin sub-record decode + `Demand` binding so pictures resolve to real textures.
