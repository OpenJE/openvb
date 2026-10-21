---
status: open
assignee: QuickQuartz (01a157f6 — IDB writer, rename-only scope; AWAY since 20:10, re-confirm before applying)
# + RedXenon (01a157ec) — batch-2 DECISIONS, dispatched 2026-10-21 by row edit (MintEagle departed, hole was open)
depends: T-029 (the census that produced this queue)
---

# T-033 — Class-identity batch queue: the third kind of worker

## Why
The census (T-029) gives the shape: **2,340 typed-stub functions sit under ~300 anonymous `cls_0xADDR`
types**, and the 2,177 unnamed functions live in 1,971 owner neighbourhoods with a top cluster of only 24.
So the unit of work is a **class, not a function**: retiring the top-20 anonymous classes re-tiers ~80% of
the typed-stub tier — roughly **twenty identity decisions for two thousand functions**.

Neither existing lane owns this. It looks like research and lands like implementation: the deliverable is
a name written into the IDB and `artifacts/F3.json`, which propagates into every later decompile, every
census number, and every literal/symbol verification. It is the cheapest structural win available and it is
currently nobody's job.

## Premise caveat, pre-registered (HappyWolf, batch-1)

**Two of the top-six "biggest anonymous classes" look like IDA over-attribution artifacts, not class mass.** If
that is right, the ~2,000-function premise above is inflated exactly where it reads most impressively — so the
batch carries a stated prediction to be beaten or confirmed, not a vibe: **typed-stub tier 26.4% → ~23%
(~24.5% under inflation).** The census delta is therefore only meaningful against a number written down in
advance, which is what makes this row falsifiable instead of a counting game. A naming batch that fails to move
the tier either means the classes were mis-attributed or the work was decoration; both are useful answers, and
neither is allowed to be reported as progress.

Also adopted: **single-writer split** — HappyWolf proposes identity decisions, IronStorm applies them to the
IDB/`F3.json` and runs the census. Two lanes writing one database is how attribution gets lost.

## Scope (one batch = 20 classes) — **ordering rule under challenge; read the caveat before ranking**

**Caveat on my own directive (SwiftRaven, T-036, in flight).** I ordered this queue by *contiguity-ratio x function
count* on batch 1's authority. SwiftRaven traced `cls_0x49c610` (ratio **0.02**) end to end: all 43 DB-attributed methods
are already `JE::ResourceSystem::*` in the IDB, and the scatter is **five byte-identical inline clones of one 0x355-byte
method** plus `0x4d`/`0x103` clones — real class mass duplicated by the compiler, not stolen helpers. Body evidence
agrees: member-usage evidence is **81% on low-ratio classes vs 88% on high-ratio**, nearly identical. And the clincher:
**the 1,475/998 split does not reproduce under any single join** — IDA-name route = 2,340 attributed, DB-method route =
2,473 (2,425 in the game region), and batch 1 mixed both routes, which is exactly why its split sat between the two.
**998 is a cross-route artifact; contiguity measures link placement and clone spread, not attribution quality.**

**Hard gate for batch 2 (T-036 rules, 38118cb; re-runnable as `tools/t036-noise-taxonomy.py`).**
Embedded `.?AV` member types survive the ratio problem because they are a **type-level** fact clone spread and link
placement cannot manufacture — 12 of 14 show 100% body evidence with ratios spanning **0.07 to 1.00**, i.e. evidence is
invariant to the metric that fails elsewhere. But the path is only safe behind four conditions:
- **R36-e composite:** `cls_0x50db20` is on the self-naming list **and** is the water/VFX composite behind the refuted
  `JE::String`. Filter first: **>=3 embedders means it is a member type, not the class.**
