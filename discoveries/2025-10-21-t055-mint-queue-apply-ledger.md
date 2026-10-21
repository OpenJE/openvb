# 2025-10-21 — T-055: Apply the T-053 mint-queue + T-054 extension (batch-7-shaped wave, sole writer). T2 23.05%→**22.67%** (−34 in-game); +T-054 extension −11; annotation-only 0-delta.

CalmRaven (queue author promoted to writer), dispatched by LoudDragon `01a15f82`. **SOLE IDB WRITER**
on `ida_original_f3` (8745) this wave (T-033 chain closed at six; this is the b7-shaped successor under
new-evidence rules). EpicRaven analyzed OFFLINE from a fresh dump; the **dump-stale boundary** = my first
write commit `65af589`/claim, relayed by LoudDragon.

## HEADER — canonical sole-storer predicate (now harness item 41 rule 3, verbatim)

> **ctor = sole OBJECT-SIDE vft storer; a `JE::CEngineInterface::meth_*` factory may hold the only other
> xref (slot installation); rival-class ctors = zero.**

Naive "vft has exactly one code xref" would have failed **14/14** candidates — every window-interface vft
has a second xref from a `CEngineInterface` factory that installs the same vft into a managed slot
(`mov [ecx+esi−0C4h], vft`). This is the exact shape b4 minted CActionbarInterface under
("other storer = factory `JE::CEngineInterface::meth_0x416a70`"). Re-verify-then-apply beat prefix-blind
application: caught on the FIRST write, not the first false negative. **T-053's own queue wording
("sole storer") is superseded by this correction.**

## 0. Pre/post state + save discipline

| phase | T2 (cls_) | T3 (sub_) | T1 (named::) | game denom | IDB mtime |
|---|---|---|---|---|---|
| PRE-WRITE baseline | **2,046** = 23.05% | 2,176 | 4,438 | 8,876 | 14:26:04 (== b6 save, disk==live, no erosion) |
| after T-053 queue (checkpoint) | 2,023 | 2,170 | 4,467 | 8,876 | 15:11:11 |
| after T-054 extension (end save) | **2,012** = 22.67% | 2,169 | 4,479 | 8,876 | **15:31:28** |
- `save_database(None,0)` returned True at checkpoint (15:11) and end-of-wave (15:31). Fresh re-read from
  the live server (post-save) confirms **every** applied name + the 0x4B28A0 comment persisted → **disk==live
  proven**. Dumps: `discoveries/wavedumps/t055-predump.json`, `t055-postdump.json`.
- Cumulative vs pre-b2 baseline 2,340: **2,340 → 2,012 = −328 fns** (b2 24 + b3 192 + b4 40 + b5 18 +
  b6 20 + T-053 queue 23 + T-054 ext 11 = 328 ✓ exact, zero gap).

## 1. T-053 QUEUE — renames applied (29 fns; delta itemized by family)

### 1a. Headline `JE::CDebugInterface` — cls_0x558740 → MINT 7 (NOT 12)
Evidence triple: string @0x692eac `CDebugInterface::CDebugInterface() m_winDebug came back NULL.` /
@0x692ef0 `...Unable to load template Lable.` · ctor EA 0x558740 · vft EA 0x692e84. Sole-storer re-check:
vft 0x692e84 code-storers = {ctor 0x558740 (object-side), factory 0x557bf0 `JE::CEngineInterface::meth_*`}
→ **passes corrected predicate**. Loads Debug.INT@0x692f4c; vft slot0/2 pre-named `JE::CDebugInterface::virt_meth_*`.
| ctor/method EA | rename applied | sole-storer recheck | verdict |
|---|---|---|---|
| 0x558740 (ctor) | `JE::CDebugInterface::ctor_0x558740` | pass (factory-only other) | MINT |
| 0x557a50 / 0x557d90 / 0x557ff0 / 0x558190 / 0x5583e0 / 0x558510 | `JE::CDebugInterface::meth_*` | contiguous, bracketed by the 2 named virts | MINT (band-membership by construction) |
| **0x434120 / 0x434250 / 0x4342c0 / 0x434760 / 0x434910** | **UNCHANGED (cls_0x558740::)** | — | **HELPER-THEFT, DO NOT MERGE** |
- Helper-theft re-confirmed at write time: the 5 at 0x434xxx live in the CFeedback GUI region
  (bracketed by CEscapeMenu 0x434110 / CEngine 0x434340 / CFeedback ctor 0x434f70), reference **zero** Debug
  strings, do **not** store the CDebug vft, and are called by CEngineInterface/CEscapeMenu/cls_0x434f70.
  Post-wave residual `cls_0x558740::` = **5** (the correct helper-theft residue), headline took 7.
