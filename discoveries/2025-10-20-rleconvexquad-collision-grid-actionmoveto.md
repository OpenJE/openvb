# 2025-10-20 — Session 16: +0xB4 = RLEConvexQuad collision factory, JE::ActionMoveTo internals (IDA restored)

Target: ORIGINAL_REFERENCE. Read-only. IDA MCP restored this session — clears the ⟲ items from
[copyfrom-caller-hunt](2025-10-20-copyfrom-caller-hunt-creature-regen-tick.md) and upgrades
its §3a. All three follow-ups confirmed in one xref sweep:

## 1. ⟲ CONFIRMED: 0x55b6be = JE::ActionMoveTo internal — retraction holds (see prior doc §1.2)

`xrefs_to 0x55b4f0` → data ref from **vftable 0x6940e4 = `JE::ActionMoveTo`** (13 entries,
ctor 0x55b300 writes it). The `call [eax+28h]` is ActionMoveTo's OWN slot index-10 dispatch,
not the entity CopyFrom. The controller-object reading in the retraction is now exact:
**`esi` = a live ActionMoveTo instance** — which also means the earlier transfer-path
observations (+0x14 mover entity, +0x24..2C dest pos, +0x35 once-flag, mapId check) are the
MOVE ACTION's internals, documented in §3 below. CopyFrom caller status unchanged (runtime
breakpoint 0x468350 remains the only path; zero static callers exist).

## 2. Creature/usable slot +0xB4 body = **RLE collision-grid QUAD factory** — key input for A's grid work

`sub_40d840` (creature +0xB4; also re-thunked by GamePlayer) →
`operator new(0x58)` → `JE::RLEConvexQuad_near_struct_Entity::ctor_0x40c9b0`.
Usable-region clone `sub_4128E0` @0x412a50 does the same for the usable family
(Door/Container inherit via +0xB4 slot 0x412a90 per class-vft ledger). Record layout
(IDA-typed; EAs from decompile):

```
RLEPrimitive_near_struct_Entity base (ctor 0x40b640):
  +0x00 vft 0x64d5e8
  +0x04 owner  = *param_10   ; the ENTITY object ptr (creature base or vb-adjusted entity,
                             ; caller passes whichever of the two is non-null @0x40d978..0x40d998)
  +0x08 bornTick = JE_TickMilliseconds_0x56af50()     ; ← RLE grid primitives are TIME-STAMPED
  +0x10..+0x18 zero (link/prev hints?)
ConvexQuad extension:
  +0x1C,+0x20 center (computed as midpoint of min/max corners)
  +0x24        = param_9  ; the caller's Entity−0x610 read = **Entity+0x1F0 yaw** reimpl-confirmed
  +0x28..+0x47 = 4 corner XY pairs (8 floats) = bbox min/max extents fed from entity
                  (bbox floats at [entity−0x7f0..] = Entity+0x10..0x24 ✓ DynamicObject bbox block;
                   per-axis clamp: <0 → 0, |v| < 4.0f (0x64d7d4) → 4.0)
  +0x48/4C min XY, +0x50/54 max XY (derived; init 0x7FFFFFFF/0x80000000 sentinel)
  tail: virt_meth_0x40a4b0(this)                      ; vft 0x64d6b8 slot0 — "compute coverage"
```
⇒ **Entities insert themselves into the `.rle` collision grid as owner-linked, tick-stamped
convex quads built from bbox(4) × yaw, min-extent 4.0.** This is exactly the per-entity
collision primitive A's SpatialGrid needs (record 0x58 bytes, four 2D corners — the grid is
2D, z comes separately). Corner→center re-basing happens INSIDE the ctor (points become
relative to +0x1C/0x20). 0x40a4b0 (coverage computation) = next decode when the grid writer
lands.

## 3. JE::ActionMoveTo internals (bonus from §1's xref) — validate A's reimpl against this

