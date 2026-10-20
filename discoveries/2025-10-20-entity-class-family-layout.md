# 2025-10-20 — F3 entity class family: factory, sizes, layouts (session 1)

Target: **ORIGINAL_REFERENCE** (`artifacts/F3.exe.i64`, exe `$OPENVB_ORIG_EXE` =
`/nix/store/a1j412798wh979ig7xp3bp9aji4zfpz0-f3demo-1.0/share/f3demo/F3.exe`).
Read-only session. Motivation: the reimpl's `src/F3/MapEntitySpawn.hpp` explicitly flags
"DEVIATION (entity layout reconstruction pending)" — this note supplies the original layouts.

All addresses are original-F3 VAs (imagebase 0x400000). .rdata dumps were taken directly from
the PE (recipe in §6).

---

## 1. The class factory (confirmed)

`F3::CreateGameEntityByClass_0x46f530` — `__stdcall (int classId) -> JE::Entity*`.
Single code xref: thunk `F3::GameWorld::virt_meth_0x46fdc0_46fdc0` (0x46fdc0) which is the
world-server **create-template** hook — consistent with the spawn chain already wired in the
reimpl (server vft+52 lookup-by-name → vft+24 create).

Switch on `classId`, each arm is `operator new(size)` → `ctor(this, 1)` → on success returns
`(v2 + *(v2[1] + 4) + 4)`, i.e. the address of the **virtual JE::Entity base** (vbptr at +0x4,
offset table entry +4 — see §3). On allocation failure returns 0. `default:` falls through to
`JE::World::CreateEntity(classId)` (engine-level classes, e.g. camera/light — not F3 entities).

| case | class | new size | ctor | observed sub-objects (from ctor) |
|------|-------|----------|------|----------------------------------|
| 0 | `F3::GameEntity` | 0x47C | 0x4678a0 | 5×12-byte RbTree heads @ +0x38; JE::Entity vb @ +0xA4 |
| 1 | `F3::GameCreature` | 0xBDC | 0x40d010 | see §4; JE::Entity vb @ +0x804 |
| 2 | `F3::GameUsableObject` | 0x498 | 0x412420 | +0x1C over GameEntity |
| 3 | `F3::GameDoor` | 0x4C8 | 0x40e7f0 | +0x4C over GameEntity |
| 4 | `F3::GameContainer` | 0x4D8 | 0x408c20 | +0x5C over GameEntity |
| 5 | `F3::GameItem` | 0x78C | 0x410180 | +0x310 over GameEntity |
| 6 | `F3::GameWeapon` | 0x808 | 0x414510 | +0x7C over GameItem |
| 7 | `F3::GameArmor` | 0x7A8 | 0x408580 | +0x1C over GameItem |
| 8 | `F3::GameProjectile` | 0x494 | 0x411c60 | +0x18 over GameEntity |
| 9 | `F3::GameAmmo` | 0x7B0 | 0x407fb0 | +0x24 over GameItem |
| 10 | `F3::GamePlayer` | 0xBFC | 0x411820 | +0x20 over GameCreature |

Size deltas: Weapon(0x808) − Item(0x78C) = 0x7C; Armor(0x7A8) − Item = 0x1C;
Ammo(0x7B0) − Item = 0x24 — supports Item→Weapon/Armor/Ammo single inheritance
*(inferred, ctor decompile of those classes pending confirmation next session)*.
Player(0xBFC) − Creature(0xBDC) = 0x20. UsableObject/Projectile are direct GameEntity
extensions (small deltas, no item content).

The reimpl's `enum kClassGameEntity..kClassGamePlayer` ordering in `MapEntitySpawn.hpp`
matches the shipped case numbers 0..10 exactly — keep it.

## 2. GameEntity field map (ctor 0x4678a0, __thiscall(this, int alsoConstructEntity))

When `alsoConstructEntity != 0` (factory path only): `mbr_0x4 = &unk_666134` (vbtable) and
`JE::Entity::ctor_0x5513a0(this+0xA4)`. Then the GameEntity fields:

