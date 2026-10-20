# Session 6 — classId readers, Reload/weapon/ammo subsystem, Creature SPECIAL + damage formula

Target: ORIGINAL_REFERENCE (`ida_original_f3`, `artifacts/F3.exe.i64`). Read-only; no IDB or source edits.
Method note: besides decompiles, this session used **raw `.text` byte scans** of `$OPENVB_ORIG_EXE`
for the vb-displaced addressing form `mod=10 rm=SIB disp32=0x160` (= Entity+0x15C given base=this,
index=`*(vbptr+4)`=Entity−4−this), because the F3.json usage DB only carries 1 of the ~8 classId
access sites. Scan recipe in §1; false positives = stack canary prologues (`[esp+160h]`), always
check SIB base/index ≠ esp.

Carries and closes queue items from [session 5] §6 and HANDOFF "known-unknowns".

## 1. Entity+0x15C classId — complete reader set (queue item 2 CLOSED)

Writers (verified again by scan, encoding `c7 84 {30|32} 60 01 00 00 imm`):
0x40d180 (=1 creature), 0x41249e (=2), 0x40e8a1 (=3), 0x408cc6 (=4), 0x4103f3 (=5, GameItem ctor —
this one was `(imm 5)` only in session 5), 0x4145e4 (=6), 0x408604 (=7), 0x411cf5 (=8), 0x408031 (=9).
GamePlayer ctor absent → player keeps tag 1 (re-confirmed: no write in ctor_0x411820).
**JE:: engine: zero readers.** All readers are F3 game logic:

| reader | function | check |
|---|---|---|
| 0x407494 | `F3::GameActionReload::OnServerUpdate_0x407270` | `classId == 9` — filter inventory walk for ammo |
| 0x40b3f7 | `F3::GameActionAttack::ResolveWeaponAndAttackMode_0x40b380` | `classId == 6` — equipped item must be weapon; on fail logs `"GameEntity::ResolveWeaponAndAttackMode(...) "` |
| 0x413658 | `F3::GameWeapon::UnloadPrimaryAmmo_0x4135c0` | `classId == 9` — find matching ammo in owner inventory |
| 0x4138b0 (disasm `0x4138a8`) | `sub_413810` = **GameWeapon::UnloadSecondaryAmmo** | `classId == 9` (same walk, secondary keys) |
| 0x408229 | `F3::GameAmmo::virt_meth_0x4082c0` | **reads own classId into a packed buffer** at `[entityVb-0x278]`, alongside ammo fields — runtime classId crosses the network wire |
| 0x43e8b6 + 0x43e8c9 | `F3::GameActionEquip::meth_0x43e760` (inventory panel refilter) | `classId(candidate) == classId(item)` — equip list groups by runtime class |
| 0x458253 | `JE::CLoadMapInterface::meth_0x4580c0` (world/inventory UI list) | `classId == 1` → "is creature" gate for the 10-slot item grid path |

**Water-tile dual-use verdict (session 5 §3 ⚠):** option (a) holds. The only writer of a
`WaterTileModelInstance*` to the same slot is `JE::Entity::meth_0x5ab3a0` @0x5ab662 (engine water
path). Every classId reader above is F3 game logic on factory-spawned / map-loaded entities; the
water path never feeds them. No further collision analysis needed — reimpl can treat +0x15C as
pure classId for F3 entities. *(inferred caveat: a water-tile entity inspected by CLoadMapInterface
would read a pointer low-word as classId ≠ 1, so it simply behaves as "not a creature" — benign.)*

## 2. GameEntity vft[0] naming question — ANSWERED for the reimpl agent

Their question: "vft[0] (0x4080c0) returns +0x20, but wire maps MAX→+0x20, CUR→+0x24 — is vft[0]
get-MAX, or did names swap?"

