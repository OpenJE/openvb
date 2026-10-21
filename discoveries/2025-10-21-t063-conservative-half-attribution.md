# T-063 — conservative-half attribution: N=199 exact, K=17 true orphans, census era closes

**EpicRaven**, row `c293f5c`, claim `300bc23`. Offline lane: raw EXE + pinned dumps + **committed
wave dumps** (`t060-postdump.json` = freshest live-IDB census; zero 8745 queries).

## 0. Gate first

`tools/ecx-flow.py selftest` — **5/5 PASS** @ EXE md5 **`a68c22f3dd1c256ec9539a70e87cb434`**
(F3.json filemd5 match; census source `discoveries/wavedumps/t060-postdump.json`, mtime = T-060
close, the LATEST banked snapshot — no writer activity since). No tool changes this row; only
`families`/`band`/`bucket_entries`/`head_hint` reuse — zero regression risk to T4.

## 1. Enumeration + disjointness (the exact N)

Set definition: F3.json class-membership over every class named in the b4 ESCALATED table
(`_Tree` five + owner-unresolved list) ∩ LIVE `cls_…`-anonymous in `t060-postdump.json`.

- **N = 199 exact.**
- Disjoint vs T-057's registered-band set: **overlap = 0** (computed against the trio's 244
  live-anon fns: `cls_0x50dbb0` 115 / `cls_0x50db20` 80 / `cls_0x4542d0` 49 — different fns, no
  double count). Disjoint vs vendor bands: this set contains none of the 46/148 vendor-epoch fns.
- **Already-hosted since b4 — the good outcome LoudDragon predicted: 28 fns** of b4's conservative
  listing are no longer anonymous: `cls_0x445350` ×8 (all minted T-055 as the HotKey chain),
  `cls_0x4a5be0` ×11 (T-055/T-054 AnimMeshInstance), `cls_0x558740` ×7 (T-055 CDebugInterface
  ctor), plus 2 RTTI re-joins inside the _Tree five (0x4FAD40, 0x57F760).
- Reconciliation vs "~321", exact: b4's authoritative per-class table sums the conservative half to
  **218** (48 `_Tree` + 170 owner-unresolved; the ledger's own "48+127" headline does not reconcile
  with its table — its footnote says the table is the authority). From 218: **−28 hosted since b4**
  (T-054/T-055 mints: 445350 ×8, 4a5be0 ×11, 558740 ×7; + b6 RTTI re-joins inside the _Tree five:
  0x4fad40, 0x57f760/0x57fad0 net −2) **+9 membership delta** vs b4's day-count (cls_0x60b0eb's 7
  link-tail members b4 recorded as "0 in-game", +1 each on 4b28a0/4b8f70 from later census drift)
  = **199**. Every one of the 199 itemized below; nothing outside is silently lost — the full
  2,042 reconciles in §4 with zero gaps.

## 2. Buckets (all 199; tool-executable, evidence bits in every row)

| bucket | count | what it means |
|---|---|---|
| LIVE | 109 | direct external flow (host+shape printed) |
| DELEG | 90 | flow via intra-band callers (callers-of-callers chain printed) |
| PAD | 0 | — |
| **ISO** | **0** | **every conservative-half fn has flow; no anonymous orphans** |

