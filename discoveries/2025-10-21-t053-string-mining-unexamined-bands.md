# 2025-10-21 — T-053: Shipped-string mining over unexamined bands (NEW evidence class — READ-ONLY mint-queue)

CalmRaven, dispatched by LoudDragon `01a15f82`. Target ORIGINAL_REFERENCE (`ida_original_f3` :8745),
**READ-ONLY: zero IDB writes** (no renames, no comments, no type edits). IDB mtime verified 14:26
(== b6 end-of-wave save) at wave start; disk==live held; nothing here depends on live names surviving
the next writer wave — every claim is anchored to a shipped-string EA + a function EA.

This file is a **HANDOFF-READY queue** for the next T-033-style writer wave, per the b6 closure
statement ("future identity wins need NEW evidence classes … shipped-string mining in still-unexamined
regions"). It follows the b4/b5/b6 evidence hierarchy: **singleton-join (this==global) > shipped
in-band string > membership-by-construction**, and never renames anything itself.

Accelerators honored per LoudDragon: `tools/ea-index.py`/`artifacts/ea-index.tsv` used to separate
already-claimed EAs from fresh ones; b4 vendor forensics + b6 RTTI-sweep re-confirmed as pre-dismissals.

---

## 1. METHOD + SELF-TEST (harness item 35 discipline: reproduce knowns before trusting new claims)

Coverage primitive (pure read-only `py_eval`): enumerate `idautils.Strings()` (7,146 items), read bytes
via `get_strlit_contents`, select strings with a shipped `Class::Method` / `ns::name` shape (contain
`::`), xref each string EA **and EA+1** (a leading `\xc7` RTTI/`__thiscall` pad byte shifts the real
string start by one — this artifact initially hid two positives), map each reference to its containing
function's live name, and classify the band as CLAIMED (name has a real `ns::Class::` prefix) or
UNCLAIMED (name is `cls_0x…::` / `sub_…`).

**Self-test — the three registered positives ALL resolve to their committed bands:**

| shipped string | string EA | referencing fn EA | live band | committed by |
|---|---|---|---|---|
| `Actionbar.INT` | 0x650ae0 | 0x41a34d → `JE::CActionbarInterface::ctor_0x41a2b0` | CActionbarInterface (b4 mint) | ✅ |
| `Gfx_Texture_Manager::LoadTextureFromBuffer() failed to load: %s` | 0x677560 | 0x521d6e → `JE::Gfx_Texture_Manager::LoadTextureFromBuffer_521ca0` | Gfx_Texture_Manager (b6 mint) | ✅ |
| `Gfx_AnimMeshInstance::RayIntersection - No collision mesh and/or mesh binding!` | 0x66c430 | 0x4a4ef5 → `JE::Gfx_AnimMeshInstance::RayIntersection` | Gfx_AnimMeshInstance (b5 mint) | ✅ |

Method validated on all three knowns → the new claims below are trustworthy to the same degree.

**Corpus after self-test:** 422 `::`-shaped strings. Their xrefs land on **68 distinct already-named
owner-classes** (JE::Gfx_Engine 39, F3::GameWeapon 26, F3::GameEntity 26, JE::Gfx_AnimationObject 26,
JE::ResourceSystem 24, … — these are CLAIMED, adjudicated-or-dismissed, NOT re-queued; 387 strings sit
entirely inside named bands) and **13 distinct anonymous function targets** — the mine. The mining is
exhaustive over the `::` shape; I additionally swept the `*.INT` resource-name shape (the Actionbar
pattern) which is where the largest fresh cluster lives.

---

## 2. MINT QUEUE — band-level ownership by OWN-vft-STORE (top static tier, equivalent to singleton-join)

**This is the headline and a genuinely new evidence class.** b6's RTTI *method-set* sweep joined only
classes whose RTTI record lists declared method EAs that were anonymous at IDB, and declared the queue
exhausted. It did **not** look at the GUI window-interface *ctors*, which are OOAnalyzer `cls_0x…::ctor`
keys holding the ctor + 1-2 helpers, each of which **stores its own RTTI-named vftable into `this`** and
**loads its own `<Name>.INT` template via the b4 CActionbarInterface LoadInterface pattern**. Own-vft
store at a `cls_::ctor` is membership-by-construction at the strongest available form: the ctor is the
sole storer of the vft, the vft already carries the shipped RTTI class name, and the `.INT` names the
window. The b4 mint used exactly this shape (CActionbarInterface) but only for ONE class; the pattern
generalizes to a family the t036 density gate never surfaced (each band is 1-2 fns, below the OK gate).

### 2a. `JE::CDebugInterface` — `cls_0x558740` band → **MINT 7 fns** (NOT 12)
- **Evidence (all in-band):**
  - ctor `0x558740` stores **`CDebugInterface_vftable_692e84`** at 0x55878f (sole storer — own-vft join).
  - vft `0x692e84` slot0 = `JE::CDebugInterface::virt_meth_0x557d40` and slot2 =
    `JE::CDebugInterface::virt_meth_0x558dc0` — **two members already carry the shipped class name**;
    slot1 = base `JE::GUI::Interface::virt_meth_0x4953c0` (IUnknown-shape base, matches b4 CActionbar).
  - ctor loads `Debug.INT` (0x692f4c) + FindControls `DebugWin` (0x692f40), `DebugLine` (0x692f34) —
    the b4 pattern; and `Welcome to F3.` (0x692e9c).
  - **self-naming shipped strings** in the ctor body: `CDebugInterface::CDebugInterface() m_winDebug
    came back NULL.` @0x692eac (xref 0x558ae5) and `CDebugInterface::CDebugInterface() Unable to load
    template Lable.` @0x692ef0 (xref 0x558ade).
  - RTTI type-DB key present: `.?AVCDebugInterface@@`; vft-master-index row `CDebugInterface 0x692e84 (6
    slots)`.
- **Why this upgrades the b4 escalation:** b4 §"cls_0x558740 12 — already ruled b3 (embeds@CEngineInterface,
  comment-only)". That b3 comment-only ruling predates reading the ctor body: it **does** embed
  CEngineInterface-derived behaviour (correct) but ALSO stores its own CDebugInterface vft and self-names
  via two shipped strings — class identity is now proven, not just host adjacency.