**No swap. Their naming (statMax@+0x20, statCur@+0x24) is correct; vft[0] = get-MAX.**
Evidence:
1. `GameEntity_vftable_666130` has **exactly one entry** (raw dump of 0x666130..0x666138:
   `0x4080c0`, then `0xfffffffc` = next RTTI object — this is the whole table). So there is no
   sibling "get cur" virtual; +0x24 has no F3-side reader in this binary at all (DB: usage = ctor
   write only).
2. Loader `cls_0x463870::meth_0x463910` @0x463910: GENT dword[6]→+0x20, dword[7]→+0x24, then
   clamp `if (v24 > v20) v24 = v20` → 0x24 is the value bounded *by* 0x24's pair → cur bounded by
   max. Direction confirms wire order = (max, cur), matching the (100,100) tail of every sampled
   GENT.
3. Ctor defaults `+0x20=1, +0x24=0` (max 1, cur 0) — a conservative "empty" state, consistent
   with cur/max, not with cur-then-max (cur=0 max=1 only reads sane as max=1).
Caveat worth keeping: with the (100,100) GENT tail this pair *could* be HP max/cur, but given
vft[0]'s single-entry table and zero observed mutation sites, this pair is **write-mostly** in
the demo binary. Don't build reimpl logic on "cur decreases" behavior until a writer appears.

## 3. GENT provenance update (corrects session 3 §1)

`cls_0x463870::meth_0x463870` @0x463870 (unconditional, ver 0): reads dwords[0..5] →
`+0x08,+0x0C,+0x10,+0x14,+0x18,+0x1C` (the last via `&this->mbr_0x18 + 1`).
`meth_0x466510` gates: `v4 = resource.header_statcount` (param+12): `v4==0` skips
`meth_0x463910` (+0x20/+0x24); `v4<2` skips one more, and `v4<3` skips one more — **but the two
gated calls land on `net::client::virt_meth_0x409bb0`, a stub that `return 1` and reads nothing**
(vft slots 22–27,29 of `net::client` @0x6a1934 all alias it). Consequence: shipped F3.exe carries
**no GENT v2/v3 data** — the 8-dword block (+0x08..+0x24) is the complete GENT→entity map.
- GameEntity+0x18 default 4: it **is** GENT dword[4] (provenance known; semantics still unnamed).
  Default_PC's GENT dword[4]=0 — matches reimpl observation.
- +0x1C default 0 = GENT dword[5].
- **+0x30/+0x34 = 7,7: NOT from GENT.** No F3-side reader found in this session either (scan for
  `disp32=0x178/0x17C` vb-forms returned only ctor writes and stack misparses). Treat as
  ctor-only defaults, likely (inferred) enum pair — still open.

## 4. Weapon/ammo/reload field map (NEW — unlocks the reimpl Reload work)

> **⚠ SUPERSEDED-IN-PART (2025-10-20 later round):** the byte offsets for the weapon magazine
> trio + caches in this §4 table were derived from inconsistent decompiler `this+N` units and
> are corrected in `2025-10-20-session17-tohit-subsystem.md` §10 (raw-verified: current
> +0x3EC/+0x3F4, capacity +0x3F0/+0x3F8, caches **+0x424/+0x428**, ammo to-hit bonus +0x3C4).
> Roles/flow below remain valid; trust session17 §10 for offsets.

All offsets below are **byte offsets in the most-derived class** (`*(this + N)` in the decompile
was dword-indexed; converted to bytes). Verified via `OnServerUpdate_0x407270`,
`UnloadPrimaryAmmo_0x4135c0`, `sub_413810` (=UnloadSecondary), `ReloadPrimaryAmmo_413A90`,
`CalculateDamage_413310`.

