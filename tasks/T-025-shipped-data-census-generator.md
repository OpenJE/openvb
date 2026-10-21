---
status: open
assignee: UNCLAIMED (01a157ef departed 2026-10-21; deliberately NOT re-assigned by chat offer —
# a row moves only by a coordinator edit + dispatch, or it is not attributable)
depends: nothing
---

# T-025 — Census every shipped data record into greppable tables

## Why
The 10d lesson: **before asking whether a bug is observable, ask whether on-disk data can answer it.**
That question settled T-019 at 0/57 in one sitting, and closed 10c from `char_desc` alone. Generalise
the method so "does shipped data ever do X" stops needing a debugger and stops needing me to dispatch.

## Scope
1. Extend the existing v5 GWAM walk into a **census generator** emitting stable tables (csv or md) for:
   all v5 GWAM records with the B11-corrected operand mapping; `char_desc` body-part modifier rows;
   item/equip records' slot and required-slot bytes; `.map` EME2 spawn kinds; `.veg`/trigger names.
2. Emit counts and value distributions per field, plus a `--query` mode ("how many records set both
   consumed and thrown?") so the answer is one command.
3. Output goes to `artifacts/census/` and is **committed**, so lanes cite a snapshot rather than
   re-deriving it and disagreeing about the walk.
4. Every table records the file, size, and a hash of the parsed region so a re-run is comparable.

## Baseline caveat (RedXenon, 01a157ec) — do not re-derive a claim and call it data

The **57 consumed / 0 thrown** figure quoted across the board is **T-019 stage 1**, and that lane already self-caught
the **asymmetric-operand trap** on its three candidate hits. Scope item 1 already names the **B11-corrected operand
mapping**, so the requirement is not new — but it becomes testable here: **the generator must print, in its own
output, which mapping version it used.** A tool that silently reproduces stage-1 pairing would re-emit a retracted
claim with the authority of a table, which is the exact failure a census exists to prevent.

## Acceptance
- Reproduces the known results as sanity checks: 57 v5 GWAM records, consumed=0 on all, and the
   `char_desc` 90% head-coverage pair {5,6} at 120/80. If a re-run disagrees with a published number,
   that is a bug in the tool or in the claim — say which, do not average them.
- Explicit "unparsed bytes remaining" count per format, so silence means not-yet-parsed, never absent.
- No code changes in `src/` — this is tooling plus a data contract.

## Refs
- `discoveries/2025-10-20-b9-gwam-record-map-for-ledger-4.md`, `...b11-gwam-slots-part1-...md`
- `discoveries/2025-10-20-session35-helmet-slot-data-closes-10c.md`
- `docs/DEFERRED-NOTES.md` entries 2 and 6 (the ones a census can settle or widen)