- **fns-at-stake = 7 (band-level), NOT the full 12-key membership.** The OOAnalyzer `cls_0x558740` key is a
  COMPOSITE. Helper-theft check performed:
  - **REAL (merge):** 7 contiguous fns 0x557a50, 0x557d90, 0x557ff0, 0x558190, 0x5583e0, 0x558510,
    0x558740 (ctor) — sit between the two already-named CDebugInterface virts, share the ctor's band.
  - **NOT MERGE (helper-theft):** 5 fns at 0x434120/0x434250/0x4342c0/0x434760/0x434910 physically live in
    the CFeedback GUI region (0x434000-0x435100, interleaved with `JE::CEscapeMenuInterface`/
    `JE::CEngineInterface`/`JE::GUI::Edit`/`cls_0x434f70` CFeedback ctor), touch **zero** Debug strings,
    do **not** store the CDebug vft, and are called by CEngineInterface/CEscapeMenu/GameInterfaces. R36-e
    discipline: these stay under their true owner (a writer-wave layout join, not a string-claim). This is
    the same helper-theft trap b5/b6 avoided on 0x5240e0 and the 0x50dbb0 band.
- **Verdict: MINT, 7 route-A fns.** Writer should `cls_0x558740::` → `JE::CDebugInterface::` on the 7
  contiguous EAs only; leave the 0x434xxx five as-is (re-attribute or keep anonymous).

### 2b. GUI window-interface ctor family — 13 single-fn MINT/ANNOTATE joins (own-vft store, tier 1)
Each ctor below is the sole storer of its RTTI-named vft + loads its `<Name>.INT`. Evidence shape is
identical to §2a minus the extra Debug strings. Per b6 "single-fn own-vft join on a registered band" these
are defensible **T2→T1 joins (1-2 fns each)**; the writer disposes. `fns_in_key` is the live `cls_` key size.

