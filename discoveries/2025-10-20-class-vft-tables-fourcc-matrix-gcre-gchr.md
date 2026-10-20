# 2025-10-20 — Session 6: per-class Entity-vft diffs, ReadFromFile fourcc matrix, GCRE/GCHR anatomy

Target: ORIGINAL_REFERENCE. Read-only. Follows [session 5](2025-10-20-gameentity-vft-decode-activation-and-inventory.md).
This session answers the reimpl's "party/protagonist bit" question, maps **every class's
Entity-vb vftable**, proves all classes share the 58-slot interface, and decodes the creature
(`GCRE`) and player (`GCHR`) template-chunk readers — including what is almost certainly the
**SPECIAL block**.

## 1. Every class's Entity-vb vftable (raw; from ctor `mov [vbdisp+esi+4], imm` sites)

Found by scanning the first 0x200 bytes of each ctor for `c7 8x/4x … imm32∈.rdata` (the 3rd
table write is the Entity-vft; e.g. GameCreature `mov dword ptr [ecx+esi+4], 0x64D6DC` @0x40d072):

| class | Entity-vft | primary vft | vbtable | overrides vs GameEntity table | +0xE4 loader |
|-------|-----------|-------------|---------|-------------------------------|--------------|
| GameEntity | 0x666044 | 0x666130 | 0x666134 | — (baseline) | 0x468ad0 |
| GameCreature | **0x64D6DC** | 0x64D7C8 | 0x64D7CC | 29 | 0x40e590 |
| GamePlayer | **0x64E884** | 0x64E970 | 0x64E974 | 29 | 0x411a90 |
| GameUsableObject | **0x64F30C** | 0x64F3F8 | 0x64F3FC | 24 | 0x4127c0 |
| GameDoor | **0x64DDEC** | 0x64DED8 | 0x64DEDC | 27 | 0x40ea40 |
| GameContainer | **0x64CFAC** | 0x64D098 | 0x64D09C | 24 | 0x408e60 |
| GameProjectile | **0x64ED9C** | 0x64EE88 | 0x64EE8C | 24 | 0x412040* |
| GameItem | **0x64E30C** | 0x64E3F8 | 0x64E3FC | 23 | 0x411320 |
| GameWeapon | **0x64FF34** | 0x650020 | 0x650024 | 23 | 0x415660 |
| GameArmor | **0x64CA04** | 0x64CAF0 | 0x64CAF4 | 23 | 0x408970 |
| GameAmmo | **0x64C4BC** | 0x64C5A8 | 0x64C5AC | 23 | 0x408570 |

All eleven tables are exactly **58 slots (+0x00..+0xE4)** (raw scan: pointer run ends at each
table+0xE8 with a non-text dword). Slot indices therefore match across the whole family:
whatever the reimpl calls "Entity virtual N", the same N works for every Game* class.
(\* GameProjectile +0xE4 = `sub ecx,[ecx-4]; sub ecx,18h; jmp 0x468A20` — a pure rebase onto
the GameEntity loader: projectiles add **no** fourcc.)

Per-class override slot lists (diffs vs GameEntity's table; the subclass entries are almost all
vbptr thunks into fresh per-class stub blocks — Creature stubs 0x40d300–0x40d480, Player stubs
0x411900–0x411a80, Usable 0x412610–0x412750, Door 0x40e8e0–0x40e9f0, Container
0x408d10–0x408e50, Projectile 0x411f00–0x412030, Item 0x410420–0x410550, Weapon
0x414680–0x4147c0, Armor 0x408750–0x4088a0, Ammo 0x4082a0–0x4083f0 — **spacing in the stubs
mirrors the shared GameEntity stub block 0x467a00..0x467ae0**, so each class re-thunks the same
virtuals plus its own extras):

- **Creature extra slots** (not overridden by any item/usable class): +0x34, +0x60, +0xb4,
  +0xcc, +0xd4 (0x40e340, 0x40d430, 0x40e350, 0x40d440×2, 0x40d450), +0xe0 (0x40d470), +0xdc.
