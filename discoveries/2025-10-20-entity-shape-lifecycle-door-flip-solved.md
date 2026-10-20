# 2025-10-20 — Session 17: Entity COLLISION-SHAPE lifecycle + DOOR STATE-FLIP SOLVED

Target: ORIGINAL_REFERENCE (IDA MCP). Read-only. This is the synthesis thread: it closes the
"which anim token flips doors" mystery (gobj/door doc §5), explains the
`Entity+0x16C = vft[+180]()` line in the lifecycle doc (+180 dec = slot +0xB4 — the shape
factory from the rleconvexquad doc), and gives A's grid work the complete per-entity contract.

## 1. The entity collision-shape object (settles session 16 §5 "consumers")

Fields (JE::Entity DynamicObject area — engine-side, all F3 entities):
```
Entity+0x16C  = collision SHAPE ptr (RLEConvexQuad* for creatures/usables; base ctor 0x40c9b0
              ; RLEPrimitive base: owner@+4, bornTick@+8 via JE_TickMilliseconds)
Entity+0x170  = byte "shape is registered in map" flag (set/cleared by +0x70/+0x74 below)
vslot +0xB4   = SHAPE FACTORY (dec 180 — this is the "vft[+180]()" in the Create chain)
                default 0x550780; creature/usable override builds the 0x58B quad (§ rleconvexquad doc)
vslot +0x70   = ATTACH-SHAPE-TO-MAP:  default 0x551cc0:
                  if (!Entity+0x170) { sub_40FF50(mapArg, Entity+0x16C); Entity+0x170 = 1; }
vslot +0x74   = DETACH-SHAPE-FROM-MAP: default 0x54de50:
                  if (Entity+0x170) { for i<mapArg[1]: sub_40F070(mapArg, i, *(shape+4)=owner);
                                      Entity+0x170 = 0; }
```
Shape's own vtable (from 0x64d5e8/0x64d6b8 families — calls seen in §2): `vft+12` =
move-to-cell(shape, cellX, cellY); `vft+16` = occupancy/block query; `vft+24` = second
query (walk-fail?); `vft+0` = destroy(destructor; called with flag 1 from SetPosition's
recreate path and ActionMoveTo cross-map arm).

## 2. `JE::Entity::meth_0x54a8a0(entity, pos3f)` = **SetPosition** — full grid dance (0x54a8a0..0x54aa12)

```
SetModelPos(meth_0x5680d0); map = WorldServer-vft[84](Entity+0x1EC mapId);
model int_0x160→meth_0x563e30(bbox 3 floats); attached gap2F4[4] list → sub_5AA4E0 each;
if (shape@Entity+0x16C && map):
   (cellX,cellY) = JE::WorldToOctreeCell(pos, map)              @0x54a959
   ok1 = shape->vft[+16](cellX, cellY, Entity+0x1F4?)            @0x54a98c
   ok2 = shape->vft[+24](param)                                  @0x54a991
   if (!ok1 || !ok2):
       this->vft[+0x74](map+0x88)                               ; DETACH (map octree at Map+0x88!)
       if (!registered-flag): shape->vft[0](shape,1) [destroy];
                              shape = this->vft[+0xB4]() [RE-CREATE] @0x54a9d4
       shape->vft[+12](shape, cellX, cellY)                      ; MOVE
       this->vft[+0x70](map+0x88)                               ; ATTACH  @0x54a9f6
map->cls_0x56c500::meth (dirty/update)                            @0x54aa04
Entity::meth_0x549450(entity, 0.0) tail                          ; occupancy notify (1.0f variant seen in door geometry)
```
⇒ every entity position change goes through **detach→(recreate)→move-cell→attach** on the
octree at `Map+0x88`. That + the §1 slot semantics = the complete grid contract for A's
`SpatialGrid` + `GameActionMoveTo` blocker-response work. NOTE: `+0x1EC` here is read as the
*map id* used with vft[84] — consistent with the reimpl excerpt (mapId@+0x1EC ✓).

## 3. ⭐ DOOR STATE-FLIP SOLVED — it's the door's ACTIVATE override, not an anim token

