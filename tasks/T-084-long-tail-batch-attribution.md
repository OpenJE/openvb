---
status: in-progress
assignee: EpicRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: offline analysis (tools/ecx-flow.py scale-out vs md5-pinned cache; NO IDB writes; queue to coordinator)
depends: T-063 (named the residue: '1,351 long-tail members of known-shaped classes — a workload number, not an ignorance number'). This row PAYS OFF that workload number.
---

# T-084 — Long-tail batch attribution: families-mode across the ~1,351 'known-shape' anonymous fns

## Objective
T-063 closed the census by naming the unknown at 17 — those are now resolved (T-073/T-075). What
REMAINS unattributed are ~1,351 long-tail fns inside already-named families: each carries a key but
no host+field attribution like the 273 got. This is the scale-out: produce per-batch MINT/ANNOTATE
queues until the cls_-anonymous count has a name behind every row that CAN have one.

## Steps
1. Enumerate current live cls_ bands by size (post T-055/064/073/075 renames; the postdump series is
   your baseline) — batches of ~100-200 fns, largest families first.
2. families/band/share-check per batch, T4/T5 gates first; verdicts only at tool strength; batch
   reports per the T-057 shape (LIVE/DELEG/PAD/ORPHAN, three queries per negative).
3. Queue MINT-READY and ANNOTATE material to the coordinator in writer-wave shape (terminal-field
   rule honored — the fixed machine, not the old one).
4. Stop-and-state per batch honestly: long-tail members whose host cannot be named under current
   evidence get FINAL-anonymous treatment — the row's end-state is 'the number has a name or a proof',
   not zero-by-force.

## Acceptance
Per-batch discoveries docs (DIRTY), running tally of cls_-anonymous (start ~1,351±), and a closing
statement when the workload number itself is exhausted. Writer waves land between batches (queue
discipline unchanged).
