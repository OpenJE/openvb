# 2025-10-20 — Entity family session 2: sub-table records, GamePlayer own-layout

Target: ORIGINAL_REFERENCE (F3.exe). Read-only. Follow-up to
[session 1](2025-10-20-entity-class-family-layout.md); same PE-raw-dump recipe applies.

## 1. The 5×12 array at GameEntity+0x38 is a set of STL tree headers

`cls_0x401b00::ctor_0x401b00` (0x401b00), called via vector-ctor iterator for 5 objects:

```
mbr_0x4 = JE_RbTree_InitSentinel_0x56e0b0(this)   // 0x401b08
mbr_0x8 = 0                                        // 0x401b0b
```

So each 12-byte element = an RbTree head `{sentinel@+0, root/sentinel@+4?, count@+8}`.
Cross-checked against `JE::cls_0x4d8d70::ctor_0x59f500` (0x59f500, the *command-registry*
tree ctor documented in `modules/openje/docs/je/cls_0x4d8d70.md`), which does the same
`buynode + self-link(left/parent/right) + count=0` — same STL `_Tree` shape. **GameEntity
owns 5 std::map-like containers**; JE::Entity (its vb) owns at least 2 more (§2 below).

Practical takeaway for the reimpl: treat `mbr_0x38[5]` as opaque key→? maps. Their *content*
semantics will come from the readers/writers (next session: `cls_0x463870::meth_0x466510`).

## 2. GameCreature ctor revisited — vbptr-relative offsets (correction to session 1 §4)

Session 1 quoted member offsets as absolute (0x6F0..0x7FC "own"). Raw vbtable dump confirms
the geometry that governs them:

- GameCreature vbtable @0x64D7CC = `[-4, +0x800]` → JE::Entity vb at **this+0x804** ✓
  (matches `JE::Entity::ctor_0x5513a0(this+2052)` at 0x40d04c and factory return).
- GamePlayer vbtable @0x64E974 (from `ctor_0x411820` 0x411852, raw `[-4, +0x820]`) →
  JE::Entity vb at **this+0x824** ✓ (`JE::Entity::ctor_0x5513a0(this+2084)` at 0x411859).
- So: Player = Creature + 0x20 own-bytes *inserted before the Entity vb* (at 0x800..0x823),
  pushing Entity 0xBDC→0xBFC sizes' tail. Sizes stay consistent (Entity vb is 0x3D8:
  Creature 0xBDC−0x804=0x3D8 ✓; Player 0xBFC−0x824=0x3D8 ✓).

This means the offsets in the reimplementation's `SpawnedEntity` excerpt
(pos@+4, mapId@+0x1EC, yaw@+0x1F0, moving@+0x3A2) are **Entity-vb-relative** and remain
valid regardless of creature/player own-size differences — good news: the Mover excerpt can
survive the class-hierarchy work as long as the vb placement is emulated.

## 3. Confirmed creature-own field block (outside Entity vb)

From `GameCreature::ctor_0x40d010` (absolute offsets in a **GameCreature** object):

| abs off | init @EA | evidence | interpretation |
|---------|----------|----------|----------------|
| +0x6F0 | 0 | 0x40d130, 0x40d2d4 | twice-reset scalar |
| +0x6F8 | `sub_40A190()` | 0x40d13b | allocator: `new 0x70` with self-linked [0],[1] (fixed node/pool head) |
| +0x6FC | 0 | 0x40d13e | |
| +0x700 | (pad/zero via memset below) | | |
| +0x704 | `sub_40A190()` (2nd pool) | 0x40d153 | second 0x70 self-linked node |
| +0x708 | 0 | 0x40d156 | |
| +0x70C | memset ×0x20 = 0 | 0x40d291 | 32-byte zeroed block (+0x70C..+0x72B) |
| +0x7F0 | `RbTree_InitSentinel(this+0x7F0)`; +0x7F4=head, +0x7F8=0 | 0x40d16b/16e | creature-own std::map #3 |
| +0x7FC | 0 | 0x40d2da | IDA-already named `maybe_agility_0x7fc` *(inferred)* |