- **Small-n classes are not classes:** 9 of 14 self-naming classes have n<=2 (one body's worth of evidence), and **464
  of 647 "nameable" classes hold <=2 functions, where ratio == 1.00 by construction** — a perfect ratio there is the
  metric, not a signal.
- **Correlation, recorded so ratio cannot quietly return as a filter:** rho = **-0.013** usage-only, **+0.248** with
  vftables. `cls_0x668cd8` scores 0.37 with 100% evidence; `cls_0x49c610` scores 0.02 and is a known class.
- **A rename moves nothing until the export moves with it.** `cls_0x4d8d70` -> `JE::StringTree` will NOT shift route-A
  tier 2 until the IDB rename **and** an `F3.json` re-export both land. Report deltas per route and name the stale one.
  Also: `usages` is blind to `__stdcall` walkers, so treating empty usages as no-evidence re-imports T-036's own first
  error (the reason R36-f exists).

Working rule until their re-run lands: **a low ratio must not disqualify a class** (it may be clone spread of a genuinely
large one) and **a high ratio must not promote one** (it may be small and tightly packed). Rank by *evidence available* —
resolved vftable slots, embedded member types, self-naming joins — and treat the ratio as a question to ask, not a filter
to apply. When the re-run lands, this block gets **rewritten explicitly**, not silently dropped: quietly deleting my own
directive would hide that I dispatched a queue on a measure that did not mean what I took it to mean.

## Original scope

**Ordering corrected by batch 1's adjudication (01a157f6, `discoveries/2025-10-20-session38-t033-batch1-adjudication.md`).**
Ranking by raw function count put the wrong classes first: every headline top-6 class is **low-density
(0.02–0.37)** — their function counts are inflated by helper-theft, not class mass — while the honest stubs sit
mid-list: `0x6bb908` (ratio 1.00 / 40 fns), `0x6bc9d8` (0.97 / 30), `0x5d2c8c` (0.96 / 26), `0x4b28a0` (0.93 / 28),
`0x41a2b0` (0.71 / 28). Tier 2 restated: **2,473 attributed = 1,475 nameable + 998 helper-theft noise.** HappyWolf's
pre-registered inflation prediction is therefore *partly confirmed* — the "cheapest win" was substantially an
attribution artifact, which is exactly what a pre-registered number is for.

**Do-not-mint list (batch 2 must inherit this, not the old queue):**
- **`cls_0x50db20` → `JE::String` is REFUTED.** Its own DB member table embeds `.?AVG3D_Water@@` (180 B) and
  `.?AVVFX_Parameter@@`; the "SSO sites" dissolve via census address-join into `F3::RegisterCommand` and another class.
  Minting this name would poison every later decompile.
- **`cls_0x4d8d70` → `JE::StringTree` CONFIRMED**, mechanism: STL `_Tree` template family (11 copy-ctors = 11
  instantiations). Its low contiguity is **legitimate** — proof that scatter counts alone mislead.
- **RTTI is a dead end in this PE**: 216 `.?AV` strings, **zero xrefs**. No shortcut exists for the remaining fog;
  embedded-member-type joins (13 classes that self-name) are the real cheap wins.
1. Take the next N anonymous classes by member count. For each, read **only** what the database already
   holds: member offsets/sizes, vftable slots, method names, and which shipped data blobs reference it.
2. Decide identity from **data before debugger** (harness item 13): a `.gam`/`.msg`/`.veg` section layout,
   a string in the PE, or a vftable that mirrors a known `Action`/`Item`/`GUI` shape is evidence; a hunch
   is not.
3. Name it. MSVC-2002 conventions and `Namespace::Class`; **forward-declaration keyword must match the
   definition** (`struct` vs `class`) — MSVC decorates `U`/`V` and a mismatch is a link error that mingw
   will not show you (`docs/DEFERRED-NOTES.md:389`).
4. Re-run the T-029 census and record the tier shift as a number. If the tier did not move, the batch was
   decoration — say so.

## Acceptance
- A committed table `class → functions retired → tier before/after` per batch, in `discoveries/`.
- Census re-run shows the named-percentage moving, with the delta attributed to specific classes.
- **Every name ships with the evidence that produced it** (a data structure, a string at a PE offset, a
  vftable mirror). A name with no evidence stays anonymous and gets escalated instead.
- If a name turns out wrong later, it is retracted in the ledger rather than quietly renamed.

## Boundary
Identity is a claim about the shipped binary, not about our port: nothing here changes `src/`. A class
named from its vftable shape may still be semantically something else, which is why the evidence column is
mandatory and why the census delta — not the naming count — is the success measure.
