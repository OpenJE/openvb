---
status: open
priority: P1 (a falsified formula is committed and running)
assignee: implementer-C (after T-009)
---

# T-012 — Combat: replace the from-behind test, which is now **proven wrong**

## Why this is P1

`GameActionAttack::Update` picks the victim's hit-reaction animation (`33/34` moving vs
`37/38/39` stationary, front/back by a `fromBehind` boolean) using a rule I substituted when the
constants were unrecovered: planar `atan2f(dx, dz)` with `|angle| > 2.356f` (3·π/4).

NiceWolf falsified it, and I re-verified the constants independently from the shipped binary's
bytes rather than taking the claim:

| symbol | value |
|---|---|
| `flt_660820` | **360.0f** (`0x43b40000`) |
| `flt_660824` | **180.0f** (`0x43340000`) |
| `flt_660828` | **−180.0f** (`0xc3340000`) |
| `flt_649C50` | **0.0174533f** = π/180 |
| `ds:0x6cb56c` (first compare constant) | **0.0f** |

There is no 2.356 and no 135.0 anywhere at those sites. The shipped test is a **degree-domain
wrap/range test**, not a radian arc: `π/180`-prescaled Δeuler → helpers `0x47272b` / `0x471357` →
`fpatan` → an x87 compare chain over `{0, −180, +180, 360}` that produces a **byte flag**, which
then feeds the `vft[+80]` gate and the damage-type switch (`effect+0xA8`).
Full evidence: `discoveries/2025-10-20-T010a-behind-test-is-wrap-not-threshold.md` §1–§3.

## Steps

1. Reserve `src/F3/GameActionAttack.cpp`, `src/F3/GameActionAttack.hpp`, and the SpawnMove test.
2. Delete the `2.356f` guess. Replace the angle derivation with the shipped shape: prescale the
   Δeuler components by π/180, and compute the flag as the range/wrap test the doc describes.
3. Keep the observable seam: the resulting flag still selects `kReactionAnimMoving` /
   `kReactionAnimIdleFlag(Set)` front/back and `SpawnedEntity::queuedAnimEvent`.
4. **Do not resolve the open sub-question by invention.** §3 leaves one thing unsettled: whether
   the `fpatan` result feeding the chain is in radians or degrees (which decides whether the
   `±180/360` compares are taken literally or after another scale). If your implementation needs
   that to pick a branch, message the coordinator and it routes back to research — the flag's
   consumer (`vft+80` + the damage-type switch) is the disambiguating evidence, and HappyWolf is
   re-deriving exactly that.
5. Update the header's deviation note: it currently says the constants are "not yet extracted".
   They are extracted; the rule is now known. Record that the old guess is dead so nobody
   reintroduces it.
6. Test: assert the flag's behavior at the four boundary angles (0, just under/over 180, 360) and
   that a hit whose angle is unchanged still yields the same anim id.

## Acceptance

- No `2.356`, `3*pi/4`, or `135` remains anywhere in `src/F3/GameActionAttack.*`
  (`grep -n "2.356\|3.pi/4\|135" src/F3/GameActionAttack.*` returns nothing).
- The five constants above appear as named values with their EAs, cited in comments.
- SpawnMove green; all 9 suite rows OK (`=> 10 pass, 0 fail` incl. app-boot); `msvc-build` exit 0.
- If step 4's ambiguity is unavoidable, the deliverable is a **filed question + a formula that
  does not pretend** (flag left at the shipped default with an explicit citation), not a guess.

## References

- `discoveries/2025-10-20-T010a-behind-test-is-wrap-not-threshold.md` (the answer + raw chain)
- `discoveries/2025-10-20-session7-effect-pipeline-and-damage-apply.md` §3 steps 4–5
- `src/F3/GameActionAttack.cpp` — the current substituted rule

## Notes (coordinator)

This is my error to hand out, not yours to inherit silently: I wrote the guess, documented it as
a deviation, and it was still wrong in a way the constants would have shown immediately. The
lesson in the board rules — substituted values must be routed, not modeled — is why this row
exists instead of a quiet patch.
