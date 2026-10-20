# 2025-10-20 — JE::Entity::Update anatomy: the four updater maps & effect lifecycle link (session 19)

Target: ORIGINAL_REFERENCE. Read-only, native IDA tools. Decompiled
`JE::Entity::virt_meth_0x550ad0` (0x550ad0, ~0x7B0 bytes; found via reader-attribution of
Entity+0x33C per session 18 §5 item 1). Names 4 of the Entity ctor containers and *merges the
effect pipeline (sessions 3/6/7/18) into the per-frame loop*.

## 1. Function identity

`Entity::Update(dtMs)` (thiscall, param_1 = delta milliseconds; `*0.001` → seconds everywhere).
Guard: `[entity+0x3A0]` byte (in session 18's placement-record block) must be set —
"entity active/placed". This is on the per-frame pump (matches reimpl's
`serverupdate-pump` doc: states 4/5/7 path `Entity::UpdateActive` 0x54c570 calls into it —
cross-verify slot).

## 2. Body sections (in order)

1. **Anim-event dispatch**: if model handle `[+0x160]`: `meth_0x549040` (current anim idx?) →
   `ModelInstance::meth_0x563eb0`; then iterate model's event list
   (`[+0x160]+0xC → +11 →` list) calling **`entity->vft[+41](child)`** per event node —
   the same event-sink surface as session 8's `dword_6F3960` step (vft+0x48 family). Two
   dispatch slots (+0x41 vs +0x48) — anim-arrived events vs scheduled ones; assign later.
2. **Water-snap**: non-Water entities (RTTI check vs `Water`) whose placement record
   `gap340[28..]` moved: octree query (`Map::GetOctree_56d960` → vft+136 clear?/vft+100
   query-kind-32/vft+92 nearest) for `Water` objects within +0.001 → `sub_5AB100(water,
   pos, ±0.01)` adjust — **entities auto-adjust Y to nearest water surface** (buoyancy!).
   Writes back the moved placement triple to gap340[28..36].
3. **Updater map #1 @ Entity+0x3C4** (sub_57C360 map): per node: value float @node+0x1C +=
   dtSec; `child = node+0x14`; `child->vft[+8](child, dt-ish args)` = **per-updater tick**;
   when `cls_0x59f7d0::meth(child)` says *finished* → collect. Collected →
   `meth_0x54f150(entity, item, 1)` (finish/remove; 0x550eec region).
4. **Updater map #2 @ Entity+0x33C** (the session-18 map#D, hot): same accumulate+tick+
   finished-collect loop; extras: nodes have `[node+0x24]` = "is-loop?" gate and a per-node
   float timeline; completion → `meth_0x54f150`. ALSO during iteration, `[node+0x28]` byte
   checked before predicate.
5. **Updater map #3 @ Entity+0x354** (session 18 map#F): same tick; completion path differs:
   `(**child)(child, 1)` (vft[0] = deleting dtor, param 1) + notify: when `[node+0x20]` &&
   `[node+0x24]`: `sub_4E1DE0/sub_4E20F0` (FX-pool!) and
   `GameActionEquip::meth_0x54cad0` (mislabelled — it's a map-erase next at gap340[16]);
   plus `cls_0x50dbb0::meth_0x51f0f0/0x54cda0` on gap340[4..] (single "current" slot with
   changed-check).
6. **Model bbox re-placement**: `ModelInstance::meth_0x563f10(dt)`; if dirty
   (`meth_0x563930`): rebuild placement record, temporarily NULL `[+0x160]`, call
   `meth_0x54a8a0` (octree re-insert), restore — **octree move on visual bounds change**.

## 3. Verdicts

- **Entity+0x33C / +0x354 / +0x3C4 / (+0x3D0)** = the entity's **updater/timer maps**
  (per-entry float accumulator @node+0x1C += dtSec; child ticked via `child->vft[+8](arg)`;
  retired when `cls_0x59f7d0::meth(child)` says finished — predicate delegates to
  child's `[child+0x164]` driver, null ⇒ done).
- **CORRECTION to first draft of this doc**: `GameEffectDamage`'s vft[+8] (thunk 0x45c0f0→
  0x45c0c0) is *not* a tick — it is "init-from-args": base `GameEffect::virt_0x4624f0(a2)`
  then `effect+0x9C = a2[+0x9C]` (copies the damage amount field!). So vft[+8] is a
  polymorphic *update-or-configure* contract whose meaning differs per child class, and the
  child type of maps 0x33C/0x3C4 is NOT yet pinned as GameEffect. Downgraded to:
  children = "timed updatable objects" per class contract.
- **Map @+0x354 is the VFX/attachment registry (strong)**: its completion path calls
  `sub_4E1DE0` + `sub_4E20F0` — the same VFX-VEG pool as `sub_4E2AD0` used for
  `demo_rain/snow/wind.veg` instances (session 11 §1) ⇒ entries are attached visual-effect
  instances; on completion they are pool-released, their single "current" slot
  (gap340[4..], cls_0x50dbb0 pair) updated with changed-check, and the active-entry flag
  cleared via `(**child)(child,1)`.
- Water-snap (§2.2) = entities adjust placement Y to nearest `Water` object within ±0.001
  via octree queries (buoyancy/wading behavior).
- Combat wiring lead: `effect+0x9C ← a2[+0x9C]` in the +8 init — if `a2` is the source
  entity's GameEntity block, GE+0x9C is the anim-lock byte — plausible "damage derived from
  attack state" wiring. **Unconfirmed**; check callers of 0x45c0c0 next round.

## 4. Open / next

1. Callers of `0x45c0c0` (who "inits" damage effects and with what `a2`) — pins the combat
   damage wiring.
2. `meth_0x54f150(entity,item,1)` = finisher body (erase + maybe OnFinished vft).
3. vft+41 vs the +0x48 step driver: which fires scheduled QueueAnimEvent vs anim-keyframe —
   pin (affects reimpl pump ordering).
4. Map @0x33C / 0x3C4 child class identification (RTTI cast sites near insert helpers
   0x550301/0x550325 region).
5. Standing: World trees B/C/G attribution, clip names, damage Apply math, +0x34 mode writer.

## 5. Ops note

IDA native tools healthy this session (post-reconnect). No IDB writes; read-only decompiles.