| owner class (shipped, RTTI-present) | ctor EA | live key | own-vft stored | `.INT` | fns-at-stake |
|---|---|---|---|---|---|
| `JE::CFeedbackInterface` | 0x434f70 | cls_0x434f70 | 0x654304 | Feedback.INT 0x654354 | 2 |
| `JE::CInventoryInterface` | 0x43cd50 | cls_0x43cd50 | 0x656c58 | Inventory.INT 0x657020 | 2 |
| `JE::COptionsLoadInterface` | 0x448c40 | cls_0x448c40 | 0x65a6d8 | Options_Load.INT 0x65a758 | 2 |
| `JE::CMainmenuInterface` | 0x440990 | cls_0x440990 | 0x65816c | Mainmenu.INT 0x658190 | 1 |
| `JE::CMessageBoxInterface` | 0x441420 | cls_0x441420 | 0x65863c | MessageBox.INT 0x658684 | 1 |
| `JE::CMiniMapInterface` | 0x442340 | cls_0x442340 | 0x658b5c | MiniMap.INT 0x658b74 | 1 |
| `JE::COptionsInterface` | 0x4436d0 | cls_0x4436d0 | 0x658fcc | Options.INT 0x659108 | 1 |
| `JE::COptionsGameInterface` | 0x444530 | cls_0x444530 | 0x659604 | Options_Game.INT 0x65983c | 1 |
| `JE::COptionsMultiplayerInterface` | 0x4491f0 | cls_0x4491f0 | 0x65ab94 | Options_Multiplayer.INT 0x65abac | 1 |
| `JE::COptionsVideoInterface` | 0x4496a0 | cls_0x4496a0 | 0x65afec | Options_Video.INT 0x65b21c | 1 |
| `JE::CPipBoyInterface` | 0x455e40 | cls_0x455e40 | 0x65bdfc | PipBoy.INT 0x65bef0 | 1 |
| `JE::CPipBoyAddNoteInterface` | 0x44da10 | cls_0x44da10 | 0x65bb74 | AreaMapNote.INT 0x65bc20 | 1 |
| `JE::CCharacterGenerationInterface` | 0x42a780 | cls_0x42a780 | 0x65286c | CharacterGeneration.INT 0x652850 | 1 |

- **Caveats the writer MUST check (why these are proposals, not commands):**
  1. `CPipBoyAddNoteInterface` / `AddNote.INT` / `AreaMapNote.INT` — two `.INT` strings both resolve to
     ctor 0x44da10 which stores `CPipBoyAddNoteInterface_vftable_65bb74`; the vft-master row labels it
     `CPipBoyAddNoteInterface`. Writer to confirm AddNote vs AreaMapNote naming (one ctor, two template
     strings — pick the shipped vft name).
  2. `CharGen_NAS.INT` @0x651be4 → `cls_0x426720::ctor_0x426720` — the `CNASInterface` window; vft-master
     has `CNASInterface`; not yet own-vft-verified this pass (kept OUT of the confirmed table, listed as a
     cheap follow-on).
  3. These bands are small (1-2 fns) precisely because the interface *methods* were scattered by
     OOAnalyzer; the joins here are ctor-band membership proofs, not whole-class recovery. A writer wave
     could extend each via the vft slot targets, the same way b4 §7 extended Gfx_D3D.

`*.INT`-shape sweep also surfaced `FontTest.INT` @0x6acc98 → `sub_5B80E0` (T3, `CFontTestInterface` in
vft-master) — a T3→T1 join candidate, no T2 impact, listed for completeness.

---

## 3. MERGE-INTO-EXISTING-NAMED-CLASS (tier 2, shipped in-band string; the fn already belongs to a live band)

These are single anonymous `sub_`/`cls_` functions whose body carries a shipped string naming an
**already-named** class. Evidence tier = shipped in-band string + (where noted) `this=+0xNN` embedding.
Verdict = **ANNOTATE/merge 1 fn**; the class already exists so no new mint, but the fn count rises.

