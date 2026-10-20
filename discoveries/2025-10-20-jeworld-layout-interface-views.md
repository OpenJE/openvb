# 2025-10-20 — JE::World layout + the `global+1` interface view explained (session 14)

Target: ORIGINAL_REFERENCE. Read-only. Closes the JE::World-layout queue item and gives
openje a raw-offset field map of `JE::World` (0x164 bytes). From `World::ctor_0x548a80`
raw disassembly (all `[esi+disp]` writes are exact).

## 1. Headline: the `GetWorldServerInstance` "+1" mystery, solved

JE::World's ctor installs **four vftptr slots in its own header**:

```
World+0x00  vft   = 0x691A1C   (primary IWorld view; 20-slot table, list in §3)
World+0x04  vft   = 0x6919E4   (server interface view  ← THE ONE ws uses)
World+0x08  vbptr = 0x691A94
World+0x0C  vft   = 0x6919C0
World+0x10  vbptr = 0x691A8C
```

`GetWorldServerInstance_0x401150` returns `global_cls_0x616328_0x70bd98 + 1` — pointer
arithmetic on `{int}` = **+4 bytes** = the address of World's +0x04 vft slot; "its `mbr_0x0`"
*is* World+0x04's vftptr. F3::GameWorld::ctor overwrites that exact slot with **0x66856C**
(session 6 §1) — fully consistent: the server interface = the +0x04 view of the world
object, engine base table 0x6919E4, F3 override 0x66856C. Same story for +0x00
(0x691A1C → F3 0x6685A4) and +0x0C (0x6919C0 → F3 0x668548).

**Singleton facts**: ctor end throws `Common::Exception("World object already exists.")`
if `global_cls_0x616328_0x70bd98` is already set, else stores `this`. So global 0x70BD98 =
the JE::World portion of the constructed GameWorld (subsystem slot 7) — the F3 object is
ONE allocation containing the World prefix; `world` member at GameWorld+0.

## 2. JE::World field map (0x164 bytes; raw offsets, ctor 0x548a80)

| off | init | note |
|-----|------|------|
| +0x00..+0x14 | vft/vbptr header | §1 |
| +0x14 | 0 | |
| +0x18 | **1** | |
| +0x1C | **1** | |
| +0x20 | 0 | |
| +0x24, +0x28 | 0, 0 | pair — timer pair? |
| +0x2C | RbTree head #1 (sentinel; head→+0x30, count→+0x34) | first of 4 containers |
| +0x38, +0x3C | 0, 0 | |
| **+0x40** | **6** | |
| +0x44 | 0 | |
| +0x58..+0x60 | 0 ×3 | |
| **+0x64** | **30 (0x1E)** | strong tick-rate candidate |
| +0x68..+0x70 | 0 ×3 | |
| **+0x74** | **1.0f (0x3F800000)** | float scale — timescale candidate |
| **+0x78** | **10** | |
| +0x80, +0x84 | 0, 0 | |
| **+0x88 / +0x8C** | 0, 0 | **net-id counter = +0x8C (RESOLVED)**: `0x46F280` (server-view vft+8) is `mov eax,[ecx+0x88]; ret` with ecx = the +0x04 view ⇒ actual field = base+0x8C. GameEntity ctor stores this as its net id at GE+0x28. |
| +0x90 | `cls_0x5aa3d0` subobject ×2 (also @+0xB8, size 40 each) | 0x548b6c/0x548b7c |
| +0xE0 | 0 | |
| +0xE4..+0xF4 | map #2 {flag byte +0xE4, buynode head +0xE8, count +0xEC, +0xF0..0xF4 pad0} + zeros to +0x104 | node = `buynode_0x59ec40` (0x30-byte nodes — std::map<string,X>) |
| +0x108 | `cls_0x56ae70` subobject (16B) | |
| +0x118 | `cls_0x569f10` subobject (16B) | |
| +0x128..+0x138 | 0 (sparse) | |
| +0x13C.. | RbTree heads #3 and #4 (+0x140/+0x144, +0x14C/+0x150) | |
| +0x154 | 0 | |
| +0x158..+0x160 | map #5 {flag, head, count} | last block, object ends 0x164 ✓ |
| tail call | `GUI::Interface::meth_0x4b9d60(...)` | (DB path noise; verify later) |

Semantics unassigned: the 4 trees + 2 maps = almost certainly {entity-by-id, entity-by-name,
template/prototype registries} — naming needs 1 reader each next round. Timing constants
cluster (+0x40=6, +0x64=30, +0x74=1.0f, +0x78=10) is the tick/game-rate config.

## 3. World primary vft 0x691A1C slot list (engine base — for openje naming)

```
[0]0x548CF0 [1]0x546690 [2]0x5455B0 [3]— [4]0x5460D0 [5]0x546B50 [6]— [7]0x545250
[8]0x5452B0 [9]0x545310 [10]0x545370 [11]0x5453D0 [12]0x545490 [13]0x545430
[14]0x546FD0 [15]0x5455A0 [16]0x545D60 [17]0x545FF0  (len 20; slots 3,6 unlisted in DB)
```
Compare F3 override table 0x6685A4 (session 6 §3 area): F3 keeps slots 1/2/13.. (same EAs)
and replaces the 7..12 band with its Create* factory family — the diff *is* the F3 hook list.

## 4. Accessor algebra (resolved same session, raw bytes)

1. **`0x46F280`** = `mov eax,[ecx+0x88]; ret` (no adjust prelude) with ecx = the +0x04
   server view ⇒ reads **base+0x8C** = the next-entity-id counter. GameEntity ctor's
   vft[+8] call ⇒ GE+0x28 net id = World base+0x8C.
2. **`0x46FD60/0x46FE40`** (on the GameWorld **vb** table 0x668494, view at base+0x394,
   thisptr slot holds 0): bodies do `sub ecx,[ecx-4]` (⇒ view) then read/write
   `[view−0xF8]` and `[view−0x230−0xE8]` ⇒ **base+0x29C** (GameWorld own field, outside the
   0x164 World prefix) and **base+0x7C** (inside World's zeroed 0x78 block — the
   `& 0x1FFFFFFF` "set id base" sink). So: +0x7C = settable id base, +0x8C = running
   next-id counter, +0x29C = GameWorld-local third counter (unassigned).
3. Four container candidates at +0x2C, +0xE4, +0x13C, +0x14C(+0x158): readers to be
   named next round.

## 5. Queue update

1. ~~JE::World layout~~ CLOSED §1–2; sub-items §4.1–4.3 follow-up.
2. Message-factory types 0/3/9/10/11 inventory (World container readers will reveal them).
3. Older: clip-name table, damage Apply math remainder, CActionbarInterface field map.
