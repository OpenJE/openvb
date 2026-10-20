---
status: done
priority: P1 (a falsified formula is committed and running)
assignee: implementer-C (UltraBear, mesh sess 01a15aff)
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

## Open acceptance item, deliberately not closed — CORRECTED 2026-10-20 (my premise was false)

**Retracted:** "`build/F3.exe` has never been produced with these combat changes in it, because
`d3d8.lib` is missing from the shared tree's link step." Both halves are wrong. `769218c` (T-012) is an
ancestor of HEAD, and the exe that `app-boot` cites — built at **17:32:53 from a verified-clean tree** —
therefore **contains these changes and booted at 124**. And `d3d8.lib` is present in the MSVC image; the
only gap was `vcvars32.bat` not putting the `prerelease` dirs on INCLUDE/LIB, now fixed in
`devenv/scripts/msvc-sdk-prerelease.bat`. I kept quoting this as a blocker for hours because it had been
written down in a row rather than re-derived.

**What is actually still open:** the SpawnMove anim-id cases (flurry discriminator, residue parameter)
have never been run **against** that binary — the behavior is compiled and boots, but those specific
observations are host-suite only. Close condition: after a `devenv-reload`, `msvc-build` exit 0, `app-boot`
exit 124, and implementer-C runs the anim-id cases against the real exe. No D3D8 involvement; nothing to
wait for. Owner: coordinator schedules, UltraBear executes.

## Final status as delivered (review), superseding two of my own instructions
1. **Withdrawn:** "restore the two-way tables as originally written". The reachable set is **{33,34,37,38,40,41}** - 39 is unreachable and 41 was missing - derived from `neg al/sbb eax,eax/and eax,3`, i.e. a step of **3** off bases 0x25/0x26. That falsifies my tables *and* the interim {37,38,39,40} reading, so restoring either would re-ship a disproved table. The byte-derived computation stands; independent confirmation requested as T-020.
2. **Implemented beyond the row:** non-first records receive **no animation at all** (0x45C312 branches past the QueueAnimEvent call), modelled as `ReactionAnimFor() == 0` rather than a different id.
3. **New shipped defect, ledger entry 5:** Apply's wrap byte `var_B1` has no zero initialiser, the apparent initialising store targets another slot because `sub esp,0Ch` @0x45C22C shifts the window, so in-band angles and both early exits read previous-call stack residue. Modelled as an explicit residue parameter.
4. Domain question **closed**: `fpatan` @0x45C29C is the last fp op before the first compare, so the input is radians by architecture and degrees is an unreachable comment only.

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

## Evidence (implementer-C, 2026-10-20)

Three independent decoders were used on the shipped bytes (IDA, objdump, and a hand decode of
the instruction encodings), because the anim tables were contested three ways by the end of this
row and only the encodings settle it.

### What the bytes actually say

1. **The five constants** - read out of the PE directly, not inferred:
   `0x6CB56C`=0.0f, `0x660828`=-180.0f, `0x660824`=180.0f, `0x649C50`=0.0174532924f, `0x660820`=360.0f.
2. **The rad-vs-deg question in §3 is CLOSED and the answer is radians.** `fpatan` (0x45C29C) is
   the last FP op before the first compare (0x45C29E); nothing scales it. x87 bounds fpatan to
   [-pi,pi], so the +/-180 and 360 compares CANNOT fire for any real input. Feeding the chain
   degrees would require a multiply the binary does not contain - the coordinator's original
   step 3 ("prescale by pi/180 as the shipped shape") was withdrawn for exactly this reason.
3. **The chain collapses to a sign test.** Simulated truth table: -181->0, -180->1, -90->1,
   -0.1->1, -0.0->0, 0.0->0, +0.1->0, +180->1, +180.1->1, +359.9->1, +360->0. i.e. literally
   `flag = (angle mod 360) >= 180`, which over [-pi,pi] is `angle < 0`. So the domain choice is
   UNOBSERVABLE through this consumer - kept as a documented assertion, not a behavior switch.
4. **The reachable reaction ids are {33,34,37,38,40,41}.** The stationary selector is
   `neg al / sbb eax,eax / and eax,3` = `(al != 0) ? 3 : 0`, a step of THREE off bases 0x25/0x26
   (37/38). This makes **39 unreachable** - both pre-existing tables `{37,38}` and `{38,39}` were
   wrong, as was the interim `{37,38,39,40}` reading in T-010a §5 - and **41 was missing**.
   Moving uses `setne` (step 1) -> {33,34}. Encodings: `f6d8 1bc0 83e003 83c025` / `83c026`.
