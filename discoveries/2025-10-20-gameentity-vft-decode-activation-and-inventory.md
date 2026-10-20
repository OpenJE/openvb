# 2025-10-20 — Session 5: GameEntity Entity-vft decode, field naming, GENT save, GameInventory

Target: **ORIGINAL_REFERENCE** (`artifacts/F3.exe.i64` + raw `$OPENVB_ORIG_EXE`, imagebase 0x400000).
Read-only; no IDB/source edits. Follows [session 1](2025-10-20-entity-class-family-layout.md),
[session 2](2025-10-20-entity-family-subtables-and-player-layout.md),
[session 3](2025-10-20-entity-file-format-gent-eftr.md),
[session 4](2025-10-20-full-class-geometry-vbtables.md).

This session decodes the **GameEntity override table** (the real class interface), names
several long-standing mystery fields, verifies the GENT **save** format from the writer side,
finds two new entity-file fourccs (`EEN2`, `EENT`), and discovers **`F3::GameInventory`**.

---

## 1. Where the virtuals actually live (corrects a wrong assumption in sessions 1–2)

- `GameEntity_vftable_666130` (primary vftptr@+0x0) is **exactly 1 slot**: `0x4080c0` =
  `return [ecx+0x20]` (get current/max stat). Confirmed by raw dump: 0x666134 onward is the
  vbtable `[-4,+0xA0]`, then .rdata strings — the table ends after 1 entry.
- The real F3 interface is the **Entity-vb vftable at 0x666044**, written at 0x4678fd
  (`mov dword ptr [ecx+esi+4], offset 0x666044`, raw bytes `c7 44 31 04 44 60 66 00`).
  It spans **58 slots (+0x00..+0xE4)**; slot +0xE4 is `0x468ad0` and the entry at +0xE8
  (0x6d2fb8, .data) terminates the run. Every GameEntity `virt_meth_0x467aX0` is a
  vbptr-adjust thunk (`sub ecx,[ecx-4]` = `2b 49 fc`) + `jmp` to a shared body, placed into
  this table. Sessions 1/2 said "vft[0] getter never overridden" — true, but misleading:
  the class semantics are ALL in the Entity-vb table.

### 1.1 Slot map (raw dump of 0x666044; thunks resolved via raw jmp rel32)