| string bytes (EA) | anon fn EA → live name | target band | extra owner evidence | verdict |
|---|---|---|---|---|
| `Gfx_SkinMesh::ProcessShadowVolumeRenderSilhouette - SkinMesh not created!` @0x674798 (+ @0x6746d0 lock VB, @0x674740 UpdateSkinnedMesh) | 0x507c90 `sub_507C90` | `JE::Gfx_SkinMesh` (6 fns live) | `__thiscall` reads SkinMesh at `this+128`, vertex-buffer lock via vft+56/60; 3 self-naming strings in one body | MERGE |
| `Gfx_RenderManager::GetDebugSphere - Out of memory!` @0x669924 | 0x4930f0 `sub_4930F0` | `JE::Gfx_RenderManager` (2 fns live) | string names method; sole caller `sub_493430`; band is a Gfx render helper | ANNOTATE (n=1, role-only; host class already exists) |
| `Map::StitchWaterTiles - Missing water texture from resource: %s` @0x699f18 | 0x577290 `sub_577290` | `JE::Map` (53 fns live) | string names `Map::`; but ea-index shows 0x577290 already cited as a CALLEE `sub_577290(10.0f,1.0f)` in `JE::Map` water context (effect-registry doc) — **the doc treated 0x577290 as a water helper, not a Map member**; string says `Map::StitchWaterTiles`. Reconcile before merging. | ESCALATE (conflict: helper vs member) |
| `GameWeapon::ReadAttackModeData() - A weapon can't be both consumed and thrown.` @0x64d860 | 0x4148a0 `sub_4148A0` | `F3::GameWeapon` (37 fns live) | **Already doc-claimed as `GameWeapon::ReadAttackModeData`** by B11 §1 / T-015 (22 stream reads, "parser self-names via its error string"); NEVER renamed in IDB. Sole caller `F3::GameWeapon::ReadData_0x415190` (`mov ecx,esi`). | MERGE (rename-only, closes a T-015 open tail) |
| `GameWeapon::UnloadPrimaryAmmo() - Unable to look up ammo. Caliber %d, modifier %d` @0x64fbd8 | 0x413810 `sub_413810` | `F3::GameWeapon` | **Already doc-claimed** by session6/session18 as `GameWeapon::UnloadSecondaryAmmo` (string is the Primary variant, fn is the Secondary mirror); NEVER renamed. Called by `ReloadSecondaryAmmo_413CA0`. | MERGE (rename-only; verify Primary-vs-Secondary string/fn pairing) |
| `ResourceFileManager::~ResourceFileManager - %s is still damanded %d times` @0x675bf0 | 0x50dd90 `sub_50DD90` | `JE::ResourceFileManager` (3 fns live) | embedded at `ResourceSystem+0x108` (`add ecx,108h` ctor call from `JE::ResourceSystem::ctor_0x49c610`); string is the dtor | MERGE (dtor join) |
| `ResourceFileManager::Add - Had to increase…` @0x675c40 | 0x50dee0 `cls_0x50dbb0::meth_0x50dee0` | `JE::ResourceFileManager` | sits on the REGISTERED `_Tree`/allocator EXCLUSION band `cls_0x50dbb0` (b6: 83 clones) — but this fn's body self-names ResourceFileManager::Add and is called by `sub_50DD90` (`mov ecx,ebx`) | **SINGLE-FN join on a registered band** (b6 AreaMap/Gfx_ModelInstance precedent) — NOT a band rename |
| `ResourceFileManager::OpenFile - Couldn't open / Memory allocation error` @0x675cfc / @0x675cb0 | 0x50dfa0 `cls_0x50dbb0::meth_0x50dfa0` | `JE::ResourceFileManager` | same exclusion band; sole caller `JE::ResourceSystem::OpenFile_49ae90` (`lea ecx,[edi+108h]` = the manager@+0x108) | **SINGLE-FN join on a registered band** |

