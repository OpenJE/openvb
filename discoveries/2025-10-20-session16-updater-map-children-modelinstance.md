# Session 16 — updater-map children identified: +0x33C carries the entity's ModelInstance(s)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Answers `2025-10-20-entity-update-updater-maps.md` §4 item 4 (0x33C child class) — partial
for 0x3C4 (same family, below).

## 1. Evidence chain

1. `Entity::meth_0x54b220(child)` (usage list of Entity+0x33C head 0x54b254):
   - `map = worldServer-by-name lookup(vft+0x54)(arg)`; `octree = Map::GetOctree_56d960(map)`;
   - for EVERY node of `Entity+0x33C` map AND of the `Entity+0x340..gap340[20]` map:
     `child->meth_0x568140(octree)` — i.e. **"re-register me into this octree"** — then next.
   - So the +0x33C children are objects that live in the map octree: renderer/sprite things.
2. Callers of 0x54b220 (2): `JE::Entity::virt_0x54bba0` (Entity vft idx15 = post-map-change
   re-placement) and — decisive — **`F3::GameActionEquip::meth_0x469cf0` @0x469f47**:
   the equip appearance updater. Its flow:
   - `mbr_0x0 = v13[+0x160]` (Entity interface field) = the entity's **ModelInstance***
     (same object type as `JE::WaterTileModelInstance::ctor_0x565540` family, model methods
     0x563xxx/0x565xxx);
   - swaps 13-slot appearance arrays (layer tables) + per-part attachment toggles via
     `ModelInstance::meth_0x563cb0(layerIdx)` for flags at entity-record +236..+240/+712
     (the equipped-look byte array — matches the "body-part table item CopyFrom+18" doc);
   - `ModelInstance::meth(vft+1)(layer, slot, 0)` per changed layer;
   - optional model swap `meth_0x54dc50(name)` + `vft[+60](v13[123].mbr_0x0)` +
     `ModelInstance::meth_0x565b00(...)`;
   - **`Entity::meth_0x54b220(entityVB, v13[123].mbr_0x0)`** — re-attaches the ModelInstance to
     the current map/octree. ⇒ the +0x33C map's children = the entity's ModelInstance objects.
3. Cross-check with `Entity::Update` (0x550ad0) consumption the other doc decoded: node payload
   `v45[5]`, per-node float at node+0x1C advanced by dtSec, `child->vft[+8](child, dtSec)` =
   **ModelInstance animation tick**, completion predicate `cls_0x59f7d0::meth(child)` =
   `child+0x164 == 0 || (child+0x164)->animFinished()` (`cls_0x48dc10::meth_0x48dc10`) —
   +0x164 = ModelInstance's current-animation object slot. Consistent with the model
   re-placement/`meth_0x563930` (bbox dirty) path in Update's tail (§4 of their doc).
4. `cls_0x59f7d0`-predicate other users: `sub_573550` (world clock sync in session 14 hosted
   pump) and `sub_573730` iterate per-cell model/thing lists and query the same predicate —
   i.e. "is this model instance's animation done" is a world-wide render gate.

## 2. Consequences for the reimpl

- The "updater maps" (+0x33C timed-models, +0x354 VFX instances, +0x3C4 variant-map) are the
  **presentation** layer: model-instance anim-tick + VFX instance lifetime. Entity+0x168 = the
  state-extension child (their ctor doc) — do NOT confuse with these.
- Equip appearance update (0x469cf0) is the writer to watch for model swap tests: it mutates a
  13-entry layer-slot array (`v22/v20/v21` triples = per-layer cur/new/old slot ids), then
  re-attaches to the octree. A minimal parity test: equip change → layer table diff →
  `vft+1(layer,slot)` calls → octree re-insert.
- Water-tile note: `ModelInstance::meth_0x568140(octree)` = octree re-insert; matches the water
  docs' rebase of entity placement on map change.

## 3. Confidence

+0x33C children = ModelInstance family: **strong** (call-graph above, one-hop, no RTTI needed).
0x3C4: same clear helper (`meth_0x54d660` at 0x5502ed) → same family, different map
(presumably per-attachment vs per-self models) — *(inferred)*; pin by finding its insert
helper's callers (Entity+0x348 field, next round if needed).
