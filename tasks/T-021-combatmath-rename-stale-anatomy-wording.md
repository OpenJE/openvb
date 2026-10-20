---
status: open
assignee: implementer-C (UltraBear)
depends: nothing (validatable without a link step)
---

# T-021 — Retire the anatomy wording that 10c disproved

## Why now, while the exe is unbuildable
10c closed with row B = **equip-slot / helmet coverage**, not body parts. The code still says
otherwise in names and comments, and stale prose outlives its retraction — it is what the next lane
greps for and believes. Comment-and-rename work, so it needs no green link.

## Steps
1. Reserve `src/F3/CombatMath.{hpp,cpp}` and `src/F3/GameActionAttack.hpp`.
2. Rename `CombatStats::dtParts56` to something that states what it is (row B / equip-slot-class-1
   coverage). Keep **two** rows — do not grow to six per-part rows; entry 2 in the ledger is
   harmless-by-data and the shipped model is two-zone.
3. Fix wording that calls `{5,6}` body parts, in the `dtParts56` declaration, `CalculateDamage`'s
   row-switch comment, `CalculateUnarmedDamage`'s twin notes, and the `calledShotPart` field comment.
4. Name policy from 10c, applied in comments only: `{5,6}` = head-family pair, **which is HEAD and
   which is EYES remains inferred** (penalty 120 favours EYES); ids 0-4, 7-8 stay numeric; code keeps
   indexing numerically. Cite `IronStorm:sess35` and `FastArrow:B8/B9` **by filename**, per item 15 —
   no bare session numbers.
5. Do **not** touch T-012's behaviour. Purely names/comments; any logic change means the row was wrong.

## Acceptance
- `grep -niE "parts56|body part|anatomy" src/F3/CombatMath.* src/F3/GameActionAttack.hpp` returns
  only places where the wording is explicitly historical ("formerly…", "disproved by 10c").
- Cross-TU check as in T-012: grep the removed identifiers across `src`, `modules/*/src`, and
  `modules/*/tests` to prove no caller referenced them, since MSVC cannot confirm it for you while
  `d3d8.lib` is missing.
- Host rows that compile these files stay green; cppcheck reports no new findings.
