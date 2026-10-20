# 2025-10-20 — T-010a residue ANSWERED: the +0xAC/+0xAD "wrap flags" are volley-batch reaction-control bytes, not angle-wrap state

Author: FastArrow (research). Residue of T-010a, picked up per coordinator dispatch
(post-1ac0390). Target: ORIGINAL_REFERENCE. Read-only. Raw citations on every claim.

## 1. Where the bytes get written (they were "ctor-zeroed, never written" in the static pass —
because the writers address them as decimal 172/173 through a different base)

`GameActionAttack::meth_0x403770` = the volley/hit-list FINISHER: iterates the pending-hit
list at `action+0xC0` (list head sentinel; nodes chain via +0), and for each node:
- `eff = CreateEffect(1)` via **srv-view vft[+40]** (`0x46FA40`, the gameworld-doc §1.1 row —
  first production use found!) → fresh GameEffectDamage (0xB0);
- string/flag setters 0x55A3E0(idx,ptr) idx 0..3 + 0x55A420(1): weapon-or-skill names
  (idx1 picks *record+0x20 string vs record-attacker-name by **record byte +0x17 = unarmed
  flag**), source/target string slots;
- numerics `*(eff+0x9C/0xA0/0xA4/0xA8) = rec[+0x08/+0x0C/+0x10/+0x14]` (damage, DT, subDT,
  amount) — the session-7 "+0x9C..+0xA8 from the record" line, now with the RECORD offsets;
- **the branch that answers 10a** (@0x4038D7..0x4038F7, decimal fields = 0xAC/0xAD):
```
pre-loop @0x4037D2..0x403804:
    local_11 = ( count(list nodes with byte[+0x19] set) >= 2 )   ; ≥2 "marked" volleys
if (thisRecord == list->first) {          ; cmp esi,[list-head+0] @0x4038E2
    eff[+0xAC] = local_11;  eff[+0xAD] = 0
} else {
    eff[+0xAD] = 1          ; (+0xAC stays ctor-0)
}
```
- then `Entity::meth_0x5506C0(victim, eff)` (session-7's "evaluates it on the victim").

## 2. How Apply consumes them (re-read of 0x45C140 raw + session-7 §5)

```
@0x45C30C: if (eff[+0xAD]) → JUMP PAST the entire hit-reaction block   ; non-first hits: silent
            (reaction requires victim alive per vft[+0x50] — session-7)
moving victim (E+0x3A6): anim 33 (front) / 34 (back) by var_B1
@0x45C336: else stationary: eff[+0xAC] ? (38/39 by var_B1) : (37/38 by var_B1)
```
`var_B1` = the ANGLE sector bit computed inside Apply itself from the victim−attacker delta,
`flt_649C50` scale, `fpatan`, and the threshold constants (HappyWolf's 10a main answer).
**+0xAC/+0xAD carry NO angle information** — the "wrap flag" label was a misnomer born of
session-7's *(inferred)* wording; the real angle/sector machinery is local and already
covered by 10a's constants answer.

## 3. Net semantics (the sentence that unblocks the model)

A multi-record volley (and the same code path serves the multi-target/AoE batch, since the
list is per-finisher) produces: FIRST victim hit plays a reaction — pair
{37,38} normally, upgraded to {38,39} iff ≥2 volleys in the batch carry the +0x19 mark;
every subsequent hit in the batch applies damage, VFX and everything else **without any
reaction animation**. So reaction suppression is FIRST-WINS per batch, and +0xAC is a
"heavy batch" selector bit, nothing to do with facing. Anyone modelling one scalar
part/reaction per action mis-models volleys (coordinator's T-011 note #1 stands — this is
the same phenomenon on the reaction side).

## 4. Bonus: sub-attack RECORD layout (rec = node payload; feeds §1) — hand to T-011

| rec off | field | evidence |
|---|---|---|
| +0x08 | damage int | 0x4038B3 → eff+0x9C |
| +0x0C | damage type | 0x4038BF → eff+0xA0 |
| +0x10 | sub-DT/extra | 0x4038BC → eff+0xA4 |
| +0x14 | amount/scalar | 0x4038CE → eff+0xA8 |
| +0x17 | unarmed flag (string-source select) | 0x40386B |
| +0x19 | batch-mark flag (≥2 ⇒ +0xAC reaction upgrade) | 0x4037F0 (counted); WRITER not pinned here |
| +0x1C/+0x20 | string/ptr slots (attacker-name / skill-string) | 0x55A3E0 idx1/2 sources |
| **+0x20** | called-shot body part for UNCALLED volleys (uncalled-shot selector writes here @0x404989→`*(rec+0x20)`) | matches coordinator T-011 list item #1 |
Also the volley-series vectors: see called-shot-part-second-source.md §5 (action+0x7C/+0x88
triplets) — the list iterated here IS those vectors' consumer.

## 5. Caveats / not claimed
- The +0x19 mark's writer not traced this pass (it is set during volley RESOLUTION, i.e.
  inside the sub-attack loop that builds the list — likely crit/multi-hit classification;
  name it "marked" not "crit" until pinned).
- Whether AoE secondary victims ride a SEPARATE finisher call (one list per victim) or the
  same list was not proven here — the per-record `Entity::meth_0x5506C0(victim…)` reads
  victim from record context (local_10 = per-node resolve of [rec]) ⇒ reaction-first-wins is
  per-LIST, not per-victim. If lanes observe second-victim reactions in the original, look
  for a second list, not different flag semantics.
