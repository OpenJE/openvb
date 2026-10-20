# 2025-10-20 — GameWorld interface geometry, effect payload decode, GENT final map, classId readers (session 6)

Target: ORIGINAL_REFERENCE (`ida_original_f3` / `artifacts/F3.exe.i64`). Read-only; no IDB/source
edits. Raw PE byte verification via section-mapped reads of `$OPENVB_ORIG_EXE`
(imagebase 0x400000, `.text` raw@0x1000, `.rdata` raw@0x247000 — recipe in this doc).
Follows session 4 (§6 queue) and session 5 (§4 queue). **Supersedes session 5 §2–3.**

## 1. GameWorld singleton: full interface geometry (raw-verified, closes session 5 §4.1)

`F3::GameWorld` size 0x168; embeds `JE::World` (0x164) at +0. Global
`global_cls_0x616328_0x70bd98` = the `GameWorld*` instance pointer (`.data` fill
`0xFFFFFFFF` pre-init → runtime-assigned; do NOT expect a static vftptr there).

`GameWorld::ctor_0x470340` raw bytes (objdump) write **four vftptrs + two vbtables**:

```
[GW+0x00] = 0x6685A4   primary  (JE::World-side, 20 slots — the DB's "GameWorld vftable")
[GW+0x04] = 0x66856C   ⭐ WORLD-SERVER INTERFACE  ← target of GetWorldServerInstance_0x401150
[GW+0x08] = vbtable 0x668644 (param_1!=0) or 0x66863C   (first dword -4, then vbdisp)
[GW+0x0C] = 0x668548   tertiary (8 slots, all JE::World thunks 0x544B30/0x546140/0x545820/
                        0x544CF0/0x547E60/0x548840/0x545AA0/0x544D10)
[GW+0x04+vbdisp] = (vbdisp - 912)            (delta dword maintained by ctor @0x4703b2)
[GW+0x08+vbdisp] = 0x668494   ⭐ FACTORY INTERFACE (Create* thunks 0x46FD20..0x470330)
```

`GetWorldServerInstance_0x401150` returns `global + 1` with `sizeof(cls_0x616328)==4`, i.e.
**`GW + 4`** = the server interface. `+0/+4` vbdisp gymnastics below use the
`[GW+8] → vbtable → vbdisp` chain; the "4th interface" `0x668494` lives at `GW+8+vbdisp`.

### 1.1 World-server interface vftable @0x66856C (12 slots, raw dump)

| byte off / slot | target | meaning (verified) |
|---|---|---|
| +0x00 / 0 | 0x436240 | F3 thunk (unnamed; small) |
| +0x04 / 1 | 0x46F250 | `JE::World::Get_mbr_0x70` — returns **double** at World+0x70 |
| +0x08 / 2 | 0x46F260 | `JE::World::set mbr_0x70` (thiscall setter) |
| +0x0C / 3 | 0x46F270 | `JE::World::Get_mbr_0x60` |
| +0x10..+0x1C | 0x544A60 / 0x544A70 / 0x544A90 / 0x545100 | JE::World engine thunks |
| +0x20 / 8 | 0x46F280 | `JE::World::Get_mbr_0x88` |
| +0x24 / 9 | 0x544B80 | JE::World |
| **+0x28 / 10** | **0x46FA40** | **`CreateEffect(typeId)`** — see §2. Used by EFDT/EFCD at 0x4685bc/0x468570 (session 3). |
| +0x2C / 11 | **0x545930** | **`JE::World::PackState(net::packet*)`** — see §4. |

⚠ Correction to session 5 §2/§3: its "vft[+8]=assign-new-net-id" and "vft[+40]=template-by-id"
were read off a *confused decompilation*. The `+40` (byte) target `0x46FA40` is correct for the
server interface (slot 10), but it is **CreateEffect**, not "template-by-id". The net-id *reader*
used by the GameEntity ctor is on the **factory interface 0x668494, slot +8 = 0x46FD60**
(§1.2). Session 5's table for "0x6685a4 partial slot map" remains true but is the primary
JE::World-side interface (IWorld client factories), not the server one.