| slot | entry | body | decoded semantics |
|------|-------|------|-------------------|
| +0x00 | 0x467a00 | sub_467E50 | deleting dtor: `Entity::meth_0x467af0` + `Entity::meth_0x54fff0`, `free(base)` if flag&1 |
| +0x04 | 0x54dea0 | — | engine (JE::Entity) default |
| +0x1C | 0x467a80 | **sub_466570** | can-see-target gate: if game-option (`byte_707CE4` vft+316(0)) then only for classTag∈{1,5,6,7,9} (Entity+0x15C): raycast through target's model/pos vs WorldServer-lookup(Entity+0x1EC); returns 0 if target hidden behind obstacle, else 1 |
| +0x28 | 0x468710 | **sub_468350** | **CopyFrom(GameEntity*)**: `RTDynamicCast(Entity→GameEntity)`; copies +0x20/+0x24 stat pair; deep-copies the **5 effect trees** (this-5 trees @GameEntity+0x38, source trees @`other+60` dwords… = other+0x38, stride 12×5 loop `local_14<5`), re-instantiating each effect node via `WorldServer->vft[+40](*(nodeval+12))` (template id!) then `vft[+8]` attach + RbTree insert (`sub_433760`); tail `Entity::virt_meth_0x552420` |
| +0x2C | 0x408080 | — | engine getter `return [this+0x2D0]` (not overridden) |
| +0x34,+0x38 | 0x408090 | — | `retn 8` empty virtual |
| +0x3C | 0x467a90 | **sub_467140** | engine-read (`Entity::virt_meth_0x54bba0`) then iterate tree at **Entity+0x168** inventory, calling each node value's vft+60 (per-item read) |
| +0x44 | 0x467e30 | **0x467be0** | engine (`Entity::virt_meth_0x54b540`) then `RTTI GameInventory`: iterates items, calls item vft+68(stream), builds RbTree of results and merges (`GUI::Interface::meth_0x4b9d60`) |
| +0x48 | 0x467ae0 | **sub_467830** | **anim-event step/advance**: reads model handle (Entity+0x160) → anim object +0xC; walks the active anim-event linked pool (`dword_6F3960` = next-index table) calling `cls_0x467780::meth_0x467780(node, animIdx, time, 0)`; in/out params = (anim event id, sub-index) |
| +0x4C | 0x467aa0 | **sub_466920** | **IsHumanoid()**: returns 1 iff Entity+0x15C==1 (creature/player) AND the string at Entity+0x218 (size@+0x14, cap@+0x18; heap when cap≥0x10) begins `"HumM"` or `"HumF"` (stricmp, 4 chars) — the EEN2 art/gender token |
| +0x50 | 0x467a10 | 0x462970 | gate: `Entity::virt_meth_0x549030(entity) || [entity−32B] ≤ 0` — "alive/usable OR stat(≤0?)". Candidate: dead entities pass when cur-stat ≤ 0. Field base: see §5 caution |
| +0x58 | 0x467a20 | **sub_4629e0** | **QueryDefaultAnimation()** — see §2: `switch(GameEntity+0x2C){1..9} × switch(+0x30 stance 1..7) → anim id` (returns 26,74,105,136,172,204,253 / 318,343 / 355 / 361,371,377,383,389,395,416 / 438,445 / 487 / 502 / 511 / 0). Log strings: `"GameEntity::QueryDefaultAnimation() - Unknown Weapon Stance set.\n"`, `"... Unknown Animation State set.\n"` |
| +0x5C | 0x4084c0 | — | engine getter bool `[ecx+0x314]` |
| +0x80 | 0x467ab0 | **sub_4671a0** | engine (`Entity::virt_meth_0x54b4e0`) + inventory iteration: skip when `global_area_map_icons[0]` +0x2C struct byte@+0x41 set (client/net flag?); RTTI GameInventory; for each item call item vft+128(param_1) |
| +0x90 | 0x467a30 | **sub_4629a0** | **LineOfSight(other)**: both entities must share **Entity+0x1EC** (== reimpl `mapId` ✓); builds segment from `other` position (Entity+0x04..0x0C floats == reimpl Mover `pos` ✓) or model-attachment world pos (`ModelInstance::meth_0x563a40`), `Entity::meth_0x4056b0` = own eye/anchor point; converts world→grid twice via `cls_0x57bc90::meth_0x57bdc0`, then cell-walk calling `Entity vft+38(this, x, y, &dz)` per occupant; returns 0/1/2 (2 = blocked-by-entity seen) |
| +0x9C | 0x468720 | **sub_468650** | engine (`Entity::virt_meth_0x54fd90`) + **equipped-item render list**: per inventory item reads item-**Entity+0x160** (model handle) → `cls_0x563900::meth_0x563900(0)` twice and appends results to the outgoing list via `sub_468310` (list of gfx model instances — weapon/armor appear attached to the creature) |
| +0xA0 | 0x467a40 | **sub_463970** | **GENT WRITE (save)** — see §3 |
| +0xA4 | 0x468300 | **sub_467E90** | **OnAnimEvent(token, record)** dispatcher — see §4 (the DM/RL/EP/UP/PU/FR/FL tokens from session 1 §5 are fully explained) |
| +0xA8 | 0x467e40 | sub_467CD0 (big fn 0x467cd0..0x468650, decompile partial) | **GetUsableSkills script call**: `Entity::GetScript_54aef0` → `script->vft[+0x10]("GetUsableSkills")` (has-function?) → `script->vft[+0x1C]("GetUsableSkills", 0x1A=26 out vals, array@Entity+0x2A4 str/26×int)`; on failure logs `"Couldn't execute GetUsableSkills on the entity %s\r\n"` (entity name string at Entity+0x2A4/cap@+0x2BC) and feeds results into a `cls_0x568c20` list ctor 0x568c20 — this is the **skills array (26 entries)** backing the character-screen/`Use` skill menus |
| +0xB0 | 0x467a50 | **sub_4637F0** | **`operator new(0x4C)` → `F3::GameInventory::ctor_0x46b970`** — inventory factory virtual |
| +0xB8 | 0x467ad0 | **sub_467640** | **Activate/OnAddToWorld** — see §2.2 |
| +0xBC | 0x467a60 | **sub_463850** | **OnDeath hook**: `JE::Entity::RunScript_54afe0(this, "OnDeath", 1 arg)`; server-gated (`&1`): throws `JE::Common::Exception("Invalid attempt to retrieve world server instance.")` if no server; reads an entity field at `*(v2+119)` (base ambiguous, §5), creates an action via `WorldServer->vft[+132]()`, `GameActionEquip::meth_0x56a1a0(action, arg)`, then `Entity::meth_0x5507f0(7)` (set entity state=7) |
| +0xD0 | 0x467ac0 | **sub_466F40** | engine (`Entity::virt_meth_0x550ad0`) + inventory iteration calling item **vft+8(param_1 stream)** — the *save* counterpart of +0x3C/+0x44 |
| +0xD8 | 0x467a70 | 0x404b70 | `ret` — deliberate no-op override |
| +0xE4 | 0x468ad0 | **sub_468A20** | **ReadFromFile chunk dispatch** — see §4 (new fourccs) |

