# 2025-10-20 — Effect-node wrapper semantics + world-server template interface located (session 5)

> **Corrected by [session 6](2025-10-20-gameworld-interfaces-effect-pipeline-closed.md)**: the
> table at 0x668494 is the **JE::World virtual-base** table inside GameWorld, not the
> IWorldServer interface (that is **0x66856C**, installed at GameWorld+4). Slot attributions
> in §3/§6 below were reassigned accordingly — kept as-is for the audit trail.

Target: ORIGINAL_REFERENCE. Read-only. Follows session 3 §4 (`EFDT/EFCD → cls_0x55a280`) and
session 4 §3 (`Entity+0x15C` tag readers still unknown).

## 1. `cls_0x55a280` is not a real class — it's an interface trampoline

- DB: size 4, single member `mbr_0x0`; `meth_0x55a280(this, chunkHdr, res)` body:
  `return (*(this->mbr_0x0 + 20))(this, param_1, param_2) != 0;` (0x55a294)
- i.e. `{void* inner_obj}` forwarding the chunk body-read to **inner vft[+20] (slot 5)**.
  The effect-node object is whatever `WorldServer->vft[+40](id)` returns; the wrapper just
  dispatches. Its real type/payload decode lives behind that slot.
- `sub_55ADC0(parent, child)` (parented append) and `sub_468310(dest, &obj)` (root push into
  the GameEntity+0x38 trees) both store the inner object pointer; the wrapper is stack-local
  (cleared at 0x4685fc).

## 2. `GetWorldServerInstance_0x401150` (full read)

- Throws `Common::Exception("Invalid attempt to retrieve world server instance.")` unless the
  server/editor flag (`global_area_map_icons[0]` getter 0x5b4f30 & 1) is set — same gate as
  session 3's effect-tree skip.
- Returns `F3::global_cls_0x616328_0x70bd98 + 1` — a secondary interface of the GameWorld
  singleton.
- Confirmed slots on that interface: vft[+8] = assign-net-id (GameEntity ctor → +0x28,
  0x4679b0); vft[+40] = template-by-id (EFDT/EFCD, 0x468570/0x4685bc).

## 3. The world template-interface vftable: 0x668494 (RESOLVED, raw)

Found via `xrefs_to(0x46fdc0)`: its only xref is the *data* pointer 0x6684ac — a vft entry.
Backtracking contiguous function pointers gives table start **0x668494** (44+ slots, entries
0x46fd20..0x4701e0). Key slots:

| slot | byte off | EA | identity |
|------|----------|----|----------|
| 6 | **+24** | 0x46fdc0 | create-entity thunk → `CreateGameEntityByClass_0x46f530` |
| 13 | **+52** | 0x470330 | template-lookup-by-NAME: vbptr-adjust to GameWorld base, reach the JE::World sub-object via `*(*(base−228)+4) − 912` (−0x390; GameWorld size 0x39C), then call **that** object's vft[+48](nameSSOstr, a3) |
| 10 | +40 | 0x46fe40 | prime candidate for the EFDT "template by id" target — decompile next session |

**This validates the reimplementation's recorded chain** ("world-server vft+52
template-lookup-by-name → vft+24 create" in MapEntitySpawn.hpp): the slot arithmetic is
correct *for this table*. Keep using it.

Separately, the DB's `GameWorld vftable @0x6685a4` (length 20) is a different interface; its
slot 10 = 0x46f8e0 = `CreateGameEncyclopedia` (raw: new 0x10 → JE::Encyclopedia::ctor 0x58f2f0
→ vft 0x668430). Slots 7..12 (0x46f820..0x46f9e0) are a per-type factory family. When doing
lockstep comparisons, pin the vftable address first — never carry slot numbers across the two
interfaces.

## 4. Resource-extension string array @0x6685f4 (raw, inline, stride 8)

Immediately after the 0x6685a4 table ends, a static `char[8]×9` array (no pointers):

```
.con  .amo  .por  .use  .dor  .arm  .wea  .itm  .crt
```

- This is the shipped extension→template data (probably iterated by the registry that the
  name-lookup 0x470330 ultimately consults).
- Order is **not** the factory-case order; includes `.por` (portal?) and has no player
  extension — GamePlayer is never spawned from map resources, consistent with §1 of session 4
  (player keeps creature tag 1 and is created by class, not file).
- Cross-check the reimpl's `ClassIdForResourceName()` against exactly this set (9 extensions
  → 10 factory classes minus `case 0 GameEntity` fallback? `.por` target unknown).

## 5. Queue update

1. Effect-node payload: confirm EFDT `vft[+40]` = 0x668494 slot 10 (0x46fe40); decode the
   object it constructs and that object's vft[+20] (the `cls_0x55a280` trampoline target).
2. F3-side readers of Entity+0x15C (class tag) — DB shows none; possibly write-only for
   debug/tooling. Check network-serialize paths for byte-level access.
3. What `.por` maps to (nearest candidate: `BuildingTrigger` ctor 0x5b1c30).

## 6. Addendum — slot 2 / slot 10 bodies, and a correction to session 3 §4

Raw decompiles of two 0x668494 slots:

```
slot 2 (0x46fd60):  return *(GameWorldbase − 808)          ; read id counter
slot 10(0x46fe40):  *(GameWorldbase − 792) = arg & 0x1FFFFFFF
                    return arg & 0x1FFFFFFF                 ; set id counter, MASKED
```

- 0x668494 is installed by `GameWorld::ctor_0x470340` at `[vb-displaced this + 8]` (raw
  0x470396: `mov [ecx+esi+8], 0x668494`) and referenced by `GameWorld::dtor_0x46f190` — it is
  genuinely a GameWorld interface vftable.
- **Correction**: with these bodies, slot 10 is an **entity-id counter setter** (29-bit id
  mask!), not the "template lookup by id" I inferred in session 3 §4. Likewise EFCD's
  `vft[+40](0)` = "reset counter to 0", EFDT's `vft[+40](eventId)` = "set counter to
  eventId". The subsequent `cls_0x55a280::meth(idValue, …)` shape then makes sense only if
  the wrapper stores the **reserved id** (not an object pointer) and its `mbr_0x0+20` call is
  a *different* dispatch than the decompiler's typing suggests — i.e. effect nodes carry a
  numeric id and the vft[+20] call target must be re-derived from the object that
  `sub_468310`/`sub_55ADC0` actually receive. **Open item, next session.**
- Durable facts regardless of interpretation: (a) F3 entity/net ids are `x & 0x1FFFFFFF`
  (29-bit), stored at GameEntity+0x28; (b) the counter lives inside the GameWorld body
  (base−792/−808); (c) GameEntity ctor's vft[+8] = *read* of that counter region.
