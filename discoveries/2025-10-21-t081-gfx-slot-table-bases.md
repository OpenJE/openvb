# 2025-10-21 — T-081 (F1): the contested .rdata table run is ONE GUI widget family — per-base OWNER table + the citation key that re-keys every "+0xNN" in the corpus

TrueCastle, sessionId `01a16096-c00e-7404-8afe-e521a9bb7240`. Carried over from SwiftMoon's T-072
sweep (F1), filed by LoudDragon `01a15f82` (`36db5ce`), claimed `c1a1c06`. Target ORIGINAL_REFERENCE
READ-ONLY; raw bytes of the md5-pinned demo `a68c22f3dd1c256ec9539a70e87cb434`; zero IDB writes;
zero doc edits (this file only — correction blocks are the checkpoint's to paste).

## 0. Verdict

**EVERY contested table base resolves to an OWNER — zero FINAL-ambiguous rows.** The disputed run
is not a Gfx_Engine neighbourhood at all: it is the **GUI widget inheritance family** rooted at
`GUI::Base` + `GUI::HeapTracker` (MI). `Gfx_Engine` has exactly ONE vft (`0x6AE7EC`, 120 slots) and
none of the ten §8a addresses is a table base — they are **slot addresses inside ten real primary
tables**, which is the whole F1 disease stated mechanically. F2's mis-anchor is confirmed and
extended (its "base 0x66E574" is itself a phantom mid-table address). The F3.json
"flattening-artifact" suspicion is **REFUTED for the vft rows** (export matches raw byte-exactly,
both Window views included) but **CONFIRMED for the size column** (family estimates, see §6).

Selftests before any verdict: `ecx-flow selftest` **5/5** (incl. T5 share-check regression);
automated RTTI walker validated on the hand-proven T-075 known-positive
(`0x66f2cc → COL 0x6d3dc8 → .?AVManager@GUI@@`, item 35); master-index rows for
GUI::Window reproduced exactly (1 slot @0x66e4cc, 75 slots @0x66e4d4).

## 1. RTTI geometry law (VS2003 x86, no RVA encoding) — and one SECTION trap worth harnessing

- **COL** `[vft-4]`: `{+0:0, +4:viewOffset, +8:0, +0xC:TD*, +0x10:CHD*}`. viewOffset is the
  primary/secondary discriminator (0 = complete/primary view; 24 = secondary).
- **CHD**: `{sig 0, attrs, maxNodes, BaseArray*}`; BaseArray[i] = **MTD*** (one deref — skipping it
  yields garbage, which is how a first pass reads phantom nodes).
- **MTD**: `{+0 TD*, +4 numParents, +8 subobjectOffset(+24 for HeapTracker), +0xC pMF(-1)}`.
- **TD**: `{+0 pMD, +4 spare 0, +8 ".?AV…@@"}`.
- **TRAP (new, item 8c-kin):** the RTTI TD+name cluster for this epoch links at **0x6FAF2C..0x700…**,
  i.e. ABOVE the `.rdata` *stated* virtual size (0x647000+0xa9790=0x6F0790) — the bytes are live (the
  loader maps the raw extent 1:1) but any walker that bounds TD candidates to .rdata-virtual finds
  **zero names everywhere**. A naive scan blaming "no RTTI" would be another scanner artifact, not a
  fact about the binary. Bounds must be 0x647000..0x708000.

## 2. Per-table ownership (the contested set, base-by-base)

| table base | viewOff | TD (demangled) | CHD chain (BaseArray self→bases) | slots | vft stores (item 49) | OWNER verdict |
|---|---|---|---|---|---|---|
| `0x669d74` | 0 | `GUI::HeapTracker` | maxNodes 1 (self) | 1 | `c7 01`@0x4a7532; `c7 46 18`@0x4b083e (mid-phase) | **OWNER GUI::HeapTracker** — export size **0** = empty policy base (item-42 zero-size ⇒ never a standalone object, always a +24 view/aspect) |
| `0x669d80` | 0 | `GUI::Interface` | maxNodes 2 (self, Base) | 6 | ctor @0x495521 + dtor @0x49562c (`c7 06`) | **OWNER GUI::Interface** — confirms T-061's parent-vft precedent (0x669D80 cited there) |
| `0x66e4cc` | **24** | `GUI::Window` | shared CHD 0x6d3d2c (Window; Base; HeapTracker@+24) | 1 (slot0=0x4b0a40) | `c7 46 18` @0x4b084b (ctor), @0x4b0a7a (dtor) — stored at **[this+0x18]** | **OWNER = GUI::Window, SECONDARY view** (the HeapTracker-interface view). REAL table, not an artifact — the master-index row is faithful but unlabeled |
| `0x66e4d4` | 0 | `GUI::Window` | same CHD | 75 (ends 0x66e600) | `c7 06` @0x4b0842 (ctor), @0x4b0a71 (dtor) | **OWNER = GUI::Window PRIMARY** |
| `0x66ea1c` | 0 | `GUI::Fragment` | maxNodes 2 (self, Base) | 17 | ctor+dtor @0x4b24d8/@0x4b26a2 | **OWNER GUI::Fragment** — T-062 slot-12 row byte-verified correct (§5) |
| `0x65575c` | 0 | `GameDialogueWindow` | 5: GDW→DialogueWindow→Window→Base+Heap | 76 | ctor+dtor @0x437eeb (`c7 06`) | **OWNER** |
| `0x65d15c` | 0 | `POIWaypoint` | 5: POI→Label→Window→Base+Heap | 106 | @0x45880b/@0x4588df | **OWNER** |
| `0x66c9bc` | 0 | `GUI::Button` | 4: Button→Window→Base+Heap | 76 | @0x4a6f04/@0x4a6fe6 | **OWNER** |
| `0x66d43c` | 0 | `GUI::Label` | 4: Label→Window→Base+Heap | 106 | @0x4a8f62/@0x4a910e | **OWNER** |
| `0x670cac` | 0 | `GUI::Edit` | 5: Edit→Label→Window→Base+Heap | 106 | @0x4c1263/@0x4c51e8 | **OWNER** |
| `0x697e4c` | 0 | `CSMenuLabel` | 5 | 107 | @0x568432(`c7 01`)/@0x568cd4 | **OWNER** |
| `0x69dcc4` | 0 | `CFloatingLabel` | 5 | 107 | @0x58fa34/@0x58faaf | **OWNER** |
| `0x69ebcc` | 0 | `ReplyLabel` | 5 | 106 | @0x591bcf/@0x591c12(`c7 01`) | **OWNER** |
| `0x69eea4` | 0 | `DialogueWindow` | 4 | 76 | @0x596473/@0x596643 | **OWNER** |
| `0x6ae7ec` | 0 | `Gfx_Engine` | 3: Engine(numPar 2)→Gfx_System_Interface(off 0)+`Singleton<Gfx_System_Interface>`(off **4**, no vft) | 120 | ctor @0x5bfe45 (`c7 06`, t007 G2a — same register-indirect mode as T-075!) + dtor @0x5c1cf1 | **OWNER Gfx_Engine — the ONE and ONLY engine table** |
| `0x67140c` | 0 | `CSound` | 1 (self) | 3 | @0x4c7d10/@0x4c8121 (the latter inside the fn T-062 called "GfxCursor ctor 0x4c8100") | **OWNER CSound** — NOT any Gfx_Engine/manager view; T-062's "vft 0x67140C→CSound COL 0x6D4098" is hereby byte-confirmed |

Family shape: every derived table's slot-50 (+0xC8) is the inherited skin-binder `0x4b06c0` and
slot-52 (+0xD0) the skin-name setter `0x4b06a0` — verified for ALL TEN §8a addresses ([slot]=0x4b06c0
and [slot+8]=0x4b06a0 in every one; §8a's "ten vftables carry 0x4b06c0" is TRUE as a fact, FALSE as
table bases). Window ctor `~0x4b0820`/dtor `0x4b0a40` install primary+secondary in sequence
(`call GUI::Base::ctor_0x4b2720`; `[esi+0x18]=0x669d74` mid-phase → `0x66e4cc`; `[esi]=0x66e4d4`) —
the exact MI ctor-chord that produces "register-indirect store" negatives under disp32-only censuses
(item 49 validated on the GUI side too).

## 3. Why the ambiguity existed (F1 diagnosis)

Three address-styles were conflated across the corpus, all written like table starts:
1. **real bases** (COL at [-4]): 0x66e4d4, 0x66ea1c, 0x6ae7ec, the ten family primaries;
2. **secondary-view tables** sitting SHOULDER-TO-SHOULDER whose [-4] is the PRIMARY's COL
   (0x66e4cc's next slot IS 0x6d3d3c — the T-075 Font/Skin adjacency trap reborn at Window scale);
3. **mid-table slot addresses** quoted as if bases (the §8a list; F2's 0x66E574 — [0x66E574-4] is
   CODE 0x4af390, no COL: a phantom by definition).
Rule-of-thumb now machine-verified here: **a base is a base iff [-4] is a COL with a .?A-name TD —
otherwise it is a slot inside somebody else's table.** Table start = walk back over code dwords
until that test hits.

## 4. Gfx_Engine side of F1 — resolved explicitly

- Every "+0xC4(196) / +0xD8(216) / +0xDC(220) / +0xE0" citation in GUILoadingRecon §2/§7/§8d/§8e and
  t007 §G2 indexes the **engine primary 0x6AE7EC** and ALL of them are byte-correct:
  [0x6ae7ec+0xC4]=0x5c0a90 ✓, +0xD8=0x5c0930 ✓, +0xDC=0x5c0970 ✓, +0xE0=0x5c0940 ✓ (slots 49/54/55/56,
  4×slot law holds, F3.json 120 slots == raw table length 120 == export vft row).
- There is no "+C manager view" table: the engine's second base is
  `Singleton<Gfx_System_Interface>` at object **+4** and stores no vft (its contribution is the
  a3-style self-cache at [0x707CE4], ctor @0x5bfe33 — pointer global, not view). T-072 F1's fear of a
  second engine-dispatched table is CLOSED: exactly one.
- What looked like four adjacent engine tables is engine-vft + COL + **string blobs** immediately
  shoulder-shoulder (`0x5f786647` = "Gfx_" LE bytes; "Gfx_Engine::CreateDevice - Failed to create
  PostRenderViewport"/"Default Viewport" sit at 0x6ae9cc+ right after slot 119, and a %.x format
  string right before [-4]). Float/string dwords mistaken for slot0/COL pairs — the same
  data-below/above-table hazard as T-075's "slot1 = Font's COL" and the master-index `0x4b4345`
  string-fragment hits.

## 5. Per-CITATION resolution table (the load-bearing deliverable, per LoudDragon's sharpening)

KEY RULES (mechanical, apply in order to any "+0xNN (M)" or "vft[N]" citation):
- R1 — find the OBJECT the call-site loads `this`/`[ptr]` from; its vft = [this+0] (primary).
  Byte-verify: `[base + disp] == cited fn` against the table in §2. If the cited "base" fails the
  [-4]-COL test (§3), re-anchor it to the enclosing table via walk-back.
- R2 — decimal `vft[N]` (StartupAndSubsystemTable/DEFERRED-NOTES style) = N-th slot = bytes 4N of
  the OWNER-primary from §2 (subsystem classes included: JE::Network's 0x543770 Tick etc. are
  slot-indexed against their own primaries, never the tables here).
- R3 — "+0xNN(delta)" WITHOUT a named base and without a call-site object: mark
  **CITATION-INCOMPLETE** (needs the site's object pinned) — none of the currently-known citations
  fall into this class after R1 is applied (see rows).
- R4 — export `size` for family classes is a cumulative ESTIMATE (Base/Window/Fragment all "3384") —
  NEVER size-evidence; RTTI geometry + ctor-write span is (item 42 as re-used in T-075).

| citation (doc:line-ish) | text | resolves against | verdict |
|---|---|---|---|
| GUILoadingRecon L44-45 (load-default sites 0x52363b/6b4/6e7) | `vft[+0xDC]/[+0xC4]/[+0xD8]` | engine 0x6ae7ec (this from [0x707CE4]) | CORRECT, byte-verified |
| GUILoadingRecon L194-195, L274, L441, L456-460 | `+0xDC probe / +0xC4 index / +0xD8 object` | engine 0x6ae7ec | CORRECT |
| GUILoadingRecon L204-205 §7 G1 | "+0xC4(196)=0x5c0a90, +0xD8(216)=0x5c0930, +0xDC(220)=0x5c0970" | engine 0x6ae7ec | CORRECT (numbers = byte offsets = 4×export-slot ✓) |
| GUILoadingRecon §323 | "+0xE0(0x5c0940 add)" | engine 0x6ae7ec | CORRECT |
| GUILoadingRecon §8a L362-363 | "10 vftables carry 0x4b06c0 (0x655824,…)" | the ten addresses are SLOT ADDRESSES (slot 50 / +0xC8) of the ten primaries listed in §2 | FACT-TRUE, FORM-WRONG → checkpoint should restate as "slot +0xC8 in ten primaries: [list real bases]" |
| GUILoadingRecon §8b L377-386 | nine-suffix table 0x6faf2c..0x6faf50 | string array in **.data**, sentinel = Fragment's TD base address at 0x6faf50 (TD collision, benign) | CORRECT with section note |
| T-062 cursor doc L107-108 | "Fragment vft 0x66EA1C slot 12 (+0x30) = 0x4b1610" | Fragment primary 0x66ea1c | **CORRECT byte-exact** ([0x66ea4c]=0x4b1610) |
| T-062 cursor doc L114 | "GUI::Window vft 0x66E4D4 slot 12 (+0x30) = **0x4b06a0**" | Window primary: [0x66e4d4+0x30] = **0x4afc00** (live `JE::GUI::Window::virt_meth_0x4afc00`). 0x4b06a0 = **slot 52 (+0xD0)** of the SAME table (live `JE::GUI::Window::virt_meth_0x4b06a0`) | **MIS-ANCHOR** (F2 confirmed): role prose survives, coordinates must re-key to slot 52/+0xD0 |
| T-072 F2 remediation text | "lives at 0x66E5A4 = slot 12 of base **0x66E574**" | 0x66E574 has [-4]=0x4af390 CODE — no COL, **phantom base**; it is slot 40 of Window primary | F2's own fix needs this fix — do not bake 0x66E574 into the correction block |
| DEFERRED-NOTES "Subsystem slot table" | `vft[6] 0x543770`, `[4]0x45E040`, `vft[16] 0x545D60`, `[1]/[2] thunks` | R2 decimal slots of each subsystem class's OWN primary | CORRECT as style; add R2 pointer |
| vft-master-index L155-160 | GUI::Window 0x66e4cc (1) / 0x66e4d4 (75) / Fragment 17 / Manager 1 / Font 1 / Skin 4 | all faithful to raw; MISSING view labels on the two 1-slot/secondary rows | ANNOTATE "secondary @+0x18 (HeapTracker view)" — NOT artifact, NOT fabricated |
| T-054 "Gfx_Engine member +0x30 / texmgr +0x2C" | object FIELD offsets, not vft displacements | engine object layout | CORRECT kind once §-keyd (label FIELD vs VFT) |
| master `0x4b4345`-style data hits | string/float payloads mimicking slots | §1 trap + §4 string adjacency | pre-dismissed, re-confirmed |

## 6. F3.json artifact adjudication (item 42/43 honesty)

- **Vft rows: NOT flattened-artifacts.** Export == raw for Window(2 tables incl. the 1-slot
  secondary), Fragment(17), engine(120), CSound(3), and slot 50 == 0x4b06c0 byte-exact.
- **Size column: artifact-laden** — Base==Window==Fragment==3384 (cumulative OOAnalyzer estimate),
  HeapTracker==0 (empty base), Button 3448 > Window 3384 < Label 3684 — chain-monotone padding, not
  sizeof. Use ctor write-span (e.g. Window writes at least through +0xB8 in the fragment epoch —
  session-29 map) and RTTI MTD offsets (HeapTracker subobject @+24) as the size law; export size is
  context only. (Same finding as t007 §(f) Fragment 3384-vs-0x158 — now established as a FAMILY-WIDE
  property, which is exactly the F3.json-untrust line from T-040 this row had to re-confirm.)
- Manager 188 == cls_0x4b28a0 size holds because 188 is ALSO ctor-write-span-verified — the one
  trustworthy size in the epoch; coincidence, not rule.

## 7. Coverage statement (bounds)

- Raw dumps: 0x66e440..0x66e680, 0x66e9f0..0x66ea90, 0x6ae7c0..0x6ae940 (all dwords classified), plus
  ±8B COL probes and full-length slot walks of all 16 tables in §2 (table end = first non-code dword;
  every end verified as float/string/COL neighbour).
- Image-wide imm32 store census for every table base (0x401000..0x7128e0 all sections) — the item-49
  pairing on the receiving side; every base has exactly {ctor,dtor} store pair, none floating, none
  double-owned.
- RTTI walker: TD scan bound 0x647000..0x708000 (the §1 section-trap fix), COL+0xC TD resolution,
  CHD→BaseArray→MTD deref chain; validated on T-075 Manager known-positive and the master-index
  Window rows before being trusted on the rest.
- F3.json cross-reads: structures rows for all 16 demangled owners + Gfx_System_Interface/Singleton
  nodes.
- Citations inventoried: the four named docs (GUILoadingRecon, t007, t062-cursor ×2 addenda,
  DEFERRED-NOTES subsystem section, vftable-master-index L155-160) — 14 rows in §5; none left
  unresolvable; CITATION-INCOMPLETE count after R1: **0**.

## 8. What this row does NOT claim

- Derived-class HeapTracker SECONDARY tables (Button/Label/... each surely carry their own
  [+0x18]-view table — e.g. 76-slot tables = 75+1 hint at it): enumerated primaries here cover the
  F1 set; the derived secondary table bases were not individually walked (follow-on, cheap: same
  walker, [-4] scan on the 76/106/107 tails).
- 0x4b0820 exact ctor head (mid-fn at 0x4b0827 in this dump — the enclosing live fn starts earlier;
  EA of stores cited, not head).
- The `vft[+0x12c]/[window+0xd18]` handler coordinates in T-061 §"slot 0x30" belong to the WINDOW
  vft per its own doc — consistent with §2 but not re-derived slot-by-slot here.
- No rename queue results from this row: the family is ALREADY live-named (73 `JE::GUI::Window::`
  fns; slot0/ctor/dtor live). The deliverable is the correction-block inputs (§5) — zero writer
  renames needed, unlike T-075.

## Raw-evidence index

COL/TD EAs: Window {0x6d3d3c→0x6f273c, 0x6d3d08→0x6f273c off24}, Fragment 0x6d3d84→0x6faf50,
Button 0x6d3bec→0x6fae20, Label 0x6d3c38→0x6f360c, Edit 0x6d3f84→0x6fb030, GameDialogueWindow
0x6d2004→0x6f2eb0, DialogueWindow 0x6d6924→0x6f2e90, POIWaypoint 0x6d2a24→0x6f3624, CSMenuLabel
0x6d62d4→0x6f2dc8, CFloatingLabel 0x6d6780→0x6ff2d0, ReplyLabel 0x6d688c→0x6ff2f0, HeapTracker
0x6d3548→0x6f261c, Interface 0x6d3578→0x6f263c, Gfx_Engine 0x6d79f4→0x7001f0, CSound 0x6d4098→0x6fb0a8.
MTDs shared per-base: Base 0x6d1a58, HeapTracker 0x6d1a40, Window 0x6d1a70 (chain-node reuse across
the family — why every derived CHD ends …→0x6d1a70→0x6d1a58→0x6d1a40). Store pairs and slot contents
per §2/§7; walker scripts `obj/t081/walker.py` + outputs `obj/t081/{tables,stores,rtti3,verify}.txt`.
