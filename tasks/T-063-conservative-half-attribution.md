---
status: done
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

## ACCEPTANCE EVIDENCE (EpicRaven, 2026-10-21)

- Gate first: selftest 5/5 @ pinned md5 a68c22f3dd1c256ec9539a70e87cb434 (census via
  t060-postdump.json, no live 8745 use, no tool changes, no IDB writes).
- Enumerated set: N = 199 EXACT (b4 conservative-half classes x live cls_-anon census; overlap
  with T-057 trio's 244 still-anon = 0; 26 of b4's listing already hosted since: 445350 x8,
  4a5be0 x11, 558740 x7 -- stated as the good outcome per coordinator request).
- Per-fn buckets: 109 LIVE / 90 DELEG / 0 PAD / 0 ISO; head_hint: all 199 call-heads.
- Classification: (a) MINT-READY 15 queued -- cls_0x48fb60 band = G3D_Node+0xA0 member class
  (construction-tier: JE::G3D_Node::ctor_0x4d7600 site 0x4D7689; 10 fns) + cls_0x558740 residual
  5 = CDebugInterface extension (live-named callers + view-adjusted this-0xB0/C4; 0x434760
  weak-tier flagged); (b) ATTRIBUTED-FOREIGN 79 (_Tree five host-field-named 46 + helper epochs
  49cc40/409820/4b8f70 = 33, b4 adjacency readings corrected by flow); (c) VENDOR link-tail 59
  (flow direction: CRT-support epoch callers); carry-overs 29 (4b28a0 T-054 singleton verdict
  re-confirmed 19 LIVE all-&0x707E48 + 9 DELEG; 0x548D10 alias); (d) TRUE-ORPHANS K=17 named
  with item-41 triplets answered (cls_0x490150 x9, cls_0x4fc0e0 x8).
- FINAL DECOMPOSITION, exact, zero gaps: 2,042 live cls_-anon = 167 vendor + 404
  attributed-foreign + 59 link-tail + 29 carry-over + 15 mint-queued + 17 true-orphans +
  1,351 unregistered long-tail (workload not ignorance). "Genuinely unknown" in the
  registered+conservative scope = 17 fns, all three queries answered each.
- discoveries/2025-10-21-t063-conservative-half-attribution.md + README row queued for
  checkpoint; queue to writer slot only, nothing applied.
