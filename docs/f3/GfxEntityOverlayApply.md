# How an entity draws its world-space overlay - the 0x54dea0 -> 0x563D30 -> 0x49D8A0 chain (2026-10-22)

Started as "verify whether the two literals in `Entity::virt_meth_0x54dea0` are floats." They are still
unverified - and the chain underneath them is worth documenting on its own, because it is the engine's
**node-variant fan-out** for a per-entity renderable, reachable from a single entity virtual.

## The call path

`JE::Entity::virt_meth_0x54dea0` (0x54dea0, 0x106 bytes) is the per-entity overlay/animation pump:

1. Gate: `gap340[28]` set **and** `[Entity-vft + 7](this)` true (0x54deb3).
2. If `mbr_0x160` (the overlay object) is live, jump to the draw leg; else, when the one-shot byte
   `+0x164` is clear, it resolves a name from a string at `+0x21C` through `meth_0x54dc50(name, 0)`
   (0x54def2), sets the latch, and - with `*(this+0x22C)` as a map id - resolves the **Map** via
   `sub_401130()` + `[mbr_0x0 + 84]` (0x54df29), calls `JE::Map::GetOctree_56d960` and hands the octree to
   `Entity::meth_0x568140` (0x54df44). That leg is a real cited path from an entity's string+map-id band
   into octree attachment, found while looking at something else.
3. Draw leg (LABEL_17, 0x54df5e+): `JE_TickMilliseconds_0x56af50()` then **the literal pair in question**,
   selected by the state field:
       if (Entity+0x1E0 != 5 || *(this+0x368) == 1)  sub_563D30(mbr_0x160, a2, 0, 0, 1028443341, -16711681)
       else                                          sub_563D30(mbr_0x160, a2, 0, 1, 1029785518, -65536)
   i.e. lifecycle state 5 takes a different colour/size pair than everything else - which is why the
   +0x1E0 field being a STATE enum (JadeBear's census; also `== 7` at 0x54903a and the ChangeState machine
   in meth_0x5507f0) matters here and not there.

## What the four hops actually do to the two literals

`sub_563D30` (0x563d30) walks `g_model_cache->count_0x1C` instances, calls
`cls_0x49cc40::meth_0x4a0dd0` per node, and recurses into children with **hardcoded** `(1028443341,
-16711681)` (0x563db0). `meth_0x4a0dd0` rebuilds two RB-tree sets (its own + `mbr_0xbc`, `mbr_0x80`),
`qmemcpy(byte_707DB0, *(v18+4), 0x40)` to publish a transform, then calls `sub_49D8A0` and recurses on
`*v20->gap0` with the same hardcoded pair. `sub_49D8A0` (0x49d8a0) is the interesting one: an
**RTTI-dispatched node-variant apply** over a node list, one arm per type:

| cast target | leaf call |
|---|---|
| `Gfx_ModelMeshPkt` | `cls_0x50e390::meth_0x50e390(this, param_1, a6, a7, a8)` |
| `Gfx_SkinMeshPkt` | transform copy `qmemcpy(v15+40, a1+108, 0x40)` then `sub_507C60(...)` |
| `Gfx_ParticleSystem` | `JE::Gfx_ParticleSystem::meth_0x51c490(v16, v11, a2, param_3)` |
| `Gfx_Water` | tested and skipped (no body in the arm) |
| `Gfx_WaterTile` | `JE::Gfx_WaterTile::meth_0x512b40(this, a2: param_3)` |
| `Gfx_LineSystem` | `cls_0x50f060::meth_0x50f4c0(this, param_3)` |

plus a tail loop recursing into child node arrays (`a1+192`, count `>>2`) and clearing `+12` (the
"transform inherited" latch, 0x49dabf).

The ModelMeshPkt leaf settles the SHAPE, if not the meaning:
`cls_0x50e390::meth_0x50e390` (0x50e390) stores the pair into adjacent dwords -
`mbr_0x47 = param_2` (a **char**), `mbr_0x48 = param_3`, `mbr_0x4c = param_4` - then calls
`(**byte_707CE4 + 232)(byte_707CE4, &mbr_0x28, param_1, 1)`, i.e. hands the sub-object to the gfx manager's
vft slot 58 with a constant `1`.

## Verdict, and what is left

- **Not supported**: my original reading that `1028443341`/`1029785518` are `0.05f`/`0.055f` used as a scale.
  Across four hops nothing does float arithmetic on them - they are forwarded as ints and stored as two
  adjacent dwords - and the call already carries a genuine `double a2@<st0>` in a separate slot, so a
  float-in-int was never needed. Retracted in `docs/f3/CreatureHpWriters.md` (commit 361f369), and this
  paragraph is the evidence the retraction was owed.
- **Still open, named**: the consumer of `mbr_0x48`/`mbr_0x4c` - the gfx-manager vft slot 58 - decides
  whether the stored pair is (colour, colour), (colour, intensity), or two floats in an int-typed field.
  One decompile from an answer; it is a Gfx-lane question, not a natives-lane one, so it is left open here
  rather than chased by whoever found the chain by accident.
- `param_1` at the top of `sub_49D8A0` is a **bitmask**, tested as `param_1 & *(node+9)[+36]` (0x49d95e) -
  so the two arms at 0x54df81/0x54df97 differ in the flag/size pair, not in a size alone.

Rule this write-up is really about: an unverified constant can be quarantined cheaply. The claim was fenced
as "numbers unproven, shape discovered", the chain got named anyway (because naming it is the durable part),
and the one hop that could settle it is recorded as a pointer rather than a guess - so the next lane inherits
a question with a boundary, not a plausible-looking sentence.