Slots +0x08..+0x18, +0x20, +0x60..+0x7C, +0x84, +0x88, +0x94..+0x98, +0xAC..+0xB4, +0xC0..+0xCC,
+0xD4, +0xDC..+0xE0 carry engine defaults (JE::Entity bodies at 0x549xxx–0x551xxx) — the base
engine table to diff against is `JE::Entity`'s own vb vft (set by ctor 0x5513a0 via
`Entity_DynamicObject_vftable_692084` for the DynamicObject vb; primary Entity vft not dumped
this session — next session item).

## 2. MYSTERY FIELDS NAMED (updates sessions 1/3 open questions)

### 2.1 GameEntity+0x2C / +0x30 / +0x34 (the "7,7 pair")
Raw evidence at `0x4629e0` (QueryDefaultAnimation, reached with `ecx` = Entity-vb ptr):

```
4629e0: mov eax,[ecx-78h]   ; switch #1, cases 1..9
4629f5: mov ecx,[ecx-74h]   ; switch #2, cases 1..7 (per class-arm)
```

`Entity−0x78 = GameEntity+0x2C`, `Entity−0x74 = GameEntity+0x30` (Entity@GE+0xA4). Therefore:

- **GameEntity+0x2C = AnimationState** (ctor default **0**; strings call it "Animation State";
  valid 1..9; **the activate path sub_467640 writes 5** at 0x467672 — so default 0 = unset and
  5 is the activated state, matching class 5=GameItem's arm? no — 5 is a state value, see
  case table in §1.1 slot +0x58; state 1 returns anim 0).
- **GameEntity+0x30 = WeaponStance** (ctor default **7**; strings call it "Weapon Stance";
  valid 1..7 — 7 = "no weapon/unarmed stance" presumably, as items/ammo share the table).
- GameEntity+0x34 (ctor 7) — still unnamed; NOT read by QueryDefaultAnimation.

### 2.2 Activation path (`sub_467640`, vft +0xB8) — first consumer of `+0x2C` and QueueAnimEvent
```
Entity::virt_meth_0x550970(entity, param_1);
GameActionEquip::ApplyOnCreateEffects_466fe0(gameEntity);   ; 0x466fe0 — "on-create effects" pass
GameEntity+0x2C = 5;                                        ; set AnimationState
f = sub_4B8310(0.0, 100.0);                                 ; rand [0,100)
GameEntity::QueueAnimEvent_4673d0(ge, 1, f);                ; queue anim-event id 1 at random time
if (entity anim object at Entity+0xC0-ish [v4[48]]) {
    dur = JE::Gfx::AnimationObject::GetDuration_4a26d0(*(v6+12), Entity+0x1EC?, Entity+0x1F0?, 0) * 1000.0;
    ; (v4[123]=Entity+0x1EC, v4[124]=Entity+0x1F0 per dword index — verify)
    v8 = sub_48CD00(0, dur);
    Entity+0x?0 (vadj+224) = GlobalTimer->vft[0]() - v8;
    Entity+0x?4 (vadj+228) = that + dur;                    ; animation phase window
}
v11 = TimerObj->vft[+20](1); v11[28] = rand(0,100);
Entity::meth_0x550720(entity, v11);
```
This is the **desync/animation-phase init**: random initial animation offset so identical
creatures don't animate in lockstep (classic technique). `QueueAnimEvent_4673d0` is the entity
anim-event scheduler — xrefs show it called from **GameActionAttack (×3), GameActionDie (×9),
GameActionEquip, GameActionIdle (×2), GameActionMoveTo (×3), GameActionOperateDoor,
GameActionPickUp, GameActionReload, GameActionSetActiveWeapon (×2), GameActionUnequip,
GameActionWait (×2), sub_45C140** — i.e. **every action schedules timed anim events that are
later fired back through the vft +0xA4 OnAnimEvent dispatcher (§4)**. That is the full
attack-timing pipeline: `Action::OnServerUpdate → QueueAnimEvent → (time passes) →
anim-event step virtual (+0x48) → OnAnimEvent(token) → DM/RL/EP/UP/PU handlers`.

