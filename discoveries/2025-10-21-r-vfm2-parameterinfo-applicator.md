# 2025-10-21 — R-VFM-2: VFX_ParameterInfo anatomy, applicator-band ownership, the 0x4F9A00 tail

**Agent:** SwiftMoon `01a16092` (PARK pull, lane contract §2) · **Coordinator:** LoudDragon `01a15f82`
**Mode:** read-only — raw PE parse of `$OPENVB_ORIG_EXE` + aligned linear disassembly (`/tmp` recipe
inherited from T-078/R-VFM: `objdump -D -b binary -m i386 -M intel --adjust-vma=0x400000`, all
sections satisfy VA = file-offset + 0x400000) + `artifacts/F3.json` (RTTI-named records).
Zero IDB, zero writes. Claim commit `252960f`. Yardstick: R-VFM + T-073 §2.

## 0. Self-test gate (reproduce R-VFM/T-073 anchors BEFORE trusting anything new) — 5/5

| # | anchor | raw result |
|---|---|---|
| G1 | VFM getter's single caller edge | `e8 cac00600` @0x48EF71 → `call 0x4FB040`, image-wide e8-scan: exactly 1 hit ✓ |
| G2 | schema constants | `0x52FB10 = b8 76 07 00 00 c3` (1910), `0x52FB20 = b8 2b 00 00 00 c3` (43) ✓ |
| G3 | 43-row builtin table head | `[0x6FC968]={0x67C280 'General'(inline-flag 01), 0x6739FC 'std::string', 0x65CBE0 'Name', 0x64D7D8 ''}` ✓ (third consumer of the pooled `""` re-seen from the other side) |
| G4 | file/block signature literals | 'VEG '@0x673A30, 'V1.1'@0x673A28, 'VFX '@0x673900, 'V1.0'@0x6738F8 ✓ (no contiguous 8-byte sigs anywhere) |
| G5 | ParameterInfo vft (T-073 claimed 0x673944, 30 methods) | raw walk: **length 1** (slot 0 = 0x4FB430), cell 1 @0x673948 = `'>'` literal — single-virtual like the rest of the epoch; '30 methods' = non-virtual member count ✓ (DB confirms length 1) |

## 1. VFX_ParameterInfo (RTTI `.?AVVFX_ParameterInfo@@`, vector element stride 0x114) — FULL MAP

Ctor-default 0x4FB2D0 + ctor-from-schema 0x4FB5B0 (raw stores) joined against the export member
table (`.?AVVFX_ParameterInfo@@` size **273**):

