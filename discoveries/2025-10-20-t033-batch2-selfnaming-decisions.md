# 2025-10-20 — T-033 batch 2 (RedXenon 01a157ec): the 13 self-naming classes — decisions by evidence rank

Ordering per coordinator instruction: **rank by evidence available, not contiguity ratio** (SwiftRaven's
`cls_0x49c610` clone finding disqualifies ratio as gate or promotion). Target list RE-DERIVED from the
live DB (`artifacts/F3.json` join: anonymous `cls_*` whose member tables embed `.?AV` shipped names) —
result reproduces the adjudication's counts exactly: **13 classes, 132 functions**. That agreement is
itself the join-route validation the row demands.

## Prediction VOIDED AND RE-REGISTERED (stated BEFORE measuring, as instructed)
Batch 1 pre-registered 26.4% → ~23% (~24.5% under inflation) **conditioned on the old top-6 queue**.
Under evidence-ordering the arithmetic changes: the 13 self-namers total 132 fns, of which `cls_0x50db20`
(83) is *not nameable* (below) ⇒ maximum retirable this batch = **49 fns = 2.0% of T2's 2,473**.
New pre-registered number: **typed-stub tier moves ≤1.3pt (26.4% → 25.1–26.0%) under the IDB-name route,
less under the DB-method route** (SwiftRaven's route disagreement applies: a tier % is meaningless without
its route — both routes' numbers get printed post-batch). If the measured delta exceeds 1.3pt, the census
tooling changed tier definitions, not the IDB — investigate before celebrating.

## Decisions
| cls | fns | embedded evidence (offset, size) | verdict |
|---|---|---|---|
| 0x50db20 | 83 | G3D_Water(180B) + VFX_Parameter | **STAY ANONYMOUS** (batch-1 refutation inherited; two unrelated value embeds = aggregate/split artifact, naming poisons) |
| 0x4468c0 | 2 | **3× HotKeyWindowFormat @+0x14/+0x94/+0x114 (128B each)** | **MINT `JE::HotKeyWindowFormatTable`** — strongest name here: three inline instances of a shipped-name GUI record type at a fixed stride = a format table by construction; keyword `struct` |
| 0x56eb70 | 2 | G3D_Mesh(156B) @+0x70, class size 268 | **MINT `JE::G3DMeshHolder`** — inline mesh by value; role beyond holding NOT claimed |
| 0x4c0190 | 6 | Gfx_Camera(300B) @+0x30, size 640 | **MINT `JE::GfxCameraHolder`** — inline camera by value (76 members say it owns more than the camera; name covers the embed only) |
| 0x4cb300 / 0x4d4650 | 5 / 1 | **identical shape**: size 152, G3D_Transform(120B) @+0x20 | **MINT PAIRED `JE::G3DTransformHolder` + suffix by address** ONLY IF byte-diff says distinct classes — same-size/same-embed pair is exactly SwiftRaven's clone pattern; QuickQuartz: check the methods byte-identical BEFORE two names (if clones, ONE name + clone note) |
| 0x40a220 / 0x4cb0a0 | 12 / 1 | std::basic_streambuf<char>(96B) ×2 aliased @+0x8/+0xc / std::strstreambuf(96B) @+0xc | **NAME-BLOCKED, escalation**: two streambuf-bearing classes (sizes 156/136) are almost certainly two instantiations/views of ONE strstream-ish type — IDA-split suspicion matches the batch-1 "171 tail-alias artifact class". Merge-check first; then mint `JE::StrStream` once, not twice |
| 0x558740 / 0x5b6400 / 0x4395a0 / 0x5b4eb0 / 0x5b7c10 | 12/5/1/1/1 = 20 | **CEngineInterface(49B) value-embed at NON-ZERO offset (+0xc4/+0x3c/+0x8/+0xc/+0x18)** | ~~the embed is a BASE SUBOBJECT~~ **CORRECTED PRE-MINT by the R36-e gate (SwiftRaven T-036, 38118cb): >=3 embedders ⇒ it is a MEMBER TYPE, not the class.** Five embedders of a 49B value makes composition the DEFAULT reading; base-subobject survives only as the minority alternative (nonzero offsets ARE legal MSVC base placement in non-primary inheritance, but that is now a hypothesis needing vbptr-adjacency/ctor evidence, not a finding). Verdict unchanged and strengthened: STAY ANONYMOUS; annotation **`embeds@CEngineInterface`** (was `derived@`), name the base onto NONE of them; distinguishing embed-vs-base is batch-3 work with openje's `CEngineInterface.hpp` |

## What the batch actually yields (pre-registered again)
Mints available now: **3–5 names / 12–16 fns** (HotKeyTable, MeshHolder, CameraHolder, Transform pair-or-one);
**83 fns (63% of the self-namers) are NOT nameable** via this join — the refuted aggregate stays fog.
The 13-cheap-wins framing was directionally right, but the win is the **COMPOSITE-EMBED DISCOVERY** — five
classes carrying a 49B CEngineInterface subobject at nonzero offsets is a type-level fact clone spread
cannot manufacture (R36: 12 of 14 self-namers show 100% body evidence, ratios 0.07–1.00 — the evidence
class is metric-invariant, unlike contiguity). Whether that fact says composition or inheritance, it
propagates into every decompile of their methods; the R36-e correction above is the honest form of the
claim. A census that prints how it knows, applied to names.
- Small-n per T-036: 7 of my 13 carry <=2 fns (one body of evidence); their wider 14-class join count
  differs from mine by one — the join is route-dependent and I state mine as `DB member-table join,
  anonymous cls_* keys only`, not as THE number.
- Export rule inherited: **no route-A tier movement exists until IDB rename AND F3.json re-export both
  land**; cls_0x49c610=JE::ResourceSystem with ZERO occurrences in F3.json proves the export can omit
  named classes outright — absence-in-export is not absence-in-binary, and my delta report will name
  the stale route rather than averaging.

## Apply handoff (QuickQuartz, rename-only scope)
Mintable strings + keywords above; two PRE-CHECKS gate the mints: (1) byte-identical test on the
0x4cb300/0x4d4650 method pairs before naming twice; (2) alias-merge test on 0x40a220/0x4cb0a0.
`struct` keyword for all (all embeds are `U`-mangled base/member records — MSVC-2002 trap per row §3).
Re-run T-029 census AFTER applies and print BOTH route counts (name-route and DB-method-route) per the
inherited constraint; compare against this doc's ≤1.3pt band, not batch 1's voided one.

## Boundary
Every verdict above is computed from DB member-table content only — no debugger, no hunches, no vftable
semantics claimed beyond size/offset arithmetic. "Derived", "holder", "table" name the STRUCTURE the DB
proves, nothing more; anything that would need behaviour to justify the name stayed anonymous or escalated.
