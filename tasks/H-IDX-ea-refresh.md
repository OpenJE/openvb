---
status: done (index refreshed 147->189 docs, --check green, observations below)
assignee: KeenStorm (owner standing order, re-invited; PARK row H-IDX, "any idle hand" - taken)
type: mechanical tooling (tools/ea-index.py refresh + --check gate; observations logged in-row)
---

# H-IDX — EA index refresh against the current corpus

Twenty-minute row per its own description: regenerate `artifacts/ea-index.tsv` from the current
`discoveries/*.md`, bring `--check` to green, and log any drift observed during the refresh.
Boundary of the tool is quoted where it matters: the index records WHERE an EA is mentioned, not
whether the mention is current - supersession judgment stays with the canonical fact sheet.

## Actions
- [ ] regenerate default-mode index (worktree truth: dirty-but-committable docs included by design -
      the TSV reflects bytes, and a later commit of the same bytes cannot desync --check)
- [ ] tools/ea-index.py --check -> exit 0
- [ ] refused-token count compared against the stale header (drift = new citation *shapes*, not new docs)
- [ ] observations written below; no doc content touched, no IDA, no build

## Observations (fill during run)
- **Scale of staleness:** the estimate said "~15 docs"; the actual gap was **42 docs and 2,260
  keys** (147->189 docs; 5,389->7,649 EAs; 8,097->12,209 mentions). The eea66da-era index predated
  the entire cursor/GUI/action arc of this shift. Refresh took 2 seconds.
- **PAYOFF OBSERVATION (the real find):** `--query 0x5a9070` returns FIVE citing documents —
  including `2025-10-20-vftable-master-index.md` (session 36), which enumerated the FULL 7-slot
  raw walk of GameActionIdle 0x6497c4, ActionIdle 0x695afc, AND ActionMessage 0x6a5f34, showing
  idx5 (+0x14) = 0x5a9070 in the shared family pool. **The gen-3 attribution question that this
  evening required three independent raw re-dumps to adjudicate (mine, HappyEagle's, WildZenith's)
  had been sitting QUERYABLE in the corpus since yesterday.** The raw-first discipline never hurt
  — the master-index walk is what raw produces — but the cheap first move for any NEW attribution
  claim is `tools/ea-index.py --query <EA>` BEFORE a fresh dump, and the board's doc-writing habit
  (EAs inline in table rows) is what made it answerable. Proposed harness/culture line: "query
  before you dump; dump to confirm or to answer what the index can't."
- **Refusal classes moved as expected and are honest:** 785 -> 1,037. Dominant growth:
  'vftable slot offset relative to unresolved table' 572 -> 656 (today's raw dumps cite +0xNN
  slot indices on lines without the table address - by-design refusal; docs that want those
  queryable put table address + slot offset in one line, as session-36's master index does).
  'outside image span' 120 -> 233 (+113: the boot-era heap pointers - 0x0A...... entity addrs
  from T-056/T-067/T-080 captures - correct to refuse: same-env addresses are not cross-doc
  keys per the pairing law; prose provenance, not index territory). masked/pattern 71 -> 102.
- **inferred-keys ledger unchanged in kind** (53 listed via --inferred; compressed prose like
  slash-lists is flagged, never silently counted).
- No doc content touched; --check green at commit: corpus_docs=189 indexed_eas=7649.
