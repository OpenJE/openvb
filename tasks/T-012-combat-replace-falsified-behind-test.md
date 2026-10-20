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
4. **The consumer is now resolved — read `discoveries/2025-10-20-T010a-behind-test-is-wrap-not-threshold.md`
   §5 (canonical §13 context) BEFORE coding, because it changes the shape of this task.**
   Apply @0x45c30c–0x45c358 selects the reaction anim as **(wrap-flag × target-order)**, not an arc:
   outer branch on `effect+0xAD` (multi-target/AoE), inner on `effect+0xAC` (is-last-target) — both
   are the session-21 §7 drain markers, not new fields. Yields `33 + wrapFlag` → **{33,34}**, or via
   the `+0xAC == 0` path `37 + (flag ? 0 : -flag & 3)` → **{37,38,39,40}**.

   Consequence for the existing tables: `kReactionAnimIdleFlag {37,38}` and
   `kReactionAnimIdleFlagSet {38,39}` are **wrong in two ways** — they split the pair on the
   front/back axis instead of target-order, and they omit anim **40**. Replace them with the
   shipped derivation rather than renaming my guesses. `kReactionAnimMoving {33,34}` is right
   but its selector must become the wrap flag, not `fromBehind`.

   The rad-vs-deg question in §3 is **moot for this consumer** (it only tests flag ∈ {0,1}) but it
   still decides *when* the flag is ever 1. Keep the single quarantined helper for exactly that,
   with the default and its consequence documented — do not resolve it by invention.

5. Update the header's deviation note: it currently says the constants are "not yet extracted".
   They are extracted; the rule is now known. Record that the old guess is dead so nobody
   reintroduces it.
6. Test: assert the flag's behavior at the four boundary angles (0, just under/over 180, 360) and
   that a hit whose angle is unchanged still yields the same anim id.

## Addendum after both research lanes converged (coordinator)

The earlier "replace the two-way idle tables with `37 + (-flag&3)` -> {37,38,39,40}" instruction is
**withdrawn** - it came from NiceWolf's §5 semantics, which they have since retracted in §5b. Both
lanes now independently agree on the raw structure (`if (rec == list->first) { +0xAC = (>=2 volleys
marked); +0xAD = 0 } else { +0xAD = 1 }` in the finisher @0x4038D7-0x4038F7, consumed at Apply
@0x45C30C/0x45C336), so the shipped rule is:

- **first record of the volley animates; every subsequent record is silent** (+0xAD = 1 makes Apply
  jump past the whole hit-reaction block),
- stationary pair is **+0xAC**-gated: clear -> {37,38}, set -> {38,39}. **My original two-way tables
  were right; keep them.** Restore `kReactionAnimIdleFlag`/`kReactionAnimIdleFlagSet` untouched if
  you already edited them,
- `kReactionAnimMoving {33,34}` stays, selected by the wrap flag (sign of angle), not by a
  front/back arc.

## Second addendum - T-014 landed while you were working (FastArrow, a252575). Read this before
## coding the first-wins rule, because it changes the *condition*, not the shape.

`record+0x19` is not an independent mark: `CalculateAndQueueDamage_0x404060` snapshots the action's
**+0x71 unarmed flag** into each queued hit record at insert time (@0x4041A1, via `v22`@esp+25h →
node+0x19 anchored on damage@node+0x08). Therefore the finisher's `local_11` is
**">= 2 UNARMED records queued"**, not ">= 2 volleys carrying some mark":

- `eff+0xAC = 1` means **unarmed flurry of >= 2 hits** -> the heavy pair **{38,39}**.
- A weapon volley never sets it, **regardless of count** -> three-shot weapon volley stays **{37,38}**.
- My earlier phrasing ("first record animates, rest silent" as the whole story) is still right about
  +0xAD (non-first records skip the reaction block entirely), but the +0xAC half is unarmed-specific.
  Counting the per-record unarmed snapshot is the correct predicate; raw record count is wrong.

Observable acceptance you get for free, and it is the best test in this file because it separates the
two readings: **same hit count, weapon vs unarmed, must produce different anim ids** ({37,38} vs
{38,39}). If your implementation gives the same answer for both, you have modelled count, not the
flag - which is exactly the mistake this addendum exists to prevent.

Two cautions from the same file, so you do not chase them: the other `+0x19` stores at
0x404B4F/0x404D03 are InitiateDialogue clone-and-requeue **offset aliasing**, not volley records; and
the 0x40386B string-select byte row is still unverified. The node-base derivation has a one-gdb-line
residual, safe to treat as pinned since the finisher reads the identical byte.

**Three independent sources now point at `+0x71` as the unarmed discriminator**, which is what makes
this predicate safe to build on rather than merely plausible: (1) FastArrow parsed the write site —
the snapshot at insert; (2) I verified it against the raw sites myself; (3) NiceWolf has long had the
*consumer* fork mapped independently, `if (this->mbr_bool_0x71)` @0x4040a8/0x4040d4 choosing
`CalculateUnarmedDamage_0x409f90` over `CalculateDamage_0x413310` (canonical §1's "+0x71 unarmed
flag" row). Producer, copy, and consumer all agree, from three separate reads. If the weapon-vs-
unarmed acceptance case ever disagrees with this, one of us is wrong in a loud, checkable way.

Related band caveat worth knowing while you are in that offset range: NiceWolf's session-21 field map
flagged uncertainty across the `+0x71..+0x78` band ("flag byte vs 0x28 record"), and T-014 explains
it — the band carries **both** attack-mode queue fields and InitiateDialogue clone-and-requeue
artifacts (the stray `+0x19` stores at 0x404B4F/0x404D03). So if you find a second, inconsistent
meaning for a byte in that range, it is aliasing between two subsystems, not a contradiction of the
unarmed-flag reading above.

Practical note for your port: `GameActionAttack` already carries `unarmed0x71`, so the per-record
snapshot is one field on `QueuedHit` (e.g. `unarmedRecord`), not new state - and it must be captured
**at queue time**, because a mid-volley weapon swap changes the action flag but not the queued record.

So the anim-table rewrite is OFF; the **behavioural** gap is the new work: model the first-wins rule,
which is observable (a multi-sub-attack unarmed volley should animate once, not N times) and is
currently not modelled at all in `GameActionAttack::Update`, because my queue drains every record
identically. If you would rather land the radian/angle-chain fix without it, say so and it becomes a
follow-up row - do not let scope creep delay the part that is settled.

## Acceptance

- No `2.356`, `3*pi/4`, or `135` remains anywhere in `src/F3/GameActionAttack.*`, and no
  two-way `{37,38}`/`{38,39}` idle table remains either (acceptance check: grep for
  `kReactionAnimIdleFlag` shows a derivation, not two paired arrays).
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