5. **First-wins is real and stronger than "different id":** `0x45C312` branches on eff+0xAD
   straight past the `QueueAnimEvent` call at `0x45C35A`, so non-first records queue NO anim.
   Implemented as `ReactionAnimFor()` returning 0 = no event.
6. **A shipped defect found in the process, filed not fixed:** the wrap byte is Apply's `var_B1`
   ([esp+0Fh) and the ONLY stores to it are `mov byte,1` at 0x45C2D2/0x45C2DB. Nothing zeroes it:
   the `mov [esp+0Fh],bl` at 0x45C1B6 hits **var_A5** (frame offset 0x1B), a different slot, because
   0x45C22C's `sub esp,0Ch` shifts the window by 0xC in between. Verified by esp-walking every
   store in 0x45C140..0x45C650 and by checking 4-byte store windows. So on an in-band angle - and
   on the two early exits at 0x45C1BA/0x45C1CB, which jump past the whole chain to 0x45C2E4 - the
   original reads UNINITIALISED stack (in practice the previous Apply's residue). We model a fresh
   0 for determinism and expose the residue as a parameter so the stickiness is reproducible.

### Tests

SpawnMove gained the flurry/first-wins/angle checks (file now 81 `check()` calls), including the
discriminator the coordinator asked for: 3 UNARMED records -> 41, 3 WEAPON records same count ->
40, plus `!= 41` to catch a count-based rule.

**Non-vacuity proved by two negative controls**, each reverted after:
- census replaced by naive `unarmedMarks = queueCount` -> `FAIL: 3 WEAPON records, SAME count...`
  and `FAIL: count-based rule would have produced 41 here` (2 failures) - so the test really
  separates count from snapshot.
- `kReactionAnimIdle` step changed 40->38 -> 3 failures incl. the reachable-set check.

### Gate state on MY run (differs from the coordinator's "9/9 + msvc clean")

`run-all-tests --msvc-build` -> `msvc=FAIL`, `GUILoading=BUILD-FAIL(undefined-reference)`,
`app=BUILD-MISSING`, others OK, `=> 8 pass, 3 fail`. All three reds are outside this row:
- `msvc-build`: `LINK : fatal error LNK1181: cannot open input file 'd3d8.lib'` - the D3D8 import
  YoungStorm's M-001 slice 1 added in `cmake.toml` is NOT in the MSVC 2002 image. This is the
  DX8-header probe's answer, arrived at by accident: the headers compiled (0 `error C`) but the
  library is absent, so the link cannot succeed. Nothing in GameActionAttack is involved.
- `GUILoading`: the INSTALLED run-all-tests payload predates the T-007 land. The repo table
  (`devenv/scripts/run-all-tests.nix`) already lists `SkinFragments.cpp` + `SkinTable.cpp`; the
  store script does not. Compiling the GUILoading row from the REPO table by hand: **exit 0**.
  Stale payload, not a regression.
- `app-boot`: consequence of the first (no `build/F3.exe`).

My files were still verified under MSVC 2002 directly: `error C=0`, and
`build-msvc/CMakeFiles/F3.dir/src/F3/GameActionAttack.cpp.obj` exports
`?HitAngleWrapFlag@F3@@YAEME@Z` and `?ReactionAnimFor@F3@@YAH_N00E@Z` (`_N` = bool, so the
struct/class mangling trap did not bite). `cppcheck-openvb`: zero findings on my two files.

### Acceptance grep caveat

The row's grep `"2.356\|3.pi/4\|135"` can never pass while citing the legitimate EA **0x471357**,
which contains "135" as a substring. All prose references to the deleted rule are now spelled out
("three pi over four", "no arc-threshold constant"), and a tightened grep
`2\.356|3 ?[*/] ?pi/4|135\.0f?\b` returns **nothing** (exit 1). Recommend the row use the
tightened form - an EA is not a threshold.

## Acceptance

- No substituted arc rule remains anywhere in `src/F3/GameActionAttack.*`. Grep with
  `2\.356|3 ?[*/] ?pi/4|135\.0f?\b` (expect exit 1 / no matches). **My original pattern**
  `"2.356\|3.pi/4\|135"` was unsatisfiable: it also matches the legitimate EA **0x471357**.
  Implementer-C tightened it; recorded so nobody re-widens it., and no
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
