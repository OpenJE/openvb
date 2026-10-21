# 2025-10-21 — T-064: Apply the T-063 MINT-READY queue. Rename wave — **T2 2,012→2,000 (−12)**.

CalmRaven (first-in-writer-slot), dispatched by LoudDragon `01a15f82` (row `8f22d36`). SOLE IDB WRITER on
`ida_original_f3` (8745) from claim commit `c8fae8b`. EpicRaven's T-063 queue (`b3ba8e7`-era, census source
`t060-postdump.json`) = **CITATION, not evidence** — every claim re-verified live at write time.

## 0. Pre/post census + save discipline (T2 itemized [A]/[B]/held)

| phase | T2 (cls_ prefix, excl. j_cls_) | T3 | T1 | game | IDB mtime |
|---|---|---|---|---|---|
| PRE-WRITE | 2,012 = 22.67% | 2,169 | 4,479 | 8,876 | 16:10:42 (T-060 close, disk==live) |
| after [A] (cp) | 2,000 | 2,169 | 4,491 | 8,876 | 16:45:03 |
| **END** | **2,000 = 22.53%** | 2,169 | 4,491 | 8,876 | **16:45:48** |

**T2 delta = −12, ALL from [A]. [B] = 0 (annotation-only rejection). Held = 0.**
Real applied count **12**, not the board's forecast ~15: the forecast assumed [A]=10 and [B]=5 renames; both
were wrong on re-verification (§2 count drift, §3 [B] helper-theft). Cumulative vs pre-b2 2,340 → **2,000 =
−340** (24+192+40+18+20 [b2–b6] + 34 [T-055] + 12 [T-064]; T-058/T-060 were delta-0 comment waves).
Dumps `discoveries/wavedumps/t064-{pre,post}dump.json`; fresh re-dump matches live → disk==live.

## 1. Item-42/43/41 re-verification performed (the wave's substance)

