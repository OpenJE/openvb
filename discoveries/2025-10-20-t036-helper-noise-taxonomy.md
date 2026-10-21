# T-036 — The 998 "helper-theft" functions are not theft: contiguity ratio measures link placement, and batch-1 mixed two join routes

All offline/read-only. No IDB writes. Routes are stated on every number because **the whole finding is that the
number depends on the route.**

* Route A = IDB function names (`mcp__ida_original_f3__list_funcs`, T-029 dump `/tmp/census_p1-7.json`).
* Route B = `artifacts/F3.json` `structures[cls_0x…].methods` keys joined onto route A.
* Route C = raw bytes of the shipped `F3.exe`, MD5 `a68c22f3dd1c256ec9539a70e87cb434` — **matches
  `F3.json.filemd5` exactly**, so byte work is legitimate offline evidence. Verified section map:
  `.text` VA `0x401000`/raw `0x1000`, `.rdata` `0x647000`/`0x247000`, `.data` `0x6f1000`/`0x2f1000`.

## 1. The falsifiable claim

> **Contiguity ratio (fraction of a class's functions within ±0x2000 of its median method EA) mostly measures where
> the linker placed code, not whether the attribution is right — and it is structurally incapable of rating the bucket
> it was used to rank.**

Two measurements, both route-stated:

1. **Rank correlation between contiguity and independent evidence is weak-to-none, and its sign depends on which
   evidence you accept** (57 anon classes with ≥8 functions):
   * rho(contiguity, member-usage-in-body only) = **−0.013** — zero relationship.
   * rho(contiguity, member-usage **or** vftable slot) = **+0.248** — weak positive.
   So contiguity carries *some* signal once vtable-backed classes are credited (my first pass under-credited them:
   `cls_0x6bb908` scored 8% on usages alone and 98% with its vftable included). But +0.25 is nowhere near a filter,
   and the metric has a structural hole that makes it worse than useless on the bucket it was built to rank:
2. **Contiguity is VACUOUS for small classes.** Of the 647 classes with ratio ≥ 0.5 — the "1,475 nameable" bucket —
   **464 hold ≤2 functions**, where "fraction of functions near the median" is 1.00 by construction (a 1-function
   class is perfectly contiguous whatever it is); 380 classes have exactly one function. The metric can only fail a
   large class, and the largest classes are exactly the ones batch-1 was trying to rank.
3. **Route A gives 2,340 attributed functions; route B gives 2,473** (2,425 inside the game region). Batch-1's
   "2,473 attributed → 1,475 nameable + 998 noise" **cannot be reproduced under either route**:

   | definition | attributed | ratio≥0.5 | ratio<0.5 |
   |---|---|---|---|
   | Route A (IDB `cls_0x…::` names, game) | 2,340 | 1,515 | **825** |
   | Route B, median of matched EAs, game | 2,425 | 1,556 | **869** |
   | Route B, median of matched EAs, all regions | 2,473 | 1,597 | **876** |
   | Route B, mean-centred, game | 2,425 | 1,344 | 1,081 |
   | window ±0x1000 instead of ±0x2000 | 2,425 | 1,405 | 1,020 |

   998 sits outside every one of these. It is the residue of **subtracting a route-A split from a route-B total**: §5
   of `2025-10-20-session38-t033-batch1-adjudication.md` quotes route-A per-class counts (`cls_0x4d8d70` 138,
   `cls_0x50db20` 83) against a 2,473 total, which is route B. The honest statement is *1,475–1,597 nameable /
   825–1,081 low-density depending on route*, not 1,475/998.

   The per-class route disagreement is visible on the headline set itself:

   | class | route A (IDB `cls_0x…::` names) | route B (`F3.json` methods) |
   |---|---|---|
   | `cls_0x4d8d70` | 138 | 133 |
   | `cls_0x55dc80` | 94 | 97 |
   | `cls_0x50db20` | 79 | 81 |
   | `cls_0x424a70` | 54 | 55 |
   | `cls_0x4542d0` | 50 | 49 |
   | `cls_0x49c610` | **0 — the IDB has no `cls_0x49c610::` name at all** | 43 |

## 2. Taxonomy: what the "extra" functions actually are

Route B, 2,473 DB-attributed anon-class methods. Categories are exclusive in the order listed.

| # | category | count | share | worked example |
|---|---|---|---|---|
| 1 | **DB EA is not an IDB function start** — vcall/branch target or secondary chunk | 73 (171 DB-wide) | 2.9% | `JE::Entity` declares `0x4154f0`; live IDB: `0x4154ef` "Not a function", `0x4154f0` resolves to `F3::GameWeapon::virt_meth_0x414680` (**chunk start, not a new method**) |
| 2 | **EA outside game region** (`≥0x5FD000`, link tail) | 48 | 1.9% | 48 anon-class EAs in the CRT/D3DX static blob |
| 3a | **Route conflict — the IDB names it under a real owner** | 85 (83 of them `JE::/F3::/net::`) | 3.4% | `cls_0x49c610`: **all 43** methods are `JE::ResourceSystem::*` in the IDB |
| 3b | **Route conflict — the IDB leaves it unnamed** (`meth_0x…`/`sub_`) | 15 | 0.6% | `cls_0x424a70` claims `0x427a40`, which the IDB still calls plain `meth_0x427a40_427a40` — a class the DB has that the IDB does not |
| 4 | **Near-clone family** (≥98% byte identity, immediates differ) | 678 | 27.4% | `cls_0x50dbb0` — `0x49dbb0/0x4b9a50/0x4c6600` are the same 0x37-byte tree-destroy, `__stdcall(cls_0x50dbb0 *Block)` |
| 5 | **Real, distinct class mass** | 1,545 | 62.5% | `cls_0x50db20` — 77 of 83 are distinct evidenced methods |
| 6 | *unproven by my metric — see the warning below* | 102 | 4.1% | `cls_0x4d8d70::meth_0x41fb90` = `char __stdcall(cls_0x4d8d70 *Block)` |
| — | *artifact floor (categories 1+2+3a+3b — the parts that are our tooling)* | **221** | **8.9%** | |

**998 − 221 = 777 functions batch-1 called noise are real methods of the class they are attributed to.**

Category 6 deserves a warning because **it is a measurement failure, not a finding.** The strictest version of this
bucket (237 functions: no member usage in body, no vftable slot, no ≥98% clone sibling) went back to the live IDB as
prototypes, and essentially every one takes its own class as `this`/`Block` —
`cls_0x4d8d70::meth_0x41fb90` = `char __stdcall(cls_0x4d8d70 *Block)`, a recursive tree-destroy walking
`Block->mbr_0x8`/`->mbr_0x0`; `cls_0x4542d0::meth_0x422370`, `0x453e70`, `0x4a3250`, `0x4b9ec0` are the same pattern in
a different class; `cls_0x4b28a0::meth_0x4b2d40` = `char __thiscall(cls_0x4b28a0 *this, HWND)`. `usages` in `F3.json`
only records field accesses IDA recognised, so `__stdcall` walkers, `void **`-typed parameters and vcall-only bodies
look empty. **Any "no evidence" bucket built from `usages` alone is measuring the exporter's blind spot, and must be
checked against prototypes before anyone calls it noise.**

## 3. The worked example, end to end: `cls_0x49c610` (ratio 0.02 — batch-1's *worst* class)

* Route B: 43 declared methods, class size 597, no vftable, 23 members. Its two biggest members are `struc` embeds of
  `cls_0x50db20` (577 B) at +0x0 and +0x14, plus `cls_0x50dbb0` (32 B) at +0x108.
* Route A: **every one of the 43 is named `JE::ResourceSystem::*`** — `ctor_0x49c610`, `Init_49bf20`, `OpenFile_49ae90`,
  `Demand_49b750`, `FileStreamOpen_49b9f0`, `MemStreamOpen_49beb0`, `Release_498eb0`…
* Route B evidence: 36/43 (84%) contain a `cls_0x49c610` member usage inside their own body.
* Why the ratio collapses: the bodies repeat across the image because the same inlined accessor is emitted per
  instantiation — five 0x355-byte bodies at `0x4ea390`, `0x50c510`, `0x518f10`, `0x579f60`, `0x57a2c0`, 93.3–93.7%
  byte-identical to each other, differing only in their SEH handler immediate
  (`68 68456300` / `78596300` / `985f6300` / `38926300` / `58926300`) and call rel32s. Median-±0x2000 can only ever
  score such a class near zero, whatever the truth.

**This class is not anonymous and was never helper theft: it is `JE::ResourceSystem`, and route B (a stale
`F3.json`) is what made it look like a 43-function mystery.** `F3.json` has no `ResourceSystem` struct at all — only
`.?AUResources@@`. This is exactly the export-lag §6 of the batch-1 adjudication predicted, now measured.

## 4. Correction to T-029's 171-entry artifact (route C + live IDB)

T-029 §2 said the 171 declared-not-present methods "are mid-function addresses inside another IDA function (IDA
legitimately merges the tails)". Measured against the shipped bytes and the live IDB, that is only partly right:

