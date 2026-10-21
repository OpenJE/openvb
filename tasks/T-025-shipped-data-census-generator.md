---
status: open
assignee: RedXenon / 01a157ec (formally dispatched 2026-10-21 after a written request; the row moved by
# coordinator edit + dispatch, never by chat offer - the process this lane itself insisted on)
depends: nothing
---

# T-025 — Census every shipped data record into greppable tables

## Why
The 10d lesson: **before asking whether a bug is observable, ask whether on-disk data can answer it.**
That question settled T-019 at 0/57 in one sitting, and closed 10c from `char_desc` alone. Generalise
the method so "does shipped data ever do X" stops needing a debugger and stops needing me to dispatch.

## Sequencing decision (coordinator, 2026-10-21): stage 2 is authorised NOW

Stage 2 was sequenced behind T-034's R3/R0. RedXenon's objection is correct and now the rule: **an
environment-blocked dependency is not a working dependency** — parking a lane on a blocked attach converts a
sequencing decision into a stall. Stage 2 needs no debugger (EME2 spawn-kind table, `.veg`/trigger names, more
`--query` verbs, snapshot re-emit), so it proceeds; if R3/R0 land afterwards, the generator's route/mapping prints are
already there to absorb the new facts.

## Scope
1. Extend the existing v5 GWAM walk into a **census generator** emitting stable tables (csv or md) for:
   all v5 GWAM records with the B11-corrected operand mapping; `char_desc` body-part modifier rows;
   item/equip records' slot and required-slot bytes; `.map` EME2 spawn kinds; `.veg`/trigger names.
2. Emit counts and value distributions per field, plus a `--query` mode ("how many records set both
   consumed and thrown?") so the answer is one command.
3. Output goes to `artifacts/census/` and is **committed**, so lanes cite a snapshot rather than
   re-deriving it and disagreeing about the walk.
4. Every table records the file, size, and a hash of the parsed region so a re-run is comparable.

## Second correctness constraint (2026-10-21, from SwiftRaven's in-flight T-036 result)

**Attribution counts are join-route-dependent, so the generator must print the route and not just the number.** T-036's
trace: the IDA-name route yields **2,340** attributed functions, the DB-method route **2,473** (2,425 inside the game
region), and the "1,475 nameable + 998 helper-theft noise" split **reproduces under neither** — it came from mixing the
two routes. A generated table emitting a bare total would re-commit that exact error with more authority than a chat
message carries. Requirements: every count names its **join route** and its **region filter**; where the routes disagree,
print **both** rather than reconciling silently.

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