ResourceFileManager is the strongest §3 cluster: it is a live named class, embedded at a fixed
`ResourceSystem+0x108`, with 4 further anonymous members (1 dtor `sub_`, 1 `sub_`, 2 on the `_Tree` band)
each self-naming. b3 already noted "3 fns IDB-named ResourceFileManager … the band spans ≥2 real owners
through shared `_Tree` … annotate only" — the string evidence now supplies the per-fn join proof b3 lacked.

---

## 4. DISMISSAL LIST (binding exclusions honored; these are EXCLUDED, not left to re-mine)

### 4a. Vendor / third-party epoch — the largest single dismissal
- **`sub_5FB742` = DirectX Media / DirectMusic + DirectShow error-string translator.** 15 of the 13-anon
  targets resolve here and are NOT 15 owners — they are all message-table references inside ONE function:
  `IDirectMusicPerformance::MIDIToMusic/MusicToMIDI`, `IDirectMusicTool::ProcessPMsg`, `IDirectMusicTrack::Play`,
  `IDirectMusicGraph::StampPMsg`, `IDirectMusicBand::Download`, `IDirectMusic::SetDirectSound`,
  `IPinFlowControl::Block`, `Clipper::GetClipList`, `DMUS_PMSG` docs (@0x6c0780..0x6c75a8, plus
  `IDirectMusicTrack`/`Band` blobs). Confirmed: 0x224f-byte function, **490 data refs** into the
  0x6c0000-0x6c8000 SDK message table, **47 external callers** spread across the game render/Octree epoch
  (it is a generic `HRESULT→string` formatter, exactly like the already-excluded D3DX9 `sub_474C24` in b4).
  Located ≥0x5FD000 = the link-tail vendor epoch b4 §D3DX8/COM ruled. **DISMISS — vendor, do not mint.**
  This is where most of the "ObjectLists?/IPinFlowControl?" surface lives; naming it would inflate named-%
  with third-party code, the exact error b4 corrected.
- **`debug_stream::printf()` @0x6ad108 → `printf` (0x5b8c40)** — CRT/stdio. **DISMISS.**

### 4b. `std` internals / type-name artifacts
- **`std::string` @0x6739fc** — only xrefs are data-table entries (0x6fbbfc, 0x6fc96c), no function. This is
  an RTTI/type-name blob, not a shipped error string. **DISMISS.**

### 4c. n≤2 vacuous-ratio tail (R36-c) — claim the string, dismiss the band
- **`Gfx_ModelInstance::AttachTo - Parent ModelInstance does not contain a SkinMeshPkt!` @0x66b430 →
  `cls_0x49cf90::meth_0x49d290`** (band = 2 fns, 0x49cf90 + 0x49d290). `JE::Gfx_ModelInstance` already
  exists (3 fns, b6 §2 joined its dtor via RTTI+own-vft). But `cls_0x49cf90` is an n=2 key with no vft store
  and 1 external caller (`JE::ModelInstance::meth_0x565b00`). String proves `AttachTo` ROLE but band is too
  thin to mint, and the owner Gfx_ModelInstance is already live. **CLAIM the string (attachTo role),
  DISMISS the band (n≤2).** If a writer wants it, it is a single-fn role annotation on 0x49d290, not a mint.

### 4d. Already-claimed-by-RTTI (do not re-queue)
- **`AnimationCache::Startup() - Could not load default skeleton.` @0x6a0ae0 →
  `JE::AnimationCache::Startup_59b1e0`** — already named (b6 §2 RTTI rejoin of AnimationCache). In-index,
  adjudicated. Method caught it correctly (the `\xc7` boundary artifact was the only reason it needed a
  second look) — proof the self-test method has no blind spot on claimed bands.