* **49** are genuinely inside another function (`0x411f00` in `F3::GameProjectile::virt_meth_0x411ef0` — T-029's own
  example, confirmed).
* **122** sit in padding where IDA created no function: 120 of 122 are 16-byte aligned, 117 are preceded by NOP/`0xCC`
  filler, 100 begin with a plausible prologue. Live IDB `0x4154ef` → "Not a function" while `0x4154f0` → resolves to
  `F3::GameWeapon::virt_meth_0x414680` (**size 0x8**). So they are **secondary chunks of multi-chunk functions** —
  `list_funcs` enumerates function *starts*, so a chunk is invisible to route A but visible to the type-DB tooling.

Consequence for the census: route A's denominator is not "all the code". Do not read 171 as "171 vanished functions";
read it as "the two routes disagree about what a function is". Only the 49 tail targets are the artifact T-029 named.

## 5. The rule (not a cleanup)

Ranking by contiguity is replaced by a **route-stated evidence gate**, applied before any rename is allowed:

* **R36-a — one route per number.** Quote attributed counts as `attributed(route=A|B)`. A delta is only meaningful
  against the same route. Never mix (this is what produced 998).
* **R36-b — start with the cheapest fact, `f3_method_index`/route A disagreement.** If the IDB already names every
  method of an anon class under one real owner, the class is **already identified** — retire it from tier 2 instead of
  queueing it (kills `cls_0x49c610`'s 43 and 100 functions overall, cost: one name join).
* **R36-c — density never vetoes, it only questions, and it is void below n≥8.** Contiguity is **vacuous for 468 of the
  647 "nameable" classes (they hold ≤2 functions, where ratio≡1.00 by construction)** — never let a ratio decide those.
  Drop a DB EA that is not an IDB function start (73) and one outside `0x5FD000` (48). A low ratio sends the class to
  *prototype/vtable review*, never to the noise bucket.
* **R36-d — declare a family, not a class, when clones dominate.** If ≥50% of a class's functions have a ≥98%-identity
  sibling, report "inline clone family, N bodies for one source method". **0.90 is too loose — measured 3.83%
  false-positive on unrelated same-size pairs; ≥0.98 is 0.20%.** Use ≥0.98.
* **R36-e — composite test beats count.** ≥3 embedders (`members[*].struc == this class`) ⇒ a *member*, not a class;
  report the host band. 7 of the classes with ≥8 functions are composites — including three batch-1 headline candidates
  `cls_0x4d8d70` (14 embedders), `cls_0x50db20` (12), `cls_0x50dbb0` (5). This is how "StringTree" could be both real
  and scattered.
* **R36-f — before calling anything "no evidence", query the prototype.** A `usages`-empty body is the exporter's blind
  spot (`__stdcall` walkers, `void **` parameters), not proof of misattribution; see §2 category 6.

Gate applied to batch-1's headline candidates (verbatim from `tools/t036-noise-taxonomy.py`): `cls_0x49c610` →
**R36-b RETIRE, the IDB already names 43/43 as `JE::ResourceSystem`**; `cls_0x4d8d70` (14 embedders), `cls_0x50db20` (12),
`cls_0x50dbb0` (5) → **R36-e composite, report as embedded struct + host band**; `cls_0x55dc80`, `cls_0x4542d0` →
**R36-d clone family**; `cls_0x424a70` (93% evidence, ratio 0.33) and `cls_0x668cd8` (100% evidence, ratio 0.37) →
**R36-f low density but large: check prototypes + xref, density never vetoes**. Note the last two: the classes with the
*best* evidence in the batch are the ones the ratio metric ranked worst — a class can be 100% evidenced and still score
0.37. Over all classes with n≥8 the gate yields 30 rename candidates, 9 clone families, 7 composites, 2 low-density-large,
1 retire, 8 weak.

## 6. Census re-run under the rule (the delta is the point)

| quantity | value |
|---|---|
| attributed, route B, as-exported | 2,473 (2,425 in game region) |
| attributed, route A (IDB names) | 2,340 (26.4%) |
| **after R36-a…R36-c** | **2,325 (26.2%)** |
| delta vs route A | −15 |
| delta vs route B | −100 |
| artifact floor (R36-excluded: non-starts + off-region + route conflict) | **221 (8.9%)** |
| batch-1's claimed noise | 998 (40%) |
| classes keeping ≥1 function after the rule | 650 of 698 |

R36 does **not** collapse tier 2 — it moves ~100 functions that were double-counted (91 of them already counted in
tier 1 by IDB name) and confirms 2,325 real attributions. **The tier-2 restatement "26.4% → 16.6% + 11.2% noise" is
withdrawn**: the noise term was mostly a route mismatch.

**Effect on the pre-registered prediction.** Batch-1 predicted typed-stub 26.4% → ~23% (→~24.5% if inflation held),
assuming tier counts were route-stable. They are not (A vs B differ by 133–151 before any renaming). The prediction is
**re-anchored to route A** and stays live: renaming `cls_0x4d8d70` → `JE::StringTree` does not move route-A tier 2
(it stays 2,340 until the IDB is renamed *and* `F3.json` re-exported); the measurable move is **tier 3 → tier 1**, at
roughly −15 from R36-b plus whatever the 13 self-naming classes retire. Judge the prediction against route A only;
against route B it is unfalsifiable.

## 7. Does this touch the "13 self-naming classes" cheap-win path? (HappyOwl's question)

**No — and the reason is worth the queue.** Embedded `.?AV` member types are a *type-level* fact inside `F3.json`
(the exporter read a real field type), not an *address-joining* fact, so clone spread and link placement cannot
manufacture it. Measured: of the 14 classes carrying an embedded named type, **12 have 100% member-usage body
evidence**, 2 are 80%+, and their contiguity ratios range 0.07–1.00 — i.e. their evidence is invariant to the metric
that fails everywhere else. The one caution: `cls_0x50db20` is on that list *and* is a composite (12 embedders, size
577), which is precisely why it produced the false `JE::String` call in batch-1. So the surviving ranking method is
**"embedded named member type" filtered through R36-e (composite test)**, not the embedded type alone.

## 8. Boundary (what this does NOT say)

This measures how **our database and IDA** describe the binary. Nothing here proves the shipped binary has 2,325
class members; it proves the 998 was not evidence of misattribution. Two answers stay distinct:
*heterogeneous-by-design* (composite/template/clone mass — real, just not one class) and
*artifact* (route conflict, non-start chunk, off-region — 221 functions). Only the second is noise.

## 9. Shown to fail (rule 3 — a check that cannot be broken is decoration)

Every number here was attacked before it was written down; five of these attacks changed the result.

1. **The rho sign depends on my own metric definition** — `−0.013` with `usages` alone, `+0.248` once vftable slots
   count. My first pass under-credited vtable-backed classes (`cls_0x6bb908`: 8% → 98%). Both numbers are printed; the
   claim is stated as "weak/none", not "zero".
2. **Contiguity is vacuous where batch-1 applied it** — 464 of 647 ratio≥0.5 classes hold ≤2 functions. Reproduced by
   `tools/t036-noise-taxonomy.py` ("VACUOUS:" line).
3. **My clone test was wrong at 0.90 and had to be tightened.** At ≥90% identity, 3.83% of *unrelated* same-size pairs
   collide (measured over 6,343 cross-owner pairs); at ≥98% it is 0.20%. R36-d therefore specifies 0.98. Negative
   control: 184 byte-permuted bodies, 0 false family matches. My first headline ("five **byte-identical** inline
   clones") was **falsified by my own normalizer** — they are 93.3–93.7% identical, distinct entry points, and they do
   *not* collapse into one family. It is corrected in §3.
4. **Byte evidence refuses to run on a mismatched binary**: `--exe` on a different F3.exe build prints
   `MD5 1e588c40… != filemd5 a68c22f3… -- byte evidence disabled` and the clone column disappears.
5. **Planted-stale test, and it caught a real bug in my rule.** I rewrote a route-A dump with `JE::ResourceSystem::`
   names stripped (simulating an IDB that has not been renamed yet). Correct behaviour: named-conflict drops 83→40 and
   `cls_0x49c610` loses its `R36-b RETIRE` verdict and falls to "check prototypes + xref". But the first version of
   R36-b also fired on those 43 **`sub_`** functions — an unnamed `sub_` is *not* an owner, so the tool declared a
   retire on no evidence, and `--class` crashed with `IndexError` on names lacking `::`. Fixed by splitting 3a
   (real owner) from 3b (unnamed) and requiring `cfu == 0` for a retire. **If a rule survives a planted stale input
   unchanged, the rule is not measuring what it claims.**
6. **"No evidence" was measuring my exporter, not the binary.** The 237-function strictest bucket was disproved by
   pulling prototypes from the live IDB — they are methods (§2, category 6). Any future batch that treats `usages`
   emptiness as noise will re-import this error.

## 10. Open / routed

* R36-b says `cls_0x49c610` is `JE::ResourceSystem` on 43/43 IDB names, and the tool prints that as a retire verdict.
  I did **not** mint the identity — name confirmation belongs to T-033, and it needs the export re-run to move any
  tier (`F3.json` is a generated file).
* 122 multi-chunk EAs: only 16 are vftable targets, so **why the exporter records chunk starts is unexplained** —
  needs the export tooling itself, which is not in this repo. Flagged, not guessed.
* 9 of 14 self-naming classes have n≤2 functions: their "identity" is one body's worth of evidence, not a class.

## 11. Reproduce

```
python3 tools/t036-noise-taxonomy.py            # routes, taxonomy table, rho, gate, re-run
```
Inputs: `artifacts/F3.json`, `$OPENVB_ORIG_EXE` (MD5 must equal `F3.json.filemd5`), T-029 dump
`/tmp/census_p1-7.json` (or re-pull per `2025-10-20-session37-t029-function-census.md` §5).
