---
status: in-progress
assignee: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (SEATED by formal commission from the human via LoudDragon — PARK R-ANNOT row)
type: product-lane edits — COMMENTS ONLY (zero behavior change; per-file commits; per-file reservations)
depends: tools/port-annot-check.py (manifest+gate), tools/ea-index.py (citation source, item-53 query-first), item 57 (cite-or-name-absence law)
---

# R-ANNOT — port-annotation backfill: 97 uncited defs / 23 files → gap=0 → checker graduates to run-all

## Fences (hard)
- NEVER edit: `src/F3.cpp`, `src/F3/RenderD3D8.cpp` (KeenStorm holds), and every def line R-FINISH
  lists (their completion deletes its own TODO — annotator skips R-FINISH-listed defs entirely;
  zero overlap is the deal).
- Comments-only. No code move, no whitespace outside comment insertion, no signature touch.
  Behavior-untouched proof: `run-all-tests` green + msvc-build rc0 at wave close (comment edits
  must survive compile — cross-TU probe per T-023 if a comment block sits inside a macro line).
- Per-file: reserve file → annotate → commit (file-scoped message with counts) → release.

## Citation ladder (per def, item 53)
1. `tools/ea-index.py --query <Name>` → shipped EA from the doc corpus (the index that lies never).
2. Ledger tables from tonight's waves (t086/t073/r-cs/wave3 ledgers, IDB name tables).
3. IDB (read-only xref-weight: lookup_funcs/f3 tools) ONLY when 1+2 disagree or are silent.
4. No source found → NAME THE ABSENCE comment ("no shipped counterpart located via ea-index
   <date>; <what it structurally is>") — silence is the violation, absence-named is compliant.

## Slice order (cheapest payoff first per coordinator; big files need R-FINISH reconciliation first)
Tail-first queue: Gregorian(1) → Map(1) → SkinDefaults(1) → StringSystem(1) → GamePathManager(1) →
CombatMath(1, mine) → Display(1, mine) → Scripting(2) → cls_0x4d8d70(2) → GameActionAttack(2) →
CombatManager(2) → SpatialGrid(4) → PathManager(6) → VFX_Parameter(5, cross-check R-FINISH sweeps) →
InterfaceFile(4) → ResourceSystem(9, R-FINISH comment-sweep overlap check) → StringFormat(5, same) →
MapEntitySpawn(10, KeenStorm's FILE? — it's src/F3/MapEntitySpawn.cpp, worktree shows M: HOLD
until file-free) → SkinFragments(12) → SkinTable(17). main.cpp `if@24` = checker-parse oddity —
investigate (maybe a lambda/if the regex caught; fix-listing belongs to the gate's graduation, not
a fake citation).
src/F3/MapEntitySpawn.cpp + SimDigest.hpp + sim-driver.cpp are CURRENTLY DIRTY per git status
(KeenStorm WIP) — treat as held until clean at slice time; re-check `git status` per file before
each reserve.

## Close
gap count after each commit via `python3 tools/port-annot-check.py | tail -1`; at 97→0 (minus
fenced lines counted honestly) post the graduation PR-ready note; run-all integration is the
coordinator's keystroke. Successor line: the R-FINISH-adjacent files re-pass once their TODOs die.
