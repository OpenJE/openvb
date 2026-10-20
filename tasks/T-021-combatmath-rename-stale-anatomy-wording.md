---
status: done
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

## Evidence (implementer-C, 2026-10-20)

Landed: openje `8a60ed5` (the two granted lines) + parent `a6b3cf4`.

- **Rename** `dtParts56` -> `dtRowB` at all five sites (declaration, ctor zero loop, the two
  `case 5: case 6:` rows in `CalculateDamage`/`CalculateUnarmedDamage`, and B's suite line).
- **Logic provably untouched**: `git diff` filtered to non-comment lines shows *only* the
  identifier rename. Nothing else in either file's executable text changed.
- **Cross-TU substitute for the missing link step** (MSVC is red on a peer's `d3d8.lib`):
  `grep -rn dtParts56 src modules/*/src modules/*/tests modules/tracing` -> only the
  explicitly-historical `Formerly dtParts56` note. No dangling reference.
- **Host rows green**: CombatMath, PathManager, ActionMoveTo, SpawnMove (all four compile
  these files and LINK the renamed member against real callers under mingw). Overall
  `8 pass, 1 fail`; the single fail is GUILoading from the stale installed payload, which
  compiles **exit 0** when the row is replayed from `devenv/scripts/run-all-tests.nix`.
- **cppcheck**: no findings on any changed file.
- **Shipped literal verified against the binary, not just preserved**: the string is present
  byte-identical at PE file offset `0x24fb48` in the shipped `F3.exe`, so our copy still
  matches what the original logs. Marked `shipped string verbatim, do not "fix"`.
- **Acceptance grep as rewritten**: every surviving `body part|anatomy` hit is a comment that
  *negates or attributes* the phrase ("not body parts", "10c disproved", "the ORIGINAL'S OWN
  wording", "do not fix") — i.e. explicitly historical, which the row permits. A raw
  `grep -niE "body part|anatomy"` returning zero would only be achievable by editing the
  shipped string, which is the corruption the row now forbids.

### Two things a future reader should not re-litigate

1. Sites the original row did not list, fixed anyway because they carried the same falsified
   claim: the `CombatStats` preamble ("+0x7C8 used for attack **modes** 5/6" — the row is
   picked by target id, and `AttackModeRecord`'s own NOTE three lines below says so, so the
   file contradicted itself) and the `kCalledShotMods` table header.
2. `{5,6}` are named **head-family as a group** only. Which id is HEAD versus EYES stays
   inferred (penalty 120 favours EYES); ids 0-4/7-8 remain numeric; the code indexes
   numerically. Naming the pair further than that is not supported by 10c.

## References
- 10c close: `discoveries/2025-10-20-session35-helmet-slot-data-closes-10c.md`;
  pair table in `discoveries/2025-10-20-t010c-conflict-resolved-equip-slot-model.md`
- `docs/DEFERRED-NOTES.md` ledger entry 2 (two-zone model, harmless-by-data)
- `tasks/T-012-…` for the verified/unverified substitution method when the link step is unavailable

## Notes (coordinator)
Assigned during the blocked window on purpose: names and comments are the only work that can be
validated without a linker. Both of C's findings are in this row — the fifth reference they refused to
edit without a handshake, and the shipped-string trap my acceptance pattern walked into.
