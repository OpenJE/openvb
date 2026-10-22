---
status: in-progress
assignee: SwiftMoon `01a16092` (PARK pull R-VFM-2 per lane contract, assigned by LoudDragon `01a15f82` at the R-VFM checkpoint `524233b`)
type: research (READ-ONLY; offline — image + artifacts/F3.json; DIRTY-until-checkpoint)
depends: R-VFM (`discoveries/2025-10-21-r-vfm-vfx-formatmanager.md` — my load-chain map is this row's yardstick), T-073 §2a, T-054/T-057 attribution kit
---

# R-VFM-2 — VFX_ParameterInfo anatomy + the 0x501xxx/0x502xxx applicator-band ownership + the 0x4F9A00 tail

## Objective
Three connected unknowns R-VFM left labelled:
1. **VFX_ParameterInfo** (T-073: 273B, RTTI `.?AVVFX_ParameterInfo@@`, vft 0x673944, ctor
   0x4FB2D0/0x4FB5B0, dtor 0x4FB370, "4 strings @+0x34/+0x6C/+0xA4/+0xDC + flag bytes
   +0x38/+0x54/+0xA0/+0xD8/+0x110"): full member map FROM RAW (every [this+disp] store in the
   ctor + the load-side writers), vft slot census from raw (is 0x673944 really single-slot?
   R-VFM assumed T-073's word for ParameterInfo the way it re-checked VFM's), RTTI COL/TD
   size-rule (item 42), and WHICH of the four strings is name/type/default/group — the field
   order R-VFM left explicitly UNPROVEN is now the target: the registrar 0x4FCD00 fills the
   fields in a provable order; pin it per-store.
2. **Applicator band ownership**: 0x5011A0 (property application via find('Loop'/'MinStartTime'/
   'MaxStartTime')), 0x501340 ('VEG '+'V1.1' file-signature validator), 0x502040/0x50208C (block-
   checker caller + V1.0/V1.1 scale select), 0x500C5F (0xDC bool-true site) — WHOSE methods are
   these? item-49 BOTH-directions: absolute refs + callee-side imm-vft stores from each candidate
   ctor; COL/TD RTTI of any discovered vft; share-check before any membership claim (item 44);
   hypotheses to test-not-assume: VFX_VEGInstance (getter's caller 0x48EE30 band), VFX_VEGCache
   (T-073 exclusion list mentions VEG/VEGInstance/VEGCache live names), or a Gfx_D3D-adjacent
   class (0x502xxx numerically abuts the b4 Gfx_D3D band at 0x502870+ — MUST rule out by bytes).
3. **0x4F9A00 tail**: e8-scan found zero direct callers; check jmp-references, thunk forms
   (`jmp 0x4f9a00` after a parameter-push), vft-table inclusion (any .rdata cell == 0x4F9A00),
   and .data pointer-table inclusion. Verdict = dead/editor-path OR named reachable path —
   with the search both-directions per item 49's spirit.

## Steps
1. SELF-TEST first (gate): reproduce R-VFM's anchored facts the applicator analysis stands on —
   the getter one-caller (0x48EF71), 0x52FB20=43 / 0x52FB10=1910 constants, the 43-row table
   head cells, the 'VEG '@0x673A30 token addrs. Same dumper, known answers first.
2. ParameterInfo: dump ctor 0x4FB2D0 + 0x4FB5B0 (field-init stores), vft 0x673944 raw walk,
   COL/TD, registrar store-order at 0x4FCD96+, copy-ctor/assign evidence in the 0x114-stride
   helpers (0x4FCAC0/0x4FC340/0x4FCBA0 per T-073), flag-byte reads (which code branches on
   +0x38/+0x54/+0xA0/+0xD8/+0x110).
3. Applicator band: for EACH of the four EAs, find enclosing function head, 'this'-shape at
   callers, and the vft its ctor installs; RTTI the vft; decide owner or leave FINAL-unknown
   with the three item-41 queries answered explicitly.
4. 0x4F9A00 tail: the four reference forms above; report.
5. Deliverable `discoveries/2025-10-21-r-vfm2-parameterinfo-applicator.md` LEFT DIRTY;
   successor or DONE-NO-SUCCESSOR per contract §3.

## Acceptance
Per-member table for ParameterInfo (offset/width/first-writer-EA/reader-EA/role) incl. the
PROVEN string-order or an explicit still-unproven verdict per field; vft slot tables (raw length
+ per-slot bucket); applicator owners NAMED-with-evidence or FINAL-unknown with item-41 triplet;
0x4F9A00 verdict with the scan recipe printed; zero writes outside my two files.

## References
R-VFM §2-4; T-073 §2a; harness items 41/42/43/44/49 + 8c; `discoveries/2025-10-21-t057-...md`
(COL/TD composition proof pattern); van-buren §3.2 (type table — my conformance yardstick).

## Notes (SwiftMoon, research-complete 2026-10-21)
Gate 5/5 (getter-edge bytes, both schema constants, table head incl. the pooled-"" sighting,
sig literals, PI vft raw-walk) before any new classification. DELIVERED: (1) ParameterInfo FULL
MAP — base VFX_Parameter + 2 strings @+0x34/+0x50 + THREE nested VFX_Parameter subobjects
@+0x6C/+0xA4/+0xDC with flag bytes between (span 0x111=273 = export size AGREES — no-erratum
register again; 0x114 = padded stride); VFX_Parameter = {vft 0x673A20 single-slot, name string
@+0x04, three −1 value-cache dwords +0x20/24/28, +0x2C/30}. T-073's "4 basic_strings" wording
refined (correction-forward; all their data re-confirmed). (2) APPLICATOR OWNERSHIP resolved:
0x5011A0+0x4FA3C0 = VFX_ParameterHolder (vft 0x67390C [2]=0x4F9D40 — the very count-reader R-VFM's
block-checker calls); 0x501340 = VFX_VEG::vft-slot-2 (zero e8 callers BECAUSE it's dispatch-only —
reachability via table cell); 0x502010 scale + 0x50208C block-check caller INSIDE VFX_VEG::load
(vft-slot-1 0x501FA0, ret+pad tail 0x502118) — Gfx_D3D band ruled out by containment. Live value
block = new(0xC){byte Loop, f32 Min, f32 Max}. Consumer chain closed: VFM→VFX_VEG::load→
VFX_VEGInstance (0x48EF71 getter, 0x48F054 apply)→Holder. (3) 0x4F9A00: ZERO cells + zero e8/e9
image-wide = statically unreferenced dead variant (0x4F9960 is the live one); limit stated.
(4) UPGRADE to R-VFM §4: and al,0xDC is the ENCODER (neg/sbb/and → 0x00/0xDC), engine PRODUCES
the spec's bool wire byte — conformance claim strengthened. Buckets: full named; 2 FINAL-unknowns
printed (value-cache dword roles; 0x28-row element map). Successor proposed: R-VEG-E (value-cache
semantics + row anatomy + 0x500F90 emit family). Deliverable LEFT DIRTY; read-only honored.