### 2.3 `mbr_0x20/+0x24` (the clamped max/cur pair)
vft +0x00 returns **+0x20** and nothing else. The primary consumer identified so far:
`CopyFrom` (§1.1 +0x28) copies the pair as the entity's own state, and session 3's clamp
`cur ≤ max`. Reimpl's `(100,100)` GENT sample = max=+0x20=100, cur=+0x24=100 →
their HP hypothesis is **consistent with all evidence found so far**, but no code yet does
`cur -= damage` here (GameEffectDamage not yet traced). Keep HP naming provisional.

## 3. GENT save format — VERIFIED from the writer (vft +0xA0 = 0x463970)

After `Entity::virt_meth_0x54f6a0` (engine EEN2/EENT write), writes:

```
u32 'GENT' (0x544E4547)      ; local_8 = 1414415687
u32 3                        ; version (loader accepts ≥1; ≥2/≥3 stubbed → 0x409bb0 return-1)
u32 sizeplaceholder          ; patched via seek-back: {size = end-start} written at +8 of chunk
u32 GameEntity+0x08..+0x24   ; 8 dwords, contiguous: f1 f2 f3 f4 f5 f6 max cur
```
(writes use `(this−39)…(this−32)` dwords where `this` = Entity ptr → GE+0x08..+0x24 exactly).
So the shipped GENT payload is **fixed 32 bytes at version 3** — matches the reimpl's observed
template stream (44-byte chunk = 12 header + 32 payload ✓). Loader (`0x463870/0x463910`) reads
6 + 2 dwords ⇒ same shape. No per-field names beyond §2; f1..f6 (+0x08..+0x18) remain raw
(save/load round-trip only; no in-game consumers found yet — see Gaps).

## 4. Entity-file chunk dispatch (vft +0xE4, sub_468A20) — FOURCC SET EXPANDS

```
switch (*(chunk+8)):
 'GENT' 0x544E4547 → 0x466510  (§3/session 3; err "Error reading entity game data from entity file.")
 'EFTR' 0x52544645 → 0x468930  (effect tree; err "Error reading effect tree from entity file.")
 'EEN2' 0x324E4545 → JE::Entity::meth_0x552760   ; engine entity-name data (843990341)
 'EENT' 0x544E4545 → JE::Entity::meth_0x551cf0   ; err "Error reading entity engine data from entity file."
 other             → return 1  (silently ACCEPTED — unknown chunks are skipped, not errors)
```
`GameItem::ReadFromFile_0x411260` (item's own +0xE4 override) adds:
```
 'GITM' 0x4D544947 → sub_4111E0 ; err "Error reading item game data from entity file."
 then tail-calls the base dispatcher.
```
**Implications for reimpl**:
- The **`EEN2`** entity-file chunk is the same fourcc the reimpl saw FIRST in every template
  stream (`{EEN2, GENT, <markers>}`) — it is JE::Entity's own name/data block
  (reader `JE::Entity::meth_0x552760`), NOT the map-side `EME2` record (different fourcc,
  0x32454D45). Decode 0x552760 before assuming anything about EEN2's field layout.
- `EENT` = second JE::Entity engine block (ctor/pos/name/flags persistence) — decodable from
  0x551cf0 + its writer counterpart region near 0x54f6a0 (called by the GENT writer above).
- Unknown fourcc = accept(1): tolerant forward-compat, worth replicating.

Per-class chunk extensions: expect `GCRE/GITM/GIWP/GIAR/GIAM/GOBJ/GDOR/GCON` handlers in the
*other classes'* +0xE4 overrides (session 2's reimpl finding that templates carry these marker
chunks now has a code home). Next session: enumerate each class's +0xE4 override fourccs.

