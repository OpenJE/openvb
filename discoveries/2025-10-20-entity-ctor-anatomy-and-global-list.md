# 2025-10-20 — JE::Entity ctor full field map; global-list self-registration; +0x168 verdict (session 18)

Target: ORIGINAL_REFERENCE (native IDA tools restored — first session back). Read-only.
`JE::Entity::ctor_0x5513a0` decompiled in full; settles the session 16 §5 / 17 §4
"+0x168 extension" item and delivers the complete 0x3D8-byte entity-vb anatomy.

## 1. JE::Entity::ctor(0x5513a0) — field map (Entity-view relative, 0x3D8 bytes)

```
DynamicObject::ctor_0x568350(+0x00..0x??)         ; base
vft(+0x00) = Entity_DynamicObject_vftable_692084  ; engine default; F3 classes overwrite
--- "space/placement" block (shared with DynamicObject, seen in sessions 2/16) ---
+0x10..+0x24  six floats (later loaded with model bbox min/max by meth_0x54dc50;
              copy to gap384[32..52] happens IN THIS CTOR before bbox exists)
+0x160  0     model-cache handle (session 3 hot field)
+0x164  0b   +0x16C  0, +0x170  0, +0x171 byte, +0x174  0
+0x168  0    ★ extension/packet pointer (verdict §3)
+0x178  bool 1 ; +0x179  1,0 ; +0x17C/+0x180/+0x184  0 ; +0x188  0b
--- strings (SSO cap 15) ---
+0x1A4 cap 15 ; +0x1AC[0..16] block (str#1 @~+0x1B0?) ; then FOUR evenly-spaced strings
   at +0x1C0?..: cap markers at gap1AC[+100],[+132],[+160],[+188],[+216] → stride 0x1C
   ⇒ FIVE more strings + the early ones ⇒ ~7 strings in the Entity body itself
   (name/model/anim-art/art2 + 3 unassigned — matches creature copying TWO of them
    (bool_0x178, gap190) into its own records, session 2 §6)
+0x288 byte 1 ; +0x28C = -1 ; +0x290 0 ; +0x29B..0x29E flags (1,0) ; +0x2A3.. 0
--- maps #A (buynode 0x30-nodes) ---
+0x2A3[63] head ; count +0x2A3[67..83] block zeros (allocator-less std::map inline)
+0x2E4/+0x2E8/+0x2EC/+0x2F0 = 0
--- tree #B ---  sentinel +0x2F4, head +0x2F8, count +0x2FC
+0x308..+0x314 = 0 ; +0x318,+0x319 bytes ; +0x31C = 0
+0x324 = sub_413540()          ; sub-object (allocator-shaped)
tree #C @ +0x330 (sentinel+head+count via gap2F4[56..64]) ; +0x338 byte
map #D @ +0x33C (sub_56A160 buynode) ; flag byte +0x344
map #E @ +0x348 (sub_4C6640 buynode) ; count +0x350
map #F @ +0x354 (sub_56A160)        ; +0x35C..+0x374 zeros
tree #G @ +0x378 (sentinel; head +0x37C, count +0x380)
--- "placement record" @ +0x384 ---
+0x384 0 ; +0x388 b=1 ; +0x38C = -1 ; +0x390 = -1     (the -1 pair = "no cell/unset")
+0x394 b ; map #H @+0x398 (sub_4C6640) ; count +0x3A0
+0x3A4..+0x3B4  COPIES of the six space floats (+0x10..0x24 mirrored)
+0x3A8 b=1, +0x3A9 b=1, +0x3AA b=0 ; +0x3BC = 1
map #I @ +0x3C4 (sub_57C360) ; +0x3CC count ; map #J @ +0x3D0 (sub_57C360) ; +0x3D4 count
object ends +0x3D8 ✓
```
(Inline names per IDA; byte offsets exact from raw EAs listed at §4.)

## 2. Global live-entity list: self-registration in the constructor (end of 0x5513a0)

```
local_14 = { this }
node = sub_433760(byte_70BE0C, byte_70BE0C[1], &local_14)   ; list insert (GUI::Interface list)
global_gui_interface_0x70be08.meth_0x401dd0(1)               ; size++ / notify
node->next fix; *node->next = node
```
⇒ **`byte_70BE0C` (global_gui_interface_0x70BE08+4) is the authoritative LIVE-ENTITY list**;
every JE::Entity adds itself at construction and (dtor side, meth_0x54fff0 0x550042 region)
removes itself. Confirms sessions 15/16: World::PackState iterates THIS list, server
warp-case iterates it, `entity+476/+0x1DC netId`, `+0x15C classId`. openje: this is the
"entity registry" — the World's own trees (§1 #B/#C/#G) are something else (candidates:
per-cell octree entity buckets / name index).

## 3. +0x168 verdict

- Zeroed in Entity ctor here (0x5513da).
- Entity::PackState (session 16 §2 step 22/23): writes has-flag, and if set calls
  `[+0x168]->vft[+32](packet)` → per-class/state extension serializer.
- Entity::UnpackState (0x54e921/0x54e935/0x54e93b — reimpl doc §2 "child unpack"): ALLOCATES
  and fills the child there (reimpl already located the call region).
- Cleanup at 0x550042 inside `meth_0x54fff0` (the entity stop/delete path, also called from
  GameEntity's deleting-dtor thunk 0x467a00 per session 1 §2).
- ⚠ NOTE water-tile writer (0x5ab6cc) stores a plain int a7 there too ⇒ the slot is
  pointer-or-int depending on entity flavor: for normal F3 entities it is the
  **state-machine child packet object** (created by UnpackState, freed by 0x54fff0,
  serialized via vft[+32]); for engine water-tiles it's a parameter. Treat as a union.
⇒ For the reimpl: the "+0x168 chain" is exactly the *entity state child* from your
  unpackstate census — the snapshot extension mechanism = "current state object packs itself".

## 4. Evidence EAs

ctor writes: 0x5513c8 vft, 0x5513ce..0x55141e space/flags, 0x551429..0x55155c strings &
flags, 0x551566..0x551592 map#A, 0x5515c8 tree#B, 0x551610 sub_413540 obj, 0x551628 tree#C,
0x551646 map#D, 0x551679 map#E, 0x5516ac map#F, 0x551703 tree#G, 0x551713..0x5517ad
placement record (+0x384 block), 0x5517c5 map#I, 0x5517f8 map#J, 0x551813..0x551851 global
list self-registration. Usage EAs for +0x168: ctor 0x5513da, cleanup 0x550042, unpack
0x54e921/935/93b, pack 0x54a863/871/882, watertile 0x5ab6cc.

## 5. Queue update

1. Name maps #D/E/F/H/I/J + trees B/C/G by their readers (the registries question, finally
   answerable per-container — start with the ones touched in ChangeState/Activate paths).
2. 0x54fff0 "entity stop" full read (dtor path; also removes from global list).
3. Older: clip names, damage Apply math, World+0x34 mode writer (reimpl runtime watch).
