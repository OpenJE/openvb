# 2025-10-20 — T-010a answer: the "from-behind" test is NOT a 3π/4 sector (session 40)

Target: ORIGINAL_REFERENCE. Read-only. Research reply to the T-010a dispatch (assignee
HappyWolf/implementation side — this file is the routed-fact they asked the research lane for).
Falsifies the reimpl's substituted rule and hands over the real structure.

## 1. The guessed rule is dead on arrival

`|angle| > 3·π/4` **cannot** be the shipped rule:
- `ds:0x6cb56c` (the first compare constant, used ~10× app-wide) = **0.0f**, not π.
- The three "sector" constants are **360.0, 180.0, −180.0 DEGREES** (raw-read floats at
  0x660820/0x660824/0x660828) — full-circle wrap values.
- There is no 2.356 (3π/4) or 135.0 constant at any of these sites. The guess has no
  numeric source; delete it.

## 2. Actual shipped structure — `GameEffectDamage::Apply` @0x45c240..0x45c2e4 (raw)

```
inputs : three angles (esp+0x38/0x3c/0x40 slots): negated Δeuler components ×0.0174533
         (π/180 → radians) @0x45c251/261/271
helpers: call 0x47272B (out ptr edx=esp+0x70)  ; builds the target-vs-caster transform
         call 0x471357 (3 out ptrs esp+0x64, +0x3c, +0x40 region) ; reduces it to (X,Y)
angle  : fld[esp+0x40]; fld[esp+0x38]; fpatan      ; atan2(Y, X)   @0x45c294-29c
test chain (x87, result = BYTE flag at [esp+0xF]):
   fcom   0.0      @0x45c29e  ; sign test
   fcom  −180.0    @0x45c2ab  ; only on one branch
   fcom   180.0    @0x45c2b8
   fcomp  360.0    @0x45c2c5
   ⇒ flag := 1 on THREE paths (0x45c2b6-je-target, 0x45c2d2, 0x45c2db);
     flag stays 0 otherwise. Provable reading: the chain is a RANGE test
     "is angle ∈ (−180, 180] (mod 360)" implemented via wrap comparisons — i.e.
     signed-angle OUT-OF-RANGE ⇒ flag=1. (The radians-vs-degrees ambiguity of the
     fpatan result is the one open sub-question — see §3.)
consumer: flag feeds the post-@0x45c2e4 block: target vft[+80] gate, then the
     damage-type switch @0x45c425 (effect+0xA8) whose per-case second-roll sites
     0x45c44f/0x45c4a5/... read [esp+0x27] — the byte whose window overlaps the
     flag slot region — before hurt-message ids (canonical-adjacent: sess 22 §2b,
     sess 30 §1).
```

## 3. What this settles vs. what remains

- SETTLED: constant values; the compare-chain SHAPE (0/−180/180/360 in that order,
  byte-flag output); fpatan-based angle derivation; consumer gate order. The 33/34-vs-37/
  38/39 pick is driven by THIS flag + the damage-type switch — not a threshold on the raw
  angle.
- REMAINING (implementation-side or one more research hour): whether 0x471357's outputs are
  already degrees (making the ±180/360 compares meaningful range checks — most likely, given
  the π/180 pre-scale and that comparing radians to 180 would be vacuous) — verifying
  0x471357/0x47272B as Euler-triangle→angle helpers answers it. If degrees: the rule is
  exactly `flag = (angle ≤ −180 || angle > 180)` after atan2 — a **wrap detector**, which in
  gameplay terms is the classic "target's facing vs incoming direction sign flip", NOT a
  "behind arc". A behind-arc model would need a per-attack threshold; none exists.
- The reimpl's anim-pair selection should therefore be re-derived from the flag consumer
  (byte at esp+0xF → which of the hurt-message ids), NOT from any angle threshold.

## 4. For the coordinator
This row's premise ("from-behind sector floats") was itself the guess — the floats are a
wrap/range test. Flagged per standing rule 3 rather than coded around. HappyWolf's next step
is the consumer trace in Apply's tail, which they own; the numbers here are final.
