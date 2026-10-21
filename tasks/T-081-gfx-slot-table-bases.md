---
status: in-progress
assignee: TrueCastle `01a16096-c00e-7404-8afe-e521a9bb7240` (filed by LoudDragon `01a15f82` in 36db5ce, carried over from SwiftMoon's T-072 sweep F1 flag, 2026-10-21)
type: research (READ-ONLY on ida_original_f3 — xref-weight queries only; NO IDB writes; discoveries file LEFT DIRTY until coordinator checkpoint)
depends: T-072 (F1/F2 flags — SwiftMoon's doc-errata sweep), T-007 (GUILoadingRecon G1/G2 — the slot-numbering style at issue), T-062 (Window vft 0x66E4D4 slot-12 claim + ten-table §8a list), DEFERRED-NOTES §8a (the "settled" wording being re-opened), harness items 40-49 (method law; 42 size rule, 49 pairing law)
---

# T-081 — F1: Gfx_Engine slot-table BASE disambiguation (Window primary/secondary family + ten-table §8a run + F3.json flattening)

## Objective
Slot citations like "`+0xD8`(216)=`0x5c0930`" are table-RELATIVE: they only mean something
once the reader knows WHICH base the displacement indexes. Around `0x66E4D4` (live: GUI::Window
75-slot vft) and `0x66E574` (secondary view?) sits a contested .rdata run; §8a lists TEN tables
(0x655824, 0x65d224, 0x66ca84, 0x66d504, 0x66e59c, 0x670d74, 0x697f14, 0x69dd8c, 0x69ec94,
0x69ef6c) that share a base+8 pattern; the Gfx_Engine singleton (0x707CE4) dispatches through
0x6AE7EC (+ the "+C manager view" cited by T-072). T-072 F1 flagged: the docs never state per-slot
table bases, and the sweep found Gfx-side table-base ambiguity UNVERIFIED. This row closes it:
full raw dump of the contested .rdata run + COL/TD chain for EVERY table base (item-42 size rule
per name), item-49 callee-side scan wherever a store into an object slot is register-indirect,
and a per-citation table-base key so "+0xD8(216)" resolves mechanically. Terminal answer per
table = OWNER (class + tier) or FINAL-ambiguous-with-reasons.

## Attack plan (in order)
1. Read the contested-wording sources: DEFERRED-NOTES §8a, GUILoadingRecon (t007) §7-G1/§8e slot
   citations, T-062 §Window slot-12, vftable-master-index rows around 0x66E4xx-0x66E5xx, and the
   F2/F1 raw readings SwiftMoon already banked.
2. RAW DUMP the full .rdata run(s): 0x66E4A0..0x66E640 (Window primary+secondary neighborhood),
   plus each of the ten §8a tables +/- 32B, plus 0x6AE7EC +/- 32B (Gfx_Engine primary) and its
   "+C manager view" if one exists. Classify every dword: code-target / COL-like / string / float /
   terminator. Tables are shoulder-to-shoulder (T-075 lesson: the "slot" that looks like a pointer
   into 0x6d3xxx is the NEXT table's [-4]).
3. For each table base: read [base-4] as COL candidate → TD → name; apply item-42 (size from export
   RTTI record + CHD/MTD geometry: CDOffset, maxNodes, MTD self-reference) to decide owner-vs-view;
   item-44 share test where a name repeats across bases.
4. Item-49 callee-side scan: for the contested slot READS ([[0x707CE4]+0xC4/D8/DC] sites at
   0x4b21bb/0x4b221d/0x5236b4 etc. — G2 already pinned bytes), verify the disp indexes 0x6AE7EC
   (engine primary, [0x707CE4]=engine this) — and find which code indexes the Window-family bases
   (who loads [[0x707E48-side]… no: window objects are heap; census `ff 90/91 + disp` sites whose
   ecx is a window this) to confirm secondary-view slot numbering (T-075 §1 sibling-table trap).
5. F3.json flattening: cross-check the export's vftable rows (master-index 1-slot rows like
   GUI::Window 0x66E4CC) against raw — decide which rows are REAL tables vs export artifacts of
   shoulder-to-shoulder packing.
6. Deliverable: per-table table (base | span | COL/TD | demangled name | size | owner/view/ambiguous)
   + a table-base KEY appended for every "+0xNN" citation style found in the banked docs (list only —
   no doc edits from this lane), so the checkpoint correction block can paste it.

## Acceptance (mechanical)
- discoveries/2025-10-21-t081-gfx-slot-table-bases.md (LEFT DIRTY) with: full raw dump analysis,
  per-table verdict OWNER (name + item-42 evidence) or FINAL-ambiguous-with-reasons, the slot-citation
  key resolving F1, an explicit statement of whether F2's base-mis-anchor pattern repeats in §7-G1/
  G2 citations (byte-checked), coverage bounds per item 35 rule 2, selftests (ecx-flow 5/5 + the
  COL->TD geometry known-positive 0x66f2cc/GUI::Manager from T-075).
- Zero IDB writes; no doc edits; coordinator checkpoint on [DISCOVERY-BATCH ready].

## Rules
- Read items 40-49; selftest gates BEFORE verdicts; byte-over-decompiler; EA-anchored everything.
- Lanes: KeenStorm (product port), HappyEagle (DXVK :96), writer-wave referee role PARKED pending
  CalmRaven reroute — this research row does not touch the IDB regardless.
