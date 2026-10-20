# 2025-10-20 — Attack OnServerUpdate state machine + damage formula (session 21)

Target: ORIGINAL_REFERENCE. Read-only, native IDA. Closes the combat thread that sessions
6/7/11/20 circled: `GameActionAttack::OnServerUpdate_0x404320` → sub-attack rolls →
`CalculateAndQueueDamage_0x404060` → `GameWeapon::CalculateDamage_0x413310` (the formula).

## 1. `GameActionAttack` object (0xC8 bytes) — field map from OnServerUpdate

| off | field (working names) | evidence |
|-----|----------------------|----------|
| +0x11 | engine Action base (entity ptr at +0x14 region; `*this->gap11` = owning entity everywhere) | RTTI casts |
| +0x14 | actor GameEntity* / +0x18 actor GameCreature* | 0x404392/0x4043fc |
| +0x20→+0x24 | weapon *id* → resolved `GameWeapon*` (RTTI) | 0x404446..0x40446b |
| +0x30 | attack-mode record ptr (mode struct: [+12] attackCountType, [+16] damageType 0..7, [+20] ammoExtra, [+24] ammoProto/rec, [+28] primaryAmmoCost, [+32] secondaryCost, [+36] flatDmgBonus, [+56] dmgMin, [+60] dmgMax, [+68] readyTimestamp) | throughout |
| +0x34→+0x38 | target id → `GameEntity*`; +0x3C target creature*; +0x40 target inventory* (when creature/usable) | 0x40440e..0x4045fd |
| +0x44 byte2 | "cheat/unlimited" mode (skips target-death/ammo checks, LABEL_36 direct) | gates 0x404410/0x40446e |
| +0x58 | ammo-deducted-this-volley flag | 0x4049ef |
| +0x5e | combat-anim scheduled time (QueueAnimEvent(7=ToCombat,0.0) returned duration + now) | 0x404841 |
| +0x60 b2 | do-sub-attack pending | 0x40487e/0x404a71 |
| **+0x65** | **next-sub-attack time (u32 dword)** — ★ DM-beat semantics corrected (see §4) | 0x4048e6..0x40492e, 0x404a48 |
| +0x68 high | "needs ResolveWeaponAndAttackMode" | 0x404560 |
| +0x6e | crit/extra flag (passed to formula) | 0x4040d4/0x4040fb |
| +0x70 | ranged flag (from resolve) | 0x40250a |
| +0x71 | unarmed flag (from resolve) | 0x402410 |
| +0x72 | melee range int (−1 = ranged) | 0x40256a/0x40250e |
| +0x73+3 | called-shot target part (9 = unset!) | 0x4025df/0x404983 |
| +0x78 | sub-attack series list (cls_0x401b00) | 0x4048b7 |
| +0x80 | next-attack pending flag | 0x404a62 |
| +0x84 | current sub-attack record | 0x404931 |

> ⚠ **ROWS +0x73+3 … +0x84 ABOVE ARE SUPERSEDED** — IDA display-skew; raw-verified region map
> (calledShotPart=+0x78; series = vector +0x7C/80/84; second timing vector +0x88/8C/90) in
> `2025-10-20-called-shot-part-second-source.md` §5. Header fix landed coordinator-side at 1ac0390.
| +0xac | target-of-choice list | 0x404967 |
| +0xbc | **damage-queue** (list keyed {casterNetId, weaponNetId}) | 0x404187 |

## 2. OnServerUpdate flow (0x404320, param_1=gameMs, param_2=clock)