- **T2 delta = −7.**

### 1b. Window-interface ctor own-vft single-joins — MINT 14/14 (15 incl. headline ctor already counted)
All 14 passed the corrected sole-storer predicate (each vft: ctor object-side storer + exactly one
`JE::CEngineInterface::meth_*` factory other-storer, `others_all_factory=true` for all 14; own `.INT` load
verified per ctor). Conservative scope: **rename the object-side vft storer (the ctor) only**, never a blind
prefix-swap of the 2-fn keys (would sweep in an unproven partner).

| class (shipped, RTTI) | ctor EA | vft EA | factory other-storer | .INT | old→new | T2 |
|---|---|---|---|---|---|---|
| CFeedbackInterface | 0x434f70 | 0x654304 | 0x434340 | Feedback.INT | cls_→`JE::CFeedbackInterface::ctor_0x434f70` | −1 |
| CInventoryInterface | 0x43cd50 | 0x656c58 | 0x43b000 | Inventory.INT | cls_→`JE::CInventoryInterface::ctor_0x43cd50` | −1 |
| CMainmenuInterface | 0x440990 | 0x65816c | 0x440910 | Mainmenu.INT | cls_→`…::ctor_0x440990` | −1 |
| CMessageBoxInterface | 0x441420 | 0x65863c | 0x441320 | MessageBox.INT | cls_→`…::ctor_0x441420` | −1 |
| CMiniMapInterface | 0x442340 | 0x658b5c | 0x442290 | MiniMap.INT | cls_→`…::ctor_0x442340` | −1 |
| COptionsInterface | 0x4436d0 | 0x658fcc | 0x442bc0 | Options.INT | cls_→`…::ctor_0x4436d0` | −1 |
| COptionsGameInterface | 0x444530 | 0x659604 | 0x444190 | Options_Game.INT | cls_→`…::ctor_0x444530` | −1 |
| COptionsLoadInterface | 0x448c40 | 0x65a6d8 | 0x4486e0 | Options_Load.INT | cls_→`…::ctor_0x448c40` | −1 |
| COptionsMultiplayerInterface | 0x4491f0 | 0x65ab94 | 0x449100 | Options_Multiplayer.INT | cls_→`…::ctor_0x4491f0` | −1 |
| COptionsVideoInterface | 0x4496a0 | 0x65afec | 0x449340 | Options_Video.INT | cls_→`…::ctor_0x4496a0` | −1 |
| CPipBoyInterface | 0x455e40 | 0x65bdfc | 0x455b90 | PipBoy.INT | cls_→`…::ctor_0x455e40` | −1 |
| CPipBoyAddNoteInterface | 0x44da10 | 0x65bb74 | 0x44a560 | AreaMapNote.INT | cls_→`…::ctor_0x44da10` | −1 |
| CCharacterGenerationInterface | 0x42a780 | 0x65286c | 0x42ca70 | CharacterGeneration.INT | cls_→`…::ctor_0x42a780` | −1 |
| **CNASInterface** | 0x426720 | 0x651b80 | 0x425450 | CharGen_NAS.INT | cls_→`…::ctor_0x426720` | −1 |

