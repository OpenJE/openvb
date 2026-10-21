# Session 38 — T-033 batch 1 adjudication: JE::String is REFUTED (water-host composite, evidence), StringTree confirmed-with-caveat, RTTI is a dead end, and the census's T2 tier needs a contiguity split (2,473 attributed → 1,475 nameable)

Joint doc (IronStorm side) for `2025-10-20-t033-batch1-identity-decisions.md` (happywolf side).
One-writer rule honored: HW supplied decisions; this file holds the IDA-join adjudication, the
census methodology correction, and the re-run plan. All addresses route-stated.

## 1. `cls_0x50db20` → JE::String: **REFUTED.** It is the G3D_Water host composite.

Route: F3.json member table (this file's joins are F3.json + /tmp/census_p1-7.json = T-029 dump).

- The DB *itself* carries the composition evidence: member +0x1C = `.?AVVFX_Parameter@@` (52B),
  member +0x20 = `.?AVG3D_Water@@` — **size 180, and 0x20 + 180 = 0x224 = 548… exact class end
  at 577 is mbr_0x240(1B)+pad; the embeds sit at the head band** (openje already ports this as
  `cls_0x50db20.hpp` including exactly `VFX_Parameter.hpp` + `G3D_Water.hpp`). A 0x1C-byte SSO
  string cannot *contain* a 180-byte water engine block.
- HW's four "SSO behavior" sites resolve, via my census address-join, into other owners:
  0x59f64f/0x59f662 are inside **`F3::RegisterCommand` (0x59f5d0)** — the command-registry's own
  key string — and 0x580c0a/0x580c55 inside `cls_0x580ba0::meth_0x580ba0`. The behaviors are
  real; the class attribution was the DB scatter, exactly the signature HW flagged for 0x50dbb0.
- Supporting shape evidence: the class carries **36 copy-ctor EAs** (0x404bc0, 0x4d35c0,
  0x4d9390, 0x4da1c0, 0x4df0d0/140, 0x4e0020/090, … 0x5b5220) — water/scene records copied at
  every map/VFX load site; its usage bands (0x4c8–0x4d5, 0x56f6xx, 0x593xxx) are the G3D_Water
  method band (`G3D_Water::meth_0x4c8d70/0x4ce730/0x4d24a0…`) and the VFX_Parameter band
  (`meth_0x45ceb0` seen in census p2 — JE::VFX_Parameter methods sit in the same region).
- **Proposed identity direction, not yet minted:** VFX-side water host. Candidates tested and
  rejected as the name: `VFX_Water` (1021B, embeds VFX_EffectInstance+VFXEFFECTTYPE_WATER_Struct
  — different shape), `Water` (984B, DynamicObject-based), `Gfx_Water` (373B), `VFX_VEGInstance`
  (80B, embeds VFX_ParameterHolder). Nothing in the named inventory matches 577B-with-water-head,
  so it stays anonymous and **escalates**: one decisive xref (who owns the 0x691Axx-style global
  instance / which chunk reader fills it) closes it. This is the row's rule working, not failing.

## 2. `cls_0x4d8d70` → JE::StringTree: **CONFIRMED**, with the mechanism now identified.

Route: census-contiguity + ctor census. 138 functions, 11 copy-ctors, usage band spread is
*explainable*: this is the MSVC STL `_Tree`/`_Tmap_traits` **template family** — every
`map<string,X>` instantiation clones the tree control block, which is why the methods scatter
across all 11 subsystems (HW's registry sites + `World::gap90[72]`). The scatter is therefore
legitimate template mass, not misattribution: same-name, many-instantiations. The 0x1C-vs-0x18
reconciliation ⚠ resolves in favor of both being *different* types — 0x18 = the MSVC
`basic_string` inside tree nodes (template, not a JE class), 0x50db20 is not a string at all.
One string type in the corpus: the std one. Name proceeds.

## 3. `cls_0x55dc80` (handed to me): **ESCALATE with a narrowed candidate set.**

Route: usage-band join (F3.json usages × census). Members concentrate 0x51f–0x521 (68/75 sampled
usages) — T-013's texture-registry band — one ctor at 0x55dc80 itself, plus the 0x403xxx
GameActionAttack copies (per-attack snapshot). Shape = frame/registry record cloned in two
contexts. Identity is NOT yet nameable from the DB alone; needs T-013's node evidence folded in.
Cross-link: T-013 row.

## 4. RTTI hypothesis: **DEAD END, recorded so nobody spends a session again.**

The shipped PE carries 216 MSVC-mangled `.?AV` name strings clustered at file 0x2f1eb4–0x30629c
(.data region), but a byte-pattern search finds **zero pointer xrefs** to any name string
(`struct.pack('<I', nameVA)` → no hits anywhere in the image, e.g. `.?AVAnimationCache@@` @VA
0x6f7a04). No TypeDescriptor/COL chain exists ⇒ names shipped without RTTI linkage (linker
residue). Usable identity data remains only: embedded member types (found 13 anon classes
self-naming that way, 132 fns incl. the 50db20 case above), vftable targets, and strings at
call sites.

## 5. Census methodology correction (T-029 addendum — the batch's biggest product)

Per top-18 anon class: fraction of attributed functions within ±0x2000 of the class's median
method EA ("contiguity ratio"):
```
cls_0x4d8d70 138 fns  ratio 0.08   (template family — legit)
cls_0x50dbb0 120       0.07        cls_0x55dc80  97  0.22
cls_0x50db20  83       0.07        cls_0x424a70  55  0.33
cls_0x4542d0  49       0.18        cls_0x668cd8  46  0.37
cls_0x49c610  43       0.02        cls_0x6bb908  40  1.00
cls_0x6bc9d8  30       0.97        cls_0x41a2b0  28  0.71
cls_0x4b28a0  28       0.93        cls_0x5d2c8c  26  0.96
cls_0x6b80d8  21       1.00
```
T2 tier total attributed = **2,473**; functions under classes with ratio ≥ 0.5 = **1,475**.
Consequence, stated bluntly: *every* headline class in the original top-20 is low-density — the
"T2 tier = 2,340" number is real as attribution but ~40% of it is generic-helper theft (HW's
finding generalized). Batch 2+ ordering must use **contiguity-ratio × fns**, and honest naming
of a low-ratio class (StringTree) must say "template family, N instantiations" rather than
claiming one class.

## 6. Re-run mechanism — the one thing this joint needs from the coordinator

Scope-3 renames (`cls_0x4d8d70` → `JE::StringTree`, and later ones) via `mcp__ida_original_f3__rename`
are live-IDB writes — my standing rule is read-only IDB. Additionally `artifacts/F3.json` is a
*regenerated export*; renames land in the IDB, and my census join runs against the JSON. So a
census re-run showing an attributed delta requires: (a) coordinator/user authorizes (or performs)
the class rename in the IDB, (b) the F3.json exporter is re-run, (c) I re-join. Until (b) happens
the *attributed* tier cannot move; what CAN move and be measured now is the T1↔T3 line (see §7).
Not a blocker — a sequencing statement, per item 16.

## 7. What batch 1 actually shifts (measurable without any IDB write)

HW's "solid names retire ~217 fns" overcounts for the census tiers: renaming the KEY does not
de-anonymize 138+79 functions; they remain attributed either way. The genuine, immediate wins:
1. 50db20's 83 fns re-tier from "nameable-stub" to "escalated composite with owner band known"
   (and one wrong JE::String name avoided — that would have poisoned every later decompile).
2. The contiguity split above re-quantifies T2 honestly: 1,475 nameable / 998 helper-theft.
3. 13 embedded-name classes (§4) enter the queue with pre-named identity for ~0 marginal cost.
Tier numbers for the re-run baseline: semantic T1 46.7% stands; T2 restated as
**26.4% → (1,475/8,876) = 16.6% nameable + (998/8,876) = 11.2% attribution-noise**.