- **Player extra**: +0xa8 = 0x411bf0 (overrides Usable/Creature's GetUsableSkills chain — the
  player-specific skills path), +0x60.
- **Door extra**: +0x70 (0x4100a0), +0x74 (0x40fc20), +0xac (0x40ea20) — the only class using
  the +0x70/+0x74 region ⇒ **door open/close state virtuals live there** *(inferred)*.
- **Usable/Container/Projectile share** the same set: +0x1c,28,30,34,3c,44,48,4c,50,58,80,90,
  9c,a0,a4,a8,b0,b8,bc,d0,d8,e4 (+ usable-only +0xb4, container shares door's 0x40e900 at
  +0x30 — note `Container+0x30 → 0x40e900` literally reuses a Door stub body).

## 2. ReadFromFile (+0xE4) fourcc matrix — the shipped template-chunk readers

Each class handles its own chunk then tail-calls the base dispatcher (chain verified:
Ammo→sub_408420… else→Item(0x411260)→else→base(0x468A20)):

| class | fourcc | reader `this` | error string |
|-------|--------|---------------|--------------|
| GameEntity | `GENT` 0x544E4547 | GameEntity−0xA4 | "Error reading entity game data from entity file." |
| GameEntity | `EFTR` 0x52544645 | GameEntity−0xA4 | "Error reading effect tree from entity file." |
| GameEntity | `EEN2` 0x324E4545 | Entity−0xA0 | "Error reading entity engine data…" (shared w/ EENT) |
| GameEntity | `EENT` 0x544E4545 | Entity−0xA0 | "Error reading entity engine data from entity file." |
| GameEntity | other | — | **accepted (return 1), NOT an error** |
| GameCreature | `GCRE` 0x45524347 | GameCreature base | "Error reading creature game data from entity file." |
| GamePlayer | `GCHR` 0x52484347 | Entity (=Player+0x824) → reads into **Player+0x808 string** | "Error reading character data from entity file." |
| GameUsableObject | `GOBJ` 0x4A424F47 | Usable−0xC0 (Entity-vb ptr) | "Error reading usable object game data from entity file." |
| GameDoor | `GDOR` 0x474F4447 | Door−0xF0 | "Error reading door game data from entity file." |
| GameContainer | `GCON` 0x4E4F4347 | Container−0x100 | "Error reading container game data from entity file." |
| GameItem | `GITM` 0x4D544947 | Item−0x310 | "Error reading item game data from entity file." |
| GameWeapon | `GIWP` 0x47574950 | Weapon−0x430 | "Error reading weapon game data from **item file**.\n" |
| GameArmor | `GIAR` (1380010311) | Armor−0x3D0 | "Error reading armor game data from item file." |
| GameAmmo | `GIAM` (1296124231) | Ammo−0x3D8 | "Error reading ammo game data from item file." |
| GameProjectile | *(none)* | rebase→base only | — |
| (Creature sub-chunk) | `GWAM` 0x4D414D47 | GameCreature | "GameCreature::ReadData() - Unexpected chunk ID. Attack Mode ID 'GWAM' expected." |

Decoded with `struct.pack('<I')` on the immediates at each cmp site (EAs: 0x40e53b, 0x4117cb,
0x41276b, 0x40e72b, 0x408bcb, 0x41126b, 0x415542, 0x40891b, 0x40851b, 0x468a36, 0x468a2e,
0x5528de, 0x5528e6, 0x40e477). The fourcc set **exactly matches the reimpl's template-signature
class resolver** (EEN2, GENT, GCRE | GITM[+GIWP|GIAR|GIAM] | GOBJ[+GDOR|GCON]) — now every chunk
also has its reader located. `GCHR` is the **only** chunk unique to GamePlayer templates:
if a template stream contains `GCHR`, class 10 is intended. *(This is the party/protagonist bit
the reimpl asked about — check `Default_PC.CRT` for a GCHR chunk after its GCRE; the class-10
create must be driven by that template, since map EME2 never selects 10.)*

