---
status: in-progress
assignee: TrueCastle `01a16096-c00e-7404-8afe-e521a9bb7240` (coordinator wave-1 assignment, 2026-10-21 — first-in CalmRaven mesh-stuck ~5h; seat is wave-1 only, CalmRaven retains first-in for wave-2 if he surfaces)
type: writer wave (sole IDB writer; rename+annotation load; T-055 ruleset verbatim)
depends: T-075 queue (doc §8), T-073 queue (doc §8/§3), T-086 retro-sweep running IN PARALLEL — if it flags a band mid-wave, HOLD those rows, route to HappyStorm/EpicRaven arbitration
---

# T-087 — WRITER WAVE-1: GUI::Manager band (27 + dtor fix) + Gfx_RenderManager extension (9) + annotation tail

## Load (source-of-truth = the two checkpointed queue docs; this row only sequences)
1. 27 renames `cls_0x4b28a0::* -> JE::GUI::Manager::*` (EXCLUDE meth_0x619b38 CRT __getptd artifact) +
   live-name FIX `JE::StringTree::dtor_0x4b5280 -> JE::GUI::Manager::dtor_0x4b5280` (T-075 §8).
2. 9 renames cls_0x490150-family -> JE::Gfx_RenderManager band extension (+ ctor_0x4950D0 identity
   comment; composite A2 rows per T-073 §8 — its 4 merges T3->T1 apply as ordinary name moves).
3. Annotation tail: T-073's 8+2 VFM vector-clone rows + T-075's §-cites as needed — comments only,
   get_cmt readback per site.

## Rules (non-negotiable, from the wave template)
Sole writer from claim commit; re-verify-then-apply at write time (item-42 RTTI sizes, item-44
SINGLE-owner via the FIXED share-check, item-49 callee-side where relevant, item-43 head checks for
comment EAs) — author-verification does NOT waive write-time checks; per-fn ledger (source row, tier,
verdict, applied/label-and-skipped); T2 itemized BY SOURCE (Manager 27, RenderManager 9, merges,
annotations-zero); mid-wave checkpoint save + end-of-wave save_database + mtime + fresh-dump disk==live
(the standing rule; wavedumps/ name+EA tables are the recovery source); names re-read at EA before each
band (churn law); NO product-file touches.

## Acceptance
Ledger discoveries/2025-10-21-t087-wave1-ledger.md + row flips for T-073/T-075 queues' board state via
relay + census line: cls_-prefix total after (start 2,000; expect ~1,964 with the merges stated per
family). [DISCOVERY-BATCH ready] at wave close.