head_hint(): **all 199 locators are call-heads** (item-43 grade — no chunk-locators in this set,
unlike T-057's PAD rows; queue needs no containment re-anchoring).

## 3. Classification per family — the FINAL DECOMPOSITION

### (a) MINT-READY — 15 fns, queue to writer (behind whatever T-056 loads)

1. **`cls_0x48fb60` (+ its `cls_0x48fce0` band) — 10 fns: member class at G3D_Node+0xA0.**
   Construction-tier proof: **`JE::G3D_Node::ctor_0x4d7600`** (LIVE name) at site **0x4D7689**
   does `lea 0xa0(%esi),%ecx; call 0x48FF20` — constructs the band object INTO the live-named
   G3D_Node. Method-tier corroboration: LIVE/DELEG edges from `.?AVG3D_Node@@` and
   `.?AVG3D_Transform@@` methods (embed shape this+0xA0 + thiscall self-chains 0x48FE50/70/F70
   inside-band). This is the same construction-side + cleanup-side agreement pattern the board
   trusts (T-059 §reading): G3D_Node's OWN ctor installs the member.
2. **`cls_0x558740` residual — 5 fns (0x434120/0x434250/0x4342C0/0x434760/0x434910):
   JE::CDebugInterface band extension.** Callers carry LIVE names: `JE::CDebugInterface::
   meth_0x558190/0x558510` call them directly; `JE::CEngineInterface::meth_*` call them with
   `this = view − 0xB0/−0xC4` — the item-42 view-adjustment shape (CDebugInterface embeds
   CEngineInterface at +0xC4 per T-057 §3's 20-pads/+0xC4 mirror). 0 vft slots; membership via
   the CDebugInterface-band call graph, same join shape that minted b6's RTTI re-joins.
   Tier note: 0x434760 is the weakest (mixed callers incl. VFX-epoch cls_0x434f70) — writer may
   apply 4 + annotate the 5th at read-back discretion.

### (b) ATTRIBUTED-FOREIGN — 79 fns (std/helper machinery; no class to mint; host epochs named)

- **_Tree five (46)** — confirmed the b6 §1 ruling per entry, with each clone family's host
  field identified: `cls_0x4f9b90` → map at **VFX_ParameterHolder+0x4** (its ctor — minted by b6
  RTTI — drives 5 entries); `cls_0x57bf40` → map at **AreaMap+0x7C** (2 already RTTI-joined);
  `cls_0x560510` → **PathManager** epoch containers; `cls_0x5a4550` → **Script_Small /
  sub_5A8030** epoch (stk-arg fan 24-25); `cls_0x4c8f30` → **sub_4DDA10 water-composite** epoch
  (this+0x64 map field; b4's "do NOT re-parent to G3D_Water" caution respected — attributed to
  the epoch, not the named water fn).
- **`cls_0x49cc40` (15)** — VFX/Model **static helper family**: LIVE edges from
  VFX_Model/VFX_ModelEffect/ModelInstance/Gfx_ParticleSystem with `stk`/`deref` arg shapes, ZERO
  embed/this shapes → generic helpers fanning across 4+ unrelated classes = item-43 signature.
  Corrects b4's "ResourceSystem-adjacent" adjacency reading: the flow says shared model utilities.
- **`cls_0x409820` (6)** — RTTI/dyncast support chunk (b4's own note said RTDynamicCast; callers
  cls_0x424a70/GameActionAttack with type-info stk args) → compiler-support machinery.
- **`cls_0x4b8f70` (12)** — **Graphics static/stream helper epoch**: near-uniform owner
  `.?AUGraphics@@` with deref args (no this+disp embed shapes) → Graphics' file/stream static
  helpers; "Common::Chunk" generic-string ruling holds; not a class.

### (c) VENDOR / link-tail — 59 fns

`cls_0x5d2c8c` 26, `cls_0x5d06a8` 14, `cls_0x60b0eb` 7 (all ≥0x5D0000) — LIVE/DELEG flows come
ALMOST EXCLUSIVELY from the CRT-support epoch `sub_5CEA30/sub_5CEB0F/sub_5CEBDF/sub_5CF4EC`
(new/free/`__EH_prolog` wrappers) + vector pads (`sub_60FDDA`/`sub_611060` stk fan) — b4's
"link-tail support region adjacent to vendor epoch" reading CONFIRMED by flow direction.
`cls_0x57bc90` 12 — same tail band but dyncast/helper fan-out reaching Entity/VisualEffect
callers (support machinery invoked from game code, still vendor-side per b4 D3DX-adjacency
ruling). Counted with the vendor bucket (b4 kept vendor 148 separate — these 59 are NOT part of
that 148, they are the in-game-tail half of the vendor ruling).

### (d) carry-overs — 29 fns, verdicts inherited, no re-litigation

- `cls_0x4b28a0` 28 → **T-054 ANNOTATE stands** (now re-confirmed through the same bucket lens:
  19 LIVE all `this==&0x707E48`, 9 DELEG — zero heap/embed/global-slot edges; membership vacuous).
- `cls_0x4a5be0` 1 (0x548D10) → b5 labelled tail-alias residual, unchanged.

### (e) TRUE-ORPHAN — **K = 17**, named, with item-41 triplets answered

1. **`cls_0x490150` — 9 fns.** Gfx **light-table consumer epoch**. Q1: LIVE 2 owners
   Gfx_Engine (deref args only), DELEG 7 via own member 0x494CA0→sub_4BFF20 — arg-flow only,
   NO this-shape anywhere. Q2: 0 vft slots, 0 thunks. Q3: 345-table full window + no
   `this==&/*global` pins (verified: NONE). Conclusion: it acts ON light records handed to it;
   nothing owns it; b5 "consumer, not member" upheld at flow tier. Legitimately unknown owner.
2. **`cls_0x4fc0e0` — 8 fns.** Self-coherent epoch 0x4FC0E0..0x4FD1xx (internal this-chains,
   `this+0x18` member stores, own thunks 0x4FB6A0/a3 which are tail-jmp forwarders WITHOUT
   table/data refs — per ratified rule they count as DELEG evidence, not vft membership). Q2:
   0 slots into the class; Q1: external owners are only the epoch's own anonymous sub_4FCxxx/
   cls_0x4fd780 members; Q3: zero global pins, full window. It IS a class-shaped domain
   (~20 fns incl. cls_0x4fd780), but the image carries no anchor (name/vft/store) linking it to
   any host or name. Future anchor sources: shipped-data cross-reference (a .sav/.ini section
   or FourCC touching 0x4FCxxx record sizes) — OUT of flow scope. Logged as the census era's
   residual number.

## 4. FINAL BOARD DECOMPOSITION — every live `cls_`-anonymous fn accounted (zero gaps)

Census source: `t060-postdump.json` LIVE names, counted against F3.json class membership.
**Total live `cls_…::`-anonymous functions: 2,042.** Exact partition (buckets mutually disjoint,
verified pairwise-overlap = 0):

```
2,042 =   167  VENDOR-epoch                        [ruled; 46 @cls_0x668cd8 + 121 D3DX8-adjacent
                                                    cls_0x6bb*/6bc* (b4-era membership 148)]
        +   404  ATTRIBUTED-FOREIGN std/helper      [T-057 trio 244 still-anon (of 273, 29 since
                                                    hosted) + 55dc80-epoch 81 (b6 §1 clones) +
                                                    T-063 79: _Tree five 46 + helper epochs 33]
        +    59  LINK-TAIL vendor-support            [T-063 (c): 5d2c8c 26, 5d06a8 14, 57bc90 12,
                                                    60b0eb 7 — flow direction proves CRT-support
                                                    epoch callers]
        +    29  carry-over annotated               [4b28a0 28 (T-054 vacuous-membership singleton)
                                                    + 0x548D10 tail alias (b5)]
        +    15  MINT-READY queued                  [T-063 (a): G3D_Node+0xA0 member 10,
                                                    CDebugInterface extension 5]
        +    17  TRUE-ORPHANS (K, named)            [T-063 (e): 490150 light-epoch 9,
                                                    4fc0e0 anchored-nowhere 8 — item-41 triplets
                                                    answered per row]
        + 1,351  LONG-TAIL, never registered        [density-gate-only failures across ~600 classes;
                                                    NOT identity-blocked, never entered a band or
                                                    escalation — the values-era backlog boundary]
```

167+404+59+29+15+17+1,351 = 2,042 exactly. Within this row's 199: 15+79+59+29+17 = 199 ✓.

**The census-era answer: in the registered-escalation + conservative-half scope, genuinely
unknown = 17 fns, each with all three item-41 queries answered and no anchor in the image. The
remaining 1,351 are unregistered long-tail members of KNOWN-shaped classes (T-036 route-A census
attributed their tier) — a workload number, not an ignorance number.**

## 5. Queue summary for the writer slot (nothing applied from this lane)

1. MINT-READY (a): 10 + 5 renames as above (tier note on 0x434760).
2. ANNOTATE (b/c): 46 _Tree host-field comments; 33 helper-epoch "generic machinery, item-43
   fan-out" comments; 59 link-tail "CRT-support-adjacent, flow from sub_5CExxx epoch" comments.
3. (e): keep escalated with the triplet answers recorded as comments at 0x490150 / 0x4FC0E0
   (both already carry b5/b4 comments — this row's text supersedes with the three-query record).
4. Long-tail note for T-064+: 1,398 density-gate-only anon remain — NOT identity-blocked, just
   never registered; that is the census-era→values-era handoff boundary line.
