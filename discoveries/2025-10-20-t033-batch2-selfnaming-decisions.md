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
| 0x558740 / 0x5b6400 / 0x4395a0 / 0x5b4eb0 / 0x5b7c10 | 12/5/1/1/1 = 20 | **CEngineInterface(49B) value-embed at NON-ZERO offset (+0xc4/+0x3c/+0x8/+0xc/+0x18)** | **KEY FINDING — the embed is a BASE SUBOBJECT, not a member**: 49B at nonzero offset = MSVC derived-class layout. These 5 are **classes DERIVED FROM the shipped `CEngineInterface` base**; minting the base name onto a derived would be the batch's worst poison. Verdict: STAY ANONYMOUS as names, but each gains a DB annotation `derived@CEngineInterface` — that is the honest artifact. Derived identity (device/GUI subsystem role) needs vftable mirrors — batch 3 candidate with openje's `CEngineInterface.hpp` cross-check |

## What the batch actually yields (pre-registered again)
Mints available now: **3–5 names / 12–16 fns** (HotKeyTable, MeshHolder, CameraHolder, Transform pair-or-one);
**83 fns (63% of the self-namers) are NOT nameable** via this join — the refuted aggregate stays fog.
The 13-cheap-wins framing from the adjudication was directionally right but the win is the **DERIVATION
DISCOVERY** (5 CEngineInterface derived classes now known-derived — that propagates into every future
decompile of their methods) more than the tier number. A census that prints how it knows, applied to names.

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