`F3::GameDoor::virt_meth_0x40ffe0` (Door Entity-vft **+0xB8 = activate-slot override**;
session 9's OperateDoor skeleton §5 ACTION resolved):

```
v3 = door handle; base activate sub_467640(v3−76-units, ...)     ; GE activation first
map = WorldServer-vft[84](*[v3[vbdisp+0x100]])                  ; door's map-id field Entity+0x100
store 1 → Entity+0x1AD-ish byte (raw +425)                      ; "operating" flag?
branch on v3[-52]-units flag ⚠ (candidate: the GDOR +0xBC/+0xBD bool pair or the +0x84
   "geometry-registered" flag seen in +0x70 door override — decompiler units unclear here):
 A) flag≠0 (close direction): PlayAnim-like 0x548d80(handleEntity, 514, 0, 1, 0, 0)
 B) flag==0 (open  direction): 0x548d80(handleEntity, 514, 0, 1, 1, 0)
 both then: sub_40F3C0; read anim obj [Entity+0x74-ish×4=+0x1D0?]→ GetDuration(anim 514)
   and SUBTRACT it from the Entity+0x94 timer (⟹ matches ge94-speed doc's door-timer:
   backdating the timer by anim length = door starts "already opened for duration" — same
   desync trick family as activate's rand offset) ;
   then SERVER-ONLY:  A) → entity->vft[+0x74](map+0x88) DETACH collision
                      B) → entity->vft[+0x70](map+0x88) ATTACH collision
```
⇒ **Door open/close = door's own activate virtual: play anim 514 (open/close by flag),
backdate Entity+0x94 by clip duration, and flip the octree registration via attach/detach
slots.** Combined with the door +0x70/+0x74 OBB overrides (gobj/door doc §4 — the door
variants do the FULL transformed-OBB swap + occupant notify, unlike the base shape-pointer
version), the whole door mechanism is closed: OperateDoor action → actor anim 121 →
door activate 0x40ffe0 → anim 514 + collision flip. ⚠ flag-units note stands (one lockstep
read of the door during open settles it — A).

## 4. Adjacent confirmations

- `GameActionMoveTo::virt_meth_0x406780` (F3 arrival hook): timestamp `meth_0x548f90`
  (Entity+0x184 = now) + `QueueAnimEvent(1 /*Idle*/, 0.0)` — arrival ⇒ back to idle anim.
- Base `meth_0x548d80(e, animId, f, g, h, i)` = **PlayAnim** family (door calls at §3 vs
  ActionMoveTo step-slot calling it with cell coords — see ⚠: one of the two arg readings is
  the decompiler's float/int confusion; door's `514` matches the clip-id space (355,442,449…
  no — 514 not seen in Queue tables; candidate door-open clip). Settle when wiring door anims.)
- `meth_0x549450(e, float)` = occupancy/push notify (door geometry passes 1.0f per occupant;
  SetPosition passes 0.0f) — engine side.

## 5. Open / next
- ⚠ door branch-flag units (§3) — lockstep one read.
- shape vft+16/+24 exact predicates (blocked-by-whom? cell cost?) — decode `sub_40FF50`
  registration + query bodies when the grid reimpl needs parity (A lane now).
- Does GameCreature's +0x70/+0x74 stay base-default? class-vft ledger shows creature does
  NOT override +0x70/74 — only doors do ⇒ doors are the only entities that swap collision
  geometry at runtime (gates may reuse later). *(from tables — quick recheck)*.
- anim 514: verify vs. the anim-id space (QueryDefaultAnimation max was 511; 514 fits the
  same table region — door-specific clips).

## 6. Evidence index
SetPosition 0x54a8a0 (cell query 0x54a959, vft+0x74 @0x54a9af, recreate @0x54a9d4, move
0x54a9e8, vft+0x70 @0x54a9f6, tail notify 0x54aa12). Base slots 0x551cc0/0x54de50 (flag
0x54de53/0x54de89, shape deref 0x54de5e). Door activate 0x40ffe0 (map lookup 0x40fd9f,
anim calls 0x40fdda/0x40fe9b, duration subtract 0x40fe2c..0x40fe44/0x40feed..0x40ff05,
attach/detach 0x40fe7d(vft+0x74)/0x40ff3a(vft+0x70)). Arrival 0x406780 (0x4067ab, 0x4067b6).
Shape primitive vtables 0x64d5e8/0x64d6b8 (rleconvexquad doc §2 ctor).
