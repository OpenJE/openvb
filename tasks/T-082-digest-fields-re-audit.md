---
status: in-progress
assignee: KeenStorm (dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: research + test-harness (no ad-hoc digest edits — findings queue to the writer/product lane)
depends: T-056/T-067/T-068/T-070/T-071/T-074/T-080 (all the shipped-side facts this row audits the port's assumptions against)
---

# T-082 — Digest-fields vs today's shipped facts: field-by-field honesty audit

Audit EVERY port digest/contract field (SimDigest fields, SimDigestContracts wordings, the stateHash
comparison set, D-2 count semantics, ActionKindName numbering, +0x314 count INCLUDING the resident,
state-9/1 activation stamp, +0x184) against the shipped facts discovered today. Checklist at minimum:
1. R3 count INCLUDES the never-finishing resident (T-070/T-071) — do any banked SHIPPED digest quotes
   or expectations predate that reading? Re-state each with source-run citation.
2. Damage is DEFERRED through the effect inbox (T-074: apply lands @0x55a501) — does our sim land
   damage inline? If yes: which compared INSTANTS differ between inline-land and inbox-land, and does
   any existing shipped-quote straddle them? Finding, not fix — queue the fix.
3. Message action = engine kind 7, finishes first tick (T-080) — contract comment audit.
4. T-067 PAIRING LAW: enumerate every stored expectation that bakes a HEAP ADDRESS (world/arena
   bases shifted under DXVK) and must re-key to constants or same-env-only.
5. kind NUMBERING: ours vs shipped 0-11 table (T-080 jump table) — one mapping comment per surface,
   verify T-071's mapping table matches the raw jump table exactly.

## Acceptance
discoveries/ file (DIRTY): per-field verdict AGREE / DRIFT / BENIGN with EAs + row cites; drifts
queued as writer/product rows, NOT applied blind; zero digest behavior changed by this row. Gates
only if a test-harness re-key lands (then full law).