## 3. GCRE anatomy (GameCreature::ReadData_0x40e360, versioned)

`v = chunk header version`:

```
always:      sub_40B6A0(creature, res)                    ; §3.1 base stat block
if v != 0:   sub_409410(creature, res)                    ; (not decoded this session)
if v >= 2:   GameCreature::ReadVersion2_0x40d9e0(creature, res)
if v >= 3:   sub_409700(creature, res)
             then SANITY CHECK over 8 dwords at GameCreature+0x70C:
             sum != 100 → "SANITY ERROR: Entity %d's baseUncalledChances don't add up to exactly 100\n"
if v >= 4:   sub_40B9E0(res)
if v >= 5:   read u32 → Entity+0x30 (DynamicObject field, engine)
then: loop *(GameCreature+0x708) times:   ; count set by sub_409700 path
             Common::Chunk::Read; fourcc must be 'GWAM'
             GameWeapon::ReadAttackModeData_0x40dba0(creature, chunk, res)
             Chunk::SkipToEnd
finally: sub_40E180(creature)             ; post-read fixup
```

The 8-dword **baseUncalledChances @GameCreature+0x70C** (= the ctor's memset(0,0x20) block,
session 2 §3 — offsets now confirmed same field) is a **percentage distribution summing to 100**
— this is the combat AI "which attack mode to pick" chance table. Paired with `GWAM`
(Game **W**eapon **A**ttack **M**ode) sub-chunks × count@+0x708: per-creature attack-mode list.
→ Ties directly into `GameActionAttack::OnServerUpdate_404320` and the `DM`/`RL` anim tokens.

### 3.1 Base creature stat block (`sub_40B6A0`, this=GameCreature*)

Straight `MemStreamRead(4)` sequence (all EAs in 0x40b6c0..0x40b9cd):

```
u32[7]  → GameCreature+0xA0..+0xB8   AND mirrored to +0x760..+0x778   ; seven stats, with a
                                    ; parallel "base" copy (current/base pair — SPECIAL?
                                    ; S,P,E,C,I,A,L is 7 — (inferred, high confidence))
u32[8]  → +0xBC..+0xD4 (v3[47..53]) and +0xDC (v3[55])               ; 7 scalars + gap@+0xD8 + 1
loop1:  n1 = read u32; then n1 × (idx u32, val u32) → array@+0xE0[4*idx]   ; sparse override table #1
loop2:  n2 = read u32; then n2 × (idx u32, val u32) → array@+0x114[4*idx]  ; sparse override table #2
loop3:  n3 = read u32; n3 × u32 → append list@+0x25C (list-head pattern)   ; id list (traits?)
loop4:  n4 = read u32; n4 × u32 → RbTree insert@+0x260 (sentinel head; sub_433760) ; id→? map
if n4==0: final u32 → +0x274
```

The dual 7-stat read+mirror at +0xA0/+0x760 is the strongest SPECIAL evidence found so far
(7 values, base copy, sparse per-index overrides — exactly how Fallout engine stat modifiers
work). The 13×0x38 / 12×0x1C Entity sub-tables from sessions 2/4 remain separate (per-knowledge
/ per-slot arrays living inside the Entity vb).

## 4. GCHR = the GamePlayer chunk (`F3::GameCharacter::ReadData_0x4117a0`)

Entire payload: **one length-prefixed string** via `JE::ResourceSystem::StreamReadString_0x49c8d0`
into `ecx+0x804` from the caller base — lands exactly on the **GamePlayer own std::string at
+0x808** (cap 15 SSO, session 2 §4 — base arithmetic: `&v6[-2084]` = Player* and +0x804+4).
IDA's `F3::GameCharacter` naming: the `this` type at 0x4117a0 is `char*`, size unproven;
treat "GameCharacter" as IDA's label for the player-data reader, not necessarily a distinct
class (its dtor/vft were not seen anywhere this session — *(inferred)*).
What the string *is* (character name? party template? .chr path?) needs the writer or a
consumer — next session (find `StreamWriteString` sibling of the GENT writer, or the EEN2 parse
where "HumMNor" style tokens enter Entity+0x218/GE strings).