- **CNAS: NOT waived — VERIFIED.** T-053 left it §named-risk (own-vft unverified). I tested the predicate
  myself: sole object-side storer ctor 0x426720 (`[esi+edx]`), CharGen_NAS.INT load, other-storer 0x425450 =
  `JE::CEngineInterface::meth_*` factory only, vft `CNASInterface_vftable_651b80` with pre-named
  `JE::CNASInterface::virt_meth_*` slots (0x42fe40/0x425500/0x425510), zero rival ctors → **ACCEPTED under my
  own re-check**, not self-waived.
- `.INT` note (LoudDragon caution honored): CPipBoyAddNoteInterface's ctor 0x44da10 loads **both**
  `AreaMapNote.INT` (0x65bc20) and `AddNote.INT`; I kept the shipped vft name `CPipBoyAddNoteInterface`.
- **T2 delta = −14.** (Partners of 2-fn keys — 0x434b70, 0x43b1b0, 0x448900 — left anonymous: no independent
  string/own-vft proof; b5 0x548d10 exclusion precedent.)

### 1c. Merges into existing named classes — 6 (T3→T1; 0 T2 change, the cls_ side is 0x50dee0/0x50dfa0 below)
String bytes + EA, referencing-fn EA, target-class live count, verdict. **All sub_ (T3) → T1: zero T2 impact.**

| string (EA) | anon fn EA | target band (live) | extra owner evidence | verdict/applied |
|---|---|---|---|---|
| `Gfx_SkinMesh::ProcessShadowVolumeRenderSilhouette - SkinMesh not created!` @0x674798 (+@0x6746d0,@0x674740) | 0x507c90 | `JE::Gfx_SkinMesh` (6→7) | `__thiscall`, SkinMesh at this+128, 3 self-naming strings | MERGE → `::ProcessShadowVolumeRenderSilhouette_507c90` |
| `Gfx_RenderManager::GetDebugSphere - Out of memory!` @0x669924 | 0x4930f0 | `JE::Gfx_RenderManager` (2→3) | string names class+method | ANNOTATE/MERGE → `::GetDebugSphere_4930f0` |
| `Map::StitchWaterTiles - Missing water texture from resource: %s` @0x699f18 | 0x577290 | `JE::Map` (53→54) | **cited bytes:** `mov ebx,ecx`(consumes this) + `mov ecx,ebx`→`JE::Map::GetOctree_56d960`@0x578d3d; caller `sub_579760` passes `mov ecx,esi` | **MEMBER confirmed** → `JE::Map::StitchWaterTiles_577290` |
| `GameWeapon::ReadAttackModeData() - …consumed and thrown.` @0x64d860 | 0x4148a0 | `F3::GameWeapon` (37→38) | doc pair B11 §1/T-015: "sub_4148A0 = GameWeapon::ReadAttackModeData"; **string shared** with already-named 0x40dba0 (weapon/creature twins, byte-identical) — attribute by DOC not by string-only | MERGE (closes T-015 tail) → `::ReadAttackModeData_0x4148a0` |
| `GameWeapon::UnloadPrimaryAmmo() - Unable to look up ammo…` @0x64fbd8 | 0x413810 | `F3::GameWeapon` (38→39) | doc session6:24-25 `sub_413810 = GameWeapon::UnloadSecondaryAmmo` (shipped Secondary reuses the Primary message text) | MERGE → `::UnloadSecondaryAmmo_0x413810` |
| `ResourceFileManager::~ResourceFileManager - %s is still damanded…` @0x675bf0 | 0x50dd90 | `JE::ResourceFileManager` (3→4) | embedded at ResourceSystem+0x108 (ctor `add ecx,108h`); string = dtor | MERGE → `::dtor_0x50dd90` |
- **Map reconciliation CLOSED (LoudDragon gate cleared with cited bytes above):** effect-registry-doc "helper"
  framing was the decompiler rendering of an implicit `this`; the function is `__thiscall` and passes `this` to
  `JE::Map::GetOctree` — it is a **member**, merged.

