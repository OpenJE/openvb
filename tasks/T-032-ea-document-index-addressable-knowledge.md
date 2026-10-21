---
status: done
verdict: tools/ea-index.py + artifacts/ea-index.tsv — 137 docs, 5,258 EAs, 7,778 mentions,
         720,346-pair recall audit with 0 silent misses, 756 tokens refused-and-counted.
assignee: SwiftRaven (offline-only)
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

## Verdict (SwiftRaven, closed)

Built and recorded in [`discoveries/2025-10-20-t032-ea-document-index.md`](../discoveries/2025-10-20-t032-ea-document-index.md).

* `tools/ea-index.py` → `artifacts/ea-index.tsv` (checked in; `--check` exits 2 when stale).
* Acceptance met and demonstrated: `--query 0x413310` → 7 docs led by the damage-formula file; an absent EA prints
  **`NO RECORD` and exits 1**; refusals are **counted**, never dropped silently (`--refused` dumps them with file:line).
* **Full recall audit: 5,258 EAs × 137 docs = 720,346 pairs, 0 silent misses** (`--verify --full`, exits 3 on any miss).
* The index was worth building mainly for the four ways it was initially **wrong**, all now fixed and documented:
  (1) `0x` cannot match `CalculateDamage_0x413310` — the corpus's dominant form; the first version found 1 of 4 citing
  docs. (2) case-splitting put 318 addresses in two keys (`0x54BE10` vs `0x54be10`). (3) a "pure digits ⇒ decimal" guard
  destroyed real suffix addresses (`QueueToCombat_463140`), so it now applies only to bare prose tokens and is lifted
  inside fenced blocks where `470377:` is a disassembly address. (4) my own *audit* had the bug first — substring
  matching found `0x400000` inside the flag constant `0x40000000` and reported a phantom miss.
* **51 keys (1.0%) are inference-only** — assembled from compressed notation like `@0x45c251/261/271`. Listed by
  `--inferred` and counted in the TSV header, so nobody mistakes them for literally-written addresses.
* Deliberately refused, with counts: 563 `vft+0x…` slot offsets (relative to a table the tool cannot resolve), 111
  out-of-image hex (FourCCs, file offsets, masks), 63 masked forms (`0x401xxx`), 5 **ambiguous `+0x…`** — the corpus
  uses `+0x` for both member offsets and globals, and merging them would fabricate relationships.
* Shown to fail: sabotaging `SUFFIX_RE` into silent no-match made `--verify --full` report 73 missed pairs in 43 docs
  and exit 3. `--check` originally regenerated before checking (so it always said "current") — that bug shipped and was caught by planting an unindexed doc.

## Boundary (print it in the tool's own header)
The index records **where an EA is mentioned**, not whether the mention is correct or still current;
superseded documents stay in the index and must be judged by the reader. It covers `discoveries/` only —
not `tasks/`, not code comments, not the IDB. Two EAs one character apart are different keys and the tool
does not know they are related.
