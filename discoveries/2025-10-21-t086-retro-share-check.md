# T-086 — Jurisprudence sweep: the FIXED retro gate run over every landed mint — 0 mint retractions, 2 per-fn re-reads escalated, 3 machine-silences disclosed

**HappyStorm** `01a16092-41cf-7127-936b-377a304178a4`, claim `c41a463`. Offline lane: pinned
cache + frozen export + committed wave-dumps. **Zero IDB writes, zero 8745 queries.** Driver:
`tools/t086-retro-share-check.py` (reserved, uncommitted — include-or-discard is the coordinator's
call; loads the flow cache once, emits every raw share-check report).

## 0. Gate + band sourcing (as APPLIED, not as proposed)

`selftest` — **6/6 PASS** first (T5 line includes the fb3d03f field-keying counterexample pair).
Bands were NOT read off the prose ledgers — they were computed from the on-disk wave dumps:

- **b2-b6 survivors** = EAs whose 2024-export class was `cls_`-anonymous (or absent) and whose
  LIVE name in `t055-predump.json` (state at b6 close, disk==live proven at hand-back) carries a
  non-vendor class label — grouped by label;
- **T-055's 41** = name-diff `t055-predump → t055-postdump` grouped by new label;
- **T-064's 12** = name-diff `t064-predump → t064-postdump`;
- **queued bands** (audited pre-landing — the retro gate's actual forward value): T-073 [A] nine +
  [A2] composite three + [B] annotated eight; T-075 §8 GUI::Manager 27 (minus the 0x619B38 CRT
  exclusion); T-064-[B]-REJECTED five (must stay SHARING — the jurisprudence counterexample);
- **vendor/foreign labels** (std::/ATL:: renames from T-057-era attribution) — REPORTED, excluded
  from the flip list per T-060's out-of-region/exclusion law.

Self-corrections during the pass (item 35 applied to myself, printed not papered): first run
produced 3 fake bands named `call_cls_0x50dbb0::meth_*_with_F3` — IDA auto-generated call-site
labels, not board mints; driver `anon_label` now treats `call_*`/`*_with_*` shapes as artifacts and
re-run; vendor labels were silently dropped in the first patch, corrected to a report-only lane.

## 1. Per-band verdict table — LANDED (64 bands: 56 PASS / 5 SHARING / 3 NO-EVIDENCE)

| band | fns | verdict |
|---|---|---|
| F3::Display | 2 | PASS |
| F3::GameCharacter | 1 | PASS |
| F3::GameCreature | 5 | PASS |
| F3::GameItem | 1 | PASS |
| F3::GamePlayerAvatarManager | 54 | PASS |
| F3::GameProjectile | 1 | PASS |
| F3::GameWeapon | 13 | PASS |
| F3::GameWorld | 1 | NO-EVIDENCE |
| JE::AnimationCache | 1 | PASS |
| JE::CActionbarInterface | 28 | PASS |
| JE::CRect | 1 | PASS |
| JE::Common | 1 | PASS |
| JE::DialogueManager | 1 | PASS |
| JE::G3DMeshHolder | 2 | PASS |
| JE::G3DTransformHolder | 6 | PASS |
| JE::GfxCameraHolder | 6 | SHARING |
| JE::Gfx_AnimMeshInstance | 13 | SHARING |
| JE::Gfx_AnimationData | 1 | PASS |
| JE::Gfx_AnimationObject | 11 | PASS |
| JE::Gfx_D3D | 20 | PASS |
| JE::Gfx_G3D_Mesh | 3 | PASS |
| JE::Gfx_HeightFog | 1 | PASS |
| JE::Gfx_Light | 1 | NO-EVIDENCE |
| JE::Gfx_RenderManager | 2 | PASS |
| JE::Gfx_SkinMesh | 1 | PASS |
| JE::Gfx_TCFX_Param | 1 | PASS |
| JE::Gfx_Texture | 1 | PASS |
| JE::Gfx_Texture_Manager | 17 | PASS |
| JE::HotKeyWindowFormatTable | 2 | PASS |
| JE::MapMaskManager | 2 | PASS |
| JE::MemorySystem | 1 | PASS |
| JE::ModelCache | 1 | PASS |
| JE::Octree2Gfx | 3 | PASS |
| JE::Res | 1 | PASS |
| JE::ResourceCache | 4 | PASS |
| JE::ResourceFileManager | 3 | PASS |
| JE::ResourceSystem | 47 | SHARING |
| JE::Script_Small | 1 | NO-EVIDENCE |
| JE::StringTree | 139 | SHARING |
| JE::Vector3 | 2 | PASS |
| JE::World | 2 | PASS |
| F3::GameWeapon | 2 | PASS |
| JE::CCharacterGenerationInterface | 1 | PASS |
| JE::CDebugInterface | 7 | SHARING |
| JE::CFeedbackInterface | 1 | PASS |
| JE::CInventoryInterface | 1 | PASS |
| JE::CMainmenuInterface | 1 | PASS |
| JE::CMessageBoxInterface | 1 | PASS |
| JE::CMiniMapInterface | 1 | PASS |
| JE::CNASInterface | 1 | PASS |
| JE::COptionsGameInterface | 1 | PASS |
| JE::COptionsInterface | 1 | PASS |
| JE::COptionsLoadInterface | 1 | PASS |
| JE::COptionsMultiplayerInterface | 1 | PASS |
| JE::COptionsVideoInterface | 1 | PASS |
| JE::CPipBoyAddNoteInterface | 1 | PASS |
| JE::CPipBoyInterface | 1 | PASS |
| JE::Gfx_BufferManager | 4 | PASS |
| JE::Gfx_RenderManager | 1 | PASS |
| JE::Gfx_SkinMesh | 1 | PASS |
| JE::HotKeyWindowFormatTable | 8 | PASS |
| JE::Map | 1 | PASS |
| JE::ResourceFileManager | 3 | PASS |
| JE::G3D_NodeRecordSet | 12 | PASS |
## 2. Queued bands (5 bands: 4 PASS / 1 intentional SHARING) + rejected-band regression

| band | fns | verdict | expectation |
|---|---|---|---|
| T-073[A] Gfx_RenderManager ext (9) | 9 | PASS | |
| T-073[A2] composite (ctor_0x4950D0+cls_0x4903e0 x2) | 3 | PASS | |
| T-073[B] cls_0x4fc0e0 (annotated, no mint) | 8 | PASS | |
| T-064[B]-rejected (expected SHARING) | 5 | SHARING | |
| T-075 queued GUI::Manager (cls_0x4b28a0, -0x619B38) | 27 | PASS | |
The `[B]`-rejected five (0x434120/250/2C0/4760/4910) still print **SHARING** under the fixed
field-keying — their dual views (−0xB0/−0xC4 into two hosts each storing their own interface vft)
are GENUINE distinct holder fields on distinct classes, so the corrected machine keeps the [B]
retraction alive instead of flattening it. That is the T5 both-truths property verified against
history, not just against the two banked rows. T-055's flagship re-reads also hold:
`HotKeyWindowFormatTable` 8-PASS, `Gfx_BufferManager` 4-PASS, all 14 ctor-own-vft single-fn joins
(incl. `CNASInterface`) PASS, `CActionbarInterface` 28-PASS, `Gfx_D3D` 20-PASS (singleton
&0x7082F8, one field key), `Gfx_Texture_Manager` 17-PASS (*[0x70B200] → `fg:` single global slot),
T-064 `G3D_NodeRecordSet` 12-PASS, T-075 `GUI::Manager` 27-PASS, T-073 nine + composite PASS.

## 3. Vendor report-only lane (9 bands: 8 PASS / 1 SHARING — NOT flip-listed)

| band | fns | verdict |
|---|---|---|
| ATL::CStringT_wchar_t_StrTraitMFC_wchar_t_ATL::ChTraitsOS_wchar_t___::CStringT_wchar_t_StrTraitMFC_wchar_t_ATL::ChTraitsOS_wchar_t___(wchar_t_const___int_ATL | 1 | PASS | |
| std::_String_base | 1 | PASS | |
| std::basic_string_char_std::char_traits_char__std::allocator_char__ | 1 | PASS | |
| std::basic_string_ushort_std::char_traits_ushort__std::allocator_ushort__::append(std::basic_string_ushort_std::char_traits_ushort__std | 1 | PASS | |
| std::basic_string_wchar_t_std::char_traits_wchar_t__std::allocator_wchar_t__ | 4 | PASS | |
| std::ios_base | 2 | PASS | |
| std::istreambuf_iterator_char_std::char_traits_char__ | 1 | PASS | |
| std::istreambuf_iterator_char_std::char_traits_char__::equal(std::istreambuf_iterator_char_std | 1 | PASS | |
| std::locale | 2 | SHARING | |
`std::locale 0x605B11` SHARING = CRT/STL machinery reached from multiple embedders — exactly the
shape T-060 closed as out-of-region proven-foreign. Reported for completeness; jurisprudence does
not re-litigate vendor labels.

## 4. The five landed SHARING bands, DECODED — none retracts a mint; two get per-fn re-reads

**Common axis:** share-check answers "whose FIELD deref produced this `this`". A band-level
SHARING is a threat only when the mint's claim was *membership in one host*. All five landed bands
were minted as **standalone class identities** (b2 struct-mint, b6 RTTI-join, b4/b5 named-class
aggregations, T-055 string/own-vft join) — multi-host reach at the field tier is either consistent
with, or orthogonal to, those claims. Per band:

### 4.1 `JE::GfxCameraHolder` (6) — machine-correct referencer census, claim unaffected
Flipping candidates 0x424880/0x4248F0/0x429D20: fields {+0x18(3 sites), +0x1C(3), +0x8C, +0xB8,
+0xD78, +0x290} = **the six host slots holding GfxCameraHolder pointers — the exact six
`new(0x280)` owners T-073 §1c enumerated** (engine ×2 @0x5C0D19/0x5C0D9D, GUI::Manager+0x8C,
CharGen+0xB8, Inventory+0x290, DialogueWindow+0xD78). b2 minted it as a standalone struct
(ledger: "embedders=0"); nobody claimed membership in one host. Multi-host POINTER fan of an
independent class = the machine describing referencers, correctly. Contrast with the [A] nine:
their every path collapsed to `fo:+0x164` — ONE field of ONE class — and the gate reads
SINGLE-OWNER/PASS. The field keying distinguishes the two situations cleanly. No re-adjudication.

### 4.2 `JE::Gfx_AnimMeshInstance` (13) — shared heap resource; RTTI-named identity stands
0x4A5640/0x4A5D70/0x4A62B0 reached via {+0x2C4 (VFX_Model), +0xD9C, +0xC, +0x4C, +0x8}; ctor tier
0x4A5BE0: heap=4, ctor-sites=4 (3× VFX_Model + 1× ModelInstance). The b6 proof was RTTI COL/TD +
the `[VFX_Model+0x2C4]` install — an **identity** proof, not a uniqueness-of-holder claim. Two
constructing hosts = a demand-shared resource object (board's own Gfx_Texture_Manager pattern,
which PASSes via its single global slot). The class name survives (it is its own RTTI class);
writer note: a comment recording the multi-host construction fan (VFX_Model/ModelInstance) would
upgrade b6's single-site phrasing at annotation tier. No retraction.

### 4.3 `JE::ResourceSystem` (47) — REAL per-fn findings: 3 candidates carry CSound-epoch field
edges alongside the singleton global — escalated, both readings named
Singleton evidence is overwhelming (`fg:0x707DA8` read via 329+17+13+51 paths); the flips:
- `0x498800` ALSO reached `*[obj+0x4]` from 0x4C74AD/C8, 0x4C75CD, 0x4C765F (CSound epoch);
- `0x499220` ALSO reached `*[obj+0x4]` from 0x4C6E6A..0x4C70DB (CSound epoch, 10 sites);
- `0x498EB0` ALSO reached `*[obj+0x50]` from 0x4C8596.
Two readings, deliberately NOT collapsed to one (guess is failure): **(R1) cached-alias** — the
+0x4 field stores the ResourceSystem singleton pointer (per-object cache of `*[0x707DA8]`), so
both routes land on the SAME instance — precedent: T-054's `byte_707CE0 ctor self-cache` catch;
**(R2) residual helper-theft** — the three fns are generic helpers CSound calls with its own
`this`, sitting in the ResourceSystem address span (T-036 artifact-floor family; T-053 already
merged 4 real members OUT of this span into ResourceFileManager). Discriminating query (cheap,
writer/coordinator-side): read the STORE sites into [csound-family+0x4] — if they load
`*[0x707DA8]`, R1 holds and the band stands; if they construct something local, those fns need
re-parenting. **FLIP-LIST entry #1 (per-fn, band unharmed either way).**

### 4.4 `JE::StringTree` (139) — reusable embedded type + the already-open T-059 heterogeneity note
Flips 0x4213F0/0x458A20/0x458EF0 view constants {+0xF8 (Map side), +0x2C4 (Entity), +0x218
(ResourceSystem cls_0x49c610), +0x4} = different hosts embedding their OWN StringTree instances at
different offsets — multi-host embedding of a value/component type, not one-instance sharing. The
b3 mint was standalone-by-its-own-vft. Also known-open: T-059's item-43 amendment flagged
`0x4B2DD0`/`0x56EE40` (currently `JE::StringTree::meth_*`) as the app-wide std::string dtor
mis-aggregation with a split-comment queue already queued — this band's machine-SHARING rows are
consistent with that declared caveat, nothing new to retract. **Reported; no escalation beyond the
existing split-comment queue.**

### 4.5 `JE::CDebugInterface` (7) — one candidate on TWO Client holder slots: escalated, slots-vs-class named
`0x558190` reached `*[client+0x8C]` @0x58F0F0 AND `*[client+0x90]` @0x58F138 — two fields of the
SAME host class (`.?AVClient@@`), inside the lazy UI-cluster bootstrap T-017 already mapped
(0x58f010 region; T-062 documented `UseSkill = dialog iface client+0xA4` — Client is a field-dense
interface registry). Rule-1 literally says >=2 holder fields ⇒ SHARING — but its PURPOSE is rival
class-identity, and both readings stay open: **(S1)** Client holds two pointers to (possibly the
same) CDebugInterface instances (slot-count ≠ class plurality — the T-073 prefix-vs-field lesson
one level up: slots-vs-class identity axis); **(S2)** one slot points elsewhere and 0x558190 is
adjacency-theft. Discriminating query (writer-side, one read each): the STORE sites into
[client+0x8C]/[client+0x90] — if both store the output of CDebugInterface's ctor/new, S1 holds and
the band stands with an annotation; if not, that fn re-reads. The other 6 candidates printed
SINGLE-OWNER (incl. 0x557A50/0x558510 via the -0xC4 view — correctly single now: ONE view constant,
the [B] counterexample family preserved). **FLIP-LIST entry #2 (per-fn, band unharmed unless the
stores say otherwise).**

## 5. Machine-silences (3 bands) — NO-EVIDENCE ≠ failure, join-layers named
`F3::GameWorld ×1`, `JE::Gfx_Light ×1`, `JE::Script_Small ×1`: single-fn bands whose fns carry ZERO
external flow-shape edges — the machine has nothing to say. Their joins were landed on OTHER
evidence tiers (RTTI COL/TD sweep for Gfx_Light; string/epoch joins per b5/b6 ledgers) which the
flow census neither confirms nor attacks. Per this row's own law ("verdicts only where the machine
speaks") they are printed as **machine-silent, disclosed, not counted as failures**. If the board
later wants tier-crossed automation, that is a new tool question, not a jurisprudence gap.

## 6. Quotable close (the honest form of the proof line)

**"64 landed bands + 5 queued bands + 9 vendor bands were run through the FIXED retro gate:
56+4 native PASS (every pending queue — T-073 nine, T-075 twenty-seven, T-064 record-set, all 14
ctor-joins incl CNAS — passes pre-landing), the [B]-rejected five still print SHARING exactly as
retracted, and of the five landed band-level SHARINGS ZERO retract a mint — two per-function
re-reads are escalated with sites (ResourceSystem×3 CSound-edge candidates, CDebugInterface×1
Client-dual-slot), three decode to the machine correctly describing multi-host reference of
standalone classes. The unknown column stayed empty and the gate now has history behind it."**

## 7. Coverage + constraints
78 domain/queued bands (every landed rename from the wave-dump diffs incl. b6 RTTI re-joins, every
accepted queue) + 9 vendor report-only bands. Raw per-candidate reports (fields + sites-under-field
+ ctor-sites) banked DIRTY at `discoveries/t086-share-check-raw.txt` and regenerate via `tools/t086-retro-share-check.py`
@ pinned md5 `a68c22f3…`. Zero IDB writes, zero 8745 queries; reservations: task file + this doc +
the driver. Deliverable DIRTY. Two lanes live and untouched: KeenStorm (port), HappyEagle (boot).