```
if (client-side(mbr_bool_0x6c)) → tiny path, maybe finish, return
resolve actor/inventory/creature/target/weapon (RTTI-guarded); any fail → cancel(meth_0x401a10(1))
unless cheat-mode: target same-map else "Target is not on the same map. Cancelling."
             : target alive  else "Target entity is already dead."
ammo check   : "Out of ammo for this weapon." → cancel
if resolve-needed: ResolveWeaponAndAttackMode_0x40b380 → melee(range=mbr_0x72)/ranged
                   (+debug overlay strings, see §5) ; called-shot unset(9) → cancel+error
auto-move-to-range: if mbr_bool_0x6d && (dist > range || LOS vft[+144/+148] fails):
                    new GameActionMoveTo(0x48) → AddAction(actor)   ← range-clamp via MOVE, returns
first strike: when clock >= modeRec[+68] (weapon attack interval) && PrepareAttack_0x402680:
   PrepareAttack fail → new GameActionReload(0x3C) ctor_0x4071c0 → AddAction ← AUTO-RELOAD CHAIN
   else QueueAnimEvent(actor, 7 /*ToCombat*/, 0.0) → mbr_0x5e = ret duration
sub-attack series: on mbr_0x60: pop series (+0x78) into current (+0x84); if hitDelay(gapA1[0]):
   gap65 = now + meth_0x4012c0(...) else gap65 = now
when gap65 due (DM-forced, §4):
   while sub-attacks remain:
     hit? = meth_0x402260(action, mode, target)         ; to-hit roll
     called-shot(9): sub_401280(part, target)
     meth_0x403320(pre-hit hook)
     HIT  → CalculateAndQueueDamage_0x404060             ; §3 formula, QUEUES damage
     MISS → debug "SubAttack MISSED"
     ammo deduct (once/volley, guard +0x58): GameWeapon::DeductPrimaryAmmo_0x412b10(+28),
                                             DeductSecondaryAmmo_0x412b90(+32)
   multi-shot (modeRec[+12] != 1): set mbr_0x60 again (next volley), else:
   finish (meth_0x403770) unless continuing; then chain: new GameActionAttack(0xC8)
   ctor_0x403e40 + meth_0x403fa0(target) + AddAction     ; sustained-fire continuation
```

## 3. Damage formula `GameWeapon::CalculateDamage_0x413310` (the big one)

Args: (weapon, modeRec param_1, actor, target(cast GameCreature), critFlag a5=+0x6e,
calledShotPart param_5).

```
param_5 == 9 → "No body part was targeted." → return 0        ; called-shot must resolve
ammoBonus: if modeRec[+28]: ammoProto = cached weapon+0x424 else
           sub_408110(caliber=weapon+0x3B4 (×4 fix), modifier=weapon+0x3B8 (×4 fix))   ; reimpl session-6 lookup!
           FatalError("Unable to find ammo. Caliber:%d, modifier:%d") on miss
           dmgMin += ammo[+0x3B8]; dmgMax += ammo[+0x3BC]; DTpen = ammo[+0x3C0]
           clamp min ≤ max
raw   = rand(modeRec[+56] + ammoBonusMin, modeRec[+60] + ammoBonusMax)   ; sub_48CD00 = dice
raw  += modeRec[+36]                                                      ; flat bonus
DTrow = (mode==5||6) ? creature+0x7C8 : creature+0x7B0   ; two DT rows, 8 dwords each,
dt    = DTrow[ modeRec[+16] ]                            ; indexed by damageType 0..7
if (dt > 0) { dt -= ammoDTpen; if (dt<0) dt=0 }          ; AMMO PENETRATION
dmg = raw − dt/2 − rand(0, dt/2)                          ; stochastic armor: half flat, half rolled
return max(dmg, 0)
```

