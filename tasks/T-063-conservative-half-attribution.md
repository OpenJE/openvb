---
status: open
assignee: EpicRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21)
type: offline analysis (tools/ecx-flow.py vs the md5-pinned cache; NO IDB writes; queue to coordinator)
depends: T-054/T-057 (the tool + families mode); T-057 covered the 273 *escalation* fns — THIS is the other bucket
---

# T-063 — families-mode attribution of the batch-4 "conservative half" (321 anonymous fns)

## Objective
T-033 closed with "321 fns stay anonymous (conservative half)" — classes that failed the t036 density
gate OR were the conservative half of an escalation. T-057 handled the three REGISTERED escalation bands
(273 fns: 50dbb0/50db20/4542d0). This row is the REMAINING ~321-fn anonymous set: run the four joins +
families mode and produce a host-attribution mint queue, the same shape as T-053->T-055.

## Steps
1. Enumerate the target set precisely from the b4/b6 ledgers' "conservative half" wording + live IDB
   `cls_0x…::` census minus the T-057 273 (no double-counting — state the disjoint set + its exact size).
2. `ecx-flow.py families`/`band --cls` per candidate: LIVE / DELEG / PAD / ORPHAN with item-41's three
   queries per negative, head_hint() grades per item 43, host+DISP per entry.
3. Classify outcome: (a) MINT-READY (named engine host found — the 445350 win-shape), (b) ATTRIBUTED-FOREIGN
   (another std/_Tree/allocator family), (c) vendor (D3DX/DShow/CRT tail), (d) still-ORPHAN (legitimately
   unknown after all three queries).
4. Queue (a) candidates to the coordinator as a writer-wave handoff (writer slot: CalmRaven first-in).

## Acceptance
`discoveries/` file, gate-first (selftest 5/5 @ pinned md5 stated before any verdict): the disjoint
conservative-half set (counted), per-family verdict, MINT-READY queue, and a FINAL decomposition of the
remaining T2 mass so the census can say "T2 = vendor + attributed-foreign + conservative-attributed +
true-orphans(N)" with every bucket named. This is the census-completion row: after it, "how much is
still genuinely unknown" has a number, not a shrug.
