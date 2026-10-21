---
status: done
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

## Verdict (stage 1 `a16a31d`, stage 2 `521fac2` + `5906d57`) — coordinator-verified, not taken on report

**Seven gates, all PASS**, run independently by the coordinator: `emap_count_Maps` 13, `eme2_count_Maps` 64,
`gwam_v5_count` 57, `gwam_consumed_nonzero` 0, `eme2_tiles_all_64_chunks` True, `eme2_first_record_default_pc`
`Default_PC.CRT`, `gitm_eqslot_armor_values` `[0,1]`. `--query eme2_kinds` reproduces
`{WEA 32, AMO 18, CON 9, CRT 2, DOR 1, USE 1, ITM 1}` from the committed table.

**What the row actually produced, beyond a tool:**
- **EME2 structure**: all 64 records tile **byte-exactly** as `{u16-len raw string, 3f pos, 3f rot, u8 flag, embedded
  sub-chunk (EEOV v2)}` — the sub-chunk lives **inside** the record stream, which sess-21's note implied but never
  stated. Spawn-kind distribution is now one query instead of a decompile.
- **A NAMED GAP, with evidence**: trigger *names* are not in any walkable chunk — EMTR payloads are counts + f32
  geometry, EBTR is a wrapper embedding EMNP/ETRE/HEAD sub-chunks, the EMNP sample is four null bytes. `triggers.csv`
  prints **0 with a printed reason pointing at the compressed region**. `.veg` scope folded into the same line: no `.veg`
  at the demo's top level, no VEG tag walk available. **Empty ≠ absent**, applied to a table that came back empty — the
  exact case the row's discipline exists for.
- **Self-corrections kept visible rather than amended away**: the first `read_jstr` claimed NUL-inclusive lengths and was
  falsified in one step by the tile trace (`0x0e` = exactly `"Default_PC.CRT"`, straight into the pos floats); the
  container expressions were re-confirmed offline (`cls_0x54e070` size 20; `+0x04` blocks / `+0x08` capacity /
  `+0x0C` 4-per-block / `+0x10` element index; push_back triple at `0x54ea03/2a/71`), so T-034's R3 will read the right
  dwords when a port frees.
- **Route and mapping provenance are printed** (`FastArrow:B11-corrected`; per-route GWAM counts), which is what turned
  "57" from a claim into a number that names the walk that earned it.

Snapshot re-emitted under an unchanged dir hash (`99ac4dab13fc`) — the tables grew, the payload did not, and the tool
says so.

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