⇒ **DT model = subtract half flat + random up to half**; ammo type carries a penetration
modifier that subtracts from DT before halving. Damage types 0..7 exist; creature carries TWO
8-dword DT rows (+0x7B0 generic, +0x7C8 used for attack modes 5/6 — called-shot to limbs
candidate). `GameCreature::CalculateUnarmedDamage_0x409f90` is the unarmed twin (same shape,
uses actor's creature record instead of weapon).

## 4. DM-beat semantics — CORRECTION/refinement of session 20

`OnAnimEvent("DM")` writes byte +0x65=1, but +0x65 is the LOW byte of the **next-sub-attack
time dword** (`*this->gap65` u32, gates at 0x4048f5/0x404a5c `v47 != 0 && v47 <= param_1`).
Writing 1 ⇒ "due since t=1ms" ⇒ the very next OnServerUpdate tick fires the pending
sub-attack series. So DM = "anim keyframe says swing landed → release the queued sub-attacks
NOW"; to-hit + damage happen in OnServerUpdate, not in the beat. Session 20's table stays
useful; this is the precise mechanism.

## 5. Debug overlay & misc

- `cls_0x558740::meth_5583e0(msg, 0x80, 128, 0x80)` via `client+0x8C` — debug message
  overlay API (RGB? tuple). All OnServerUpdate/Resolve debug chatter enumerated in §2.
- "Damage for %d queued." — formula result debug line.
- Weapon attack interval = modeRec[+68] absolute clock; QueueAnimEvent(7) duration seeds
  the FIRST strike delay (mbr_0x5e) — the anim→gameplay coupling the reimpl pump needs.
- Entity netId slot used AGAIN four more times (caster/weapon/target/queue-key at +0x1DC /
  view+480) — slot 100% settled.

## 6. Queue update

1. ~~Who DRAINS the damage queue~~ **CLOSED below (§7)**.
2. To-hit roll `meth_0x402260` (skill/AC math) — quick twin of §3.
3. Standing: clip names, World trees, +0x34 mode writer, UnpackState cross-check (session 17 §3).

## 7. Damage queue DRAINED at attack-finish — full combat chain closed (same session)

`GameActionAttack::meth_0x403770` (the finisher, called from OnServerUpdate §2 tail) walks the
queued-hit list (+0xC0 head) and per entry:

```
1. world3rdIntf(sub_401130)->vft[+40](1)         ; = the SAME effect factory (session 6 §3)
                                                   ; ⇒ instantiates GameEffectDamage (0xB0) per hit
2. meth_0x55a360(eff, weapon/entity)              ; configure: weapon, source (+idx1/0/2/3 setters
3. meth_0x55a3e0(eff, idx 0..3, value) ×5        ;   on eff — idx1 = weapon OR +0x20 alt, 2 =
4. meth_0x55a420(eff, 1)                          ;   target-ish, 3 = 0)
eff+0x9C = payload.dmg      ★ the session-7 "Apply reads +0x9C (was 0 in demo)" field —
eff+0xA0 = payload.w1       ;   NOW explained: combat writes real values here at drain
eff+0xA4 = payload.w2       ;
eff+0xAC / +0xAD = flags ("last target of action" / multi-target marker)
5. JE::Entity::meth_0x5506c0(target, eff)         ; ATTACH effect to TARGET entity
```

Then list cleared (+0xC0 reset, count 0). ⇒ The attached `GameEffectDamage` enters the target's
updater-map machinery (session 19): ticked, `isFinished` via `cls_0x59f7d0`, expiry →
`vft[+40] Execute (0x45c6b0) → Apply (sub_45C140)` reading the now-populated `+0x9C` damage —
solving session 7 §4's "the whole damage pipeline is inert" caveat: it was inert *for EFDT
files* only; live combat effects get values via THIS queue instead. EFDT-created tree effects
remain payload-less by design.

Also resolves the session-5 §1/§6 confusion (`cls_0x55a280` trampoline): `vft[+40](type)`
returns the factory-created effect OBJECT — the wrapper typing was noise; both call sites
(EFDT tree push, combat drain) use the identical factory.

**END-TO-END COMBAT CHAIN (final)**:
```
cursor cmd 15/16 → sender id 91 → server 0x46de30: GameActionAttack + Entity::AddAction
→ OnServerUpdate: resolve weapon/mode, move-to-range, QueueAnimEvent(ToCombat),
  sub-attack series, DM-beat releases timing, to-hit (0x402260),
  CalculateDamage (rand(min..max)+bonus − (DT−pen)/2 − rand(0,(DT−pen)/2)),
  ammo deduct (DeductPrimary/SecondaryAmmo), queue {casterNetId,weaponNetId}→dmg
→ finisher: per queued hit create GameEffectDamage, set +0x9C dmg, attach to target
→ target updater-map tick → Execute → Apply (hit-direction atan2 prelude, 0x45c140 body)
→ HP state change (remaining detail: Apply's math tail 0x45c420+ and HP write).
```
