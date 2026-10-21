---
status: done
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

## ACCEPTANCE EVIDENCE (EpicRaven, 2026-10-21)

Gates-first: selftest 6/6 @ a68c22f3… before any verdict; t064-postdump as live family source;
zero 8745/IDB use. Set recounted exact: **1,377 fns / 614 classes** (matches T-063 ~1,351 +
drift), disjoint from all covered families. Single-pass sweep: LIVE 984 / DELEG 222 / ISO 171;
share-check (terminal-field machine) PASS 1,132 / SHARING 82 / NO-EVIDENCE 163.
Queues (discoveries/2025-10-21-t084-long-tail-batch-attribution.md, DIRTY awaiting checkpoint):
- MINT-READY 323 fns grouped by host, tier-1 182 fns/22 hosts (Gfx_Engine+0x114 x5,
  Map epoch embeds x11, Gfx_ParticleSystem x9, GUI::Skin+4 x6, DialogueManager+8 x6,
  Gfx_LineSystem+0x108 x3, Client/World+0x108/0x118 members x9, Gregorian+0x10 x3, …),
  tier-2 141 incl. heap-ctor children + 9 weakest-tier rows explicitly marked
  read-back-before-mint (class-root unresolved callers).
- ANNOTATE-FOREIGN 82 per-fn SHARING concentrating in 5 helper epochs (cls_0x40cc80/401b00/
  4dff90/4abc60/515040 — last two pad-resolution-confirmed cleanup machinery, ties to T-059).
- FINAL-anonymous-with-proof 171 (142 vendor-epoch extension of b4 ruling + 29 linkage
  residue), item-41 triplets answered per fn.
- Honest remainder: 801 known-used / not-yet-known-owned — named, regenerable, and the
  designated first consumer of T-056 values-era cross-refs.
Throttle note per step 4: recommend T-073/075 wave lands first; T-084 tier-1 as next single
writer load; tier-2+annotations batched behind (queue cheap to hold here). Status relay:
T-084 in-progress→DONE for table mirror.