vftable 0x6940e4 (idx → EA):
```
0  0x55b5e0  dtor-ish/settle
1  0x55b440  2  0x55b450  ; small virtuals (accessors?)
4  0x55b4f0  ★ Execute/tick (see below)
5  0x55b600  6  0x55b3f0  7  0x55b340
8  0x406e90  9  0x406e90  ; ★ F3 override (SAME stub both slots): sets a1->__vftable = -1
                            (action "invalidated/poisoned" — the shipped done-marker idiom)
                            and *outParam = 0
10 0x55b370  ★ step: calls vft+0x14 virtual (&x,&y out) [path cell-center query],
              then Entity::meth_0x548d80(mover, x, y, 0,0,0) = SETPOS on mover
11 0x55b3b0
```
field map (from slot-4 body 0x55b4f0..0x55b700 raw + xrefs):
```
+0x11/+0x14  mover entity* (Entity+0x3A2 moving flag set @0x55b6ad; mapId +0x1EC checked)
+0x18        dest/target object (pos floats at +4/+8/+C, id at +0xC)
+0x1C        path object (from 0x54be10(this->+0x20) — path-cache lookup by id @+0x20)
+0x24..+0x2C dest position copy (3 floats) from path/target
+0x35        once-applied byte (guard @0x55b6a9 sets 1 before self-dispatch)
+0x40/+0x44  scratch pointers set pre-dispatch
+0x4C        frame-delta-ish arg slot
slot-4 flow: path = GetPath(+0x20 id); store +0x1C;
  if (path.mapId(+0x1EC) != mover.mapId) → this->vft[+0x10](1) [cross-map request arm]
  else → copy path pos floats; if (+0x18)==0 → new(0x60) [request object]; self-dispatch
        slot10 (step-to-cell) + slot11(+0x2C) variants.
```
⇒ A: your `JE::ActionMoveTo` reimpl (job issue, budget interpolation, target re-request) should
match: (a) done = **vft poison −1** convention (F3 slots 8/9 stub 0x406e90), (b) step virtual
(idx10) = path-cell-center → SetPosition, (c) the once-guard byte @+0x35, (d) the +0x20-id →
0x54be10 path-cache lookup (that's likely your PathManager seam), (e) mapId-mismatch branch
calls slot idx-? (+0x10/4) with param 1 = cross-map re-issue. Slot indices align with
JE::Action_vtbl numbering — map before trusting my idx math (decompiler used
`__vftable[1].virt_meth_0x4` = raw +0x14 — verify).

## 4. ⟲ regen-body status: still raw-only (semantics stand); NEW precise task for lockstep
`0x409be0` (creature/player +0x60 virtual): the ⚠ `Creature+0x28` last-param store is NOT
net-id-clobbering if the virtual is only invoked by the engine regen scheduler (not on the
spawn net path). One lockstep read settles: log [creature+0x28] before/after a regen tick on a
net-spawned creature. Entity+0x174 accumulate + GetDerived(14)/cap GetDerived(0) stand as
regenerate-while-stationary semantics.

## 5. Open items
- 0x40a4b0 (quad coverage compute) — decode when A wires the grid insert.
- Who CONSUMES the 0x58 quads (grid store + iteration during cell queries) — the `near_struct_Entity`
  name suffix suggests a spatial "find nearest entity" helper — chase the consumer next.
- ActionMoveTo idx→Action_vtbl alignment (task for whoever owns JE::Action docs).
- CopyFrom caller: runtime-only, unchanged conclusion.

## 6. Evidence index
xrefs: 0x55b4f0 ← [0x6940e4 data, ctor 0x55b309, dtor 0x55b47d]; 0x40c9b0 ← 0x40d9b0 (creature
+0xB4 body), 0x412a50 (usable body sub_4128E0); 0x409be0 ← 0x40d433 (creature), 0x4119a6
(player). RLEPrimitive ctor 0x40b640 (owner 0x40b678, tick 0x40b684); ConvexQuad ctor 0x40c9b0
layout §2 EAs in-body. ActionMoveTo slot bodies §3 EAs. IDB names:
`JE::RLEConvexQuad_near_struct_Entity`, `JE::RLEPrimitive_near_struct_Entity`,
`JE::ActionMoveTo::virt_meth_0x406e90` (owner-label caveat: it lives in the ActionMoveTo vft,
F3 TU range — F3-provided override registered in engine class table, not an F3 class method).
