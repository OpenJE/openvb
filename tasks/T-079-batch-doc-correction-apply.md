---
status: in-progress
assignee: JadeBear `01a1639b`-peer (pulled 2026-10-22 ~10:4x from the coordinator-maintained T-079 row; seat's coordinator was dark at claim — relay sent, no contest; predecessor applies: T-078 dispositions + item 48 landed by coordinator at fb3d03f)
type: docs-apply ONLY (zero IDB writes, zero src/, zero prefix boot; annotate-never-rewrite law)
depends: T-072 deliverable (the per-file errata LIST — discoveries/2025-10-21-t072-doc-errata-sweep.md), T-081 §5 re-key table (paste-ready), T-082 §6 corpus rule, T-057/T-064 composition proof for I1
---

# T-079 — BATCH CORRECTION APPLY (docs-errata sweep's mechanical pass, finished)

## Scope (README row's remaining "1.5 of ~6")
1. **Frame-0-leap three-way reconciliation appendix** — SimDigest.md / OrigCapture.md /
   ParityOracleDesign.md (+ WorldTickPump wording if the appendix needs the other magnitude;
   T-056 R2 0->1s carry). One reconciliation block, same text, cited to T-045/T-056 bytes
   (`0x56af50 = a1 b8 c2 70 00 c3` session-elapsed; `+0x4C` seeded store `0x5466bd`).
2. **I1 interface-stub relabels (8 files)** — CGameInterface, COptionsGameInterface,
   CPipBoyInterface, CPipBoyAddNoteInterface, CGameFloatingTextInterface, GameDialogueInterface,
   GameCSMenuInterface, GameCSSkillMenuInterface: annotate the "## Inheritance" mermaid blocks
   per T-072 §9.3 (port-model label over COMPOSITION proof — embeds at +DISP, T-057 33 sites,
   single COL 0x6D6510 offset-0).
3. **DEFERRED-NOTES.md** — D1–D4 + D7–D9 clock/vft reconciliation appendix; D5/D6/D10/D12/D13
   per-section annotations; D11 one-line pointer to harness item 40 (the old "persist in docs"
   rule is retired — file the harness bug instead). P-CMBT's 2 doc items fold here per PARK.
4. **Per-file §4 correction blocks** — GUILoadingRecon, EntityActionQueueSeam, CombatTurnDispatch,
   ScriptNativeActionDoors, StartupAndSubsystemTable, GUIStringTags, GamePathManager,
   WorldYearSeedConsumer, T013-registry-port-design, AttackPositionOverrideDoors,
   GITMItemRecordLayout — each APPENDs its §4/§5 block at file end, stale sentence stays visible.
5. **Clean-pending DONE marks + dead-pending flips** (T-041/T-043/T-044 lines etc. per §2 table).

## Laws honored
- **Verify-not-trust at APPLY time**: every byte claim being pasted gets re-checked against
  `$OPENVB_ORIG_EXE` (PE-parsed image base 0x400000) or the frozen raw dumps in the sweep §3
  before it lands — SwiftMoon's list is evidence-backed, not waived (tasks/README §5). Catches
  during apply get named in the ledger, including against the sweep.
- **Corpus rule (T-082 §6)**: cite RAW table dwords, never F3.json export slot-maps.
- **annotate, never rewrite** (board state rule); correction blocks carry the refuting EAs.
- **Slot laws**: no IDB writes (read-only image/offline tools only), no src/, no prefix — zero
  collision with WildKnight's runtime row + R-USERRE-B.
- README row flip is coordinator-only: RELAY the progress/final state; task file here is the
  working ledger.

## Ledger
- [x] claim filed (this file), reserves taken, batch order set
- [ ] batch 1 (frame-0-leap reconciliation appendix)
- [ ] batch 2 (I1 ×8 relabels)
- [ ] batch 3 (DEFERRED-NOTES appendix + annotations + P-CMBT 2 items)
- [ ] batch 4 (§4 per-file correction blocks)
- [ ] batch 5 (pending flips / DONE marks)
- [ ] close: coverage statement + relay

## Cliff bank
Resume at the first unchecked batch; per-batch commit with the verification receipts.