| off | init | evidence | notes |
|-----|------|----------|-------|
| +0x00 | vftptr ← GameEntity vftable 0x666130 | 0x4678ef | |
| +0x04 | vbptr ← 0x666134 | 0x4678d4 | MSVC virtual-base pointer; vbtable `[-4, +0xA0]` raw-dumped |
| +0x08 | 0 | 0x467929 | |
| +0x0C | 0 | 0x46792c | |
| +0x10 | 0 | 0x46792f | |
| +0x14 | 0 | 0x467932 | |
| +0x18 | **4** | 0x467935 | nonzero-default field, 7 xrefs at 0x46764d..0x467760 (entity-file read fns) — probably a type/mode tag; *open question* |
| +0x1C | 0 | 0x46793c | |
| +0x20 | **1** | 0x46793f | returned verbatim by vslot-0 getter `0x4080c0` — a state/visibility flag consumed via vft[0] |
| +0x24 | 0 | 0x467946 | |
| +0x28 | world-server-assigned id, else 0 | 0x4679b3 / 0x4679b8 | `if (F3::global_area_map_icons[0]->Get_mbr_0x4_0x5b4f30() & 1) mbr_0x28 = WorldServer->vft[8]()` — "net id" from the running world server, 0 in client/editor |
| +0x2C | 0 | 0x4679be | |
| +0x30 | **7** | 0x4679c6 | |
| +0x34 | **7** | 0x4679c9 | (0x30/0x34 are a pair — cursor/target team? faction? *open question*) |
| +0x38 | 5×12-byte objects, ctor `cls_0x401b00::ctor_0x401b00` | 0x467921 | each = RbTree head `{sentinel@0, count@4, node@8}` (`JE_RbTree_InitSentinel_0x56e0b0`, see JE note §5) |
| +0x8C | **−1** | 0x467963 | sentinel-init — *open question* |
| +0x90 | 0 | 0x467949 | |
| +0x94 | **1092616192 = float 8.0f** | 0x46794f | |
| +0x98 | **2** | 0x467959 | |
| +0x9C | byte 0 | 0x4679cc | |
| +0xA0.. | (JE::Entity vb body; ends 0x47C−0xA4=0x3D8 bytes) | | |
| entity+24 (abs ~+0x108?) | `JE::Entity::virt_meth_0x550750(entity, 5)` | 0x46797e | sets Entity flag/mode to 5 during GameEntity ctor |
| gfx_line_system.gap78[adj+24] | 0 | 0x467989 | write into Entity's GfxLineSystem sub-struct |
| gap1AC[adj+0xD8] | 1 | 0x4679d9 | byte flag inside Entity subobject (adj via vbptr) |

IDA member index corroborates: named members only exist at 0x0,0x4,0x8..0x38,0x8C..0x9C plus
the Entity struct member `.?AUEntity@@_0xa4` (size 984). Fields 0x40..0x88 are the 5 trees +
unaligned padding; nothing is named there yet.

Virtual slots observed on GameEntity (from method index; vftable order *not* yet fully dumped):
- `0x4080c0` — get mbr_0x20 (this is the only entry in vftable @0x666130 length 1: the rest are
  inherited Entity slots)
- `0x467a00` — dtor-ish: calls `meth_0x467af0`, `JE::Entity::meth_0x54fff0`, conditional `free`
  of the Entity-array tail (`param_1 & 1` = deleting dtor flag)
- `0x467e30` — thunk → `JE::Entity::meth_0x467be0` (vbptr-adjusted)
- `0x468300` — thunk → `sub_467E90`
- `0x468710` — thunk → `sub_468350`
- `0x468ad0` — **entity file load** (see §5): dispatches chunk fourcc `EFTR` / `GENT`
- `0x467a10..0x467ae0` — 14 one-instruction stubs (all `virt_meth`, bodies at 0x467aX0 return
  consts/no-ops — typical per-class override stubs, likely script/effect hooks)

## 3. Virtual-base geometry (why `this + 2052` shows up everywhere)

MSVC virtual inheritance layout confirmed empirically:

- GameEntity: vftptr@+0x0, **vbptr@+0x4**; vbtable raw at 0x666134 =
  `int32[-4, +0xA0]` → JE::Entity vb sits at vbptr+0xA0 = **this+0xA4**.
- Factory return `(v2 + *(v2[1]+4) + 4)` = this + vbtable[1] + 4 = Entity* — the factory hands
  out the **Entity interface pointer**, not the GameEntity pointer. Every caller of
  CreateGameEntityByClass receives a `JE::Entity*` and gets back to F3 fields via
  `thunk-adjust (this - *(this-4))`.
- GameCreature: vbptr slot written to 0x64D7CC with table `[-4, +0x800]`; ctor line
  `JE::Entity::ctor_0x5513a0(this + 2052)` confirms Entity at **this+0x804** (= 2052).
  Its vftables (0x64d7c8 primary + 0x64d7cc vbptr vft) share slot0 `0x4080c0`
  (get mbr_0x20 is *not* overridden; the GameCreature vftable merely re-exposes it).