### 1.2 Factory interface vftable @0x668494 (slot+8 and slot+24 pinned)

Raw head: +0 0x46FD20, +4 0x46FD40, **+8 0x46FD60**, +0xC 0x46FD80, +0x10 0x46FDA0,
+0x14 0x46FDB0, **+0x18 0x46FDC0** (`CreateGameEntityByClass_0x46f530` thunk — its ONLY xref;
this is the reimpl's "vft+24 create" — confirmed, byte offset 0x18=24 ✓), +0x1C 0x46FDD0 …
through ~+0x50 (0x46FD20..0x470330 family; full 21-slot walk deferred).

`0x46FD60` raw disasm (`sub ecx,[ecx-4]; sub ecx,0x230; mov eax,[ecx-0xF8]; retn`) =
`*(u32*)(GameWorld + 0x70)` — i.e. **the same `World::mbr_0x70` double that slot 1/2 get/set**
(lownnord dword only). Therefore:

> **`GameEntity+0x28` (session 1 "net id") = current value of `JE::World::mbr_0x70`** — a double
> at World+0x70, also serialized 4th in `PackState` (§4). Candidates for its meaning: world/game
> time or next-object-handle; *name pending* — but it is NOT a self-incrementing per-entity
> counter: **the same value repeats for entities created in the same "tick"** unless the server
> updates mbr_0x70 between creations (setter vft+8 is the only visible writer; writers inside
> JE::World internals at 0x548B43 pending). ⚠ For the reimpl: EME2 instances in Mainmenu all had
> flag/id 1 — consistent with "counter sampled at ctor time", not unique per-entity id.

## 2. Effect-tree node payload decoded (closes session 3 §4 / session 5 §4.1)

Dispatcher `cls_0x463870::meth_0x468490` (server/editor-gated; client returns 1 immediately).
Per node: `Common::Chunk` header {tag, ver, size}; body:

| tag | dwords | node object |
|-----|--------|-------------|
| `EFDT` (0x44544645) | u32 childCount, u32 (discarded) | `serverIntf->vft[+40](0)` → `CreateEffect(0)` → **`JE::Condition`** (new 0x70, ctor 0x5A8D30 → Effect base) |
| `EFCD` (0x54434645) | u32 childCount, u32 (discarded), **u32 effectTypeId** | `CreateEffect(typeId)`: **0 → JE::Condition, 1 → F3::GameEffectDamage (new 0xB0, ctor 0x45C040), anything else → `FatalError("World::CreateEffect() - Unknown effect type %d.")`** |

The shipped title has exactly **TWO effect types**: engine Condition and F3 GameEffectDamage.
(`JE::World::CreateEffect_544bf0` handles 0; F3 thunk 0x46FA40 special-cases 1. `GameEffect.md`
/ `GameEffectDamage.md` class names confirmed.)

`cls_0x55a280` (session 5 §1) forwards the *chunk body* to `inner->vft[+0x14]` (byte 20):

- **Condition payload reader** = `0x6A5ADC+0x14 → 0x5A8F20`:
  1. base pass-through `0x5A0BB0` returns 1 (JE::Effect base reads NOTHING);
  2. `char[260]` C-string → `JE::String::Assign` into **Condition+0x48** (std::string);
  3. u32 → **Condition+0x64**;
  4. if chunk ver ≥ 2: u32 → **Condition+0x68**.
- **GameEffectDamage payload reader** = `0x66073C+0x14 → 0x462600` — **empty (returns 1)**.
  Damage nodes carry NO serialized fields; their runtime fields come from elsewhere.
  ctor defaults (0x45C040 + GameEffect base 0x462470): Damage own `+0x9C=0,+0xA0=0,+0xA4=0,
  +0xA8=9` (type tag **9=damage**), `+0xAC=0,+0xAD=0`; GameEffect-common block `+0x48..+0x98`
  zeroed (13 dwords) with `+0x84=1`.
- Tree assembly: `childCount` loop re-invokes the node parser (0x468600..0x468626); non-root
  nodes parent via `sub_55ADC0(parent, node)`, roots push into `GameEntity+0x38` trees
  (`sub_468310`); the `cls_0x55a280` wrapper is stack-local — **stored pointers are the inner
  effect objects** (session 5 §1 re-confirmed).
- Vtable walk note: GameEffectDamage vft 0x66073C raw: [0]0x45C090 [1]0x45C0A0 [2]0x45C0F0
  [3]0x45C120 [4]0x4625C0 [5]0x462600 [6]0x4625E0 [7]0x408D30 [8]0x45C100 [9]0x404B70
  [10]0x45C6B0 [12]0x4626A0 [14]0x462710 [15]0x462740. GameEffect (pure) vft 0x66535C differs
  only at [2]0x4625B0,[3]0x462930,[10]0x462620. Slot5 identical (empty reader) on both.

## 3. GENT — FINAL field map (closes the reimpl's open request; corroborated by community spec)

Reader chain `cls_0x463870::meth_0x466510` (per-version):
`meth_0x463870` (v0, always) && (ver≥1: `meth_0x463910`) && (ver≥2: `0x409BB0`) && (ver≥3:
`0x409BB0`) — **`0x409BB0` is a `return 1` stub: GENT v2/v3 add NO fields.**

| payload off | → GameEntity | name | evidence |
|---|---|---|---|
| +0 | +0x08 | **HoverSR** (STF string ref id) | 0x463870 read 1 |
| +4 | +0x0C | **LookSR** | read 2 |
| +8 | +0x10 | **NameSR** | read 3 |
| +12 | +0x14 | **4th string ref** (community name "UnkwnSR" — likely UseSR) | read 4 |
| +16..+23 | +0x18, +0x1C | **gap/flags pair** — community spec: "may contain flags or a small struct"; ctor defaults **4, 0** | read 5,6 (`&mbr_0x18+1`) |
| +24 | +0x20 | **MaxHealth** | `meth_0x463910` first dword, ctor default 1 |
| +28 | +0x24 | **StartHealth (current)** — clamp `Start = min(Start, Max)` verbatim in 0x463910 (0x463951: if (+0x24 > +0x20) +0x24 = +0x20) | ctor default 0 |

→ **Reimpl's `(0,0,0,0,0,0,100,100)` template guess CONFIRMED: 100/100 = Max/Start HP.**
GameEntity vft 0x666130 slot 0 = `0x4080C0` = raw-verified `return this->mbr_0x20` =
**GetMaxHealth** (session 3's "get cur-stat" mislabel corrected).
Still unnamed after this: `+0x30/+0x34 = 7,7` — DB shows **only ctor writers, zero readers**;
GENT doesn't cover them (v0 reader stops at +0x1C, v1 at +0x24). *(inferred)* possibly a
second (unused in shipped data?) stat-pair given the same cur/max adjacency pattern —
recommend the reimpl NOT reserve semantics for them yet.
`GameEntity+0x18` gap-flags is likewise read nowhere in the DB (writer-only at 0x467935); the
equip UI (§4) also never touches it. Treat as opaque template metadata for now.

## 4. Entity+0x15C classId — readers FOUND (closes session 4 §3 ⚠ / session 5 §4.2)

The DB missed these because every access is via dynamic `[base+vbdisp+0x160]` (SIB, disp =
0x15C+4). Raw byte scans (`8b [mod=10,rm=SIB] sib 60 01 00 00`, excluding `[esp+…]` stack
locals) plus decompile verification yield exactly **three F3-side readers + one JE serializer**:

1. **0x408229 `GameAmmo::virt_meth_0x4082c0`** — RTTI-casts an `Entity*`→GameAmmo, then copies
   `ammoEntity.vb->mbr_0x15C` (read at `+352`) into a state block (the classId travels with
   ammo-stack state).
2. **0x43E8B6 `GameActionEquip::meth_0x43e760`** (case 1) — **equality comparator**: walks an
   Rb-tree comparing `candidate->E+0x15C == item->E+0x15C` → **classId is the inventory/equip
   sort-key by class**. (Same function compares `E+0x3C0` (=960, case 3) and `+0x1C` count
   (case 2) — separate sort keys.)
3. **0x458264 `CLoadMapInterface::meth_0x4580c0`** (CWorldContainerInterface) —
   `flag = (E->mbr_0x15C == 1)` → **"is creature" check** controlling container/loot listing
   (else RTTI `GameItem` filter enumerates children).
4. **0x54597A.. `JE::World::PackState_545930`** (server intf slot +0x2C) — world-state packet:
   packs `World+0x44,0x48,0x6C,0x70,0x74,0x7C,0x80,0x84`, then **per entity: `*(Entity+476)`
   and `*(Entity+348)` (= +0x15C decimal ✓)** then `cls_0x54a610::meth` (entity state pack).
   → classId is **network-serialized** — clients learn entity class without RTTI.

**Water-tile dual-use (session 4 §3) resolved**: the JE water path at 0x5AB662 writes a
`WaterTileModelInstance*` into the same dword but only for engine-created water-tile entities;
every F3-side *reader* above treats it as an integer tag (compare/equal/pack — never a
dereference). The two populations never meet. Option (a) from session 4 is correct.

## 5. GamePlayer (class 10) provenance — answer to reimpl §7.3 / session 2 offer

`GamePlayer::ctor_0x411820` has exactly **two callers**:

1. Factory case 10 (`CreateGameEntityByClass_0x46f530` @0x46f73f) — but nothing in map/EEN2/GCRE
   data paths passes class 10 (reimpl already proved this empirically; factory's only external
   entry is the `0x46FDC0` thunk on factory intf slot +24).
2. **`cls_0x424a70::meth_0x42e870`** @0x42eb19..0x42eb35 — `operator new(0xBFC)` +
   `GamePlayer::ctor(this, 1)`. The function is the **character-generation / player-avatar
   (re)build** routine: appearance record tables at `byte_6F2B34` (16-byte records keyed by arg
   `a2`), **gender switch** (`v103==1` → `off_6F2B3C` male vs `off_6F2B40` female texture
   pointers), socket-model attachment loops (sockets 2/3/4/5 = hands/back/etc., model-index
   tables `byte_6F29B8` (≤10 entries) / `byte_6F2A08` (≤8)), "None" token defaulting, and it
   REPLACES an existing avatar (`mbr_0xb4` change → destroy old entity via its dtor vft[+4],
   construct new GamePlayer). `cls_0x424a70` (0x124 bytes, 56 methods) is that player/avatar
   manager (RTTI region suggests association with `CCharacterGenerationInterface` docs in
   openje — *inferred*, confirm when we map its vft-less call graph).

→ **Verdict for the reimpl: class 10 is NEVER selected from template/map data; it is born when
the player avatar (re)spawn runs at game start / character generation / after death-reload.
Your EME2 path is correct to produce class 1 for Default_PC.** Model GamePlayer as a separate
creation event ("replace protagonist entity"), reusing all Creature fields + the +0x800 map*
/ +0x804 string own-block from session 2/4.

## 6. Corrections ledger (what earlier discovery docs got wrong)

- Session 5 §2: server-interface slot +8 ≠ "assign-new-net-id" (that is factory-intf +8, and it
  *reads* `World::mbr_0x70`, doesn't allocate); vft[+40] = CreateEffect, not template-by-id.
  Server interface table = **0x66856C** (12 slots) — now located, session 5's "unlocated" item
  closed.
- Session 3 §1 "vft[0] 0x4080c0 = get cur-stat": it returns **+0x20 = MaxHealth** (§3).
- GameEffectDamage has **no serialized payload** (§2) — anything "damage amount"-like is set at
  runtime (weapon table linkage pending).
- ⚠ (same day, later pass) §2's Condition payload reading is **RESPECCED**: the 260B string is a
  SCRIPT name (Condition = script gate; lazy-loads `conditions.amx` via srv-intf +0x24, eval is
  server-only), C+0x64 indexes a param vector at C+0x28 → see
  `2025-10-20-ge94-speed-door-anittimer-gentsave.md` §12c. §10 rows 25+ corrected by its §12a
  (missed middle `sub ecx,0x230`: those getters read JE::World fields, not "GW+0x320s";
  GameWorld own storage = +0x164..+0x393 registry only).

## 7. Queue for session 7 (proposed)

1. `World::mbr_0x70` writers: 0x54596E-adj region + `sub_4D8D70` base init (0x548B43) → pin the
   semantics of the "id" the GameEntity ctor samples (time? seed? handle?).
2. `cls_0x404bc0` / `cls_0x40a220` creature sub-table record semantics (session 2 open item) —
   cross-check against GCRE reader (`van-buren-docs` GCREc section now exists as ground truth!).
3. `GetUsableSkills` + Reload linkage (session 4 §6.4, still open).
4. EEN2 parse completeness: art/gender token + EEOV sub-chunk — community spec sections for
   EEN2c should be compared against the reader (find via cls_0x463870 sibling readers).
5. Factory intf 0x668494 full slot walk (map every 0x46FDXX Create* thunk → class).

## 8. Direct questions to the reimplementation agent

1. Your EME2 `record.flag` — if you also store per-entity "net id" from `+0x28` equivalents,
   note §1.2: in the original that value can repeat across entities (sample of a shared
   counter). If your lockstep compares it, expect equality-groups, not unique ids.
2. You asked for the GENT field map — §3 is final. Do you want `HoverSR/LookSR/NameSR/+4th`
   resolved via STF (`English.stf`) lookups next, or is name-at-load deferred?
3. Effect tree: only types 0/1 exist in the shipped data (Condition, Damage, **no payload on
   Damage**). Any `Use`/`Reload` behavior you're building cannot depend on damage *template*
   values — confirm where you expect damage numbers to come from (weapon/attack tables), so we
   can chase that in session 7 instead.
4. `GameActionEquip::meth_0x43e760` sorts inventory trees by **classId (E+0x15C)** — if your
   SpawnedEntity keeps classId flat-only, that's fine, but replicate tag values 0–9 (esp.
   player=1) because `CWorldContainerInterface` checks `==1` for creature loot windows.

## 9. RECONCILIATION with the concurrent discovery docs (same date, other author — 2025-10-20 ~02:35)

Files `2025-10-20-gameworld-interfaces-effect-pipeline-closed.md`,
`2025-10-20-session6-classid-readers-weapon-ammo-combat.md`,
`2025-10-20-ext-classid-resolver-player-flag.md`, `2025-10-20-gameentity-vft-decode-*`,
`2025-10-20-class-vft-tables-*`, `2025-10-20-gameeffect-damage-anatomy.md` were written
concurrently with this session by the other agent working in this folder. Overlap is mostly
consistent; resolve as follows:

1. **AGREE**: server intf = 0x66856C @ GW+4; factory thunks 0x668494 belong to the
   JE::World-vb-dispatched interface (vbtable 0x668644 = [-4, +0x390], vb vftptr @ GW+0x398,
   delta dword @ GW+0x394 — raw-verified here too; their "GameWorld is 0x39C bytes" supersedes
   the DB size 0x168); CreateEffect type mapping; two effect types; Damage has no payload
   reader; classId readers incl. ours (they additionally found 0x407494 ==9 Reload, 0x40b3f7
   ==6 Attack, 0x413658/0x4138a8 ==9 Weapon::Unload* — adopt their superset table).
2. **CONTRADICTION — their §2 slot table is misaligned; bytes say** (readdump recipe:
   `python3 /tmp/readdump.py $OPENVB_ORIG_EXE 66856c 12`; section map: imagebase 0x400000,
   .rdata vaddr 0x647000 raw 0x247000):
   `0x66856C: [+0]436240 [+4]46F250 [+8]46F260 [+C]46F270 [+10]544A60 [+14]544A70
   [+18]544A90 [+1C]545100 [+20]46F280 [+24]544B80 [+28]46FA40 [+2C]545930`.
   Their row "slot 2 / +8 = 0x46f280, next-entity-id read used by GameEntity ctor" is wrong on
   two counts: (a) byte +8 of 0x66856C is **0x46F260 = mbr_0x70 SETTER** (decompiled — writes
   `this+0x70 = a2`); 0x46F280 (World+0x88 getter) lives at byte **+0x20**; (b) the GameEntity
   ctor net-id read does **not** go through 0x66856C at all: decomp at 0x4679b0 computes
   `v6 = GW+8+vbdisp(=0x390) = GW+0x398` (factory intf), calls its vft[+8] = **0x46FD60**,
   which raw-decodes to `return *(u32*)(GW+0x70)` (sub [ecx-4]=delta0, sub 0x230+0xF8=0x328).
   **⇒ entity+0x28 = low dword of World::mbr_0x70 (double), NOT mbr_0x88.** mbr_0x70 is
   get/set via server-intf slots 1/2 (+4/+8) and is packed 4th in World::PackState. Their
   "0x88" label should be corrected before the reimpl bakes it in.
3. **ANSWERS to their open items**:
   - `sub_46FB20` ext-resolver (their flag-origin question): xrefs show its ONLY caller is
     thunk `0x46FE90` = factory-intf (0x668494) slot **+48** — i.e. the flag arrives from the
     engine-side `CreateObject(path, flag)` dispatch; every F3-side caller is `call
     [worldVbIntf+48]`. Also note this doc's §5: the OTHER GamePlayer birth path
     (`cls_0x424a70::meth_0x42e870`, char-gen) calls the ctor **directly**, bypassing the
     resolver entirely. Two paths, one class.
   - `0x408229` (GameAmmo) "packed buffer at [entityVb-0x278]" — from our decompile the target
     is a state struct reached via `v2 = this - *(this-1)` (self-vb decode) with writes at
     v2[-52]/[-4]/[-3..-1].gap fields; consistent with their "classId crosses the wire".
4. **Unique to this doc** (not in theirs): full GENT reader decomposition incl. the v2/v3 stub
   finding (0x409BB0 = return 1) + community cross-check against
   `van-buren-docs/Van-Buren-Chunk-Specification.md` (their GENTc section = our wire map,
   names HoverSR/LookSR/NameSR/UnkwnSR, MaxHealth→+0x20, StartHealth→+0x24, clamp proof);
   World::PackState per-entity serialization of +0x15C and +0x1DC; four-interface + two-vbtable
   table; tertiary intf 0x668548 (8 JE::World accessor slots).

## 10. Factory interface 0x668494 FULL slot walk (closes §7.5; 45 slots, raw-decoded)

Encoding pattern for every slot: `sub ecx,[ecx-4]` (remove vb delta) then optional
`sub ecx,0x230` (= back to GW base) then field access/jmp. `this = GW+0x398`, delta=0.
Byte offsets below are into 0x668494. All targets decompiled/disassembled this session.

| +byte | idx | target | semantics |
|---|---|---|---|
| 0 | 0 | 0x46FD20 | get `World+0x48` |
| 8 | 1 | 0x46FD40 | set `World+0x48` |
| 16 | 2 | **0x46FD60** | **get `World+0x70`** ← the GameEntity ctor net-id read (entity+0x28) |
| 24 | 3 | 0x46FD80 | set `World+0x70` (arg) |
| 32 | 4 | 0x46FDA0→0x544D90 | JE::World pass-through |
| 40 | 5 | 0x46FDB0→**0x46F290** | **`CreateGameAction(GameActionType)`**: Die 0x2C/0x404e50, Idle 0x24, MoveTo 0x48, PickUp 0x30, Wander 0x44, Wait 0x24, FloatingSpeechText 0x34, Equip 0x30, Unequip 0x28, SetActiveWeapon 0x44; default → `JE::World::CreateAction`. F3 enum cases cover 10 of the GameAction* doc classes — sizes are the reimpl's allocation reference. |
| 48 | 6 | 0x46FDC0→**0x46F530** | **CreateGameEntityByClass** (session 1 factory; sole caller site) |
| 56 | 7 | 0x46FDD0→**0x46F7C0** | **`new GameMap (0x258)` + ctor 0x46c8e0** — CreateGameMap |
| 64 | 8 | 0x46FDE0 | `++World+0x7C` → return `(id & 0x1FFFFFFF) \| 0x60000000` |
| 72 | 9 | 0x46FE10 | `++World+0x80` → return `id & 0x1FFFFFFF` (no tag) |
| 80 | 10 | 0x46FE40 | set `World+0x80 = arg & 0x1FFFFFFF` |
| 88 | 11 | 0x46FE70 | `World+0x80 = 0` (reset) |
| 96 | 12 | **0x46FE90→0x46FB20** | **ext→classId resolver** (the concurrent `ext-classid-resolver` doc's function; only caller confirmed) |
| 104 | 13 | 0x470330 | **create-by-name**: unpack std::string ptr, vb-decode, re-dispatch `vft[+48](path, flag)` — the reimpl's recorded "vft+52 by-name" ✓ (chain: +52 → +48 → 0x46FB20 → factory +24) |
| 112 | 14 | 0x46FEA0 | `++World+0x84` → `(id & 0x1FFFFFFF) \| 0x20000000` |
| 120 | 15 | 0x46FED0 | set `World+0x84 = arg & mask` |
| 128 | 16 | 0x46FF00 | `World+0x84 = 0` |
| 136 | 17 | 0x46FF20 | `++World+0x88` → `(id & 0x1FFFFFFF) \| 0x40000000` |
| 144 | 18 | 0x46FF50 | set `World+0x88 = arg & mask` |
| 152 | 19 | 0x46FF80 | `World+0x88 = 0` |
| 160 | 20 | 0x46FFA0→0x46FCC0 | **world reload**: `JE::World::ReloadConfig(vb)` + RTTI-cast global client → `sub_455A70` + `cls_0x41a2b0::meth_0x4172d0(1)` |
| 168–192 | 21–24 | 0x5457E0 / 0x547C30 / 0x544CE0 / 0x5458E0 | JE::World engine pass-throughs |
| 200 | 25 | 0x46FFF0 | get `World+0xF0` ⚠ **CORRECTED** — see ge94 doc §12a: every thunk from slot 25 down includes a middle `sub ecx,0x230` (vb→World base 0x168); true targets are World+0xF0..0x100, +0x3C, and the +0x154 deref-chain, NOT "GW+0x320s". F3::GameWorld own region is ONLY +0x164..+0x393 = the 140-slot ammo/prototype registry (their session9/10 docs) + vb-delta +0x394 + vb-vftptr +0x398. Object 0x39C fully accounted. |
| 208–352 | 26–44 | remaining: 0x470010 (+0xF4), 0x470030/50/70/90/b0/d0/f0/0110 (`World+0xF8/FC/100` get-set family + stubs), 0x470130/140/150/160/180/1A0/1B0/1C0 (`+0x154` cursor-chain: raw ptr, `*`, `*+4`), 0x4701E0→0x4701F0 tail; JE pass-through jmp targets 0x544D90/0x5457E0/0x547C30/0x544CE0/0x5458E0/0x5477E0/0x544CA0/0x544CB0/0x544D70/0x544D80 | corrected targets per §12a; all within JE::World — no GameWorld-own storage beyond the registry |

### 10.1 ID-space model (strongly implied, *name pending*)

Four allocation counters in `JE::World` with mask `0x1FFFFFFF` and space-tags:

| counter | tag | factory slot |
|---------|-----|-------------|
| World+0x7C | 0x60000000 | +64 |
| World+0x80 | none (plain ids, set/resettable) | +72/80/88 |
| World+0x84 | 0x20000000 | +112/120/128 |
| World+0x88 | 0x40000000 | +136/144/152 |

Entities' `+0x28` is NOT one of these (it samples `World+0x70`, a double) — so
"net id" per entity is likely a **world/game-time stamp, not a unique handle**; unique handles
probably come from the tagged counters when objects are registered elsewhere. The concurrent
pipeline-closed doc's "0x46F280 = next-entity-id" should be re-read as: 0x46F280 = plain
getter of `World+0x88` (server intf byte +0x20), whose ALLOCATOR is slot 17 (+136).

### 10.2 For the reimpl
- Your GameAction classes' sizes are in the slot-5 row — matches docs/f3/GameAction*.md set.
- Map spawn objects should be created via the +24 chain (by-path via +52→+48→+24) to mirror
  original ordering: name resolve → classId (ext + flag) → factory.
- If you implement save/load ids, use the 4-counter tag model (0x20/0x40/0x60 spaces, 29-bit).

## 11. Singleton assignment site + `World+0x70` semantics pass (final loose end of this session)

- `JE::World`-region ctor `cls_0x4d8d70::ctor_0x548a80` (called by GameWorld::ctor at 0x470377)
  is what writes the singleton: @0x548CCF `global_cls_0x616328_0x70bd98 = this`, guarded by
  `"World object already exists."` throw @0x548CBD. Confirms §1: global == World-body/GameWorld
  base pointer; all reads of it (GetWorldServerInstance etc.) are post-construction only.
  The ctor also throws unless this is the first world → demo has exactly ONE GameWorld.
- ctor initializers for the fields in play (dword-index decode: `this+N` = byte N*4):
  `+0x6C=0 (this+27)`, `+0x70=0 (this+28)`, `+0x74=0x3F800000=1.0f (this+29)`, `+0x78=10
  (this+30)`, counters `+0x7C/+0x80/+0x84/+0x88=0`.
- **`World+0x70` is NEVER incremented anywhere** (DB: only ctor-init 0x548B43, setter 0x46F264,
  PackState read 0x54596E; plus the factory-intf thunk 0x46FD60 sample used by entity ctor).
  So entity+0x28 cannot be a per-entity unique handle. Model: it is a **world-level stamp**
  written by the server via server-intf slot +8 (0x46F260) at new-game/join time, replicated
  to clients in PackState position 4, and copied into every entity created afterwards
  *(inferred use: session/game id or world seed; the neighbouring float +0x74=1.0 + int
  +0x78=10 look like a small rules block)*. Adjacent engine gate 0x544A70: `if (mbr_0x68 >
  arg && worldMode(mbr_0x34) not in {1,2}) return 0` — world mode field at +0x34 ∈ {0,1,2,3}
  (PackState requires 3 = "normal world").
- **Consequence for the reimpl + reconciliation with `pipeline-closed` §2**: the row
  "next-entity-id read = 0x46F280 (World+0x88) at server-intf +8" conflates: 0x46F280 sits at
  byte +0x20 of 0x66856C and is a plain getter; the actual ++allocator for World+0x88 is
  factory-intf slot +136 (0x46FF30, tag 0x40000000). Entity+0x28 samples World+0x70 only.
  If you want a unique-id analog for spawned entities, use one of the tagged allocators
  (factory +64/+72/+112/+136), NOT the ctor-stamped +0x28 field.
