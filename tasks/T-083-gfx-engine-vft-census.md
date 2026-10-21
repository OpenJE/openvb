---
status: open
assignee: SwiftMoon (dispatched by LoudDragon `01a15f82`, 2026-10-21 — the anatomy twin of your errata sweep)
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
