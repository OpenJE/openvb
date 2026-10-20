# 2025-10-20 — To-hit formulas COMPLETE: ranged/melee twins + called-shot table (session 23)

Target: ORIGINAL_REFERENCE. Read-only, native IDA. Closes session 22 §3.2. The full VB2-style
to-hit math, verified in both branches; combat math is now 100% documented except the
victim-side message handler (deliberately delegated to runtime).

## 1. Dispatcher `GameWeapon::CalculateToHit_0x413560`

```
null-check (attacker, target, cached-ammo-proto) else "Recieved a NULL pointer in critical data."
ammoProto->vft... [+8] flag → RANGED (0x413150) else MELEE (0x412fe0)
```
(weapon+0x265 = cached ammo prototype via `sub_408110(caliber=weapon+0x3B4, mod=weapon+0x3B8 (dword-index ×4 fix))`
— the session-9 ammo registry; FatalError "Unable to locate ammo. Caliber:%d, modifier:%d".)

## 2. Ranged (0x413150)

```
base = modeRec[+0x28]                      ; attack-mode to-hit bonus
     + attacker.GetTotalSkill(MELEE if modeRec[+8]==2 (throwing) else FIREARMS)
     + ammoProto[+0x3C4]   if modeRec[+0x1C] (uses ammo)     ; AMMO ACCURACY MOD
range = throwing? attacker.GetDerived(THROWING_RANGE)
                  : weapon.GetEffectiveRange_0x412f50(modeRec)
dist  = 3D Euclidean(attacker.pos, target.pos)        ; pos = entity-view +0x4/+0x8/+0xC floats
penalty = (200 / ((range-10)/5)) * max(0, int(0.2*dist - 2))
armor   = target.GetDerived(EVASION) + (part!=9 ? CalledShotTable[part] : 0)
chance  = clamp(base - armor - penalty, 0, 95)        ; ★ cap 95, floor 0
```

## 3. Melee (0x412fe0)

Same base (MELEE skill always) + ammo mod; **out-of-reach ⇒ 0** (`dist > GetEffectiveRange`);
no distance penalty; armor/evasion + called-shot identical; clamp 0..95.

## 4. Called-shot part table `dword_0x64F858` (raw)

```
idx : 0   1   2   3   4   5    6   7   8
mod : 10  40  40  40  40  60  120  60  <0x7F7F7F7E garbage/sentinel — treat as unset>
part 9 = "no called shot" (formula skips table)
```
⇒ six meaningful parts; difficulty order: idx0 easiest (10), torso-ish 40s, idx5/7 60,
idx6 hardest (120). Part INDEX semantics (head/limbs order) still to pin — likely via the
GWAM/called-shot writer (reimpl's session) or the hurt-message ids from session 22 §2b.

## 5. Attack-mode record (modeRec) offsets — consolidated, verified so far

| dword | byte | meaning |
|-------|------|---------|
| [2] | +0x08 | attack style (2 = throwing; else firearm/melee split via weapon) |
| [4] | +0x10 | damage type index (0..7 → DT column) |
| [7] | +0x1C | primary ammo cost (nonzero ⇒ ammo mods apply) |
| [9] | +0x24 | flat damage bonus |
| [10] | +0x28 | to-hit bonus |
| [12] | +0x30 | attack count/type (1 = single → no continuation) |
| [14]/[15] | +0x38/+0x3C | damage dice min/max |
| — | +0x44 | ammo proto ptr (used by damage fn as [24]→rec) |
| [20] | +0x50 | ammo extra (passed as param to formulas) |
| [+68] | | ready timestamp (weapon attack interval) |

## 6. Named accessors — FULL ENUMS DUMPED (closes §8.1 same session)

`GameCreature::GetTotalSkill_0x409830(F3::Skill)` — base from skill-array obj `cls_0x409820
::meth_0x409820(skill)`, then per-skill SPECIAL combos (classic total-skill math):

| skill | formula (base = rank points) |
|-------|------------------------------|
| FIREARMS, MELEE | **base + 50** |
| UNARMED | END + 2·STR + base |
| BARTER, PERSUASION | 3·CHA + base |
| DECEPTION | INT + 2·CHA + base |
| MECHANICS, MEDIC | PER + 2·INT + base |
| OUTDOORSMAN | END + 2·PER + base |
| SCIENCE | 3·INT + base |
| SNEAK, SECURITY | AGI + 2·PER + base |
| STEAL | PER + 2·AGI + base |
| other | debug "UNKNOWN SKILL: %d", returns rank |

`GameCreature::GetDerivedAttribute_0x409a60(F3::DerivedAttribute)`:

| attr | formula |
|------|---------|
| ACTION_POINTS | AGI + 20 |
| CARRY_WEIGHT | 25·(STR+1) |
| COMBAT_SEQUENCE | 2·PER |
| CRITICAL_CHANCE | LCK |
| EVASION | **[creature+0x7FC]** (stored, not computed — IDA's "maybe_agility_0x7fc" is
  really the evasion stat) |
| HEALING_RATE | END |
| MELEE_DAMAGE | STR |
| PERCEPTION_RANGE | 3·PER + 40 |
| PERK_RATE | 3 |
| SKILL_RATE | 2·INT + 5 |
| THROWING_RANGE | 3·STR |
| HIT_POINTS | `AP/6000 → 1.0/(AP/6000)` = **6000/AP** (!) — smells like a shipped bug or a
  float trick; VERIFY AT RUNTIME before porting (reimpl: log actual HP for known AP) |
| RADIATION/FLASH/BIOLOGICAL_RESISTANCE | **0 — unimplemented in this build** |

**SPECIAL layout nailed** (from the case bodies): GameCreature STR=+0x760, PER=+0x764,
END=+0x768, CHA=+0x76C, INT=+0x770, AGI=+0x774, LCK=+0x778, (+0x7FC = evasion storage).
This completes the +0x760..0x7FC area left open by session 2's creature census.

## 7. Ammo prototype stat block (finalized!)

`ammoProto` (registry 0x408110 result, GameAmmo*): **+0x3B8 dmg-min bonus, +0x3BC dmg-max
bonus, +0x3C0 DT penetration, +0x3C4 to-hit bonus** — the four ammo modifiers, all consumed
exactly as shown above. (Session 9/10 ammo registry: fully wired now.)

## 8. Remaining combat items

1. ~~Skill + derived-attribute ENUM dumps~~ CLOSED (§6).
2. Called-shot index→body-part mapping (+ slot 0x7FC evasion source: who writes it).
3. Victim message handler (HP write) — delegated to reimpl runtime (session 22 §3.0).
4. HIT_POINTS formula sanity via runtime (6000/AP looks wrong-on-purpose).
