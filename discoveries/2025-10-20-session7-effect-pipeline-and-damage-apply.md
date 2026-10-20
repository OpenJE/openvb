# Session 7 — effect pipeline CLOSED end-to-end + GameEffectDamage::Apply full body

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Extends `2025-10-20-session6b-effect-node-dispatch.md` and the parallel
`2025-10-20-gameeffect-damage-anatomy.md` (fills their two open items: pump caller + Apply body
0x45c420+). Also settles the +0x20/+0x24 health question with a decisive new consumer.

## 1. The pump chain (was: "pump-caller open")

```
JE::Entity::meth_0x5506c0(effect)                 @0x5506c0   ← engine per-entity evaluation step
  0x55a3a0(effect, Entity.gap1AC[48])              ; bind/set current-tick context (Entity-side)
  0x55a460 / 0x55a4b0                              ; state prep
  GameEffect::meth_0x55ae10(effect)                ; THE TREE WALK @0x55ae10 (recursive)
  if effect->vft[0]() done?  -> meth_0x55a570 (remove/fire)
  else -> queue into Entity.mbr_0x2E0 list (0x54ea00) ; "still pending, re-check next tick"
```

- Tree walk `GameEffect::meth_0x55ae10`: guards `Effect+0x24` (completed flag — if set, return);
  calls **vft slot +0x14 (idx5) "Evaluate"**; on true: sets `+0xA=1`, recurses into the children
  list `mbr_0x1c` (node payload `node[2]`), collects finished children via vft[0], then removes
  each via `sub_55A590` + `meth_0x55a570`.
- External callers of the walker: `GameEffect::virt_meth_0x4626a0` (slot +48) @0x55aec4
  (recursive), `GameActionEquip::meth_0x55af80` @0x55af9a, `JE::Condition::virt_meth_0x5a8e00`
  @0x5a8e87 (base engine Condition path), `JE::Entity::meth_0x5506c0`.
- **Server-mode gate:** `GameEffect::virt_meth_0x462620` (GameEffectDamage slot10 calls it first)
  @0x462633 THROWS `"Invalid attempt to retrieve world server instance."` when the server flag is
  clear. ⇒ in the shipped client-mode demo, pumping an effect tree throws; combined with the
  client-skip in the EFDT/EFCD parser (session 6b) the whole EFTR pipeline is confirmed **inert
  in demo play**. Only tree *construction* (ctor defaults + skipped payloads) happens.

## 2. Evaluate (GameEffect idx5 @0x462600) is not the no-op stub — correction

The trampoline target and the Evaluate slot are *different* dispatches that happen to share bytes:
- **EFDT/EFCD body reader** (called via the `cls_0x55a280` trampoline with the *resource* args):
  `GameEffect::virt_meth_0x462600` @0x462600 = `net::server::virt_meth_0x5a0bb0` = **no-op stub
  `return 1`** (session 6b stands: payload bytes never parsed).
- **Pump Evaluate** (called by the tree walk): `GameEffect::virt_meth_0x462620` @0x462620 —
  server-gated; body: `Effect+0x90 = Entity+0x2A0 (via [effect+0x48], the victim) + clock`,
  where the clock comes from **JE::World-vb table (0x668494) slot +116 =
  `GameWorld::virt_meth_0x470070`** = `*(JE::World − 0x38C)` (time-source object; neighbor of
  the id-counter pair at −792/−808) then its `vft[+12]()`. *(First draft of this section keyed
  the slot against the wrong table and mis-called it PackState — corrected.)*
  ⇒ `+0x90` = **scheduled-fire time** (absolute), `Entity+0x2A0` = the entity's own effect
  delay/interval value. Damage fires when the time is reached: GameEffectDamage::virt_0x45c6b0 =
  set Effect+0xB=1 → 0x462620 (arm time) → if armed → Apply sub_45C140.

## 3. GameEffectDamage::Apply (sub_45C140) — full decode (0x45c140..0x45c667)

Order of operations (victim = `[effect+0x48]`, dmg = `[effect+0x9C]`):

1. `victim->Entity-vft[+0x50]` (=Entity::virt_0x549030, `state byte Entity+0x1E0 == 7`) →
   if already DEAD/dying → return 0 (no double-damage).