## 5. New class: `F3::GameInventory`

- Size **0x4C**, `operator new(0x4C)` @0x463808 → `F3::GameInventory::ctor_0x46b970`.
- RTTI pair exists: base `JE::Inventory`, derived `F3::GameInventory` (`__RTDynamicCast` sites
  at 0x466f94, 0x467c43, 0x4671f4, 0x468693) — so `GameInventory : JE::Inventory` (non-virtual,
  casted from the engine pointer).
- Storage: **Entity+0x168** (Entity-vb ptr `[E+vbdisp+0xC8]`, e.g. `mov esi,[eax+esi+0C8h]`
  @0x467c1f) — a pointer, presumably set by the +0xB0 factory virtual.
- Contents: RbTree/map at `*(inv+3)` (MSVC iterator loop `i != *(v5+3)`, node value `i[2]` =
  **item pointer (a GameItem/entity interface ptr — vtables of items are invoked directly)**.
- Virtual interface observed: vft+44 (lookup/`operator()` on slot), vft+8/+68/+128 on ITEMS
  inside the save/load/notify loops; +0x9C render-model aggregation reads items'
  Entity+0x160 model handle.
- *(Caution)*: several pointer expressions above have vbdisp/−820/−980 arithmetic the
  decompiler renders inconsistently; where a field offset is load-bearing, the raw
  instruction bytes and EA are cited (0x467c1f, 0x4629e0 etc.) — verify before hardcoding.

## 6. Effect-tree node payload (session 3 hand-off item) — mechanism answered

`cls_0x55a280::meth_0x55a280(obj, chunkHeader, res)` =
`return (*(obj->mbr_0x0 + 20))(obj, chunkHeader, res) != 0` — i.e. the node is a 4-byte holder
whose `mbr_0x0` is the **effect instance object created by `WorldServer->vft[+40](eventId)`**
(template lookup), and the payload parse is **virtual dispatch slot +20 on the effect class
itself** (per-effect-type `ReadFrom`). There is no single wire format: each effect type
implements its own reader at effect-vft+20. Enumerating effect types = next-session queue:
start from WorldServer vft+40 implementation (cls_0x616328 +1) and the EFEV→EFDT/EFCD eventId
space. The CopyFrom deep-clone (§1.1 +0x28) confirms node identity is
`(template eventId @ value+0xC)` + instance params.

## 7. WorldServer (`cls_0x616328`, global 0x70bd98) virtual slots seen this session

| slot | call site | inferred semantics |
|------|-----------|--------------------|
| vft[+8] | 0x4679b3 (ctor) | allocate **net/entity id** (→ GameEntity+0x28) — session 1 |
| vft[+20] | 0x467750 | returns/creates timer-ish object (field+28 float written) |
| vft[+40] | 0x4683e6, 0x468490(session 3) | **GetEffectTemplateById(eventId)** → effect instance |
| vft[+84] | 0x4629a0, 0x466601, 0x46818c | **lookup-by-id** (arg = Entity+0x1EC == reimpl mapId?? see caution) — likely `GetEntityByNetId` |
| vft[+132] | 0x467a27 (OnDeath hook) | **CreateAction()** (result RTTI-casts to GameActionEquip path) |
Also singleton layout: `[0]` vft (vft[0] = GetTime ms? used @0x4676f5), `[1]` = world server,
`[2]` = timer/service object. `*(byte_707CE4)` = separate game-options singleton, vft+316.

*Caution*: at 0x4629a0 the lookup arg was `gap1AC[64]` (Entity+0xEC by the decompiler's layout);
at 0x466570/0x468178 it reads `[E+vbdisp+200/332]`. These need one careful disasm pass to
settle +0xEC vs +0x168 vs +0x1EC vs +0x218 — flagged so the reimpl doesn't trust our shorthand.

## 8. Corrections to earlier sessions

- Session 1 §2 row "+0x2C | 0": ctor zeroes it, but it is the **AnimationState** (§2.1), set to
  5 during activate (§2.2). Not a spare byte.
- Session 1 §7.2 asked about `+0x30/+0x34 = 7,7`: **answer**: the pair is really `+0x2C/0x30`
  being state/stance (raw `[ecx-78h]/[ecx-74h]`); `+0x34` remains unnamed.
- Session 1 §4/Session 2 §3 GameCreature absolute offsets stand, but remember the rule of
  thumb used by IDB member-names: for Entity-relative reads, `field = E_ptr + disp` with
  `E_ptr = this + 4 + vbtable[+4]` — several `[reg-78h]`-style reads mean `GE−0x78+0xA4` NOT
  `GE−0x78`. Always confirm via raw instruction, as in §2.1.

## 9. Gaps / questions (deliberately aimed at the reimpl agent)

1. **QueueAnimEvent/OnAnimEvent pipeline** — you're building the per-frame live pump next.
   The contract you need: `GameEntity::QueueAnimEvent_4673d0(entity, animEventId, timeMs-float)`
   inserts into a pool (`dword_6F3960` next-index list, nodes `cls_0x467780`); stepped by vft
   +0x48 (sub_467830) using the *animation object's* event track; fires the vft +0xA4 dispatcher
   with a 2-token record `{SSO-string@+0x18 "DM"/"RL"/..., int arg@+0x10}`. If your pump
   implements "action → timed anim event → token callback", this is the shipped design. Q: do
   you want the 0x467780/0x4673d0 node layout decoded in a follow-up session?
2. **GamePlayer classId deviation**: with `IsHumanoid()` reading Entity+0x15C==1, the original
   has NO runtime notion of "player" for animation/humanoid purposes — tag stays 1 (session 4).
   Any reimpl player-specific behavior keyed on a distinct class id is a deviation.
3. **GENT f1..f6 (+0x08..+0x1C)**: save/load round-trip exists (§3) but no gameplay reader was
   found this session. Candidates for next chase: GameWorld per-map-list processing,
   GameEffectDamage (cur-stat decrements), interface code calling the vft[0] getter.
4. **Effect-type enumeration**: vft+20 `ReadFrom` per effect class (via WorldServer vft+40).
   Say the word if you want the full effect catalogue (would unlock EFTR payloads + footstep
   FX templates + `ApplyOnCreateEffects` semantics).
5. **New chunks EEN2/EENT in the entity file**: your map-side EME2 parser ≠ entity-file
   EEN2 reader (0x552760). Which stream does your current code consume? (Should be the map; if
   your spawn path ever opens per-entity/template files, dispatch on these fourccs and remember
   unknown-fourcc = accept.)

## 10. Evidence index (this session)

GENT writer 0x463970; CopyFrom 0x468350; LOS 0x4629a0; can-see gate 0x466570; anim step
0x467830; QueryDefaultAnimation 0x4629e0 (raw `mov eax,[ecx-78h]` @0x4629e0,
`mov ecx,[ecx-74h]` @0x4629f5); IsHumanoid 0x466920 ("HumM"/"HumF" @0x666…); OnDeath
0x463850; activate 0x467640; OnAnimEvent 0x467E90; GetUsableSkills 0x4687e0..0x468859;
inventory factory 0x4637F0 (`new 0x4C`), inventory field read @0x467c1f
(`mov esi,[eax+esi+0C8h]`); chunk dispatch 0x468a20 + GameItem 0x411260; Entity-vft dump
0x666044+0x40..+0x124 raw .rdata; fourcc immediates: 843990341='EEN2', 1414415685='EENT',
1297369415='GITM' @0x41126b/0x468a36/0x5528de/0x5528e6.

## 11. Suggested next sessions (my queue)

- Per-class +0xE4 ReadFromFile overrides: full fourcc × class matrix (completes template
  coverage; pairs with your `ClassIdFromTemplateChunks`).
- `EENT` reader 0x551cf0 + writer near 0x54f6a0 (engine entity fields — pos/name/mapId live).
- Effect catalogue: WorldServer vft+40 → effect classes → vft+20 readers (EFTR payload truth).
- `GameCreature` Entity-vft diff vs GameEntity (which slots creature overrides — AI/state),
  then `GamePlayer` diff vs creature (the "party/protagonist bit" the reimpl asked about).
- Settle §9.3 by chasing the vft[0] (`0x4080c0`) *interface-pointer* call sites
  (`call dword ptr [reg]` patterns can't xref — use raw grep for `FF ??` after `mov ?,GameEntity_vftptr`).
