---
status: done
verdict: the 998 was never noise — contiguity measures link placement, and batch-1 mixed two join routes.
         Artifact floor is 221 (8.9%), not 998 (40%); 777 of those functions are real methods.
assignee: SwiftRaven
depends: T-029 (census), T-033 batch-1 adjudication (the number this explains)
---

# T-036 — Where did the 998 "helper-theft" functions come from?

## Why
Batch-1 adjudication restated tier 2 as **2,473 attributed = 1,475 nameable + 998 helper-theft noise** — i.e.
**40% of the anonymous-class attribution is not class mass.** Nobody has explained the mechanism, and it sits under
every census number, every ordering decision, and every future decompile: if a lane cannot say why 998 functions got
pulled into classes they do not belong to, the census is a count of an artifact, not of the shipped binary.

## What
Characterise the noise from the database and the IDB (offline first, item 13):
1. Sample the low-density classes (contiguity 0.02–0.37, esp. the six batch-1 headline candidates) and classify what
   the "extra" functions actually are: thunks, shared statics/helpers, vcall tail targets (the 171-method artifact from
   T-029), CRT glue, or genuinely unrelated methods.
2. Find the **mechanism**: is it IDA's automatic class assignment by address proximity, our `F3.json` export joining
   member ranges onto address spans, or real shipped layout? One example class traced end-to-end beats a hundred counts.
3. Propose the fix as a **rule, not a cleanup**: e.g. density floor before a rename is allowed, or an export-side
   distance check, or "never trust a class with N>80 members and ratio<0.4".

## Acceptance
- A taxonomy table: noise category → count → one worked example (EA + why it is misattributed).
- The sum reconciles against **998** within the sample's error, or says plainly where it does not.
- A concrete rule that a future batch can apply, and a re-run of the census showing the attributed count **after** the
  rule — the delta is the point.

## Boundary
This measures how *our database and IDA* describe the binary, not how the binary is structured. If a class turns out to
be genuinely heterogeneous, say so: helper-theft and shared-base-class are different answers, and only one of them is an
artifact.

## Verdict (SwiftRaven, closed)

**Answer: helper-theft is the wrong mechanism.** Recorded in
[`discoveries/2025-10-20-t036-helper-noise-taxonomy.md`](../discoveries/2025-10-20-t036-helper-noise-taxonomy.md);
re-runnable as `python3 tools/t036-noise-taxonomy.py`.

1. **The 998 does not reproduce under any single join.** Route A (IDB `cls_0x…::` names) = 2,340; route B
   (`F3.json` methods) = 2,473 (2,425 in game region). Split by ratio≥0.5: A → 1,515/825, B → 1,597/876. Batch-1
   subtracted a route-A split from a route-B total. The tier-2 restatement "26.4% → 16.6% + 11.2% noise" is withdrawn.
2. **Measured artifact floor = 221 (8.9%)**, not 998: 73 DB EAs that are not IDB function starts + 48 off-region
   (link tail) + 100 route conflicts. **777 of the 998 are genuine methods.**
3. **Worked example (`cls_0x49c610`, ratio 0.02 = batch-1's worst)**: all 43 methods are already `JE::ResourceSystem::*`
   in the IDB and 36/43 carry member-usage evidence. It was never anonymous or stolen — `F3.json` is a stale export.
   Its "scatter" is 5 copies of one 0x355-byte inlined body (93.3–93.7% identical, differing SEH-handler immediates).
4. **The ranking metric is void where it was used**: rho(ratio, evidence) = −0.013 (usages only) / +0.248 (usages+vftable),
   and 464 of the 647 "nameable" classes hold ≤2 functions, where ratio≡1.00 by construction.
5. **Rule R36-a…f delivered as a gate, not a cleanup** (§5 of the doc), implemented in the tool, and the census re-run
   under it: attributed **2,325 (26.2%)**, delta −15 vs route A / −100 vs route B.
6. **T-029 correction**: only 49 of its 171 vanished methods are mid-function tails. The other 122 are **secondary
   chunks of multi-chunk functions** (live IDB: `0x4154ef` "Not a function", `0x4154f0` → `F3::GameWeapon::virt_meth_0x414680`),
   so `list_funcs` is not a complete enumeration of code entry points.
7. **Boundary held**: this describes our database and IDB, not the shipped layout. Composite/embedded-struct and
   template/clone mass is heterogeneous-by-design, not noise — `cls_0x4d8d70`, `cls_0x50db20`, `cls_0x50dbb0` have
   14/12/5 embedders and must be reported as embedded structs, not renamed as one class.
8. **For the queue (HappyOwl's question)**: the 13–14 self-naming classes are a *type-level* fact, so clone spread and
   link placement cannot manufacture them — 12 of 14 show 100% body evidence and their ratios span 0.07–1.00. It
   survives as a ranking method **only when filtered through R36-e** (the composite test), because `cls_0x50db20` is on
   that list *and* is the water/VFX composite behind the refuted `JE::String` call.

Routed, not guessed: why the exporter records chunk starts (needs the export tooling, not in this repo); R36-b names
`cls_0x49c610` → `JE::ResourceSystem` but I did **not** mint it (T-033 + an export re-run own that move).