2. `dmg==0` → return 0. (And per the stub reader, `+0x9C` is only ever the ctor 0 ⇒ inert demo.)
3. **HP update @0x45c1a3:** `SetCurrentHealth(victim, victim->[+0x24] − dmg)` via
   `F3::...::meth_0x4629c0` @0x4629c0: `mbr_0x24 = min(param, mbr_0x20); return old`.
   → **DECISIVE for the reimpl naming question: GameEntity+0x20 = MaxHealth, +0x24 =
   CurrentHealth.** vft[0] (0x4080c0 → +0x20) = GetMaxHealth; +0x24's only readers are Apply and
   the alive-gate helper `GameEntity::virt_meth_0x467a10` (`Entity::virt_0x549030 || +0x24 <= 0`
   @0x46298f — i.e. death check = `state==7 || curHP<=0`).
4. If `[effect+0x10]` (attacker entity id) non-zero: resolve attacker, compute 3D delta
   victim−attacker (Entity-vb +0x1EC/0x1F0/0x1F4 world position triple per the reimpl's Mover
   excerpt; plus +0x1F8-style facing), negate components ×`flt_649C50` (radian scale),
   `fpatan` → hit angle; `var_B1` = "hit from behind/sector" flag from two float thresholds
   (`lpSrcStr_006cb56c`, `flt_660820/824/828` ≈ ±π/4·k — *(inferred exact sectors)*).
