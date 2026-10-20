---
status: open
assignee: unassigned (offline-only; safe for any lane, no IDB)
depends: nothing
---

# T-032 — EA → document index: make what we already know addressable

## Why
`discoveries/` is **130+ documents citing ~5,100 distinct EAs**, and there is no way to ask
"what do we already say about `0x45C140`?" without grepping prose. That is a real, measured cost:
item 15 (seven files with role labels written by the wrong lane) existed because attribution was done by
reading prose headers, and lanes demonstrably re-derive things that are already answered in a file nobody
can find. Every research minute spent re-answering a solved question is the cheapest waste on this board.

## What
`tools/ea-index.py` — pure offline, no IDA, no build:
1. Walk `discoveries/*.md`, extract every EA (`0x[0-9A-Fa-f]{5,6}` and `sub_[0-9A-Fa-f]+`, `cls_0x…`,
   vftable slots), mapping **EA → set of documents**, with the document's own title line as the summary.
2. Emit `artifacts/ea-index.tsv` (`ea <TAB> count <TAB> docs`) plus a `--query 0x45C140` mode that prints
   the citing files and the first line of each citing section heading.
3. Regenerate on demand, never by hand. The generated file is checked in so a diff shows corpus growth.

## Acceptance (falsification, per rule 2)
- `--query` on a known EA (e.g. `0x413310`) returns the damage-calculation documents.
- **A query for an EA that appears nowhere must print `NO RECORD`, not empty-with-zero-exit.** Show it.
- **A document with an unparseable EA is reported, not skipped silently** — the tool counts what it
  refused to read and prints that number.

## Boundary (print it in the tool's own header)
The index records **where an EA is mentioned**, not whether the mention is correct or still current;
superseded documents stay in the index and must be judged by the reader. It covers `discoveries/` only —
not `tasks/`, not code comments, not the IDB. Two EAs one character apart are different keys and the tool
does not know they are related.