Naming decision on [A] (no shipped string, writer's call like `JE::Gfx_BufferManager`):
collision-checked against the whole function DB (0 hits on every candidate), epoch-consistent prefix
(`JE::G3D_*` — siblings G3D_Node/Transform/SpotLight), body-bounded (fixed **3-slot record array**: ctor
zeroes 3 records, `0x48f940` writes value-pairs at stride 0x20 ×3, `0x48f870` `rep movsd` 6 dwords/record),
consumed at `JE::G3D_Node::meth_0x4caaa0` (`lea esi,[eax+0A0h]`). Chosen: **`JE::G3D_NodeRecordSet`**.

## 2. [A] MINT — `cls_0x48fb60`+`cls_0x48fce0` → `JE::G3D_NodeRecordSet` — **12 fns, not the doc's 10**

**Count correction (stated as real number, not forecast).** Construction proof re-derived live and it
CONFIRMS a single composite object at `G3D_Node+0xA0`:
- `JE::G3D_Node::ctor_0x4d7600`: `lea edi,[esi+0A0h]` @0x4d7640 → `mov ecx,edi; call cls_0x48fce0::ctor_0x48fce0`
  @0x4d764d → `mov ecx,edi; call cls_0x48fb60::meth_0x48ff20` @0x4d7689. **Both calls carry the same
  `this` = the +0xA0 slot.** (EpicRaven's `lea ecx,[esi+0A0h]` phrasing is the register-flow `edi` hop —
  verified; the claim holds, my first literal-match check was too strict — same lesson as the `\xc7` pad.)
- `cls_0x48fce0::ctor_0x48fce0` constructs fields then calls `cls_0x48fb60::ctor_0x48fb60` with `ecx=esi`
  (the SAME base). So fb60 + fce0 are one object split by OOAnalyzer (b5 §1a `cls_0x502870`/`cls_0x490060`
  class). Vft-less, ~0x60 B, max member offset touched 0x5C.

| fn EA | old | new | head | readback |
|---|---|---|---|---|
| 0x48fb60 | cls_0x48fb60::ctor | `JE::G3D_NodeRecordSet::ctor_0x48fb60` | ✓ | ok |
| 0x48ff20 | cls_0x48fb60::meth | `…::ctor_0x48ff20` (record-init on +0xA0) | ✓ | ok |
| 0x490010 | cls_0x48fb60::ctor | `…::dtor_0x490010` | ✓ | ok |
| 0x48f870 / 0x48f940 / 0x48fe50 / 0x48fe70 / 0x48fee0 / 0x48ff70 / 0x48ffd0 | cls_0x48fb60::meth_* | `…::meth_*` (SetRecords_0x48f940 named) | ✓ | ok |
| 0x48fce0 | cls_0x48fce0::ctor | `…::ctor_0x48fce0` (outer ctor) | ✓ | ok |
| 0x48fac0 | cls_0x48fce0::meth | `…::meth_0x48fac0` | ✓ | ok |

12/12 renamed, all `ok:true` + readback. Residual `cls_0x48fb60::`/`cls_0x48fce0::` = **0**. T2 **−12**.
Not absorbed: `cls_0x48fe90::ctor_0x48fe90` (single caller 0x48ffd0, own key, unproven as same object —
left anonymous, no adjacency-only re-parent per b2 rule).

## 3. [B] REJECTED — `cls_0x558740` residual 5 → helper-theft, annotation-only, T2 delta 0

**This reverses my own T-055 hold correctly and rejects T-063's [B] as stated.** T-063 claimed the 5 are a
"CDebugInterface band extension" via the item-42 view-adjust. I verified the geometry and it points the
OTHER way: the `-0xB0`/`-0xC4` view-adjust proves **sharing, not membership**.
- `0x434250` is called by `0x434340` (recovers `edi=[esi-0B0h]`, stores **CFeedbackInterface**_vftable_654304)
  **AND** `0x557bf0` (recovers `edi=[esi-0C4h]`, stores **CDebugInterface**_vftable_692e84) **AND** 0x558190/
  0x558510. A single function correct across TWO different interface outer bases = **`GUI::Interface` base
  machinery**, and it calls the `JE::GUI::Interface::meth_0x4b9d60` cleanup helper (the same helper T-059 §
  reading-1 and my T-060 annotation cover).
- `0x434120`: also reached by `F3::GameInterfaces::virt_meth_0x4396c0` (`this=[esi+0x0C]`) and
  `JE::Client::virt_meth_0x58f010` (`this=[esi+0x8C/0x90]`) — different outers = fan-out (item 43 signature).
- `0x4342c0`: sole caller is the **CFeedback** path 0x434340. `0x434760` (doc-flagged WEAK): callers
  0x434910 + `cls_0x434f70` (CFeedback ctor region) via recovered `edi`. `0x434910`: caller GameInterfaces
  via a GameInterfaces slot, not CDebug.

**Item-41 triplets answered per row** (Q1 callers-of-callers: multiple unrelated owners; Q2 table→thunk→band:
zero vft slots into the band; Q3 full .rdata/.data/.data1 window: no `this==&/*global` pin). Renaming under
CDebugInterface would be promoted noise — exactly what b4/b5 refused at 0x5240e0. **Verdict: ANNOTATE, NO
RENAME, T2 delta 0** — comments at all 5 record the shared-GUI-base reading so the next agent doesn't
re-litigate. 5/5 readback OK; band `cls_0x558740::` stays at 5 (the correct T-055 residue, now explained).

## 4. Wave ledger — counts

| bucket | fns | T2 | disposition |
|---|---|---|---|
| [A] G3D_NodeRecordSet (composite, +0xA0) | **12** | **−12** | MINT, construction-tier, writer-named + collision-checked |
| [B] cls_0x558740 residual | 5 | 0 | REJECTED as helper-theft → annotation-only |
| held-for-arbitration | 0 | 0 | none (both sections resolved on evidence) |
| forecast vs real | — | — | forecast −15; real **−12** ([A] +2 over doc's 10; [B] −5 not renames) |

## 5. Persistence + hand-back
cp 16:45:03 ([A]) + END `save_database` **16:45:48** ([A]+[B]); post-save fresh re-dump re-read:
[A] 12/12 = `JE::G3D_NodeRecordSet::*`, [B] 5/5 = `cls_0x558740::*` + comment present, T2=2,000/T3=2,169/
game=8,876 → **disk==live**. Reservations to release; writer slot handed back. KeenStorm read-only unaffected.

## 6. Follow-on for the coordinator
- [A]'s 12-vs-10 is a census count update (composite object), not an identity conflict — board census note
  can quote 12. T-063 §4 decomposition's `MINT-READY 15` becomes `12 minted + 5 rejected-to-annotation`.
- [B] rejection is a verdict on T-063's own [B] claim; EpicRaven's view-adjust data was correct, the
  *interpretation* (extension vs shared-base) differs — the -0xB0 CFeedback path is the falsifier. Flagged
  for their awareness (not an arbitration trigger; no DISP/byte conflict).