### GameWeapon (0x808) own block
| offset | role | evidence |
|---|---|---|
| +0x3B4 | primary caliber | cmp with ammo+0x3B0 @0x413697/0x413ada; passed to ammo-prototype lookup @0x413b9d |
| +0x3B8 | primary ammo modifier (type) | stored as `ammo.modifier` @0x413b8e; server-synced @0x413c10 |
| +0x3BC | magazine capacity (primary) | `!v4 → "This weapon doesn't even use ammo."` @0x413abd |
| +0x3C0 | current rounds in magazine | `capacity - cur` = space to fill @0x413b10 |
| +0x3C4 | cached loaded-ammo instance ptr | set @0x413b33; cleared on unload |
| +0x3C8 | reload duration / cooldown ticks | `mbr_0x2e` compare @0x407521 |
| +0x3D0 | secondary caliber | `sub_413810` mirror checks @0x4138e7 |
| +0x3D4 | secondary modifier | |
| +0x3D8 | secondary capacity | *(inferred from symmetry)* |
| +0x3DC | secondary current | |
| +0x3E0 | secondary cached ammo ptr | cleared @0x413a01 |
| +0x3EC | reload-delay (primary) | read in OnServerUpdate @0x4074f2 |
| +0x3F4 | reload-delay (secondary) | @0x4074fd |

### GameAmmo (0x7B0) own block
| offset | role | evidence |
|---|---|---|
| +0x3B0 | caliber | `param_1[236] != weapon 0x3B4 → "Wrong caliber of ammo passed for reload."` @0x413ae1 |
| +0x3B4 | modifier/type | matched against weapon +0x3B8 |
| Entity-vb **Entity+0x3BC** (raw `[entity−4 + 0x3C0]`, same −4 bias as classId 0x160→0x15C) | **stack count** (dword) | reload takes `min(space, count)` @0x413b7e; on empty removes the stack (`meth_0x551b60` @0x413c44 = destruction path); `GameServer::meth_0x552d40(netId(Entity+0x1E0), count)` @0x413c75; **equip UI sorts stacks by it** @0x43e92e/0x43e945; ammo→net pack reads `[3B4h]` @0x408240 |

So: ammo instances and items share a common Entity-vb tail field **Entity+0x3BC = quantity** (raw disp 0x3C0 on the Entity−4 base) —
strong new candidate for reimpl's "stack count" storage (Mover excerpt +0x31C if their base is
Entity−4, i.e. the interface minus vftptr).

### Reload chain (GameActionReload::OnServerUpdate_0x407270)
1. owner `RTTI→GameCreature` else FatalError `"…doesn't belong to a game creature."`
2. weapon = equipped-slot lookup (`GameActionEquip::meth_0x54be10`), `RTTI→GameWeapon`;
   if unset: `Get_mbr_0x98_463b60(creature)` → equipped item via `GameInventory::meth_0x4693d0`;
   weapon's initial ammo type copied from `[weapon+0x3B8]`-family.
3. ammo = explicit ptr, else **walk owner inventory RbTree**: root `*(owner + 0x16C)`,
   begin `*(root + 0x0C)`, node payload `node[2]`, next `*node` — filter `classId==9` +
   (caliber,modifier) == weapon's primary (or secondary when action flag `BYTE2(mbr_0x28)` set).
4. when the queued duration elapses: `GameWeapon::ReloadPrimaryAmmo_413a90` /
   `ReloadSecondaryAmmo_413ca0`; moves `min(magazine_free, stack_count)` rounds, updates the
   weapon's +0x3EC and the stack's Entity+0x3BC, server-syncs by netId.
5. debug HUD string `"Calling weapon's Reload function now."` emitted through the global
   interface logger `cls_0x558740::meth_0x5583e0`.

### Ammo prototype registry
`sub_408110(caliber, modifier)` → RTTI-casts the global world (`global_cls_0x616328_0x70bd98`) to
`GameWorld` → `meth_0x46fac0`: bounds-check `caliber ≤ 20 (0x14)`, `modifier < 8`, returns the
**default GameAmmo instance for that (caliber,modifier) pair** (used to synthesize ammo when
unloading a loaded weapon with no matching stack in inventory).

## 5. Combat damage formula (CalculateDamage / CalculateUnarmedDamage / queued)