### 1d. _Tree-band single-fn joins (b6 precedent) — 2 fns (cls_→T1)
`JE::ResourceFileManager::Add_0x50dee0` (was `cls_0x50dbb0::meth_0x50dee0`, string @0x675c40) and
`::OpenFile_0x50dfa0` (was `cls_0x50dbb0::meth_0x50dfa0`, strings @0x675cfc/@0x675cb0). These sat on the
REGISTERED `_Tree`/allocator EXCLUSION band; per b6 (AreaMap/Gfx_ModelInstance) a **single-fn** join with
self-naming-string + `this=+0x108` (Add called by dtor `mov ecx,ebx`; OpenFile by ResourceSystem::OpenFile
`lea ecx,[edi+108h]`) is permitted; **the `cls_0x50dbb0` band itself is NOT renamed** (residual 118).
**T2 delta = −2.**

## 2. T-054 EXTENSION — applies with delta by SOURCE (EpicRaven doc = citation, re-verified live)

### 2a. `cls_0x445350` band → `JE::HotKeyWindowFormatTable` (8 fns, MINT) — T2 −8
- Containment chain re-verified at write time (EpicRaven §2): thunk 0x4468b0 `sub ecx,[ecx−4]; jmp 0x4465e0`;
  handler 0x4465e0 does `mov ebx,ecx` (interface view) → `lea esi,[ebx−0x198]` (0x4465ed) → band calls with
  `mov ecx,esi` = **table base**; ctor 0x4468c0 installs interface vft `COptionsHotKeysInterface_vftable_659db0`
  at member +0x198 (0x446949) and outer vft 0x65a290 at [esi] (0x446922); F3.json `cls_0x4468c0` size 457,
  members 3× `.?AVHotKeyWindowFormat@@` @+0x14/+0x94/+0x114 + `mbr_0x198` usage 0x44691c. Factory-2nd xref
  0x445763 = `JE::CEngineInterface::meth_*` → EXPECTED per corrected predicate.
