---
status: in-progress
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
- key count / delta vs stale (5,389 era, eea66da)
- docs newly indexed since staleness
- refusal-class movement
- anything the index surfaces about today's correction chains (multi-doc EAs)
