---
status: in-progress
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
2. Rename `CombatStats::dtParts56` → **`dtRowB`**. Approved target: "row B" is already the corpus's
   term for this datum (FastArrow:B8 "row B = armor slot-class 1"; the 10c close "row B is head/helmet
   coverage"), so it states what it is without smuggling in **head** or **body part** — the two things
   10c marks as still inferred. Keep **two** rows; do not grow to six per-part rows (ledger entry 2 is
   harmless-by-data and the shipped model is two-zone).
3. **Cross-lane grant — option (a), coordinator's answer to C's blocker.** The rename has a fifth
   reference in implementer-B's lane: `modules/openje/tests/je/CombatMath/CombatMath.cpp:124`
   (`t.dtParts56[0] = 100;`) plus its `// second DT row when called-shot PART is 5/6` comment on :123.
   **C edits those two lines in the same commit as the rename.** A split would leave an intermediate
   state where B's suite is red — exactly the one-lane-outage shape in harness item 14 — and
   comments-only would still hit the acceptance grep, so the row must not be marked half-done. B has
   been notified. No other test or source file may be touched; confirm with `grep -rn dtParts56`
   returning nothing.
4. Fix wording that calls `{5,6}` body parts: the `dtRowB` declaration, `CalculateDamage`'s
   row-switch comment, `CalculateUnarmedDamage`'s twin notes, and the `calledShotPart` field comment.
5. Naming policy from 10c, in comments only: `{5,6}` = head-family pair, **which is HEAD and which is
   EYES remains inferred** (penalty 120 favours EYES); ids 0–4, 7–8 stay numeric; code indexes
   numerically. Cite **by filename** per item 15 — `IronStorm:sess35`, `FastArrow:B8/B9` — no bare
   session numbers.
6. Do **not** touch T-012's behaviour. Purely names/comments; any logic change means the row was
   misread — stop and say so instead.

## Acceptance
- **Identifiers/comments.** `grep -rn dtParts56` across `src`, `modules/*/src`, `modules/*/tests`
  returns nothing, and no **comment** asserts that ids 5/6 are anatomy (wording explicitly marked
  historical is fine).
- **Shipped string literals are excluded from that grep and asserted separately.** My original pattern
  was unsatisfiable, and C caught it before coding to it: `src/F3/CombatMath.cpp:102` contains
  `"GameWeapon::CalculateDamage - No body part was targeted.\n"`, a **verbatim shipped log string**
  (cited in `attack-state-machine-damage-formula.md:74` and `called-shot-part-second-source.md:30`).
  Editing it to satisfy a grep would corrupt string fidelity — the original's own words say "body
  part". So: the literal stays **byte-unchanged**, plus a
  `// shipped string verbatim, do not "fix"` marker at both sites so the next lane does not tidy it
  away. This is the third broken acceptance pattern from the same mistake in this project (after
  T-012's `135` grep matching the legitimate EA `0x471357`): a forbidden-token grep must be written
  against **our prose**, never against the original's constants or strings.
- Cross-TU verification as in T-012 (grep the removed identifier against real callers), since MSVC
  cannot confirm it while `d3d8.lib` is missing.
- Host rows compiling these files stay green; `cppcheck-openvb` reports no new findings.

## References
- 10c close: `discoveries/2025-10-20-session35-helmet-slot-data-closes-10c.md`;
  pair table in `discoveries/2025-10-20-t010c-conflict-resolved-equip-slot-model.md`
- `docs/DEFERRED-NOTES.md` ledger entry 2 (two-zone model, harmless-by-data)
- `tasks/T-012-…` for the verified/unverified substitution method when the link step is unavailable

## Notes (coordinator)
Assigned during the blocked window on purpose: names and comments are the only work that can be
validated without a linker. Both of C's findings are in this row — the fifth reference they refused to
edit without a handshake, and the shipped-string trap my acceptance pattern walked into.