## 4. GamePlayer ctor 0x411820 (0xBFC byte object)

```
if (param_1) { *(this+4) = &unk_64E974; JE::Entity::ctor_0x5513a0(this+2084); }   // 0x41184a..
F3::GameCreature::ctor_0x40d010(this, 0);                                          // 0x41186d
*this        = &GamePlayer_GameCreature_GameEntity_vftable_64e970;                  // 0x411875
*(this + *(this[1]+4) + 4) = &GamePlayer::vftable;                                 // 0x41187e (Entity vb vftptr)
Entity.vbptr-back = *(this[1]+4) - 2080;                                           // 0x411892
*(this+0x81C) = 15; *(this+0x818) = 0;                                             // 0x411895/9f std::string size/cap (cap=15) — string object at +0x808
*(this+0x200... actually 512) — see next:
this[+0x800] = operator new(0xC) -> JE::cls_0x4d8d70::ctor_0x59f500 or 0            // 0x4118ba..d6
```

(0x20 own region = +0x800 map ptr, +0x804..0x807 uninit pad, +0x808 std::string (buffer +0x808..0x817, size +0x818, cap +0x81C)).

- **GamePlayer owns a heap 0x30-byte tree header (`cls_0x4d8d70`, the command-registry-style
  map, node size 0x30 per openje notes)** at +0x800 and an **empty std::string (SSO, cap 15)**
  at +0x808 (`*(this+2056)=0` at 0x4118ad, cap=15/size=0 writes at 0x411895/0x41189f).
  This matches "Player = Creature + 32 bytes" exactly: 4 (map*) + 4 pad + 0x18 (std::string).
- Player vftables: primary @0x64E970 ("GamePlayer_GameCreature_GameEntity_vftable") and
  vbtable @0x64E974 — **one shared primary slot-0 again 0x4080c0 (get mbr_0x20)**: the
  mbr_0x20 getter is never overridden anywhere in the family.

## 5. cls_0x40a220 (13×0x38 array inside Entity-vb gap340) — record shape

ctor `cls_0x40a220::ctor_0x40a220` (0x40a220):
```
gap10[4]=0; gap10[8]=15; mbr_0x4 byte=0; gap34 dword=15; mbr_0x30=0; gap10[16] byte=0;
```
→ a 0x38-byte record containing **two std::string SSO pairs** (cap 15 @+0x18 and @+0x34)
and an int @+0x30; base struct cls_0x496bd0 occupies ~0x34. Used as the 13×0x38 array in
GameCreature (0x40d11e) — **13 records with (name?, value:int, name2)** = prime candidate for
an 13-entry skill/perk table. NOT confirmed yet; next-session xref pass on `cls_0x40a220`
writers will name it.

## 6. cls_0x50db20 (12×0x1C array inside Entity-vb gap1AC[104..]) — record shape

ctor (0x404bc0): `int_0x18=15; _0x14=0; byte_0x4=0` → 0x1C record = {dword@0, SSO string
(cap 15/size 0 at +0x4)}. Used at Creature-ctor (0x40d0ff) with 12 entries, and there the
two strings are seeded from **two std::string fields of the Entity vb itself**
(`bool_0x178` and `gap190` with cap checks at 0x40d1a0..0x40d209 → `meth_0x401bd0(dst,src,strlen)`
which is `std::string::assign`). So: "name/model copied into record[0]" pattern at construction.

## 7. What stays unknown (for the reimpl's decision list)

1. Semantics of the 5 GameEntity maps (§1) and 2+1 creature/§3 maps: keys/values unknown;
   reimpl can keep them opaque until EME2 spawn needs them.
2. 13×0x38 (§5): skill table hypothesis unconfirmed.
3. `mbr_0x94 = 8.0f` (session 1 §2) — scale? height? needs first consumer analysis.
4. vbtable for each of UsableObject/Door/Container/Item/Weapon/Armor/Ammo/Projectile —
   dumpable in 5 min via §6 recipe of session 1 once their ctors are read (session 3 queue).
