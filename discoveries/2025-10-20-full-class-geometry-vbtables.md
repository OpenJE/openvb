# 2025-10-20 — Full class geometry: every vbtable, Entity-vb offset, and per-class field writes (session 4)

Target: ORIGINAL_REFERENCE. Read-only. Offsets here were taken from **raw PE disassembly**
(`objdump -M intel`) and **raw vbtable dword dumps**, not from the decompiler's member display —
several decompiler offsets in sessions 1–2 were off by the vbptr −4 base; this document is the
corrected truth. All Entity-relative fields below use `E = this + 4 + vbtable[+4]`.

## 1. Inheritance tree (ctor-call evidence, confirmed)

```
JE::DynamicObject → JE::Entity (virtual base of everything below)
F3::GameEntity ─┬─ F3::GameCreature ── F3::GamePlayer
                ├─ F3::GameUsableObject ─┬─ F3::GameDoor
                │                        └─ F3::GameContainer
                ├─ F3::GameProjectile
                └─ F3::GameItem ─┬─ F3::GameWeapon
                                 ├─ F3::GameArmor
                                 └─ F3::GameAmmo
```
Evidence: each ctor's first call is the direct base ctor with `param=0` (see EAs in §2).
Door and Container are **siblings** (both call GameUsableObject::ctor 0x412420), NOT
Door←Container. Item calls GameEntity::ctor 0x4678a0 directly.

## 2. Definitive geometry table

| class | size | ctor | base ctor | vbtable | Entity at | primary vft | Entity-vft |
|-------|------|------|-----------|---------|-----------|-------------|------------|
| GameEntity | 0x47C | 0x4678a0 | JE::Entity 0x5513a0 | 0x666134 [-4,+0xA0] | this+0xA4 | 0x666130 | 0x666130-adj |
| GameCreature | 0xBDC | 0x40d010 | GameEntity 0x4678a0 | 0x64D7CC [-4,+0x800] | this+0x804 | 0x64D7C8 | 0x64D7?? |
| GamePlayer | 0xBFC | 0x411820 | GameCreature 0x40d010 | 0x64E974 [-4,+0x820] | this+0x824 | 0x64E970 | 0x64E90C? (0x41187e imm unchecked) |
| GameUsableObject | 0x498 | 0x412420 | GameEntity 0x4678a0 | 0x64F3FC [-4,+0xBC] | this+0xC0 | 0x64F3F8 | **0x64F30C** |
| GameDoor | 0x4C8 | 0x40e7f0 | GameUsableObject 0x412420 | 0x64DEDC [-4,+0xEC] | this+0xF0 | 0x64DED8 | 0x64DDEC |
| GameContainer | 0x4D8 | 0x408c20 | GameUsableObject 0x412420 | 0x64D09C [-4,+0xFC] | this+0x100 | 0x64D098 | 0x64CFAC |
| GameProjectile | 0x494 | 0x411c60 | GameEntity 0x4678a0 | 0x64EE8C [-4,+0xB8] | this+0xBC | 0x64EE88 | 0x64ED9C |
| GameItem | 0x78C | 0x410180 | GameEntity 0x4678a0 | 0x64E3FC [-4,+0x3B0] | this+0x3B4 | 0x64E3F8 | (ctor 0x4101e9) |
| GameWeapon | 0x808 | 0x414510 | GameItem 0x410180 | 0x650024 [-4,+0x42C] | this+0x430 | 0x650020 | 0x64FF34 |
| GameArmor | 0x7A8 | 0x408580 | GameItem 0x410180 | 0x64CAF4 [-4,+0x3CC] | this+0x3D0 | 0x64CAF0 | (0x4085d9 region) |
| GameAmmo | 0x7B0 | 0x407fb0 | GameItem 0x410180 | 0x64C5AC [-4,+0x3D4] | this+0x3D8 | 0x64C5A8 | (0x408007 region) |

Own-field regions (inserted *before* the Entity vb, MSVC ordering non-virt base → own → virt
base): Creature own = 0xA4..0x803; Player own = 0x800..0x823 wait — Player own starts at 0x800
(map*) through 0x823; Usable own = 0xA0..0xBF; Door own = 0xC0..0xEF; Container own =
0xC0..0xFF; Projectile own = 0xA0..0xBB; Item own = 0xA4..0x3B3; Weapon own = 0x3B4..0x42F;
Armor own = 0x3B4..0x3CF; Ammo own = 0x3B4..0x3D7.

## 3. Entity-shared fields written by F3 class ctors (Entity-vb-relative)

