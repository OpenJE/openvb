# 2025-10-20 — Entity & Map PackState wire field order; World+0xE4 = map registry (session 16)

Target: ORIGINAL_REFERENCE. Read-only (via direct JSON-RPC to `$OPENVB_IDA_ORIG_MCP_PORT`
after the pi MCP client lost the ida server on its restart — see note at end).

## 1. Correction to session 15 §1-C

The tree at World+0xE4 iterated by `World::PackState` holds **JE::Map objects, not
prototypes**: its per-record serializer `cls_0x56d7f0::meth_0x56d7f0` throws
`"PackState() method may only be called on normal maps."` (string @0x69187x region).
⇒ **World+0xE4 = loaded-maps registry (mapId → JE::Map*)**, and PackState = world meta +
live entities + all loaded maps. That is exactly the state a warp/join must replay, and it
matches the server flow (case-0x46 map-by-name + entity iteration).

## 2. `JE::Entity::PackState` (0x54a610 — DB's "cls_0x54a610" is the Entity-view base)

Field order written to the packet (entity-view-relative offsets; `45f130`=u32,
`45f1b0`=bytes(strlen+1), `45f0b0`=raw byte/string, `46cfb0`=byte):

```
 1 str   @ +0x1C4          ; entity name
 2 raw   @ +0x1E0
 3 u32   @ +0x1E4
 4 u32   @ +0x1EC
 5 u32   @ +0x04           ; ┐ the MOVER position triple —
 6 u32   @ +0x08           ; │ matches reimpl excerpt pos@+4/+8/+0xC
 7 u32   @ +0x0C           ; ┘ (floats)
 8 u32   @ +0x1F0
 9 u32   @ +0x1F4
10 u32   @ +0x1F8
11 str   @ +0x21C
12 str   @ +0x238
13 str   @ +0x254
14 u32   @ +0x28C
15 u32   @ +0x290
16 u32   @ +0x180
17 u32   @ +0x184
18 byte  @ +0x296
19 byte  @ +0x297
20 u32   @ +0x298
21 u32   @ +0x29C
22 byte  flag (has-extension: [obj+0x168] != 0)
23 IF flag: [obj+0x168]->vft[+32](packet)   ; per-class extension serialize (slot 8)
```

Notes:
- Buffer-overflow guard throws `net::overflow<net::buffer>` between the raw bytes writes
  (0x54a7cd, 0x54a814) — the engine's canonical "packet full" behavior.
- `+0x168` extension pointer = the per-derived-class packable attached to each entity
  (candidate: the F3 GameEntity data block or a class-specific state obj). Its vft[+32] is
  the subclass PackState — this is where GameCreature/GameItem extras ride.
- The `+0x04/+0x08/+0x0C` triple in THIRD position (after name+3 scalars) is the definitive
  confirmation of the reimpl's Mover pos@+4 layout on the wire.

## 3. `JE::Map::PackState` (0x56d7f0) field order (partial, first half)

```
 guard: [map+0xB8] == 3 else throw "…normal maps."   ; same mode-enum as World (+0x34==3)
 str @+0x10 (name) ; u32 @+0x28 ; str @+0x30 ; str @+0x50 ; str @+0x6C ; str @+0x94 ; …
```
(rest truncated this round — continue next session; map id @+0xB4 per §1's PackState call
packing [rec+0xB4])

## 4. Mode-enum unification (informs session 15 §4)

Map checks `[+0xB8]==3`, World checks `[+0x34]==3` — both strings say "normal". So 3 = the
engine's NORMAL-object state in a SHARED mode-enum across engine objects (World/Map/…),
written by whatever engine init path promotes raw objects to normal — NOT an STL count.
IDA's "count" write at World+0x34 (ctor) is likely the mode field initialized to 0 =
raw. Static writer still unfound (grep negative) → remains an open item, but the semantic
class of the field is settled: mode ∈ {0=raw, 3=normal}.

## 5. Queue

1. Rest of Map::PackState + its UnpackState sibling (0x56xxxx cluster) — save/warp parity.
2. Entity extension object @+0x168: who allocates, which classes, vft[+32] bodies per class.
3. UnpackState on world side (mirror of PackState at 0x545930) — the client apply path
   (pairs with reimpl's unpackstate doc).
4. Older: clip names, damage Apply math.

---
*Ops note: the `ida_original_f3` pi-MCP connection dropped when devenv restarted the server
(8745 listening again, tools/list OK via curl). Analysis continued via direct JSON-RPC
helper; the pi harness may need `/mcp` reconnect (user-side) to restore native tooling.*