Implication for reimpl: the current `SpawnedEntity : JE::Mover` excerpt avoids all this. If you
later want 1:1 binary-compatible layouts, the cheapest order is (a) fix GameEntity own-fields
(§2), (b) give JE::Entity its real 0x3D8-byte body, (c) add vbptr indirection lazily or flatten
the hierarchy with a stored `entity*` (behavioral parity, no ABI parity).

## 4. GameCreature ctor 0x40d010 highlights (size 0xBDC = GameEntity 0x47C + 0x760 own)

Own fields (outside the Entity vb) with ctor evidence:

| off | init | evidence | note |
|-----|------|----------|------|
| +0x6F0 | 0 | 0x40d2ce | reset twice (also 0x40d2d4) — post-init "clear" field |
| +0x6F8 | `sub_40A190()` | 0x40d123 | allocator call — resource handle? |
| +0x6FC | 0 | 0x40d13b | |
| +0x704 | `sub_40A190()` | 0x40d141 | same allocator |
| +0x708 | 0 | 0x40d153 | |
| +0x70C..0x72B | memset 0 ×0x20 | 0x40d291 | |
| +0x710 | mbr (usage only) | 0x40d286 | inside memset-zeroed block tail |
| +0x730, +0x748, +0x7B0, +0x7C8 | 0 (loop, 6 iterations of 4 dwords) | 0x40d2a0-a8 | arrays: 6× stride pattern over 0x714..0x734, 0x744..0x764, 0x7A4..0x7C4, 0x7C4..0x7E4 *(approx; pattern = 4 zeroed qword-triples ×6)* |
| +0x7EC | 0 | 0x40d264 | |
| +0x7F0 | RbTree sentinel | 0x40d2da | +0x7F4 count 0, +0x7F8 = 0 |
| +0x7FC | 0 | 0x40d2da | IDA already names this `maybe_agility_0x7fc` — creature stat block tail *(inferred name, original nudge)* |
| +0x804 | JE::Entity vb (0x3D8) → ends 0xBDC | | |

Inside the Entity vb (adjusts via vbptr, all shown as `gap1AC[...]`/`mbr_*` in decompilation):
- two more std::string-style SSO buffers: `gap1AC[52..67]` cap 15 / `gap1AC[80..95]` cap 15,
  both filled from `Entity.mbr_0x178/0x190` string via `cls_0x50db20::meth_0x401bd0(dest, s, strlen(s))`
  at 0x40d1cd/0x40d209 → **name/model copy at creature-init**
- `dynamic_object_0x0.gap1AC[56]=0, [60]=15`, `[88]=0, [92]=15`, `[72]=0`, `[96]=0`, `[100]=0` — SSO headers
- 12×0x1C-byte array `gap1AC[104..]` with ctor `cls_0x50db20::ctor_0x404bc0` (0x40d0ff) — 0x1C-byte
  records ×12: **candidate for per-creature slot/equipment table (12 slots?)** *open question*
- 13×0x38-byte array at `gap340[36..488]` ctor `cls_0x40a220::ctor_0x40a220` (0x411e18 region; called 0x40d11e)
  — 0x38 records ×13: **candidate for skill/perk lists**
- DynamicObject stat block zeroed+seeded at 0x40d21b..0x40d25e:
  `mbr_0x14=1, mbr_0x18=0, mbr_0x1c=1, mbr_0x20=0, mbr_0x24=18, mbr_0x28=0, mbr_0x2c=0, mbr_0x30=0, mbr_0x34=0`
  — `18` default matches **SPECIAL-era max/stat default (Fallout: New Vegas = 18 base points?)** *(inferred)*
- `memset(&entity.mbr_0x38, 0, 0x34)` twice (0x40d277, 0x40d2bd) and
  `memset(&gfx_line_system.mbr_0x2c, 0, 0x148)` (0x40d284)
- `JE::Entity::meth_0x4097a0_4097a0(this+2052)` at 0x40d2c5 — entity post-construct hook
- `gfx_line_system.gap78[vbadj] = 1` (0x40d180) — "is-creature" flag in shared GfxLineSystem field

Behavioral strings attached to GameCreature vftable region in .rdata (dumped, §6):
`GameCreature::Reload(int)` errors: `"- making a safe attack from entity %s\n"`,
`"- No Documentation found"` → the virtual slot for `Reload(int)` exists on creature
(and `GameWeapon::Reload(int) - Attack Weapon %s ...` nearby) — ties into
GameActionReload / GameActionAttack RE, **next session target**.

