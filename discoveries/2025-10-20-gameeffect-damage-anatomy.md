# 2025-10-20 — GameEffect/GameEffectDamage anatomy (session 7)

Target: ORIGINAL_REFERENCE. Read-only. Partial-open of the combat-formula door
(session 6 §8 follow-up). The full `Apply` body (0x45c140, ~0x500 bytes) is only
partially analyzed here — this note is a field map + what each observed use implies.

## 1. Base-class split of the 0xB0 damage object (now evidenced)

`sub_462470` = **GameEffect ctor** (called by GameEffectDamage ctor 0x45c040):
```
JE::Effect::ctor_0x55b230(this, type)          ; engine Effect = first 0x48 bytes
zero dwords at 0x48,0x4C,0x50,0x54,0x5C,0x60,0x64,0x68,0x6C,0x70,0x74,0x78,0x84(skip),0x88,0x90,0x94  (this+72..148 in the decompile)
[+0x84] = 1
vftptr  = GameEffect_Effect_vftable_66535c
```
So: `JE::Effect` body = 0x00..0x47 (72B — the CreateEffect new(0x70) covers Effect+Condition
extras; engine-base allocation stays 0x70); `GameEffect` adds 0x48..0x9B; `GameEffectDamage`
adds 0x9C..0xAF (0x14 bytes: +0x9C,+0xA0,+0xA4,+0xA8=9,+0xAC byte,+0xAD byte).

Session 6 §8's "(inferred) split" now: Effect 0x48, GameEffect 0x54, Damage 0x14 = 0xB0 ✓.

## 2. Damage object field map (observed uses inside `Apply` sub_45c140 + slot-10 0x45c6b0)

| off | seen as | evidence | meaning so far |
|-----|---------|----------|----------------|
| +0x0B | byte =1 set before apply | 0x45c6b3 (slot10), 0x45c35f | "effect started/activated" flag (on Effect base) |
| +0x10 | ptr arg to 0x54be10 lookup → reads [res+4],[res+8],[res+0xc] (float x / int / int) | 0x45c1b1..0x45c1d7 | source entity-ish handle for **position/orientation** |
| +0x48 | entity ptr, used with vb-adjust then `vft[+0x50]` twice as gate | 0x45c171, 0x45c2e4 | target/caster entity; gate call = "can-damage?" check; also `[+0x48 → +0x24]` read in 0x45c1a6 |
| +0x84 | =1 at GameEffect ctor | 0x4624c4 | enabled/state (base) |
| +0x9C | compared !=0 then consumed; `cmp [esi+0x9c],ebx` | 0x45c190 (ebp), 0x45c39a | **damage amount** (0 = no damage event) |
| +0xA0 | dword loaded in apply | 0x45c374 | (candidate: attacker strength / damage type param) *open* |
| +0xA4 | `cmp …,5` + switch-ish loads | 0x45c391, 0x45c3bf | **damage-type enum 0..5** (DT: 6 types — matches Fallout-style DT set) *(inferred)* |
| +0xA8 | = 9 constant set in ctor | 0x45c073 | effect kind id (9=damage in the effect-type list; note the factory used 1 — two different numbering spaces, flag for lockstep) |
| +0xAC | byte flag tested | 0x45c336 | (candidate: ignore-armor?) *open* |
| +0xAD | byte flag tested | 0x45c30c | (candidate: crit?) *open* |
| vft+40 (slot10) | Execute: set +0xb=1, GameEffect::virt_0x462620 gate, then Apply | 0x45c6b0 | damage apply entry |

## 3. Geometry inside the first third of Apply (partial)

```
[+0x10] → 0x54be10 lookup → {a4: float, i8: int, iC: int}     ; some model/space query
target vb-entity +0x1F4/+0x1F8/+0x1FC: three float fields     ; target position? (yaw/pitch? xyz?)
deltas = fld(f)+fsub(entity+0x1F4/0x1F8/0x1FC region)
each delta × 0.0174532923 (= π/180, float const at 0x649c50)  ; degrees→radians on each axis
calls 0x47272b / 0x471357 (vector helpers), fpatan → yaw atan2
```
So Apply starts by computing a **direction/angle set from a ±180° folded atan2** between
caster-ish fields (+0x10 lookup) and target Entity-vb floats @+0x1F4..+0x1FC — a hit-direction
computation (probably for directional armor/critical-multiplier tables later in the body).
The remainder (0x45c420..0x45c644) is unanalyzed: expected DT-vs-armor math and the HP
decrement. Next session if combat is the reimpl's next milestone.

## 4. Notes for the reimpl

- GameEntity vb float cluster at +0x1F4/+0x1F8/+0x1FC sits exactly in the Mover-excerpt range
  your `SpawnedEntity` models (pos@+4, yaw@+0x1F0) — **yaw@+0x1F0 vs floats@+0x1F4..: check
  whether your excerpt's yaw is actually pitch and +0x1F4..0x1FC = rotation euler** (this
  function loads all three as a group).
- The damage-type enum at +0xA4 with bound `5` is the first hard number for the combat model;
  names require the ITM/effect-file writers (nothing populates +0x9C/+0xA4 in the ctor — they
  come from the effect tree data — which, per session 6 §4, the demo *skips reading*).
  ⚠ Consequence: in the shipped demo, `Apply` always sees ctor defaults (damage=0 → early
  `return 0` at 0x45c19a). **The whole damage pipeline is inert at runtime** unless another
  code path fills the fields (e.g. scripted effects via GameScripting — watch there next).

## 5. Queue

1. Rest of sub_45c140 (the actual math block 0x45c420+).
2. Who else constructs/sets GameEffectDamage fields (xrefs to 0x45c040 and to +0x9C/+0xA4
   stores) — expect GameScripting / projectile hits.
3. `GameEffect::virt_0x462620` gate semantics.
4. (older) Entity+0x15C readers; `.por`; JE::World layout.