- **Name resolved empirically (NOT EpicRaven's `cls_0x4468C0`):** the LIVE IDB already names the outer object
  (ctor 0x4468c0, meth 0x4461e0) `JE::HotKeyWindowFormatTable` (prior wave). `.?AVCOptionsHotKeysInterface@@`
  RTTI is size 0 (the +0x198 view sub-object), so it is NOT the outer owner — the **table** is. Band entries
  (this=table base) therefore join `JE::HotKeyWindowFormatTable::`. Renamed: 0x445350/445380/4455d0/445970/
  445a40/445e20/446080/4461d0 (individual EAs, b5 0x548d10 discipline). Residual `cls_0x445350::` = 0.
- All 8 were `cls_0x445350::` (T2) → T1. **T2 delta = −8.**

### 2b. `cls_0x507810` + `cls_0x5067c0` + dtor `sub_507580` → `JE::Gfx_BufferManager` (4 fns, MINT) — T2 −3
- Chain re-verified: engine ctor 0x5bfe10 `push 0x50; new; call ctor_0x507810 (0x5bffec); mov [esi+30h],eax
  (0x5bfffc)` = **Gfx_Engine+0x30 heap member, new(0x50)**; ctor self-registers `byte_707CE0` (0x507831);
  dtor 0x5c1d41 `mov edi,[esi+30h]; mov ecx,edi; call sub_507580; j__free` (0x5c1d3f/41/47), `sub_507580` zeroes
  `byte_707CE0` (0x50764d) + frees StreamingVertex/Index buffers @this+40/44/48 + RbTree pool;
  `cls_0x5067c0::meth_0x506890`/`0x5067c0` called `this=*[0x707CE0]` (engine meth_0x5bef20 `mov ecx,[707CE0]`)
  = **same object**, two OOAnalyzer fragments. ctor new(0x50)=80 == F3.json cls_0x507810 size 80 == cls_0x5067c0.
- **Object decompiled (name is mine — no shipped string):** ctor allocates `Gfx_StreamingVertexBuffer`(+0x28)
  + `Gfx_StreamingIndexBuffer`(+0x30) + RbTree; Reset/Process grow VB(0x1C0000)+IB(0x60000) device pages. →
  dynamic VB/IB device-buffer manager. Name `JE::Gfx_BufferManager` (collision-checked: zero existing).
  Applied: `::ctor_0x507810`, `::Reset_0x5067c0`, `::Process_0x506890`, `::dtor_0x507580`.
- 3 were `cls_` (0x507810, 0x5067c0, 0x506890 — the latter two under cls_0x5067c0) → T1 = **T2 −3**; dtor
  `sub_507580` was T3 → **T3 −1**. Residual `cls_0x507810::`/`cls_0x5067c0::` = 0.

### 2c. `cls_0x4b28a0` — **ANNOTATE-ONLY, NO RENAME** — T2 delta = 0
- Independently verified EpicRaven §3: **142/142 external callers pass `this==&0x707E48`** (the 3 initially
  "unproven" — 0x4518de/0x442142/0x4518b2 — load `mov ecx, offset unk_707E48` at 0x4518d9/0x44213d/0x4518a9,
  my 8-instr window was just short); intra-band edges are valid `this`-propagation. Membership **vacuous**:
  zero heap-ctor/`*global`/embed/install edges, zero vft slots into the band (one static object at global
  0x707E48). Naming stays BLOCKED (no string, no RTTI). Comment set at 0x4B28A0 (671 B, read-back confirmed);
  **no rename**. 28-fn band fully retained as `cls_0x4b28a0::`.

## 3. Accepted vs skipped, itemized
| family | proposed | accepted | skipped (why) |
|---|---|---|---|
| Headline CDebugInterface | 12 key | **7** | 5 @0x434xxx helper-theft (hard exclusion) |
| Window-interface ctor singles (incl CNAS) | 14 | **14** | 0 (CNAS verified, not waived) |
| ctor-key partners (2-fn keys) | 3 | 0 | no independent string/own-vft proof (conservative) |
| Merges-into-existing | 6 | **6** | 0 (Map reconciled to MEMBER) |
| _Tree single-fn joins | 2 | **2** | 0 (band not renamed) |
| T-054 band 445350 | 8 | **8** | 0 |
| T-054 buffer manager | 4 | **4** | 0 |
| T-054 4b28a0 | annotate | **annotate** | rename deliberately skipped (vacuous membership) |

## 4. T2 delta by SOURCE (LoudDragon required three ways)
| source | T2 change |
|---|---|
| T-053 queue (headline 7 + ctor-singles 14 + _Tree singles 2) | **−23** |
| T-054 extension (445350 band 8 + buffer-manager cls_ side 3) | **−11** |
| annotation-only (4b28a0) | **0** |
| **total** | **−34** (2,046 → 2,012 = 22.67%) |
- Plus T3→T1 (no T2 effect): 6 T-053 merges + 1 buffer-manager dtor (sub_507580) = **T3 2,176→2,169 (−7)**;
  the +5 T1 gap (T1 4,438→4,479 = +41 vs 34+7=41 ✓) reconciles exactly.

## 5. Persistence + hand-back
Mid-wave checkpoint save 15:11:11; end-of-wave save 15:31:28; fresh-dump re-read confirms all names + comment
persist → **disk==live proven, writer slot ends clean.** Dumps in `discoveries/wavedumps/` (pre/post).
Name-churn note: every row here is anchored to string/ctor/vft/call EAs; live names re-read from the server at
write time, not from EpicRaven's offline dump (which pre-dates the first write).
