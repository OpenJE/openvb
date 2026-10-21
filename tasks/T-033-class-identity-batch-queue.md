---
status: open
assignee: none (batch 4 DONE 2025-10-21 by GoldYak `01a15f83` under LoudDragon dispatch — see Results-batch-4 below; batch 5 unowned). Prior grant note retained: PureKnight `01a15cb7-e27e` (apply GRANTED via coordinator after all prior holders departed; rule in force: the grant follows whoever can reach the shared IDB MCP session on 8745). Caveat: T-040 — route-B deltas unmeasurable (no export path).
depends: T-029 (the census that produced this queue)
---

# T-033 — Class-identity batch queue: the third kind of worker

> **BATCH 2 APPLIED 2025-10-21 (PureKnight 01a15cb7-e27e, grant via ZenNova):** 6 classes minted → 24 route-A fns
> retired, T2 26.40% → 26.09% (0.31pt, inside RedXenon's re-registered ≤1.3pt band; route-B stale per T-040).
> Pair-suffix died (0x4d4650 = converting ctor of cls_0x4cb300); fstream class renamed only on its dense 7-fn band,
> 3 labelled-noise fns left anonymous (R36-e composite, embedders=9). Ledger + pre-checks:
> `discoveries/2025-10-21-t033-batch2-apply-ledger.md`. Batch 3 queue = the R36-e host-band residuals (cls_0x4d8d70,
> cls_0x50dbb0, cls_0x40a220 residual, five CEngineInterface embedders) + `OK rename candidate` classes from
> `tools/t036-noise-taxonomy.py` gate output (30 classes currently pass).

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

## Acceptance correction after T-040 (`c204c51`)

Any instruction here to "re-run the `F3.json` exporter" is **unsatisfiable on this machine** — stage 2 (`ooprolog`) is
gone and the installed OOAnalyzer cannot export facts. Deltas are therefore measured on the **live IDB (route A)**, with
`f3-json-check` pinning the 2024 export against silent drift. Renames still land in the IDB; they simply do not flow into
`F3.json` here, and a future reader must not infer from an unchanged export that a rename was never applied.

## Results — batch 3 APPLIED 2025-10-21 (`783fb3e`, coordinator GO after per-item draft review)
> 199 route-A renames, all ok:true. `cls_0x4d8d70`→**JE::StringTree** (145 fns: 142 pre-dump + 3 live-only
> found by post-census prefix diff — 0x5478b0/0x5479a0/0x547ba0, last role-named `insert_0x547ba0`, the
> World-load pool insert T-041 cites); `cls_0x424a70`→**F3::GamePlayerAvatarManager** (54 fns; sole non-factory
> `GamePlayer::ctor_0x411820` caller, call-scan self-test recovered BOTH known positives — factory
> 0x46f73f + char-gen 0x42eb35, exactly 2 total). **Post-census T2 26.09%→23.93% (−2.16pt, 192 retired)** vs
> registered ≈23.9% — MET; 196-vs-192 gap itemized (4 link-tail fns ≥0x5FD000 + 1 dump-merge dup). Escalated
> conservative half per rulings: cls_0x50dbb0 120 (std::_Tree across ResourceSystem cfg reader + entity-ref
> fallback — push-site precision refuted the coordinator's earlier draft framing BEFORE writes), cls_0x55dc80
> 94, cls_0x4542d0 50, cls_0x49c610 declared-at-0 (route-disagreement demo: all 43 already JE::ResourceSystem),
> cls_0x40a220 residual 3 labelled noise, cls_0x50db20 79 annotation-only. **VENDOR: cls_0x668cd8 (46 fns,
> verbatim `ID3DXMeshOptimize...` D3DX9 strings) — not renamed, not excluded; reported SEPARATELY so named-%
> is honest in both directions.** Cumulative b2+b3: 26.40%→23.93%. BLOCKED TOOLING (item 35 disclosure, no
> silent loss): `ida_original_f3::set_comments` rejects every argument shape (harness delivers string where
> schema demands array/object; `rename` coerces, comments path does not) — 13 comment texts preserved VERBATIM
> in the ledger §APPLY for re-issue by whoever has a working path (or batch-4 with a fixed tool). Batch-4
> queue: t036 OK-candidates (cls_0x6bb908 41, cls_0x6bc9d8 30, cls_0x41a2b0 28, cls_0x4b28a0 28…) — 644 anon
> classes / 2,124 fns remain. Ledger: `discoveries/2025-10-21-t033-batch3-hostband-ledger.md`.

## Results — batch 4 APPLIED 2025-10-21 (GoldYak, sole writer; ledger `discoveries/2025-10-21-t033-batch4-ledger.md`)
> **RESTART-EROSION FOUND + REVERSED FIRST**: the 13:13 devenv/imhex restart reloaded `F3.exe.i64` from disk
> and ALL IDB writes since the prior disk-save were gone — live census read T2 2,341 (26.37%), not the
> registered 2,124. Batch-2 (24) + batch-3 (StringTree 142 prefix + avatar 54) + the committed `0x548a80
> → JE::World::ctor_0x548a80` pair (docs/DEFERRED-NOTES.md:360) = 221 renames re-applied from ledgers ONLY,
> 221/221 ok, saved; T2 hit **2,124 = 23.93% EXACT** (registered b3 value reproduced bit-for-bit). Two lost
> names stayed LOST as lost-unledgered per ruling (0x46f250/0x46f260 — c23 doc has roles, no committed
> name+EA pair; zero T2 impact, still `JE::`). **Tooling unblocked**: harness `set_comments` still string-coerces
> (re-disclosed), but the SAME server accepts the array over direct JSON-RPC — all 13 b3-preserved comments
> re-issued VERBATIM, 13/13 ok, get_cmt readback SET. Batch-4 writes: **40 renames, 40/40 ok** — 3 mints on
> per-class evidence: `cls_0x41a2b0→JE::CActionbarInterface` (28; sole vft-0x6505b8 store + Actionbar.INT +
> ~40 shipped control names + in-band named trio), `cls_0x502870→JE::Gfx_D3D` (10; in-band named
> CreateDevice IS a cls_0x502870 DB method + DeviceReset/DXErr strings), `cls_0x5240e0→JE::Gfx_G3D_Mesh::
> ExportGfx{Model,Skin}Mesh` (2 of 7; shipped error prefixes name the class; residual 5 labelled-not-minted).
> **Queue heads #1/#2 = VENDOR, not wins**: `cls_0x6bb908` 41 proven D3DX8/COM (QI memcmp vs
> IID{648B1CEB-…}+IUnknown inside the d3d8.dll/D3DX8 string blob; IUnknown IID shared-xref'd by the 0x474xxx
> D3DX9 band) — 8-class vendor-adjacent band **148 fns, not renamed, not excluded, reported separately**
> (comment at vft 0x6bb908). `_Tree` family ruling extended to 5 new small clones (48 fns). Escalated
> owner-unresolved incl. heads #4 `cls_0x4b28a0` (27) and `cls_0x4a5be0` (anim-player, role-only evidence) —
> 321 fns conservative half, per-class evidence in ledger §3. Registered escalations 0x50dbb0/0x55dc80/
> 0x4542d0 untouched as ordered. **T2: 2,124 (23.93%) → 2,084 (23.48%), −0.45pt batch-4; cumulative honest
> vs pre-b2: 26.40% → 23.48% (−256 fns = b2 24 + b3 192 + b4 40; no averaging). Save discipline now
> proven on-disk: mtime 13:13→14:03, three save_database(True) checkpoints, post-save dumps match.**
> Batch-5 leads in ledger §7.

## Boundary
Identity is a claim about the shipped binary, not about our port: nothing here changes `src/`. A class
named from its vftable shape may still be semantically something else, which is why the evidence column is
mandatory and why the census delta — not the naming count — is the success measure.