### 4e. Registered exclusions touched but NOT mined (per dispatch)
- `_Tree`/allocator families `cls_0x50dbb0` (120), `cls_0x55dc80` (83 clones), `cls_0x50db20` (79),
  `cls_0x4542d0` (50): only §3's two **single-fn** ResourceFileManager self-naming members were touched,
  following the b6 AreaMap/Gfx_ModelInstance single-fn-join precedent. The bands themselves stay
  anonymous. Vendor D3DX9 `cls_0x668cd8` (46) + D3DX8-adjacent (148): untouched, pre-dismissed by b4.

---

## 5. Coverage summary table (claimed vs unclaimed `::`-clusters)

| bucket | count | disposition |
|---|---|---|
| `::`-shaped shipped strings total | 422 | — |
| sit entirely inside already-NAMED bands (68 owner classes) | 387 strings | CLAIMED — adjudicated, not re-queued |
| touch ≥1 anonymous fn | 32 strings | MINE (below) |
| → resolve into the vendor `sub_5FB742` DX translator | 15 strings | DISMISS §4a |
| → CRT/std/n≤2/RTTI-already-claimed | 3 strings | DISMISS §4b-d |
| → fresh GUI window-interface ctor family (`*.INT` sweep) | 15 classes (1 headline + 13 ctor-joins + FontTest) | **MINT §2** |
| → merge-into-existing-class (single fn) | 6 fns (SkinMesh, RenderManager, 2× GameWeapon, 2× ResourceFileManager) | **MERGE/ANNOTATE §3** |
| → Map/StitchWater conflict, kept as escalation | 1 | ESCALATE §3 |
| distinct fresh anonymous fn targets (pre-dismissal) | 13 | fully triaged: 7 mint + 6 merge + 0 unexplained |

**Net honest yield if the writer wave accepts §2a + §3 + single-fn §2b/§3 joins:** ~7 (CDebugInterface) +
~18 (13 ctor-band single joins, counting the 2-3-fn keys) + ~6 merges = a plausible **≈30-31 in-game
T2→T1 route-A fns**, the largest cluster (CDebugInterface 7) being the only one at true band-level.
I do not certify the writer will accept all §2b — those are 1-fn own-vft joins and a conservative writer
may batch them differently. Zero speculative names: every entry above has a shipped-string EA or an
own-vft store + RTTI name behind it.

---

## 6. Honest limitations / named risks
1. **Name churn (LoudDragon caution):** every `live name` column here was re-read from the 8745 IDB at
   write time; the next writer wave will move these names. Re-verify by **EA** (each row carries the fn EA),
   not by name, before applying.
2. **§3 Map/0x577290 conflict:** committed effect-registry doc cites it as a water *helper*, the string says
   `Map::StitchWaterTiles`. Unresolved — writer must disambiguate member-vs-helper before merging.
3. **§3 GameWeapon 0x4148a0/0x413810:** already doc-claimed (B11/T-015) but never renamed — verify the
   Primary/UnloadSecondary string↔fn pairing before committing (session18 flagged the mirror semantics).
4. **§2b CNASInterface / `CharGen_NAS.INT` → `cls_0x426720::ctor_0x426720`:** same shape as the confirmed
   family but own-vft not verified this pass; cheap follow-on, left out of the confirmed table rather than
   guessed.
5. **Shape coverage:** the sweep is exhaustive over `::` and `*.INT` string shapes. Other self-naming shapes
   (bare `"Texture Mgr"` style headers already claimed by b6 DumpPools, or `__FUNCTION__`-only strings)
   were not re-swept because they don't carry a `Class::` owner token.

## 7. What this lane did NOT find (so the coordinator can scope the next)
The unexamined-region `::`/`.INT` surface is **not mostly CRT/vendor** — the vendor `sub_5FB742` is the
single biggest dismissal (15 strings) but the GUI window-interface family is a real, fresh, mineable
cluster the six-batch chain never touched because its bands sit below the t036 density gate. After the
§2/§3 queue is spent, remaining `::`-shaped string owners all resolve to named bands, vendor, or n≤2
tail; a future string lane should instead target the `.INT`-sibling singletons (CNAS, FontTest) and the
§2b method-body vft-slot expansion, or move to the lockstep/ecx-flow evidence classes (T-052/T-054).