| off | field | evidence |
|---|---|---|
| +0x00 | **VFX_Parameter base subobject** (52B; own vft 0x673A20; ctor 0x4FE1E0 runs FIRST, then 0x4FB2F5 overwrites `[this]` with 0x673944 — textbook derived-ctor vft sequencing) | 0x4FB2EE/0x4FB2F5 |
| +0x34 | SSO string #1 {buf 16B, size +0x48, cap +0x4C=0xF inline-init} | 0x4FB2FB–0x4FB30A |
| +0x50 | SSO string #2 {size +0x64, cap +0x68} | 0x4FB30D–0x4FB313 |
| +0x6C | **VFX_Parameter subobject #2** (ctor 0x4FE1E0 on `lea ecx,[esi+0x6C]` @0x4FB316) | |
| +0xA0 | flag byte (row of subobject #2; `bl` store @0x4FB353) | |
| +0xA4 | **VFX_Parameter subobject #3** (0x4FB323) | +0xD8 flag 0x4FB34D |
| +0xDC | **VFX_Parameter subobject #4** (0x4FB333) | +0x110 flag 0x4FB347 |
| end | last byte +0x110 ⇒ span 0x111 = **273 = export size — AGREES (no erratum; 4th confirm-register direction per R-VFM law)**; vector stride 0x114 = 4-byte padding of 273 | |

Copy-from-schema 0x4FB5B0 walks the same grid by +0x1C/+0x38 strides (string copies @0x4FB5DD–
0x4FB62E incl. `call 0x5006F0` = VFX_Parameter copy for the nested subobjects) — and its callers
0x4FB8EB/0x4FBFCB/0x4FC383 sit inside the vector insert/clone cluster T-073 mapped ✓ (chain
closes on the registrar's 0x4FB2D0 @0x4FCD9D — two construction paths: default+fill vs copy).

**Refinement of T-073 §2a wording** (correction-forward, their data all stands): "4 basic_string
members @+0x34/+0x6C/+0xA4/+0xDC" — the export + raw say: strings @+0x34/+0x50, and +0x6C/+0xA4/+
+0xDC are whole **VFX_Parameter subobjects** (each carrying its own name string @+0x04 and value
block). Same member addresses; richer types.

### VFX_Parameter (52B, export `.?AVVFX_Parameter@@` size 52 — ctor span 0x04..+0x33, AGREES)
`{vft 0x673A20 (single slot 0x4FF950), name SSO string @+0x04 {size +0x18, cap +0x1C=0xF} (0x4FE1F4–
0x4FE1F1), value-cache dwords +0x20/+0x24/+0x28 = −1 sentinel (UNSET), +0x2C/+0x30 = 0}` —
typed readers: `toBool 0x4FF970` / `toFloat 0x4FE280` branch on `cmp [this+0x20],0xE` (SSO
length dispatch into the value string; heap path reads `[this+0x30]` @0x4FF996), and
0x4FF1B0 = typed default WRITER (`[this+0x20]=len`, jump-table 0x4FF540, 12 default sites incl.
'false'@0x671EAC/'0'@0x661160). FINAL-unknown (labelled, not guessed): the exact per-slot
meaning of +0x24/+0x28 (the three −1s most plausibly = bool/float/int typed-value caches keyed
by the §3.2 type switch; no read in the sampled paths distinguishes them).

## 2. APPLICATOR-BAND OWNERSHIP — answered, band is two RTTI-named classes, not Gfx_D3D

| site | owner (vft slot / method) | role |
|---|---|---|
| **0x5011A0** | `VFX_ParameterHolder::meth_0x5011A0` (export 52B, vft 0x67390C: [0]0x4FADB0 [1]0x4FAF20 [2]**0x4F9D40**) | property APPLICATION: `new(0xC)` block → `[holder+0x30]` = the 3-slot live value {byte@0, f32@4, f32@8}, then find('Loop')→`0x4FFB60`→`[blk][0]`, find('MinStartTime')→`0x4FE280`→`fstp [blk+4]`, find('MaxStartTime')→`fstp [blk+8]`, defaults on miss 0/0.0 (0x5011E1/0x501205/0x50120D) — 'Loop'/Min/Max are the FIRST three applied; same method family continues the schema list |
| **0x4FA3C0** | `VFX_ParameterHolder::meth_0x4FA3C0` | find-by-name: assigns query into temp string, walks the vector at `this+0x4` (element **0x28**, `lea esi,[eax+0x28]` @0x4FA450 — the holder's OWN 40-byte entry rows, distinct from ParameterInfo 0x114), end-compare `[this+0x8]`, miss ⇒ nullptr |
| **0x501340** | `VFX_VEG::virt_meth_0x501340` (export 60B, base ParameterHolder @+0x00, vft 0x673AAC: [0]0x501AE0 [1]0x501FA0 [2]0x501340) | THE file-header validator: `'VEG '`@0x50136D + `'V1.1'`@0x501383 (stream cmp `0x4B8130`). Zero direct e8 callers — it is vft-DISPATCHED (slot 2): reachability proven by table membership, which also closes the R-VFM "who checks the file header" gap |
| **0x502010–0x50202C** ('V1.0'→1.0f / 'V1.1'→0x3f8ccccd scale) + **0x50208C** (the ONLY caller of block-checker 0x4F9960) | containment walk: function head 0x501FA0, ret+pad tail **0x502118** ⇒ both inside `VFX_VEG::vft-slot-1 0x501FA0` | **the .veg loader body**: version-scale select + per-block header check. (R-VFM asked 'whose 0x50208C' — VFX_VEG::load, NOT the Gfx_D3D epoch: 0x501xxx–0x502118 is a contiguous VEG-class span; the b4 Gfx_D3D band starts at 0x502870 — ruled out by containment, not adjacency-assumption) |
| 0x500C5F | within the 0x500Bxx–0x500F9x value-emit family (DB-unattributed function; VEG epoch) | **UPGRADE to R-VFM's §4-C3 claim:** the `and al,0xDC` is NOT a compare — `0x4FFB60`→`neg al; sbb al,al; and al,0xDC` ⇒ the engine ENCODES bool to the spec's 0x00/0xDC wire byte (0x500C5B–0x500C68 stores the byte into the row). Conformance strengthened: the encoding is PRODUCED, not merely tolerated |
| 0x48F054 → 0x5011A0 | VFX_VEGInstance band (same 0x48E/0x48F region as the getter caller 0x48EF71) | the consumer loop closes: **VFM (global schema registry) → VFX_VEG (file reader, load 0x501FA0) → VFX_VEGInstance (per-effect script object) → Holder::apply 0x5011A0 → live value block {Loop, Min, Max}** |

## 3. THE 0x4F9A00 TAIL — closed as dead-as-shipped

Search BOTH ways (item-49 discipline generalized): (a) image-wide 4-byte cell scan for the value
`0x4F9A00` across ALL sections = **0 hits**; (b) relative `e8/e9` decode across .text = **0 hits**.
No vft cell, no data cell, no direct call/jmp. 0x4F9A00 is the second (string-compare-form)
implementation of the block-header reader — the live one is 0x4F9960 (u32-compare form), called
exactly once from VFX_VEG::load @0x50208C. Verdict: **statically unreferenced in the shipped
image** (dead variant / link-era residue). Honest limit: runtime-computed pointers (base+disp
folds) are not excluded — but with zero cells anywhere, no static path exists to compute it from.
FINAL-unknown reduced to: "was it ever called? unanswerable statically; doesn't matter to the port".

## 4. Buckets, counts, successor

* ParameterInfo: **complete** (11 field rows + 2 ctors + dtor + single vft slot) — export sizes
  AGREE on both classes (273/52/56-VFM… no errata here: the no-erratum register used again).
* Applicator band: **named** — VFX_ParameterHolder (2 methods + slot2-count-reader), VFX_VEG
  (3-slot vft, load body mapped), VFX_VEGInstance (driver edges 0x48EF71/0x48F054).
* FINAL-unknowns with reasons printed (2): value-cache dword roles +0x24/+0x28 (three −1
  sentinels, type-switch plausible, no distinguishing read sampled); the 0x28-entry holder row
  layout beyond {name-string@+0x00?, +0x28 stride} (find walks it; full field map would need
  the row-ctor — cheap follow-on, see successor).
* **Successor (contract §3):** propose PARK `R-VEG-E` — "VFX_Parameter value-cache semantics
  (+0x20..+0x28) + the ParameterHolder 0x28-row element anatomy + the 0x500F90 value-emit
  family attribution" — small, offline, feeds the audio/cursor port era right where R-CS does.

## 5. Hygiene

Read-only honored: image + F3.json + /tmp scratch; zero IDB connections; zero docs/discoveries
edits; this file left DIRTY. Every EA above re-read raw this sweep; T-073/R-VFM used ONLY as
the §0 yardstick (their member-grid wording refined, all their data re-confirmed: 5/5 gate +
0x114 stride + the 43-row table + the single-virtual epoch pattern). Self-test-first generalizes
across three consecutive rows now — the gate caught nothing this time BECAUSE the upstream data
was right, which is the same outcome measured from the other direction.