## 5. Alive/death gate — cur-stat finally consumed at GameEntity+0x24

Slot +0x50 body `0x462970` raw: `Entity::virt_meth_0x549030(e)` else `return [esi-80h] <= 0`
with esi = Entity ptr ⇒ **[GameEntity+0x24] = current stat ≤ 0**. One of the first hard
consumers of the GENT cur/max pair (session 3 §1): semantic = "engine-alive OR current ≤ 0
(dead)" gate. Consistent with HP interpretation (§2.3 session 5); still no code seen *decrementing*
+0x24 — `GameEffectDamage` remains the place to check.

## 6. Consolidated corrections

- Session 4 §2 "GamePlayer Entity-vft: 0x41187e imm unchecked" → **0x64E884** (raw @0x41187e).
- Session 4 missing creature Entity-vft ("0x64D7??") → **0x64D6DC** (raw @0x40d072).
- Item/Weapon/Ammo/Armor/Usable/Door/Container/Projectile Entity-vfts in session 4 §2 table
  are confirmed correct by this session's independent ctor scan.

## 7. Questions/next steps for the reimpl agent

1. `ClassIdFromTemplateChunks` should now use the exact reader fourccs above: `GCHR`⇒10 (check
   Default_PC!), `GCRE`⇒1, `GOBJ/GDOR/GCON`⇒2/3/4, `GITM/GIWP/GIAR/GIAM`⇒5/6/7/9, none⇒0/8.
   Q: does `Default_PC.CRT` contain `GCHR`? If not, class 10 is selected somewhere else entirely
   (and we'd need the entity-create call site in the *player boot* path, not the map path).
2. You're wiring the live pump: per-creature **GWAM attack-mode + baseUncalledChances(+0x70C)**
   is the attack selection data your `GameActionAttack::OnServerUpdate_404320` reimpl will want —
   say the word and session 7 decodes `ReadAttackModeData_0x40dba0` field-by-field.
3. The +0xA0..+0xB8 7-stat block (SPECIAL?) and the two sparse override tables (+0xE0/+0x114)
   are cheap fidelity wins for any stat display — we can decode `sub_409410`/`ReadVersion2` next.
4. Confirm your `SpawnedEntity` never allocates 0x804/0x824-relative offsets without going
   through the §1 table — Door/Container extra vslots (+0x70/74/AC) are door-state and will be
   needed for `GameActionOperateDoor` (its QueueAnimEvent callers are already listed, session 5 §2.2).

## 8. Evidence index

Entity-vft writes: 0x40d072, 0x41187e, 0x412481, 0x40e852, 0x408c80, 0x411cc0, 0x4101e2,
0x414573, 0x4085e7, 0x408010. GCRE: 0x40e360 (dispatch), 0x40b6a0 (base block),
0x40e3d7+0x40e3e2 (sum-100 loop), 0x40e477 (GWAM fourcc), 0x40dba0 (attack mode reader).
GCHR: 0x4117a0→0x411670 (`add ecx,804h` @0x411675). Fourcc cmps: see §2. Alive gate: 0x462990.

## 9. My queue after this session

- `ReadAttackModeData_0x40dba0` field map (GWAM) + GameActionAttack consumption of it.
- `sub_409410`, `ReadVersion2_0x40d9e0`, `sub_409700`, `sub_40B9E0` (GCRE v1–v5 content).
- `GOBJ/GDOR/GCON/GITM/GIWP/GIAR/GIAM` reader field maps (small, mechanical).
- EENT reader 0x551cf0 / writer 0x54f6a0 (engine entity fields).
- Effect catalogue (WorldServer vft+40 → effect class list → vft+20 readers).
- GCHR string consumers + Player/UsableObject vslot semantic pass (GetUsable* chain).
