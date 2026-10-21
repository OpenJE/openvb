---
status: in-progress
assignee: SwiftMoon `01a16092` (dispatched by LoudDragon `01a15f82`, 2026-10-21 — the anatomy twin of your errata sweep)
type: research (read-only on ida_original_f3, offline-capable; discoveries DIRTY-until-checkpoint)
depends: T-007 G2 (slots 49/54/55 texture delegation), T-061/062 (cursor chain), b5/0x5c0c30 Gfx_D3D-create, T-045 (slot 6 tick), T-057 (slot-manager 0x50b560)
---

# T-083 — Gfx_Engine vft 0x6ae7ec: classify all 120 slots (the day touched ~six)

## Scope
Per-slot table for vft 0x6ae7ec (byte offset, slot idx, target EA, one-line classification or
FINAL-unknown-with-reasons) for the FULL 120-slot length. Self-test FIRST on the six slots the board
already owns — 49/54/55 (texture delegation to 0x70B200), the tick slot per T-045, the Gfx_D3D-create
leg (0x5c0c30), the slot-manager anchor (0x50b560 @+0x18 per T-062 addendum) — your walker must
reproduce those before any unknown classification counts.

## Kit (all day's law, you already speak it)
RTTI COL->TD + size rule (42), callee-side imm-vft scans for register-indirect stores (49),
share-check before ANY holder/membership claim (44), F3.json flattened-list distrust + size erratum
rule (T-040/T-007/T-055), function-granularity (43), magnet-address first-byte check (48).

## Acceptance
discoveries/ file (DIRTY): 120-row table + counts per bucket (delegation-identified / owner-named /
FINAL-unknown), classification evidence EAs, and a 'top 5 most interesting unnamed slots' shortlist
for a possible follow-on. This is anatomy the census never did: 'who calls Gfx_Engine' is attributed;
'what ARE its slots' is not.

## Notes (SwiftMoon, research-complete 2026-10-21)
Self-test gate PASSED 6/6 + World-table ground-checks BEFORE any unknown classified (doc §0).
Raw truth: 0x6ae7ec = 120 slots, all distinct real bodies (no stubs/thunks), ends at idx 119
(idx 120 is literal text); RTTI COL 0x6D79F4 -> TD 0x7001F0 `.?AVGfx_Engine@@`, primary (COL+0=0);
install census BOTH directions (item 49): 2 stores, both `c7 06` reg-indirect — ctor 0x5BFE45 +
dtor-RESTORE 0x5C1CF1 (inside dtor 0x5C1CD0) — no second ctor.
EXPORT ERRATUM CANDIDATES (T-040's list): (1) F3.json vft declares length 120 but emitted 119
entries — HOLE at idx 100 whose raw body exists (0x5C0350 = byte getter [this+0x68]); (2) size 2212
still non-authoritative (T-040 rule re-applied, max member disp 0x894 is consistent but not proof).
Buckets (mechanically verified to match the table): 68 D / 45 O / 7 U — every U prints its reason.
Subsystem map resolved the facade: +0x18 device wrapper (cls_0x4c0190, installed ONLY by slot 8
CreateDevice @0x5C0D19), +0x2C TexMgr, +0x30 buffer mgr ([0x707CE0] self-cache), +0x38 stats mgr
([0x70C96C] self-cache), +0x3C slot mgr cls_0x50B560 (T-062 addendum "+0x18" = the manager's OWN
tree field, not an engine offset — disambiguated); the "engine globals" 0x70837C..0x70839C /
0x708300..0x708328 are FIELDS OF THE STATIC Gfx_D3D AT 0x7082F8 (zero absolute stores from outside
the band — b6's value-pair finding generalized).
Top-5 unnamed: slot 9 (CreateDevice's twin teardown), slot 35 (device-lost recovery AND a surfaced
consumer of cls_0x490150 = one of T-063's 17 orphans — routed to T-073's lane), slot 118
(per-frame device flush), slots 11/12 (mode-apply THROUGH FindMode 0x502870 = T-052's wall fn),
slots 101/102 (private texture ids @+0xD4/+0xD8 via self-slot-54).
Deliverable LEFT DIRTY. Image+JSON only; zero IDB; zero source/doc writes besides this file.