| E offset | value written | writers (EA) | notes |
|----------|---------------|--------------|-------|
| +0x15C | **1=creature, 2=usable, 3=door, 4=container, 5=item, 6=weapon, 7=armor, 8=projectile, 9=ammo** | 0x40d180, 0x4124a5(?0x41249e), 0x40e8a1, 0x408cc6, 0x410??? (imm 5), 0x414??? (imm 6 → raw `c7 84 30 60 01 00 00 06` @0x4145e4), 0x408604, 0x411cf5, 0x408031 | **Equals the factory case number.** GamePlayer does NOT rewrite it → player keeps tag 1. GameEntity (case 0) and JE::Entity::ctor write nothing here. **UPDATE (session 11): readers found** — server Use/Open handlers (packet ids 78/79) dispatch on `*(entity+348)` (==0x15C) to pick GameActionOperate{Door,Container}; this slot is the runtime classId for request routing, not debug-only. See `2025-10-20-server-receive-handler-semantics.md` §2. |
| +0x15C ⚠ | `WaterTileModelInstance*` | JE::Entity::meth_0x5ab3a0 @0x5ab662 | **Collision to resolve**: the engine's water-tile entity setup stores a pointer to the *same* slot. Interpretation options: (a) F3 entities never take the water-tile path, slot is dual-use by entity origin; (b) my E base is off by 4 for the F3 writes. Raw bytes support (a): both compute `this + vbdisp + 0x160` with identical pattern. Only 1 DB usage of Entity+0x15C existed (0x5ab662) — F3-side *readers* are unaccounted; find them next session. |
| +0x160 | 0 (GE ctor), then model-cache handle set at runtime | 0x467989-adj → **actually +0x174** for GE: see next row. +0x160 runtime: JE::Entity::meth_0x54dc50 (release old via `sub_5853E0(g_model_cache)`, load new, then read model bbox → DynamicObject +0x10..0x24) | +0x160 = cached model handle (38 DB usages — engine hot field). `meth_0x54dc50` also maintains entity "name up to '.'" string at DynamicObject gap1AC[108]. |
| +0x174 | dword 0 (GameEntity ctor) | 0x467989 | 6 DB usages, name pending |
| +0x178 | byte 0 (Projectile) | 0x411d2e | |
| +0x179 | byte 0 (Usable/Door/Container/Projectile) | 0x4124cf, 0x40e8c2(?), 0x408ced(?), 0x411d06 | "usable-family" byte flag |
| gap78/other | JE::Entity::virt_meth_0x550750(entity, 5) in GE ctor | 0x46797e | sets Entity mode=5 at construction |

## 4. Per-class own-field writes (from raw disassembly; offsets absolute in the most-derived class)

- **GameUsableObject (0x498)**: `[+0xA0]=0` dword; `rep stosd [+0xA4..+0xBB]=0` (6 dwords);
  E+0x15C=2; E+0x179=0.
- **GameDoor (0x4C8)**: bytes `[+0xBC],[+0xBD]=0`; `[+0xC8]=0` (SSO string buf start, size
  `[+0xD8]=0`, cap `[+0xDC]=15` → std::string at +0xC8..+0xDF); `[+0x57-slot]` helper
  `sub_56E0F0()` result → list head at `[edi]` (near +0xE0 region — see ctor 0x40e88x);
  E+0x15C=3.
- **GameContainer (0x4D8)**: like door plus second std::string (+0xE4..+0xFB: size +0xF4=0,
  cap +0xF8=15), `[+0xC0]=0`, `[+0xD8]=0/[+0xDC]=cap`, E+0x15C=4.
- **GameProjectile (0x494)**: `[+0xA0]=0` byte, `[+0xA4]=0`, `[+0xA8]=0`, `[+0xAC]=0`,
  `[+0xB0]=0`, `[+0xB4]=0`; E+0x15C=8; E+0x178=0; E+0x179=0; tail call
  `JE::Entity::meth_0x467be0(this+0xA4→vb-adjusted, 0)` at 0x411d35.
- **GameItem (0x78C)**: `RbTree_InitSentinel` at 0x410205 (own tree!), SSO strings with
  cap 15 at +0xCC area, +0xE4/0xE8, +0x108/0x10C; `[+0x160]=0` own-dword (NOT E-relative —
  own region); more own init beyond 0x410245 (truncated here — full log pending).
- **GameWeapon (0x808)**: adds string (cap 15 @+0x3E4, size +0x3E0) and `[+0x408]=0`;
  E+0x15C=6.
- **GameArmor (0x7A8)**: E+0x15C=7 (own writes pending — small region 0x3B4..0x3CF).
- **GameAmmo (0x7B0)**: E+0x15C=9 (own region 0x3B4..0x3D7, writes pending).

## 5. Practical guidance for the reimpl (updated from HANDOFF §Decisions)

- **The vbtable question is answered for you**: every Entity-vb offset is in §2 above. If you
  keep the flat `SpawnedEntity : JE::Mover` model, store `entityVbOffset` per class from that
  table when you ever need per-class field access. The Mover-relative fields stay valid for
  ALL eleven classes because they all share the same JE::Entity vb body.
- **The +0x15C tag is the runtime `classId`**: your `SpawnedEntity.classId` should mirror
  where the original puts it (Entity+0x15C, value == factory case). Player deliberately
  shares the creature tag (1) — if your reimpl gives player its own id, that's a deviation
  to note.
- The E+0x15C dual-use (§3 ⚠ row) is the open RE item; we'll find F3 readers next session.
- Door/Container sibling relationship — check your `ClassIdForResourceName`: .DOR→3, .CON→4,
  both must *create as* UsableObject descendants, not Container→Door.

## 6. Not yet done (queue)

- Full Item own-field log (ctor past 0x410245) + Weapon 0x7C-byte own block.
- F3-side readers of Entity+0x15C / +0x174 / +0x179.
- cls_0x55a280 effect-node payload (session 3 handoff item).
- `GetUsableSkills` + Reload linkage.