- `RandRange(lo,hi) = sub_48CD00`: `lo + rand() % (hi-lo+1)` (swaps args if inverted; `==` returns lo).
- **Weapon damage** `GameWeapon::CalculateDamage_413310(attackDesc*, …, target, calledTarget, part)`:
  `dmg = rand(count@desc+0x38, sides@desc+0x3C) + skillBonus@desc+0x24`;
  if `attack flag desc+0x10` (uses-ammo): ammo (from prototype cache +0x3C4) adds
  ammo bonuses (raw @0x4133d0-0x4133e0): `count adder = ammo+0x3B8`,
  `sides adder = ammo+0x3BC`, `DT-pierce = ammo+0x3C0`;
  body-part DR `= target[4·part + (0x7B0 | 0x7C8)]` — **row selector is the CALLED-SHOT PART**
  (param_5): parts 5/6 → +0x7C8, parts 0-4/7 → +0x7B0, `default:` logs
  "GameWeapon::CalculateDamage() - Unknown CALLEDTARGET encountered." and **still computes**
  (falls back to the 0x7B0 row, does NOT return 0) — raw 0x413414-0x413442 (implementer-B
  C-33 verified; my earlier style-reading/*(inferred)* labels superseded);
  `dmg -= DR/2 + rand(0, DR/2)`, floor 0. `calledTarget == 9` ("no body part") → damage 0.
- **Unarmed** `GameCreature::CalculateUnarmedDamage_409f90`: same skeleton; `+MELEE_DAMAGE`
  derived when descriptor slot `[4] == 4` — **[4] = MODE DAMAGE TYPE, not part (coordinator
  ruling 2026-10-20; my earlier arm/hand inference VOID)**; **the zero-on-9 guard here is CONDITIONAL**:
  `if (param_4 == 9 && !RTTI(usable))` → only creatures with unset part return 0;
  GameUsableObject targets fall through to usable-DR math (implementer-A C-27 raw note @0x409fec;
  the WEAPON-path guard at 0x41333d stays unconditional — the two sites differ); vs `GameUsableObject` targets DR =
  `usable[part*4 + 0xA4]` (the 6-zeroed-dword ctor block — doors/containers take full damage by
  default).
- `GameActionAttack::CalculateAndQueueDamage_0x404060` builds record
  `{damage, part, attackerPerk, weaponOrFist, targetId…}` into `mbr_0xbc`'s list and logs
  `"Damage for %d queued."`. Flag fields: `mbr_bool_0x71` = unarmed, `mbr_bool_0x70`,
  `mbr_bool_0x6e` passed through.
- UsableObject ctor zeroes `[+0xA0]` + `rep stosd [+0xA4..+0xBF]` (session 5 §4) — **confirmed
  purpose: per-body-part damage resistance array.**

## 6. GameCreature attribute block (first consumers found)

`GetDerivedAttribute_0x409a60(DerivedAttribute)` switch — base offsets **in GameCreature**:
| attr | formula | evidence line |
|---|---|---|
| ACTION_POINTS | `+0x774(agility) + 20` | 0x409a80 |
| CARRY_WEIGHT | `25 * (+0x760(strength) + 1)` | 0x409a90 |
| COMBAT_SEQUENCE | `2 * +0x764(per)` | 0x409a9e |
| CRITICAL_CHANCE | `+0x778(luck)` | 0x409aaa |
| EVASION | `+0x7FC` (separate stored value, not SPECIAL) | 0x409ab6 |
| HEALING_RATE | `+0x768(end)` | 0x409ac2 |
| MELEE_DAMAGE | `+0x760(str)` | 0x409ace |
| PERCEPTION_RANGE | `3*+0x764 + 40` | 0x409ade |
| PERK_RATE | `3` | 0x409ae9 |
| SKILL_RATE | `2*+0x770(int) + 5` | 0x409af9 |
| THROWING_RANGE | `3*+0x760` | 0x409b08 |
| HIT_POINTS | `int(1.0 / (AP * (1/6000.0)))`, `1000` when AP==0 | 0x409b1d–0x409b4c |
| RADIATION/FLASH/BIOLOGICAL_RESISTANCE | 0 | fallthrough 0x409b60 |
unknown enum → debug `"GameCreature::GetDerivedAttribute() - Unknown attribute requested: %d"`.

So **SPECIAL is confirmed at GameCreature +0x760(str) +0x764(per) +0x768(end) +0x76C(?char)
+0x770(int) +0x774(agi) +0x778(luck)** — matches the Fallout line the reimpl agent already has;
derived formulas above are the shipped ones (note: HP from AP is `6000/AP`, not the F2 table).

## 7. Notes on tooling quirks discovered (for future sessions)

- `mcp__ida_original_f3__xrefs_to_field` currently fails schema validation on every payload shape
  tried (array, single object, with/without `type`) — use raw byte scans + `xrefs_to` instead.
- The F3.json usage DB lags the IDB: Entity+0x15C shows 1 usage, IDB/raw scan shows ≥8. For
  hot fields, prefer `rg`-style scans of `.text` for the SIB encoding over DB lookups.
- The `(inferred)` marker applies to §4 +0x3D8 (symmetry), §5 "0x7C8 = alternate armor array",
  §5 "part==4 = arm", §3 "+0x30/+0x34 enum pair".

## 8. Queue for next session / questions to the reimpl agent

1. **Entity+0x3BC stack count** (§4; raw disp 0x3C0 with the Entity−4 base bias) — please check your inventory port: if you model stack count
   anywhere else, that's a deviation; the original keeps it on the Entity vb tail (all entity
   classes have it, not just items).
2. ~~Still open, nobody's turf: **GameEntity+0x30/+0x34 = 7,7**~~ → CLOSED by the parallel
   discovery line: `2025-10-20-gameentity-vft-decode-activation-and-inventory.md` §2 decodes
   +0x2C=AnimationState/+0x30=WeaponStance semantics; see that doc + README index.
3. GameEntity+0x18 = GENT dword[4] (unnamed semantics). Candidates we'll chase: something compared
   with 4 (default) in net/GUI code. *(inferred)*
4. ~~Effect-node dispatch~~ → **CLOSED**, see
   [2025-10-20-session6b-effect-node-dispatch.md](2025-10-20-session6b-effect-node-dispatch.md)
   and the parallel `2025-10-20-gameworld-interfaces-effect-pipeline-closed.md` (agree on
   0x66856C; the trampoline target vft+20 is a shipped no-op stub).
5. Armor equipping (`GameArmor` own block at 0x3B4..0x3CF) — the damage formula shows armor is
   consulted per body part via `0x7B0/0x7C8` arrays on the **creature**, so armor→creature
   application is somewhere in `GameActionEquip` — unchased. NOTE: the parallel GCRE doc names
   +0x7B0/+0x7C8 as current/base damage-resistance mirror sets (6 body entries each), matching
   my §5 use of them as per-part DR arrays.
6. ~~where the weapon's +0x3C8/+0x3EC reload durations are initialized~~ → **CLOSED (addendum
   below)**: reload duration = anim-event 120 duration, not a weapon field.

## 9. Addendum — GIWP wire layout (weapon own-block init) + §4 corrections

`GameWeapon::ReadFromFile_virt_0x415660` dispatches fourcc `GIWP` (1347897671) to
`GameWeapon::ReadData_0x415190`, whose unconditional block `sub_412CD0` reads **15 fields**
(2003-style, weapon base = GameWeapon−0x430, i.e. Entity-vb; all offsets below are
weapon-base-relative bytes), then a version-gated pair, then a GWAM sub-chunk list:

| wire # | offset | role (per §4 usage) |
|---|---|---|
| f1 | +0x3B0 | (unused pad) |
| f2 | +0x3B4 | primary caliber |
| f3 | +0x3B8 | primary magazine capacity *(replaces my 0x3BC guess)* |
| f4 | +0x3BC | primary ammo modifier |
| f5 | +0x3C0 | (primary damage bonus? — read at 0x4133d6-adj sites, unconfirmed) |
| f6 | +0x3C4..+0x3DF | **std::string** (StreamReadString_0x49c8d0) |
| f7 | +0x3E8 | secondary caliber |
| f8 | +0x3EC | secondary modifier |
| f9 | +0x3F0 | secondary magazine capacity |
| f10 | +0x3F4 | secondary damage value |
| f11 | +0x3F8 | (ammo-related flag?) |
| f12 | +0x3FC | attack/anim type — `sub_438500` consumer + `[eax+244h]` @0x424c53 |
| f13 | +0x400 | **GWAM attack-mode record count** |
| f14,f15 | +0x404, +0x408 | trailer |
| gate `v4≥1` | +0x3B8, +0x3C4 re-overwrite | ReadData_0x415190 @0x4151f8 (`v4=*(chunkHdr+12)`) |

Role anchors (confidence): f2/f4 = primary (caliber, modifier) and f7/f8/f9/f10 = secondary
(caliber, modifier, capacity, damage) are **evidence-locked** by `ReloadPrimaryAmmo_413A90`
(caliber cmp @0x413ada, capacity @0x413aae `*(this+252)`, cur @0x413b10 `*(this+251)`) and
`OnServerUpdate_0x407270` (`*(this+240/241)` `*(this+253)` @0x41382c+ = secondary trio).
The f5/f11/f14/f15 rows are positional only *(inferred)*.

> **⚠ rebase caveat added later:** session17 §10 raw reads put primary current/capacity at
> **+0x3EC/+0x3F0** and the caches at **+0x424/+0x428** — i.e. the absolute layout contradicts
> treating this reader's `this` as the weapon base. The absolute-offset column above therefore
> needs re-derivation (solve the reader's base delta from one anchor) before use. The
> field ROLES and order are unchanged. **UPDATE (session18): rebase solved — reader base ==
> weapon base; final table in `2025-10-20-session18-weapon-layout-final-giwp.md` (fixes the
> f3/f4/f5/f13 rows in §9 above: cap=+0x3F0, mod=+0x3B8, secCal=+0x3C0, GWAM count=+0x404).**

Post-read in the dispatcher @0x41556b-0x4155cb:
- `[this+0x3F0] = registry lookup result` — **weapon netId field is initialized at load**, and
- when the (caliber,modifier) pair is nonzero, the ammo-prototype cache is filled:
  `[Weapon+0x3C4] = sub_408110(primaryCaliber, primaryModifier)`, `[+0x3E0] = (secondary pair)`.
  → these are the "cached loaded-ammo instance ptr" slots I noted in §4; they are NOT serialized.
- `sub_4148A0(weapon, chunkHdr, res)` per GWAM record (count at +0x400) — field map already in
  `2025-10-20-gcre-versions-gwam-queueanimevent-table.md` (creature and weapon share the
  attack-mode record machinery; if the +0x400 collision with the Creature GWAM count bothers
  anyone, re-verify the `this` rebases there).

**Reload duration (my §8.6):** `GameActionReload::meth_0x407250` = 
`GameEntity::QueueAnimEvent_4673d0(creature, animId=120, 0.0)` → returns the *scheduled
reload animation's duration* (fallback 1). So there are no weapon duration fields to initialize —
the cooldown is the anim-event 120 duration from the GWAM/clip tables (cross-ref
`2025-10-20-anim-event-scheduling-pipeline.md`). §4 rows +0x3C8/+0x3EC/+0x3F4 are therefore
re-tagged: +0x3C8 = cached-prototype-2 (not a field), +0x3EC/+0x3F4 = secondary capacity/damage
per the GIWP table above. The `OnServerUpdate` reads at weapon dword idx 242/243 (bytes +0x3C8/
+0x3CC, "reload-delay primary/secondary") are **outside** the GIWP table — they read live state
(the cache/space fields above), not serialized durations; treat those two §4 rows as superseded.