## 5. Entity resource-file chunks (from `GameEntity::virt_meth_0x468ad0_0x468ad0`)

```
int chunkTag = *(a2 + 8);
if (chunkTag == 0x52544645 /* 'EFTR' */)   → cls_0x463870::meth_0x468930(entity-164, a2, res)
    on failure: "Error reading effect tree from entity file."
else if (chunkTag == 0x544E4547 /* 'GENT' */) → cls_0x463870::meth_0x466510(entity-164, a2, res, a4)
    on failure: "Error reading entity game data from entity file."
else: tail-jump 0x5528D0 (shared error path)
```

- `'EFTR'` = **Ef**fect **Tr**ee chunk; `'GENT'` = **G**ame **Ent**ity data chunk.
  (Adjacent string in .rdata: `"Unexpected effect chunk ID. Event ID 'EFEV' expected."` →
  the effect-tree reader expects `'EFEV'` event sub-chunks next; another string:
  `"Unknown effect chunk ID."`)
- On failure it calls `JE::ResourceSystem::ReleaseAndFree_0x498f30(global_resource_system_0x707da8, res)` —
  resource lifetime pattern confirmed: entity file loader consumes/releases the `JE::Res*`.
- `cls_0x463870` is the "entity game data" aggregate reached at `this - 164` from the
  Entity-vb pointer (0x47C−0xA4−... — it is the GameEntity own-data block, i.e. fields §2).
- `JE::LogDebugString_0x497120` — the debug logger used by all load errors.
- Footstep-FX strings adjacent in .rdata (`FX_FootR_Hum_Sand_Base_01.veg`, `FX_FootL...`,
  tokens `FR FL UP EP PU RL DM`) belong to the effect-tree/event code in the same TU —
  the **DM** token pairs with the "safe attack" reload logic, **EP/PU** plausibly equip/put-up.

## 6. Raw dump recipe (no IDB needed)

To re-derive any .rdata content in this note:

```python
import struct
pe = struct.unpack_from('<I', data, 0x3c)[0]           # data = F3.exe bytes
imagebase = struct.unpack_from('<I', data, pe+24+28)[0]  # 0x400000
# section headers: pe+24+opt+i*40, {name, vsize, va(12), rawsize(16), rawptr(20)}
# VA→off: off = rawptr + (va - (imagebase+secva))
```

Verified constants: GameEntity vftable 0x666130 → slot0 0x4080c0; vbtable 0x666134 =
`ff ff ff ff a0 00 00 00`; GameCreature vbtable 0x64d7cc = `ff ff ff ff 00 08 00 00`.

## 7. Gaps / questions for the reimplementation agent

1. **Which 5 RbTree heads live at GameEntity+0x38?** They are the most-referenced GameEntity
   fields in entity-file reads (0x46764d..0x467760, 10 xrefs on mbr_0x4). If your spawn path
   needs any of them initialized, confirm against §2 defaults (all zeroed sentinel heads).
2. **mbr_0x18=4, mbr_0x30/34=7, mbr_0x98=2 defaults** — no names yet. If your EME2→entity
   init writes them, we should name them; if not, keep the raw defaults for fidelity.
3. **Player at +0xBFC** differs from Creature by only 0x20 own-bytes — is the party UI data
   elsewhere? Next session I plan to decompile `GamePlayer::ctor_0x411820` and the 12×0x1C /
   13×0x38 arrays (`cls_0x404bc0` / `cls_0x40a220`) which likely *are* the skill/slot tables
   (12 == Fallout NV's 12 inventory slots? 13 == 13 SPECIAL+skills? probably not — flag it).
4. **Do NOT spend time on binary-compatible vbptrs** unless a test compares raw dumps — the
   factory only hands out `JE::Entity*`, so a flattening shim is behaviorally safe.

## 8. Suggested next discovery sessions (my queue)

- `GamePlayer::ctor_0x411820` + Player-specific vft slots (compare creature)
- `cls_0x404bc0` / `cls_0x40a220` array-record decodes (creature skill/slot tables)
- The entity-file reader family `cls_0x463870::meth_0x468930` / `meth_0x466510` (EFTR/GENT chunk
  wire format — pairs with the EME2 record work already done)
- `GameCreature::Reload(int)` vslot + `GameActionReload` linkage (strings evidence above)
