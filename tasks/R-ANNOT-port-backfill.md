---
status: REACHABLE-ZERO (97→9; the 9 are all R-FINISH-fenced in ResourceSystem.cpp — see graduation)
assignee: SwiftOwl `01a16198-6cbb-71af-9ff0-c353db5d95f9` (re-seated post-OOM 2026-10-22 by QuickNova formal re-issue of the dead seat; predecessor: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (SEATED by formal commission from the human via LoudDragon — PARK R-ANNOT row)
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

## SwiftOwl claim note + live state (2026-10-22, post-OOM resync)
- Checker truth at claim: **31 uncited of 356 (8%) across 3 files** (QuickNova's relay figure of 80
  was a beat stale; statelessness held — HappyStorm's uncommitted SkinTable slice in the openje
  worktree explains the 48→31 drift; salvage landed at claim time, attributed to its author).
- Coordinator fences re-issued (QuickNova assignment): SKIP all R-FINISH COMPLETABLE files —
  incl. `modules/openje/src/JE/ResourceSystem.cpp` (its 9 uncited are FENCED, no longer in queue;
  supersedes the old "ResourceSystem(9…)" slice-order entry). Fence-3 verification per
  instruction, nothing touched: `src/F3.cpp 0/34`, `src/F3/RenderD3D8.cpp 0/8`, `src/main.cpp 0/2`
  — all already cited-clean.
- REACHABLE manifest (mine): SkinFragments.cpp 12/14 (CheckSlotGeometry@34, ReservedFieldsAreZero@56,
  AuditOne@60, AuditWidgetTree@92, DeriveTextureKey@96, Contains@109, CollectOne@116,
  CollectTextureNames@126, mgrIndex@182, mgrObject@186, GfxTextureSlots::GfxTextureSlots@198,
  mgr@199) → MapEntitySpawn.cpp 10/24 (ClassIdForResourceName@59, ClassIdFromTemplateChunks@93,
  FindLoadedMap@160, LoadedMapCount@167, PlaceEntityAtEntryPoint@171, EnqueueEntityAction@259,
  PushActionOntoEntity@294, PumpEntity@379, JoinCombatThroughSeam@576, FindSpawnedEntity@816).
  MapEntitySpawn is worktree-CLEAN now — the old KeenStorm M-hold lifted, re-checked at reserve per
  this file's own rule.
- Your-reachable-zero = 31 − 22 = 9, ALL in fenced ResourceSystem.cpp; graduation note authored at
  that line with the fence math stated honestly (gap≠0 while R-FINISH lives; the 9 delete
  themselves-at-completion per the zero-overlap deal).
- Reserved at claim: SkinFragments.cpp, MapEntitySpawn.cpp, SkinTable.cpp (salve scope only —
  comments-only commit of HappyStorm's in-flight +24, authorship credited in-content).
- Post-77dc997 the rest of the dead-lane worktree is reconciled (docs tracked, ea-index green at
  197 docs/7,842 EAs via SageBear's H-IDX pull); the openje submodule pointer was explicitly
  "left for its owner" — that owner is this lane, hence the SkinTable bump here, nothing else
  rides it.

## GRADUATION NOTE (SwiftOwl, 2026-10-22) — the annotation-stock is CLEARED to reachable-zero
- Trajectory this seat: 31 → 19 (SkinFragments 12, db50b5c/1010c56) → 9 (MapEntitySpawn 10,
  7a73234) + the salvaged SkinTable 17 (463af13) and the 22 the seat never owned (slices 1–2,
  HappyStorm). Full campaign: 97 → **9**.
- The 9 survivors are ALL `modules/openje/src/JE/ResourceSystem.cpp` = R-FINISH COMPLETABLE
  ("ResourceSystem comment sweep" item) — per the zero-overlap deal each deletes itself at
  completion, when its owner implements it (R-FINISH is UNCLAIMED paper; this lane did not touch
  it, fence respected).
- Fence honesty census at close: `src/F3.cpp 0/34`, `src/F3/RenderD3D8.cpp 0/8`, `src/main.cpp
  0/2` verified untouched and clean; the old `main.cpp if@24` parse oddity is dead — the current
  regex pass reads 2 defs, both cited. No file outside R-FINISH's list carries a gap.
- Every citation written by this seat: rung-2 ledger (ea-index --query hits, T-007/T-074/T-080/
  T-085/session-21/23 docs) or rung-3 IDB (only rung-1+2-silent case: 0x57B8A0 meth_0x57b8a0,
  f3_address_lookup, recorded in its comment); absence-comments name the lookup + date + what
  the function structurally is.
- Proof gates at close (comment-only waves must survive both): `run-all-tests` 10 pass / 0 fail
  (app-boot 124 unchanged) + `msvc-build` rc=0 with MapEntitySpawn.cpp recompiled — both run
  2026-10-22 post-commits.
- GRADUATION ASK (coordinator keystroke): wire `tools/port-annot-check.py --gate` (incremental:
  fails only files changed vs HEAD) into run-all NOW. It does not need the 9 to vanish — full
  gap=0 is R-FINISH's to mint; the incremental gate makes any NEW or RE-OPENED uncited def a
  build failure, which is the law's actual teeth. At R-FINISH's ResourceSystem completion the
  plain checker can be promoted from --gate to strict in the same style as the other graduates.
