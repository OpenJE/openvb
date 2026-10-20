# 2025-10-20 — Map::PackState complete order + full IWorldServer vft slot map (session 17)

Target: ORIGINAL_REFERENCE. Read-only, via direct JSON-RPC fallback on :8745 (pi MCP client
still not re-registered for ida_original_f3; server healthy). Completes session 16 §3 and
consolidates the world-state snapshot story. Cross-refs: reimpl doc
`serverupdate-pump-unpackstate-state-census.md` covers the entity INBOUND side (UnpackState
0x54e690) — this doc is the OUTBOUND map side.

## 1. `JE::Map::PackState` (0x56d7f0) — complete field order

```
guard: [map+0xB8] == 3  else throw "PackState() method may only be called on normal maps."
 1 str   @ +0x10        ; map name (resource name)
 2 u32   @ +0x28
 3 str   @ +0x30
 4 str   @ +0x50
 5 str   @ +0x6C
 6 str   @ +0x94
 7 u32   @ +0x118
 8 list  @ +0x12C       ; intrusive circular list; node+0x08 -> member;
                        ; per node: u32 [member + 476]  (== +0x1DC = NET ID of entity on map)
                        ; terminator: u32 0
 9 byte  raw 0          ; net::packet::meth_0x45f0b0(0)
```

⇒ Map packs **its membership as a list of entity net-ids**. Combined with session 15/16:

```
World::PackState = [8 meta u32s]
                 + [entities: netId u32, classId u32, Entity::PackState(23 fields,+ext chain)] ×N, u32 0
                 + [maps:      mapId u32,  Map::PackState (incl. member net-ids)] ×M, u32 0
                 + [tail u32 world field (gap90[76]-ish)]
```
The full multiplayer/warp state = exactly these three sections. Membership is *by id*, not
inline — receiver resolves ids against its entity list.

## 2. IWorldServer vftable 0x66856C — FULL slot map (12 slots; consolidates scattered notes)

| slot | +bytes | EA | identity (evidence: prior sessions, re-verified via raw table dump) |
|------|--------|----|----------------------------------------------------------------------|
| 0 | +0 | 0x436240 | **get World+0x3C** (u32) |
| 1 | +4 | 0x46F250 | **get World+0x70** (read as 64-bit/double at that slot) |
| 2 | +8 | 0x46F260 | **set World+0x70** (u32 store, returns value) |
| 3 | +12 | 0x46F270 | **get World+0x60** (u32) |
| 4 | +16 | 0x544A60 | JE::World passthrough |
| 5 | +20 | 0x544A70 | JE::World passthrough |
| 6 | +24 | 0x544A90 | JE::World passthrough |
| 7 | +28 | 0x545100 | JE::World passthrough |
| 8 | **+8 slot idx 8** | **0x46F280** | **read next-entity-id (World base +0x8C)** — GameEntity ctor → GE+0x28 |
| 9 | +36 | 0x544B80 | JE::World passthrough (used as "mgr" in server case-28) |
| 10 | +40 | **0x46FA40** | **effect-type factory** (1=GameEffectDamage 0xB0, else JE create) |
| 11 | +44 | **0x545930** | **World::PackState** (xref-confirmed: this table @0x668598) |
| 12 | +48 | 0 | end of table |

(slots 0–3 are plain World field accessors — note +0x70 is BOTH a settable field here and
meta-field #4 of the PackState block {+0x44,+0x48,+0x6C,+0x70,+0x74,+0x7C,+0x80,+0x84};
together these 8 packed ints + the 4 accessor fields = the world "game clock/seed" surface
*(inferred; assign names as consumers appear)*.)

## 3. Cross-check task for the codec builders (either agent)

Session 16 §2 (OUTBOUND 23-step entity order) vs reimpl's §2 (INBOUND UnpackState 0x54e690):
mirror-verify field-by-field BEFORE building tests — pack/unpack mismatch = the classic
netcode bug source. Known asymmetry to check first: packer writes raw byte @+0x1E0 via
0x45f0b0 — confirm the unpacker consumes exactly one raw byte there, not u32.

## 4. Queue update

1. ~~Slots 1/2/3 assignment~~ CLOSED (raw decompiles: get+0x70/set+0x70/get+0x60, slot0
   get+0x3C).
2. +0x168 entity-extension chain owners (which F3 class attaches what; the vft[+32] bodies
   are the per-class state — GameCreature likely first).
3. Older: clip names, damage Apply math, World mode-writer (reimpl runtime watch).
