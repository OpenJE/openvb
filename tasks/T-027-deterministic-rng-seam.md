---
status: done
assignee: implementer-B (YoungStorm, 01a15802)
depends: nothing
---

# T-027 — Make damage draws reproducible without faking the formula

## Why
B's own correction: `CalculateDamage` calls `rand()`, so **any test asserting its value asserts a coin
flip** — which invalidated a test C and I had both signed off on. The fix cannot be "compute the
average", and it cannot be quietly swapping the generator, which is a fidelity change.

## Scope
1. Route every gameplay draw through one function (e.g. `CombatMath::RollDraw()`), defaulting to the
   shipped behaviour.
2. Add a **test-only** seed/inject path, documented as a deliberate deviation in
   `docs/DEFERRED-NOTES.md` — visible in the tree, not smuggled in a comment.
3. Grep for other unseeded draws in the sim (to-hit roll, reaction chance, voice chance, crit chance,
   scatter) and list them; do not convert them all in this row, but enumerate the surface.
4. No test may assert a damage value while drawing from the live generator — say so in the notes file.

## Acceptance
- A damage case asserts a real number and passes deterministically across 100 runs.
- The deviation is registered in `docs/DEFERRED-NOTES.md` and cppcheck reports no new findings.
- `grep -rn "rand()" src/F3` leaves only the seam, or the remainder is explained.

## Refs
- `tasks/T-012-...` (both C and I treated an expected damage value as valid; it asserted nothing)
- `discoveries/2025-10-20-attack-state-machine-damage-formula.md` (all the roll sites)

## Landed (implementer-B, 01a15802)

Measured surface, not assumed: the shipped generator is `sub_48CCC0`, a Knuth/MMIX LCG over one global
dword lazily seeded from `time32` — **not** `rand()`, which our port was using as the un-injected
fallback. Recovered with `docs/f3/CombatRngSeam.md` §1 (disasm verbatim + the two constants + the
`(state >> 8) & 0x7FFF` emit), and the draw surface enumerated from xrefs to `sub_48CD00`: 18 direct
sites across 10 functions, plus the public `JE::RandomNumber` wrapper. Scope item 3 therefore has a
better answer than a grep of our own tree could give: "crit" and "scatter" are not separate generators,
and `sub_45C140` alone consumes 8 draws (relevant to T-026 — see the note added there).

Checks run: CombatMath suite green with the new golden-stream cases; `msvc-build` exit 0 (MSVC 2002
compiles the LCG and the `time()` seed); app-boot 124 with the fresh exe; `grep -rn "rand()" src/F3`
leaves only the two comments that explain what it replaced. Deviation registered as D-1 above.

