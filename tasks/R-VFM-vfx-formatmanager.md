---
status: in-progress
assignee: SwiftMoon `01a16092` (PARK pull R-VFM per lane contract, dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: research (READ-ONLY; offline-capable — image + artifacts/F3.json + live-name mirrors; DIRTY-until-checkpoint)
depends: T-073 §2 (the anchor chain — my yardstick/self-test), T-040 size rule, T-083 kit (self-test-first vft census)
---

# R-VFM — VFX_FormatManager anatomy: the vft census T-073 left open + .veg spec conformance + the size erratum

## Objective
T-073 anchored the epoch (singleton BSS 0x70811C, vft 0x67396C, load/parse mislabeled dtor
`cls_0x4fd780`, getter's sole caller VFX_VEGInstance). What it did NOT do: enumerate the vft's
ACTUAL length/slots (vftable-slots.tsv carries ONE entry for 0x67396C — idx 0 → 0x4FD660, which
also disagrees with T-073's dtor-at-0x4FD280 store story — reconcile), name the load/parse method
chain end-to-end, judge the parser's grammar against `van-buren-docs/File-Format-Specifications.md`
(.veg/VEG sections), and put the SIZE question on record (export 44 vs live ≥ 0x38 by T-073's own
"+0x34 stores" observation — measure the true span raw).

## Steps (read-only; self-test-first per T-083 discipline)
1. SELF-TEST (gate): reproduce T-073's six anchored facts from raw bytes — vft stores @0x4FD29D and
   @0x4FD7AD (both = 0x67396C), static-init thunk 0x641980 `mov $0x70811c,%ecx; call 0x4fd780`,
   atexit 0x645E60 → 0x4FD280, getter 0x4FB040 with its single caller 0x48ee30-region, and the
   0x114-stride VFX_ParameterInfo element math. Gate passes only if my dumper = T-073's dumper.
2. VFT CENSUS: raw-walk 0x67396C like T-083 §1 (length measured at first non-code cell / next
   table; COL/TD RTTI check item-42; per-slot bucket D/O/U with body EAs); reconcile idx0
   0x4FD660 (tsv) vs dtor 0x4FD280 (T-073) — is the table's slot 0 the dtor and 0x4FD660 a
   different slot? vftable-slots.tsv idx semantics from tools/ea-index/T-032 headers.
3. LOAD/PARSE CHAIN: 0x4FD780 body — stream open 0x52FB10, parser 0x4FCD00 (+ helpers), token
   set, the signature/`Loop`/float/bool grammar; map which VFM fields the parse fills
   (+0x04 vector<FormatEntry>, +0x18/+0x28 vector<VFX_ParameterInfo>).
4. .VEG CONFORMANCE: diff observed grammar vs van-buren File-Format-Specifications VEG/.veg
   sections; name every conformance gap BOTH ways (spec says X, bytes do Y).
5. SIZE ERRATUM: max [this+disp] store across the whole family (ctor/dtor/parse) → true object
   span; report export(44) vs live(N) as separate rows per T-040; also RTTI-adjacent sizes.
6. Deliverable `discoveries/2025-10-21-r-vfm-vfx-formatmanager.md` LEFT DIRTY; successor line
   per lane contract 3 (or DONE-NO-SUCCESSOR).

## Acceptance
120-style per-slot table for the ACTUAL vft length; six self-tests printed; ≥1 numbered
conformance verdict per spec section touched; size row "export 44 / live ≥0xNN (evidence:
store @EA writes [this+0xNN])"; zero writes outside my two files.

## References
T-073 §2a-c; `discoveries/vftable-slots.tsv` L2213; `van-buren-docs/File-Format-Specifications.md`
(VEG); T-083 method (`discoveries/2025-10-21-t083-gfx-engine-vft-census.md`); harness items 40–49.

## Notes (SwiftMoon, research-complete 2026-10-21)
Gate 6/6 (T-073's six anchors re-read raw) before any classification. HEADLINE: the vft is
exactly ONE slot — 0x4FD660 = scalar deleting dtor (single-virtual class; everything else is
direct-call, so the anatomy is member/chain-based, not dispatch-based); export length-1 AGREES
(rare no-erratum census). Size erratum landed: named export 44 is 12 bytes short — load zeroes
through [this+0x34] @0x4FD7E0 ⇒ span ≥0x38=56, and the export's OWN sibling cls_0x4fd780 says
56 (self-contradiction across its split identities; reported floor-vs-truth per T-040).
Chain: CRT static-init 0x641980 → load 0x4FD780 (re-arms vft — T-073's 'dtor-is-load' verified at
0x4FD7AD) → 1910-bounded schema-string array @0x6FCA18 → 0x52FA30 (14-entry name walk @0x6FE82C)
→ registrar 0x4FCD00 (43 builtin name/type/default rows @0x6FC968; SSO assign; 'false'/'0'
defaults at 0x4FF1B0 bool/float branches; VFX_ParameterInfo ctor 0x4FB2D0; grow 0x4FD680 ÷20).
.veg conformance 5-for + 1 FINAL-unknown (0x4F9A00 caller-set): engine VALIDATES 'VEG '+'V1.1'
(0x50136D/0x501383, split 4-byte tokens; spec's own C# only SKIPS it — engine stricter);
'VFX '+'V1.0'+8-skip+count conforms (0x4F9994/0x4F99B2); FULL §3.2 type vocabulary in .rdata
(float/bool/VFX_Byte/enum/std::string/Vector/Vector2/Color/Rotation/Resource/TextureUVs) and the
0xDC bool-true encoding is LIVE — `and al,0xDC` @0x500C5F. Cross-row bonus: a THIRD consumer of
the pooled empty string @0x64D7D8 (T-078 disposition re-vindicated at a new site). Self-caught
during drafting (counts vs table, 1910-vs-14 loop bound, '0 0 0' as schema-data not call-arg).
Deliverable LEFT DIRTY; successor proposed: R-VFM-2 (VFX_ParameterInfo 273B field anatomy +
0x501xxx applicator-band ownership) — PARK it or fold into R-CS's audio/cursor era.
