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
- [x] regenerate default-mode index (worktree truth: dirty-but-committable docs included by design -
      the TSV reflects bytes, and a later commit of the same bytes cannot desync --check)
- [x] tools/ea-index.py --check -> exit 0
- [x] refused-token count compared against the stale header (drift = new citation *shapes*, not new docs)
- [x] observations written below; no doc content touched, no IDA, no build

## Observations (fill during run)

### Refresher 2026-10-23 (claimed as PARK standing row by peer-loop session, 'any idle hand')
- **Pre-state:** --check exit 2, `STALE: corpus now says (corpus_docs=202 indexed_eas=8061) but checked-in
  index does not`. Checked-in header was corpus_docs=200 / 7,913 EAs / 12,873 mentions / 1,061 refused.
- **Post-state:** regenerated -> corpus_docs=202 / 8,061 EAs / 13,151 mentions / 1,069 refused; --check exit 0;
  `--verify` recall spot-check 60 EAs x 202 docs = 12,120 (EA,doc) pairs, 0 missed.
- **Drift source (which docs, the useful bit):** exactly two additions since the index's last commit -
  `2025-10-22-entity-band-vbptr-roundtrip.md` (12a3a91) and `2025-10-22-server-packet-dispatch-idmap.md`
  (b0998fc). Both were landed AFTER the 81a07c8 refresh commit. Knowns-and-refusals moved accordingly:
  inference-only keys 87 -> 89; refused 1,061 -> 1,069 with the delta almost entirely 'vftable slot offset
  (relative to an unresolved table)' 665 -> 672 (+7, the +0xNN-slot-on-its-own-line shape, by-design refusal)
  and 'outside image span' 241 -> 242 (+1). All other refusal classes FLAT — no new citation *shapes* this
  window. The id-map doc citing page-offsets/echo-ids refused as outside-span as expected.
- Process note: ran --refused BEFORE regenerating to snapshot the stale header's classes, then after, so the
  delta is measured not eyeballed. Index regenerated with the two dirty-but-committed-since docs included by
  the TSV-reflects-bytes rule; commit below may include them.
- Payoff sample: `--query 0x401100` count moved 2 -> 3 (now also cites the id-map doc). Standing rule re-set
  in motion: --check green BEFORE any attribution claim, refresh is a standing 'any idle hand' row.

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

## 2026-10-24 refresh (executor seat, this window)
- Trigger: `tools/ea-index.py --check` STALE (corpus 211 docs / 8,106 EAs vs index's 202 / 8,061
  from 9f2df73). Refresh: 211 docs / 8,106 EAs / 13,443 mentions / 1,093 refused tokens
  (1,061 -> 1,069: outside-span +8 — the heap-pointer captures class, correct to refuse per the
  pairing law). NO new refusal categories in kind.
- Drift sources (this window's landings): discoveries/2025-10-24-s3-inj-deep-input-readings.md,
  docs/f3/SimDigest.md qi= section, docs/f3/OrigCapture.md r5/qi rows, docs/f3/P-CMBT design doc
  era — the S3-INJ + R-PORT-Q + P-CMBT closure wave.
- `--verify` recall spot-check 60 EAs x 211 docs = 12,660 (EA,doc) pairs, 0 missed.
- --check green at commit: corpus_docs=211 indexed_eas=8106.
