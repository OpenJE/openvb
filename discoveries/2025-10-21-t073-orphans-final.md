# T-073 — Final adjudication of the 17 true-orphans: BOTH families move. A = MINT-READY; B = named epoch, container tier

**HappyStorm** `01a16092-41cf-7127-936b-377a304178a4`, row claim `b737f69`.
Lane: **zero IDB writes, zero 8745 queries** — the entire row ran offline (raw EXE
`a68c22f3dd1c256ec9539a70e87cb434` + `obj/t054` pinned objdump cache + frozen `artifacts/F3.json`
via native f3_* tools + committed `t064-postdump.json` LIVE names). KeenStorm's product-port files
and HappyEagle's debugger stubs untouched. Deliverable left DIRTY per researcher protocol.

## 0. Gate first

`python3 tools/ecx-flow.py selftest` — **6/6 PASS** @ pinned md5 `a68c22f3…`, INCLUDING the new
T5 line (share-check gate reproduces the banked item-44 rulings: `0x434250=SHARING
0x434120=SHARING cls_0x445350-band=PASS 0x507810=PASS`). Ran band/families/share-check for both
families after the T-076 tool landed (T-076 share-check is the pre-queue gate; the earlier
T-063-era triplets were re-derived at CURRENT tool strength, per the row's demand).

## 1. Family A — `cls_0x490150` ×9 → **MINT-READY** (Gfx_RenderManager band extension; host GfxCameraHolder+0x164)

### 1a. The census count reconciles to the live IDB
Export `cls_0x490150` has **10** methods; `t064-postdump` shows 9 still `cls_`-anonymous because
`0x4915E0` was already renamed `JE::Gfx_RenderManager::RenderShadowVolumes2` (self-naming string
@0x669828 family — T-053/b5 era). The 9 orphans are therefore the still-unnamed member set of the
same this-object as an already-live-named class. 9 = 10 − 1 ✓.

### 1b. Composite correction (T-064 tier finding, first-literal rule re-earned)
`cls_0x490150` (size **117**, members 0x0–0x74) and `cls_0x4950d0` (size **117**, members 0x0–0x74)
are **ONE object** — same size, same member-offset set, and `cls_0x4950d0`'s only method is
`ctor_0x4950D0` which initializes exactly the `cls_0x490150`-shaped field set. The OOAnalyzer split
is the same family as batch-2's G3D_NodeRecordSet ("always test same-this construction before
accepting a doc's fn count"). `ctor_0x4950D0` body (cache @0x4950D0):

- `mov %eax,(%esi)` @0x495105 — stores the **host pointer** (arg) at obj+0x00 (back-pointer;
  read back at 0x4914A3 as mbr_0x0);
- two `_List` scaffolds via `0x616EC8` (alloc-pair ctor, sentinels 0x494F00/0x495090, 0x494B00/0x492240);
- three list/range headers via `0x4C6640`, `0x57C2E0`;
- **two heap-member embeds via `0x56E0B0` at this+0x5C and this+0x68** — the selftest T2b known-positive
  function (`ModelInstance ctor` family) — construction-side corroboration at the tool's own reference grade;
- `+0x34` zeroed (lazy), `+0x38/40/48/49/74` byte flags, `+0x3C = 0x80000000`.

### 1c. Construction tier — the single-owner proof (item 44 rule 2, at b5/Gfx_BufferManager shape)
`ctor_0x4950D0` has **exactly ONE call site in the whole image** (pinned-objdump call census:
1/1) — at **0x4C02E8**, inside `JE::GfxCameraHolder::ctor_0x4C0190`:

```
0x4C02C9  push $0x78           ; new(120) — the export size 117 padded
0x4C02D0  call 0x6054B0        ; operator new
0x4C02E6  mov %eax,%ecx
0x4C02E5  push %esi            ; esi = the GfxCameraHolder being constructed
0x4C02E8  call 0x4950D0        ; THE band ctor, sole site
0x4C02FB  mov %eax,0x164(%esi) ; sole write to GfxCameraHolder+0x164
```

`cls_0x4c0190`'s `mbr_0x164` has exactly two usages in the export: **0x4C02FB (the ctor store)** and
**0x4248D7 (a READ — `mov 0x164(%ebx),%ecx` inside GfxCameraHolder::meth_0x424880)**. Sole object-side
storer = ctor, per CalmRaven's corrected predicate (item 41 rule 3). **GfxCameraHolder** is not a new
claim — it is a committed batch-2 MINT (`t033-batch2-selfnaming-decisions.md` line 24 +
`batch2-apply-ledger.md` line 69/91: struct, 640 B, `.?AVGfx_Camera@@` 300 B embedded @+0x30 —
re-verified live this pass: ctor_0x4C0190 @0x4C01D5 calls `JE::Gfx_Camera::ctor_0x495ee0` at
`esi+0x30`).

GfxCameraHolder instances: `new(0x280)→ctor_0x4c0190` has **exactly 6 sites**, all verified
(0x42BA92 CCharacterGenerationInterface::ctor → field+0xB8; 0x43D035 CInventoryInterface::ctor →
+0x290; 0x4B5832 GUI::Manager::ctor → +0x8C; 0x594023 JE::DialogueWindow::meth_0x593660 → +0xD78;
0x5C0D06/0x5C0D8A both inside `JE::Gfx_Engine::virt_meth_0x5c0c30`, stored to **engine+0x18 (store
@0x5C0D19)** and **engine+0x1C (store @0x5C0D9D)** respectively, each followed by
`GfxCameraHolder::meth_0x424880` + mode constants (+0x20 = 3 / 2)). The three remaining `push
$0x280` occurrences (0x588D19, 0x58A6A5, 0x58A6EC) are **640-×-320 video-mode arguments** into
0x5B5640/0x5B5B10 — not allocations (self-checked; false-positive retired the same era as the \xc7 lesson).

### 1d. this-shape census — every edge into the band is the SAME holder field
Every external entry into the extended object set resolves to `ecx = *[X+0x164]`, X = a
GfxCameraHolder instance. Verified sites (from band/families output + manual body reads):

| caller | shape | callee |
|---|---|---|
| Gfx_Engine::virt_meth_0x5c1940 (vft 0x6AE7EC idx35, device-reset path — strings "Release Video Resources"/"Attempt to Reset Device") | `*[*[this+0x18]+0x164]`, `*[*[this+0x1C]+0x164]`, `*[node+0x8]+0x164]` (walks the +0x24 list) ×6 | 0x494B40 |
| sub_4BFF20 (Gfx_D3D render-state applier, `0x503320(this=&0x7082F8, state, value)` family) | `*[in-ecx+0x164]` | 0x494CA0 |
| Gfx_Engine::virt_meth_0x5be010 / 0x5be060 / 0x5be080 (idx76) / 0x5be0a0 / ~0x5c0629 / ~0x5c064c | `*[…+0x164]` | sub_490330, 0x4903E0, EnableHeightFog 0x490400, byte [this+0x40] read, sub_493CD0, sub_493430 |
| GfxCameraHolder::meth_0x424880 @0x4248D7 | `mov 0x164(%ebx),%ecx` | sub_4901D0 |
| JE::DialogueWindow::meth_0x593660 @0x594189 | `mov 0x164(%ebx),%ecx` | sub_4901D0 |
| sub_493430 @0x49347C (`lea 0x50(%ebx),%ecx` list-walk, this-forwarded) | `this=in-ecx` | 0x4930F0 GetDebugSphere |

`EnableHeightFog` body confirms the object: `test [esi+0x44]`, `mov %al,[esi+0x40]` (mbr_0x40/0x44
as per export), warning push `$0x669760` = the shipped string
**"Gfx_RenderManager::EnableHeightFog - [WARNING] no height fog created"**. So the class label
`JE::Gfx_RenderManager` (already live for 3 fns) names **this exact this-object** — same join shape
as T-053's ResourceFileManager extension (self-naming siblings + fixed embed slot + sole storer).
The third string-named member, `GetDebugSphere` (0x4930F0, "Gfx_RenderManager::GetDebugSphere -
Out of memory!" @0x669924), is reached through sub_493430 whose this is again `*[X+0x164]`
(0x5C0653/0x5C0669) — i.e., **all three live Gfx_RenderManager names sit on the band object.**

### 1e. item-44 machine test — printed VERBATIM, contradiction faced LOUD

`ecx-flow.py share-check --cls cls_0x490150` (exit 1):
```
 0x490150..0x4931E0  NO-EDGE  (8 entries — intra-band leaves, family-legal soft)
 0x494B40  SHARING   holders=*[*[?+0x8]+0x164](2), *[*[in-ecx+0x18]+0x164](2), *[*[in-ecx+0x1C]+0x164](2)
 0x494CA0  SINGLE-OWNER  holders=*[in-ecx+0x164](1)
 VERDICT: SHARING  (exit 1)
```
`share-check --addrs <9 orphans + ctor>`: same, plus `0x4950D0 SINGLE-OWNER (heap=1,
suggested-owner cls_0x4c0190)` — the exact b5/T-054 shape that minted Gfx_BufferManager.

**The machine SHARING is a prefix-counting artifact, and this row says so explicitly rather than
silently re-reading it:** the three "holder" expressions end at the **same field +0x164 of the same
class** — they differ only in how the caller locates a GfxCameraHolder instance (engine current
slot +0x18, engine +0x1C, scene-list node payload). All GfxCameraHolder instances in the image come
from the six verified `new(0x280)→ctor_0x4c0190` sites (§1c); the band object has NO other
constructor and NO other store site. There is no second holder class, no view-constant pair (zero
adjusted thunks resolve into the band — families [Q2] `thunks=none`, band join [B]=0), and rival
ctors = 0. Item-44's discriminating case ([B]: "-0xB0 AND -0xC4 views from TWO host classes each
storing their own interface vft") is absent on every axis. Named residual gap (does not move the
verdict): the scene-list **node+0x8 producer** was not individually located this pass — it carries
no ownership weight because a payload dereferencing +0x164 as a band-object slot must be one of the
six holder instances (no other band object exists to point at).

### 1f. Standing-verdict supersession (LoudDragon requirement 1, verbatim)
T-054: "**`cls_0x490150` … verdict CONSUMER-of-Gfx_D3D … never minted**"; T-063 §(e)1: "it acts ON
light records handed to it; nothing owns it; b5 'consumer, not member' upheld at flow tier."
**This row supersedes the ownership conclusion, not the consumption observation.** The object IS a
Gfx_D3D/D3D-device consumer (calls via `*[0x708300]` device vft at slots 0x118/0x130/0x14C —
SetSamplerState/SetTexture/SetTextureStageState-shaped — and `0x503320` render-state setter on
`&dword_7082F8`) **AND** is a composition member of `JE::GfxCameraHolder` at the single slot +0x164.
"Consumer" and "member-of-a-different-host" are orthogonal axes; T-063's "nothing owns it" was a
flow-tier statement made BEFORE the construction-tier query (sole `new(0x78)`→ctor site) — the
missing query class was exactly the D-join `new→ctor→[X+disp] install` that the tool implements
(join D) and that this row ran to completion. The legal shape "CONSUMER-of-Gfx_D3D *and* member of
GfxCameraHolder, minted as Gfx_RenderManager extension" is proposed.

### 1g. Gfx_RS_* mapping (row asked for it)
The 14-class `JE::Gfx_RS_*` family (0x535ED0–0x542BE0, static instances across 0x70B318–0x70BB68 —
the atexit dtor thunks at 0x646180–0x6467C5 dump that shape directly) is a **separate epoch**:
its instances carry their own vfts (0x5363C0/0x536AC0/0x536C20/0x536D70/0x536ED0/0x537030/0x537180/
0x5372D0/0x537430/0x537590/0x5376E0/0x537830/0x537980/0x537AE0 dtor group) and appear nowhere in the
band's edge sets (band/families owners list never mentions Gfx_RS; no xref). The "light-table
consumer" reads therefore do NOT point at Gfx_RS — dismissed with evidence, no adjacency guess.

## 2. Family B — `cls_0x4fc0e0` ×8 → **FINAL NEGATIVE for a domain-class mint, with a POSITIVE named-epoch reclassification: `std::vector` instantiation clones hosted by the RTTI-named `VFX_FormatManager` singleton**

T-063 called this family "a coherent epoch, anchored nowhere". **The anchor exists — the row was
looking for a class-shaped owner and the epoch's true shape is container machinery.** Both halves
are evidence, so the honest verdict is tier-corrected rather than guess-upgraded.

### 2a. What the 16-byte object actually is
`cls_0x4fc0e0` (export size **16**, members **+0x4/+0x8/+0xC** only — the exact `_Rep` header:
begin/end/capacity) and its sibling fragments `cls_0x4fb050` (same member set!), `cls_0x4fd150`,
`cls_0x4fd780` are the **template instantiations of two vector types** pinned by byte-level strides
(pinned-objdump stride census, per-function):

- `std::vector<VFX_ParameterInfo>`, **element size 0x114 = 276** (`imul $0x114` + the MSVC
  reciprocal-divide magic `0x76b981db>>7` at 0x4FC0F5–0x4FC119, 0x4FC17F, 0x4FC1AC…; the dtor-loop
  `add $0x114,%esi` at 0x4FB57B walks elements and destructs each through ITS vft `call *(%eax)`).
  **276 = 273-byte `VFX_ParameterInfo` rounded** — `.?AVVFX_ParameterInfo@@` is an RTTI-named,
  LIVE-CLASS-ALREADY-LIVE object (size 273, vft 0x673944, ctor 0x4FB2D0/0x4FB5B0, dtor 0x4FB370,
  30 methods, 4 `basic_string` members @+0x34/+0x6C/+0xA4/+0xDC + flags +0x38/+0x54/+0xA0/+0xD8/+0x110).
  Band members: ctor_0x4FC0E0 = assign(range), ctor_0x4FC6C0 = copy-fill at `[element+0x4]`
  (nested header), meth_0x4FB560 = clear+free (element vft-destruct loop), meth_0x4FB0B0/0x4FBB60/
  0x4FCAC0/0x4FC340/0x4FCBA0 = size/reserve/insert machinery (helper cluster
  0x4FB4E0/0x4FB850/0x4FB8A0/0x4FB940/0x4FBF10/0x4FBF80 all carry the 0x114 multiplies).
- `std::vector<FormatEntry>`, **element size 0x14 = {dword tag, nested vector@+4}** — the
  stride-`0x14` loop cluster sub_4FCC30/CC60/CD00-part1/D150/D1A0/D250/D2E0/CB30/C760 + thunks.

### 2b. The host and the singleton chain (the "anchor" T-063 said was missing)
`JE::VFX_FormatManager` (RTTI `.?AVVFX_FormatManager@@`, export size 44 — superseded by LIVE
evidence 56: parse fn stores vft at +0x0 and zeroes fields through **+0x34**; T-040 erratum rule:
export sizes are not authoritative) owns the containers at fixed fields:

```
VFM { vft 0x67396c @+0x00; vector<FormatEntry> {begin,end,cap} @+0x04..0x10;
      vector<VFX_ParameterInfo> header (the 16-byte band object) @+0x18..0x27; member @+0x28 }
```
- `VFX_FormatManager::dtor_0x4FD280` (LIVE name) stores its vft at `0x4FD29D`, then destroys
  +0x28 (via 0x4FB510), **+0x18 via 0x4FB560** (the band dtor — the ONLY `this=+disp` view constant
  anywhere in the family, `this=in-ecx+0x18`), +0x4 via 0x4FD150 (entry-vector clear).
- `cls_0x4fd780::dtor_0x4fd780` is mislabeled at the tier: it stores the same vft 0x67396c @0x4FD7AD
  and opens a stream (`0x52FB10`) then runs the parser sub_4FCD00 (`>`/`=`/`" \t\n\r,;"` token
  walk) — it is **VFM's load/parse method**; its sole caller is `unknown_libname_488` @0x641980, a
  CRT **static-init thunk** `mov $0x70811c,%ecx; call 0x4fd780`.
- The object is the **global singleton at BSS 0x70811C**: atexit thunk `0x645E60:
  mov $0x70811c,%ecx; jmp 0x4fd280` (its neighbour 0x645E70 is the Gfx_D3D singleton 0x7082F8 — same
  static era). Getter `sub_4FB040: mov $0x70811c,%eax; ret` has exactly ONE caller image-wide:
  **`JE::VFX_VEGInstance::meth_0x48ee30`** — the .veg script instance class.
- The adjusted thunk `0x4FB6A0 = add $0x4,%ecx; jmp 0x4FB560` is NOT membership evidence for some
  hidden table — it is the **nested-vector destructor thunk** used by the entry-clear loop
  (element base +0x4 → nested header) and by the constructor's SEH handler pads at 0x6350A0–0x63511B
  (those sub_6350xx "callers" in the band output are non-head EH chunks — item 43 locator rule,
  T-060 containment re-anchor precedent).

### 2c. Domain strings (T-053 method, applied)
Real string xrefs in the epoch (.rdata 0x6738F8–0x673968 — the same 112-byte block that immediately
precedes VFM's vft): **"V1.0"**, **"VFX "**, **"MaxStartTime"**, **"MinStartTime"**, **"Loop"**,
**"float"**, **"bool"**, **" \t\n\r,;"** (tokenizer), plus **">"**/**"="** (comparison tokens), and
"vector<T> too long" @0x64795C (the std::vector bad_length throw) — pushed from VFM::Load/parse
helpers (0x4FC7EF–0x4FC9F1) and, independently, from `JE::VFX_ParameterHolder::meth_0x5011a0`
(0x5011C4/0x5011EB/0x501214 — the holder registers the same three standard parameter names).
Cross-reference: van Buren `File-Format-Specifications.md` §VFX Block: signature **"VFX V1.0"**,
`typeName` ("VFX_EmitterShape"), typed properties incl. `VFX_TriggeredEffect` int — and the board's
own VFX-era docs: `2025-10-20-attach-register-entity-vfx-maps.md` (VEGInstance* in the attach entry
layout), `2025-10-20-eeov-attach-trio-eme2-flag-is-pumpenable.md` (footstep **.veg** usage),
`2025-10-20-canonical-fact-sheet.md` line 30 (weather rain/snow/wind **.veg**, shared VEG pool),
vftable-master-index (VFX_VEG 0x673aac, VFX_VEGInstance 0x6696f8, VFX_VEGCache 0x6729d4).
Lineage to T-037's script-native door notes: the .veg files arrive through the same
ResourceSystem-stream tier that the script natives read (0x52FB10 opener); no new door claimed —
the consumer edge is the single VFX_VEGInstance::meth_0x48ee30 call site cited above, cited not guessed.
(17 other 0x63xxxx "strings" in the first sweep were SEH handler immediates `push $0x634exx` —
discarded, the \xc7-adjacent artifact class re-encountered and re-retired.)

### 2d. item-44 machine test — printed VERBATIM
`ecx-flow.py share-check --cls cls_0x4fc0e0` — **VERDICT: PASS (exit 0)**:
```
 0x4FB0B0/0x4FBB60/0x4FC340/0x4FCAC0  NO-EDGE (intra-family leaves — legal)
 0x4FB560  SINGLE-OWNER  view-constants=this+0x18(1@0x4fd2bb) plain-this=11
 0x4FC0E0  SINGLE-OWNER  plain-this=2
 0x4FC6C0  SINGLE-OWNER  plain-this=3
 0x4FCBA0  SINGLE-OWNER  plain-this=3
```
The single view constant (+0x18) is VFM's field. PASS therefore licenses "single host", but the
host relation is **container-instance ⟶ member-fields**, and the band functions are compiler-generated
template code — which the board's law keeps at the annotation tier (b3 `_Tree` precedent; T-057
"classes stay non-mints, nothing remains anonymous-foreign"; item 43 function-granularity).
**No domain-class mint is proposed for B.**

### 2e. item-41 triplets for B, at current strength
- **Q1 callers-of-callers**: complete — every external caller of every band fn is a VFM-epoch
  function (VFM::dtor, VFM::load/parse + its assign/merge cluster 0x4FD310/0x4FD680/0x4FD6F0,
  entry-vector helper cluster, SEH pads of the same ctors, atexit/static-init thunks). No foreign host.
- **Q2 thunk-adjusted table resolution**: 0x4FB6A0 (add $0x4) / 0x4FB6A3 (plain) / 0x4FB6B0 (→0x4FB510)
  / 0x4FD240 resolved as container dtor thunks + SEH-pad targets; **no vft-table slot points into the
  band** (families [Q2] `slots=0` for every family; band join [C] `slots into band: 0`).
- **Q3 full-window vft census**: raw scan of all **345** tables across .rdata+.data+.data1 — zero
  slots into the band (band report line "[C] … tables scanned: 345").

## 3. Coverage statement + tool inventory + honest gaps
Weapons applied: ecx-flow selftest (6/6) → band/families/share-check ×2 families + share-check
--addrs (ctor tier) + band --addrs (Gfx_RenderManager trio); pinned objdump function-start call
census (whole .text) for ctor/caller counts; vft-store census (movl imm $0x67396c/$0x673944 stores);
COL/TD-adjacent RTTI verification for the touched names (`.?AVVFX_FormatManager@@` TD@0x6FBBCC-8,
`.?AVGfx_Engine@@`, `.?AVVFX_ParameterInfo@@`; item 42 size-checks: VFM 44⊂live-56 SUPERSEDED with
byte cite; GfxCameraHolder/Gfx_RenderManager carry NO RTTI — struct/string tiers, inapplicable);
T-053 string mining incl. the SEH-immediate rejection pass; same-this composite check (A: 117=117);
stride/magic-divide container fingerprinting (B); t064-postdump LIVE-name reconciliation for all
counts. **Zero IDB writes, zero 8745 queries** — nothing this row needed the live server for.
Named gaps (each immaterial to the verdicts, stated not padded): scene-list node+0x8 producer
(A, §1e); `0x4FB510` +0x28-member exact semantics (B, small body unread); the 8 NO-EDGE per-entry
rows in A's share-check are intra-family delegation, cross-checked via families
(`[Q1-delegated] … owners=sub_4BFF20/Gfx_Engine` for 7 of them; 0x4915E0 delegated via its own
band caller 0x491FC0, which is `this=in-ecx`-forwarded).

## 4. WRITER-QUEUE (for CalmRaven's slot; nothing applied from this lane)

### [A] MINT — `JE::Gfx_RenderManager` band extension (T2 −9)
Proof set per fn: same this-object as the three string-named live members (export membership +
this-shape §1d); sole ctor/install (construction §1c); item-44 machine output printed §1e with the
prefix-artifact refutation. Rename rows (`JE::Gfx_RenderManager::meth_<ea>_<ea>`, body-bounded,
collision-checked against t064 dump):
```
cls_0x490150::meth_0x490150  cls_0x490150::meth_0x490510 cls_0x490150::meth_0x4911b0
cls_0x490150::meth_0x4913e0  cls_0x490150::meth_0x491b10 cls_0x490150::meth_0x491fc0
cls_0x490150::meth_0x4931e0  cls_0x490150::meth_0x494b40 cls_0x490150::meth_0x494ca0
```
Expected T2 delta: **−9**.

### [A2] COMPOSITE follow-ons (writer-discretion tier; same object, fragments outside the 17-set)
- `cls_0x4950d0::ctor_0x4950D0` → `JE::Gfx_RenderManager::ctor_0x4950d0` (T2 −1) — sole ctor, heap=1
  share-check SINGLE-OWNER row printed.
- `cls_0x4903e0::meth_0x4903E0/0x491F90` (T2 −2) — this-shape verified §1d.
- merge-tier (T3→T1, no T2 change): `sub_4901D0` (lazy +0x34 create), `sub_490330`, `sub_493430`
  (GetDebugSphere driver), `sub_493CD0` (slot-1 trampoline callee; body not re-read — cite shape only).
Forecast band-wide: **−12 T2** if [A]+[A2] fully applied; [A] alone is −9. State the forecast as
such — do not let −12 enter any ledger as booked before read-back.

### [B] ANNOTATE — `cls_0x4fc0e0` ×8 (T2 delta 0 by construction) + comment on sibling fragments
Per-entry comment text, container tier, host-named:
```
0x4FB0B0/0x4FBB60/0x4FC0E0/0x4FC340/0x4FC6C0/0x4FCAC0/0x4FCBA0/0x4FB560 —
  "std::vector<VFX_ParameterInfo> instantiation clone (element 0x114 = RTTI .?AVVFX_ParameterInfo@@
   273B; header {begin,end,cap}=+0x4/8/C). Host: VFX_FormatManager singleton @BSS 0x70811C,
   fields +0x18 (direct 16B header) and +0x4 (vector<FormatEntry>, element 0x14 = {tag,
   nested vector@+4}); clear-loop dtor 0x4FB560, thunks 0x4FB6A0/A3/B0 = nested dtor/SEH pads
   (non-head EAs, item 43). vft slots into band: 0 (345-table full-section scan)."
```
Plus: comment at `cls_0x4fd780::dtor_0x4fd780` = "VFX_FormatManager::load/parse (stores vft
0x67396c @0x4FD7AD; stream via 0x52FB10; sole caller = static-init thunk 0x641980 — 'dtor' label is
an OOAnalyzer role artifact)"; comment at 0x6350A0–0x63511B pad cluster if the writer wants the
T-060-style containment note (head: VFM::dtor / ctor cluster).
Also a census-wording correction for the archive: T-063 §(e)2 "coherent epoch, anchored nowhere,
tail-jmp-only thunks" → **anchored: RTTI-named VFX_FormatManager/VFX_ParameterInfo epoch,
container-machinery tier; thunks explained (nested-dtor add-$0x4 adjust + SEH pads)**.

### Exclusions / non-claims
- No semantic method names invented for [A] (role names deferred to a values-era row; the device
  vft slots 0x118/0x130/0x14C and the render-state setter shape are cited, not renamed).
- `0x4915E0` excluded from [A] (already live-named).
- Gfx_RS_* family: dismissed as separate epoch §1g. `VFX_Parameter`/`ParameterInfo`/`ParameterHolder`/
  `VEG`/`VEGInstance`/`VEGCache`: live names untouched — proposed labels create no collisions
  (`VFX_ParamRecord`-style new mints are NOT proposed; that was the B trap).
- No claim that the VFM singleton's own size is settled beyond ≥0x38 (parse-zero evidence); 44 stays
  superseded-not-renamed.

## 5. Protocol
Row status: both 17 orphans ADJUDICATED — **A = MINT-READY (single-owner proof + printed machine
test + loud prefix-artifact refutation); B = FINAL honest negative for the domain-mint, reclassified
from "anchored nowhere" to "anchored, container tier"**. T-063's K=17 becomes: 9(+12 composite)
mint-queued, 8 annotated-in-epoch. Zero IDB writes, zero 8745 queries, zero reservations beyond my
task file + this doc; README reservation released per coordinator order (claim b737f69, table flips
coordinator-side). File stays DIRTY until checkpointed.