5. **Hit-reaction animation** @0x45c30c (skipped if `[effect+0xAD]` set; also requires victim
   still alive per vft[+0x50]):
   - victim moving (`Entity-vb+0x3A6` byte — the reimpl's `moving@+0x3A2` family):
     anim = `33` (front) / `34` (back)
   - else: `[effect+0xAC]` byte: set → `38/39`, clear → `37/38` (front/back by same flag)
   - queued via `GameEntity::QueueAnimEvent_4673d0(victim, anim, 0)`; then `[effect+0xB]=1`.
6. **VFX .veg pick** — switch on `[effect+0xA0]` (damage type 0..5) with `[effect+0xA4]`
   (material) sub-cases @0x45c37a-0x45c3ed:
   | +0xA0 | .veg |
   |---|---|
   | 0 | `+0xA4==5 → FX_HitBloodHighVelocity_01.veg`, `dmg>0 → FX_HitBloodStandard…`, else `FX_HitRicochetMetal_01.veg` |
   | 1,3,4 | blood (HighVelocity iff `+0xA4==5` else Standard) *(per case table)* |
   | 2 | `FX_HitElectrica…` (electric) |
   | 5 | `+0xA4: 1→FX_HitFire_01.veg, 2→blood-std, 3→FX_HitLaser_01.veg` |
   Spawned through `net_critical_section::meth_0x5512b0(victim, name, 0, 1)` @0x45c420.
7. **Sound/voice idx** — switch on `[effect+0xA8]` @0x45c42d (8 cases; ctor default **9 →
   default case**, i.e. in demo data the victim-voice roll is the default branch): each case
   `sub_48CD00(0,1)` coin-flips into small anim/sound id sets (1/2, 10/11, 5/6, 35/38, 20/23,
   16, 4..8…) resolved against `[attackerEntity+0x164]`; passed to `cls_48dc90::meth` (VFX/
   audio spawn helper). *(inferred: death-scream/hit-grunt variant ids)*
8. **Script event** @0x45c561: `sub_55A220(effect, victimEntity, "OnDamage", payload={dmg,
   attackerId, DT}, 4)` — per-entity `OnDamage` script hook dispatch (JE::Res-style event bus).
9. **Server sync** @0x45c591: `GameServer::meth_0x46d020(server, victimNetId(Entity-vb+0x1E0
   dword idx 480 = Entity+0x1DC… actually [victim+vb+0x1E0] = Entity+0x1DC per the reimpl base),
   dmg, …)`.
10. **Death conversion** @0x45c5b9: re-check vft[+0x50] (now dead):
    - `attacker = worldServer 0x66856C vft[+8]` → idx2 = **action factory** (`0x46f260` @0x668574,
      `typeId=0` → Action; the same factory GameEntity ctor uses @0x4679b0)
    - `__RTDynamicCast(Action → GameActionDie)`, then field init:
      `die+0x1C = [effect+0xA0] (DT)`, `die+0x28 = byte([effect+0xAC]+3?)`, `die+0x20 =
      [effect+0xA4] (material)`
    - queue onto victim: `net_critical_section::meth_0x550720(victimEntity, dieAction, 0)`
      @0x45c626 → the entity action pipeline (mode-5 queue that the ctor armed via
      `Entity::virt_meth_0x550750(entity, 5)` — session 1 note "mode 5 at construction" now has
      its payoff: **mode 5 = "accepts queued actions"**).
11. return 1.

## 4. Effect type space (was: "engine CreateEffect table open")

`GameWorld::virt_meth_0x46fa40` (0x66856C slot +40) + `JE::World::CreateEffect_544bf0`:
- **type 0** → base `JE::Condition`-effect, size **0x70**, ctor 0x5a8d30 (engine container node)
- **type 1** → `F3::GameEffectDamage`, size **0xB0**, ctor 0x45c040
- **anything else** → `FatalError "World::CreateEffect() - Unknown effect type %d."`
So the shipped binary registers exactly ONE concrete effect type; trigger-type classes
(`EffectTreeCircleTrigger` @0x6aa1f0, Ring/SemiCircle @0x6a9920/0x6a9d88, base EffectTreeTrigger
@0x6a7218 — all with slot10 "execute" bodies 0x5b22e0/0x5b1fe0/0x5b2100/0x5aa530) exist in the
engine vftables but are NOT reachable from the F3 factory — dead template classes in F3.exe.
*(DB: Effect base vftable @0x693c8c; slot2 = the copy-from-template node @0x55ac10, which
clones a source tree via the factory `vft[+40](*node+12)` + slot10 copy-apply @0x55ad3c —
relevant if the reimpl ever clones entity effect trees.)*

## 5. Net implications for the reimpl (actionable)

- Keep classIdTag (+0x15C) as-is; nothing new required from the pump since it is server-gated.
- If you implement Reload/Attack damage application, the **HP trio** is:
  `MaxHealth=+0x20, CurHealth=+0x24, SetCurHealth clamps to Max (0x4629c0), IsDead =
  (Entity.state@+0x1E0==7 || CurHealth<=0)`. Damage apply order: gate→clamp HP→(angle)→
  QueueAnimEvent(33..39)→VFX .veg by DT table→OnDamage script event→server sync→on death create+
  queue GameActionDie via action factory.
- Hit-reaction anim ids **33/34 (moving front/back), 37/38/39 (standing)** — feeds the
  QueueAnimEvent table already documented in
  `2025-10-20-gcre-versions-gwam-queueanimevent-table.md`.
- The EFTR pipeline can remain unimplemented for demo parity: construction no-op (stub reader) +
  pump throws in client mode. Document as intentional.

## 6. Still open (next session candidates)

- ~~Evaluate time source~~ → **CLOSED** by `2025-10-20-session12-serverupdate-ondeath-flip.md`
  §5: the returned member is the **JE::Gregorian** calendar singleton; see that doc for the
  raw slot arithmetic and the wall-clock companion `JE::TickMilliseconds`/dword_70C2B8.
  (Historical, kept for the audit trail:) **NARROWED (same-session follow-up)**: the `(*(*v3+116))(v3)` call in
  0x462620 dispatches to **`GameWorld::virt_meth_0x470070`** (JE::World-vb table slot +116):
  `return *(this − vbdisp − 860)` — a pointer member at **JE::World−0x38C** (neighbour of the
  id-counter pair at −792/−808, session 5 §6). Its `vft[+12]()` = current-time read on that
  object. Remaining: identify the −0x38C object class (clock/time source) — one more slot walk.
- `GameEntity+0x18` consumers (only GENT dword[4] provenance known).
- `Entity+0x1E0 == 7` state writer — who sets the dead state (presumably GameActionDie::Execute)?
- Weapon `[entity+0x3B4..]` ammo prototype *registry* backing (`GameWorld::meth_0x46fac0` table
  storage — caliber/modifier matrix location inside GameWorld).
- Exact float thresholds at `lpSrcStr_006cb56c`/`flt_660820-828` for the from-behind sectors.
