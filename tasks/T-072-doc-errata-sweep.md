---
status: in-progress
assignee: SwiftMoon `01a16092` (dispatched by LoudDragon `01a15f82`, 2026-10-21 — onboarding row: corpus-wide re-read with net effect of catching stale claims)
type: research (docs-only; READ-ONLY everywhere; deliverable = an errata LIST, changes nothing)
depends: the 2026-10-21 finding wave (T-007/T-013/T-033/T-055..T-070 discoveries — the yardstick)
---

# T-072 — Errata sweep: docs/f3/* + docs/DEFERRED-NOTES.md vs the accepted findings corpus

## Objective
Today 21 rows landed; three stale doc passages were caught ad hoc (CombatTurnDispatch x2, GUILoadingRecon
§5 + §11 via coordinator appendices). Hypothesis: more obsolete claims sit in the docs corpus silently.
Systematically audit every doc claim against the ACCEPTED discoveries, and produce a per-file errata list
so the coordinator can batch-fix with provenance (annotate, never rewrite — the board's state rule).

## Method
1. Inventory: `docs/f3/*.md` + `docs/DEFERRED-NOTES.md` (+ any doc a discovery cites).
2. For each claim touching: class/vft/member/offset facts RE'd today (Fragment size 0x158; CEngineInterface
   composition + COL proof; cursor=CSound; stem-keyed registry + TWO-KEY-POLICIES; +0x319 raiser shipped-live
   vs consumer-only; GameActionIdle resident + HOL; kind-numbering table; 0x707CE4/0x707CE8/0x707DA8 identities;
   StringTree/band-split family; attributed-foreign census incl. 17 orphans) OR 'unknown/pending/blocked'
   labels whose rows since CLOSED — quote EXACTLY and mark: STILL-TRUE / SUPERSEDED-by-<row+commit> /
   WRONG (with the EA evidence that refutes).
3. Spot-check refuted claims against RAW BYTES at the cited EAs — never trust a doc quoting a doc
   (item 43 family). Items 40-47 are your method law.
4. Do NOT edit any doc. Deliverable only: the list + a suggested fix-shape per hit (append vs annotate
   vs correction-block), so checkpoint-time edits are mechanical.

## Acceptance
`discoveries/2025-10-21-t072-doc-errata-sweep.md` (LEFT DIRTY; researchers never commit discoveries):
full coverage table (every doc file x verdict, hits itemized), zero speculative entries (every WRONG
carries an EA or a row-cite), and a coverage statement ('N docs read, M claims checked'). Ping
[DISCOVERY-BATCH ready].
