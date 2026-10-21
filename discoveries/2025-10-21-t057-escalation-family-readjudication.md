# T-057 — escalation-family re-adjudication: 273 escalated fns, per-entry owners, ZERO isolated

**EpicRaven**, row `00bfdb8`→claim `7667628`. OFFLINE lane unchanged: raw EXE + `artifacts/F3.json`
dump, no 8745, no IDB writes; writer slot stays CalmRaven's.

## 0. Pins & gate FIRST (standing rule)

- EXE md5 **`a68c22f3dd1c256ec9539a70e87cb434`** (= F3.json filemd5); F3.json mtime 2026-10-20
  22:39:50 — names below are PRE-T-055-rename locators only (LoudDragon stale-boundary note; this
  row's evidence is addresses + bytes throughout).
- `tools/ecx-flow.py selftest` — **4/4 PASS on this build BEFORE any verdict below**:
  T1 317 edges &0x7082F8 · T2 4×new(0xC8)→ctor_0x4A5BE0 + 1 strong/4 weak installs ·
  T2b embed known-positive ModelInstance+0x44 · T3 36×*[0x70B200].
- Flow build totals (supersede T-054's numbers — the tool gained tail-jmp edges, jump-table
  wake, and SEH/atexit **pad-thunk recognition**: `mov|lea [ebp-x],%ecx [; add $d,%ecx] ;
  jmp|call T` where the pad VA is itself dword-referenced in .rdata/.data — 3,253 pads found,
  3,204 vector/atexit `jmp` + 49 EH-`call` handlers; 48,581 edges; 345 tables).

## 1. Census method (re-runnable)

`python3 tools/ecx-flow.py families --cls X` (per-entry external/delegation/shape/slot table)
plus per-family `[Q1] caller-class tables + called-by`, `[Q2] table→thunk→family`, and the
transitive delegation resolution added for this row. Item-41's three queries per family are
answered mechanically by Q1-delegated / Q2-slots / Q3-full-window lines; the tables below are
the curated extracts. **Buckets: LIVE** = direct external flow · **DELEG** = flow via band
callers (callers-of-callers, transitively) · **PAD** = reached only through data-referenced
SEH/atexit pad thunks (compiler cleanup, host named by installer/handler scope) · **ISO** = none.

| band | entries | LIVE | DELEG | PAD | **ISO** |
|---|---|---|---|---|---|
| cls_0x50dbb0 | 136 | 71 | 50 | 15 | **0** |
| cls_0x50db20 | 83 | 73 | 10 | 0 | **0** |
| cls_0x4542d0 | 54 | 23 | 25 | 6 | **0** |

**273/273 escalated fns now carry a named flow story; zero anonymous orphans.** vft-slot census:
0 slots into ANY of the 273 entries across .rdata+.data+.data1 (Q2 blanket answer; scanner
proven-live by T-054's 445350 thunk-table positive and Gfx/TexMgr pins).

## 2. cls_0x50dbb0 — std::_Tree instantiation aggregate: CONFIRMED FOREIGN, hosts now NAMED

T-033's "foreign-family" verdict for this band STANDS and is now *evidenced per entry*: these are
per-instantiation container methods whose code is shared machinery, each embedded in a named
host class at a fixed field offset. The 50 single-owner entries (host = sole external caller):

| band entry | host class | embed this= | note |
|---|---|---|---|
| 0x50CD40/0x50DBB0/0x50DC60/0x50DFA0 | `cls_0x49c610` (ResourceSystem) | this+0x108 | map field @ResourceSystem+0x108 — incl. the band HEAD itself |
| 0x51EFD0/0x51FB10/0x51FFC0/0x5201D0 | `cls_0x55dc80` (→Gfx_Texture_Manager) | this+0x24 | the manager's map+list field (KeenStorm §d map at mbr_0x24/0x28) |
| 0x54C300 | `.?AUEntity@@` | this+0x394 | |
| 0x49E8C0 | `.?AVGfx_ModelInstance@@` | this+0xB8 | |
| 0x4B2E80/0x4B43F0 | `.?AVFont@GUI@@` | this+0x2C | |
| 0x522FF0/0x5233E0 | `.?AVManager@GUI@@` | this+0x48 | |
| 0x4B9A50/0x4BB2D0 | `cls_0x4b8f70` | this+0xD8 | |
| 0x56FB70/0x570160 | `.?AUMap@@` | this+0xBC | |
| 0x5CB1B0 | `cls_0x50b560` | this+0x18 | epoch trio continues: 0x5CAB80/0x5CAF10←sub_5CB5C0, 0x5CAC40←sub_5CB6C0, all this+0x18 |
| 0x4910D0/0x491110/0x492C40 | `cls_0x490150`/`cls_0x4903e0` | this+0x1C/0x50 | Gfx epoch (the consumer-not-member class) |
| 0x592EF0 | `.?AVCEngineInterface@@` | **this−0x10** | negative-disp subobject view (adjustment shape) |
| 0x54E9A0/0x54EC60 | (engine-factory epoch) | **&0x70BE14** | pointer-slot-ish global pins, singleton-adjacent |
| 0x559060/0x559D60 | (Gfx stats epoch) | **&0x70BED8** | |
| 0x523320 | `.?AVFragment@GUI@@` | **&0x707E48** | GUI-root consumer, not member |

Multi-owner LIVE entries (21) show the same shape across ≥3 hosts (Entity/GameActionEquip/
DialogueWindow/VFX_VEGInstance/Map/MapEffect top the aggregate edge counts) — textbook template
instantiation. The 15 PAD entries are vector/atexit dtor pads + EH handlers (e.g. 0x507550 via
pad 0x635653 (`mov -0x14(%ebp),%ecx; add $0x1c,%ecx; jmp`), the pad dword-referenced at
.rdata 0x6E528C inside a contiguous pad-dispatch array — the 0x506/507 epoch cleanup family
adjacent to T-054's byte_707CE0 lane).

**Verdict: PROVEN-FOREIGN as a class — no owner to mint; annotation material per entry.**
T-033's blanket "foreign-family" wording can upgrade to "per-entry attributed instantiation
family (hosts named above)".

## 3. cls_0x50db20 — std::basic_string/ios machinery aggregate + THE T2-12 SETTLEMENT

Census: 73 LIVE / 10 DELEG / 0 PAD / 0 ISO; the biggest fan-in aggregate of the three
(top owner-edges: cls_0x424a70×111, GameActionEquip×106, cls_0x41a2b0×74, cls_0x4d8d70×65,
Graphics×63, ResourceSystem×57 …). Band members include the string primitives themselves
(0x401B20 string ctor, 0x401BD0 assign/append, called with `this = host_field` from ~every
class that owns a string member). **Verdict: PROVEN-FOREIGN (STL string/ios machinery); the
band is not a class and never was.**

**T2-12 (CEngineInterface embeds), embed-tier instead of member-name guess:**

1. CEngineInterface's own ctor identified: **0x580330** (`movl $0x69A95C,(%esi)` +
   init-delegate 0x495600) and variants 0x580350/0x580360. Construction census found
   **33 embedded-ctor sites across >=27 distinct host classes** — not 5. The batch-2 five
   (`cls_0x558740`+0xC4, `cls_0x5B6400`+0x3C, `cls_0x4395A0`+0x8, `cls_0x5B4EB0`+0xC,
   `cls_0x5B7C10`+0x18) ALL confirm at embed tier (edge `this=in-ecx+off` ->
   0x580330/0x580350, sites 0x55877D/0x5B644E/0x4395C2/0x5B4ED2/0x5B7C4C). Named RTTI
   interface family joins the list with measured offsets: GameCSMenu+0xD54 (0x568E8F),
   GameCSSkillMenu+0xD58 (0x435CDB), CMainmenu+0x10 (0x4409CD), CMessageBox+0x28 (0x44149C),
   CMiniMap+0x34 (0x44237D), COptions+0x34 (0x44370D), COptionsGame+0x10C (0x444576),
   COptionsVideo+0xCC (0x4496E6), COptionsMultiplayer+0xC (0x44922D), CLoadingBar+0xC
   (0x4407FD), GameDialogue+0x27C (0x4380F1), DialogueInterface+0x278 (0x594402),
   CFloatingText+0x37550 — a 222KB host (0x5903A0), CGameFloatingText+0x37554 (0x43911C),
   EditToken+0x0-primary (0x5B3E92), cls_0x455E40+0x1E0, cls_0x43CD50+0x2C0, cls_0x42A780
   +0x128, cls_0x41A2B0+0x29C, cls_0x434F70+0xB0, cls_0x44DA10+0x18, cls_0x448C40+0x74,
   cls_0x426720+0x38, cls_0x433A90+0xC, cls_0x4407C0/440990/441420/442340/4436D0/444530/
   4491F0/4496A0 (the ctor twins of the interfaces above) — plus **T-054's hotkey outer
   `cls_0x4468C0` +0x198 (site 0x446928)**. Every GUI interface class embeds exactly one.
2. **COMPOSITION PROVEN, BASE DISPROVEN (RTTI-level):** vft 0x69A95C carries exactly ONE COL in
   the image — .rdata 0x6D6510 = {sig 0, offset 0, cdOffset 0, TD `.?AVCEngineInterface@@`,
   CD 0x6D6500} — and it is the ONLY vft sharing that COL. An MSVC non-primary BASE subobject
   forces a per-derived-class vft copy with a derived-TD COL and nonzero offset; 27+ distinct
   classes constructing into ONE complete-object table means every site embeds a **standalone,
   complete CEngineInterface value member**. Batch-2's "base-subobject minority alternative" is
   hereby a CLOSED falsification, not a pending hypothesis.
3. The +0x84/+0x8C/+0x8D member-slot stores (the old member-name-guess anchors) are NOT
   CEngineInterface-specific: the owner-keyed store census shows those displacements written
   by 20+ classes' own container/string fields (CSound, Manager@GUI, GameActionEquip,
   AnimMeshInstance, G3D_Mesh, Map, OCTREE_Camera/OCTREE_Rect2D, VFXEFFECTTYPE_* structs,
   cls_0x41a2b0, cls_0x50db20 members...). Superseded by per-owner store evidence; keep them
   only as per-class field annotations, never as embed inference.

## 4. cls_0x4542d0 — mixed clone chunks (Granny/GUI-dialogue/model-map epoch): CONFIRMED FOREIGN

Census: 23 LIVE / 25 DELEG / 6 PAD / 0 ISO (the prior single orphan 0x4BB6B0 resolved by the new
EH-`call` pad rule: handler 0x4BB755 — `mov -0x14(%ebp),%ecx; call 0x4BB6B0` — dword-referenced
at .rdata 0x6E12EC, i.e. SEH cleanup call inside `ctor_0x4BB6E0` scope). Entry chunks cluster by
host field: 0x4A2630 = the model/anim map field called at `Entity+0x370`/`+0x3C0`/`+0x3CC` via
seven thin wrapper classes (cls_0x467780/4a26d0/4a2750/4a1f30-family = the Granny epoch),
0x4BB390/0x4BB6E0 at `cls_0x4bc5f0+0x18`, 0x57E230/0x57E9F0/0x580180 at `AreaMap+0xA8`,
the 0x596-0x598 chunk fan under sub_598630/sub_5994D0 dialogue-epoch owners, and the 0x453-0x455
chunk under **GameActionEquip** (b5-noted tail alias family). Multi-host string/tree clones like
§2/§3. **Verdict: PROVEN-FOREIGN aggregate; per-chunk hosts named.**

## 5. MINT-READY queue for the writer wave

**No new class mints come out of this row** (that IS the census-honesty result: three of the
largest T2 blocks are proven non-classes). Queue = annotations only:

1. ~20 per-entry `instantiation-of-<HOST>@+0x<DISP>` comments for the §2 single-owner table
   (50dbb0) and §4 chunk heads (4542d0) — kills future "unknown host" re-mining.
2. `embeds@CEngineInterface@+0x<OFF>` on the confirmed ctor-owner classes (the batch-2 five +
   the newly-enumerated ≥22, sites listed in §3.1) + one comment at 0x580330 recording it as
   `CEngineInterface::ctor_0x580330 (vft 0x69A95C)` construction site; record the COL-0x6D6510
   composition proof at 0x69A95C (or the CEngineInterface vft anchor).
3. PAD/EH entries (§2 list + 0x4BB6B0): comment `SEH/vector-iterator pad-reached cleanup —
   not a dispatchable method` to stop future xref hunts.
4. Board wording: T-033 closure's "~250 foreign-family" → "273 attributed: 167 LIVE-host-named,
   85 DELEG-chain-attributed, 21 PAD-cleanup, 0 orphan" with the §1 table as the citation.

Re-verify at write time per standing rule; my side is md5-pinned.
