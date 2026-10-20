# 2025-10-20 — CANONICAL FACT SHEET (session 32: corpus audit) ★ reference doc

Cross-checked against raw bytes this session (×4-index fix, id-block check, packet registry
recount). **When any older doc conflicts with this sheet, this sheet wins** — each row states
its verification basis. Everything else stays as audit trail.

## 1. Canonical packet-id registry (F3 game layer)

| id | name | dir | evidence |
|----|------|-----|----------|
| 11 | **ENTITY DIE REQUEST** (console `kill` only — NO auto-death in F3, sess 34) | C→S | GameClient::meth_0x589980 → server 0x46de28 CreateGameAction(0) |
| 9..70 (9,10,15,26,37,40,45,56–61,64,65,67,70) | engine control (spawn-record/script-attach/join/warp/state) | both | JE::Server::virt_meth_0x556ea0 (sess 9/11) |
| 41/42 | ITEM_EQUIP / ITEM UNEQUIP (engine-side; logged pre-switch client recv) | S→C | 0x461a4f/5b |
| 72,74,75,76,77,80,106,107 | client commands, DEAD in demo | C→S | senders sess 10; handlers read-discard sess 11 |
| 73 dialogue / 78,79 operate / 81 talk(GetInfo) / 82 pickup / 83 give / 90 reload / 91 attack | live client commands | C→S | senders + handlers + GameParty… sess 10/11/12/31 |
| 84 | creature/weapon-mode poke + server reply echo | both | handler 0x46e4dc |
| 85 | **GAME_ID_ENTITY_DAMAGE** (floater + HP sync) | S→C | log-string map sess 30; client handler 0x461d27… |
| 86 | HP-pair stream field set (UnpackState-adjacent) | S→C | client case 'V' |
| 87 | entity+0x08 dword set | S→C | case 'W' |
| 88 | **GAME_ID_ENTITY_SETCURRENTHP** (SetCurHP chokepoint both ends) | S→C | sess 30/31 |
| 89 | **SetActiveWeapon request** (fix sess 34: factory(0xB) server-side) | C→S | sess 31/34 |
| 92 | inventory remove/add (GameInventory at entity+360; vft+60/+56) | S→C | sess 31 |
| 95 | **Equip** (GAME_ID_ITEM_EQUIP server-side; factory(9)) | C→S | sess 34 |
| 96 | **Unequip** (factory(0xA)) | C→S | sess 34 |
| 97/98/99 | journal ops (GameJournal) | S→C | sess 31 |
| 100/101 | weapon query → server reply | both | sess 11/12 |
| 102/103 | WEAPON_SET_PRIMARY/SECONDARY_AMMO | S→C | names sess 30, handlers sess 31 |
| 104 | GAME_ID_INTERFACE_HIDE | S→C | sess 30/31 |
| 105/108/109 | string/music?, container-open UI, debug print | S→C | sess 31 |
| 110/111/112 | weather demo toggles (rain/snow/wind .veg, shared VEG pool) | C→S | sess 11 §1 |

## 2. Canonical entity identity/id block (JE::Entity view, size 0x3D8)

| off | role | basis |
|-----|------|-------|
| +0x160 | model-cache handle (PlayAnim target) | sess 3/19/26 |
| **+0x15C** | **classId tag** (0..9 = factory case; Player keeps 1) — read by operate-dispatch (78/79), packed in PackState | sess 4/11/15 |
| +0x168 | active state/effect child (pack ext vft+32; union w/ water param) | sess 21/28 corrections |
| +0x174..0x17C | flags (usable-family byte 0x179 etc.) | sess 11/14 |
| +0x1DC | id used on wire (join/broadcast/PackState field) | sess 9/15/21 |
| +0x1E0 | **instance/script key — serialized as a BYTE by Entity::PackState (0x54A610 via 0x45F0B0)** ⇒ ≤255 slot number, NOT a wide id; passed as script-key dword by GetScript/AddHP (dword load of the byte) | sess 15/22/28 + audit this session |
| +0x1E4 | stream-read sibling id (UnpackState reads from stream) | audit this session |
| +0x1EC | **mapId (VERIFIED sess 38: Entity::SetMap 0x54BBA0 sole functional writer @0x54BC9A; GetMapById via vb-vft+84; refcount ±1 at map+0x118 — same field Map::PackState emits as its int#2)** | reimpl doc + raw verify |
| +0x2A4..+0x2C0 | script name string + cached Script_Small* (+0x2BC) + fail memo (+0x2C0) | sess 28 |
| +0x384+ | placement record (-1 unbound pair) | sess 24 |
GE own-block: +0x20 maxHP / +0x24 curHP (GENT f7/f8; SetCurHP 0x4629c0 clamps 24→20) ✓✓
GE+0x2C animState (1..9; **10=corpse**), +0x30 weaponStance(1..7), +0x9C dying-lock.

## 2b. Wide id-counter ambiguity (E6, updated sess 38)
Two counters exist: GameWorld+0x29C (vb-view getter 0x46FD60 — the value the GameEntity ctor
stores to GE+0x28) vs World+0x8C (ws-view idx8 getter 0x46F280; next-id user in server flows)
with setter 0x46FE40 (World+0x7C, 29-bit mask). Roles likely: instance serial vs allocator.
+0x1EC is now SETTLED (mapId, sess 38) — remaining question is which of 0x29C/0x8C feeds
packet-visible ids → E6 unchanged.

## 2c. Slot-numbering correction (sess 37)
Where older docs said "ws vft+8 = next-id getter": byte +8 on 0x66856C is idx 2 (0x46F260); the next-id READ is idx 8 = byte +32
(0x46F280 → [view+0x88] = World+0x8C), and the GameEntity-ctor net-id fetch uses the VB view (0x668494 idx 2,
0x46FD60 → GameWorld+0x29C). See §2b for the remaining counter question (E6).

## 3. Canonical weapon/ammo fields (post ×4-fix)

weapon own (0x3B4..0x42F): **caliber +0x3B4, modifier +0x3B8, cached ammoProto +0x424**
(×4 dword-index fix applied to attack/tohit docs this session). ammoProto: dmgMin +0x3B8,
dmgMax +0x3BC, DTpen +0x3C0, toHitMod +0x3C4 (all verified from raw `this+N` sites).
modeRec (attack record): [+8]style [+0x10]dmgType [+0x1C]ammoCost1 [+0x24]flatDmg
[+0x28]toHitBonus [+0x30]countType [+0x38/+0x3C]dice [+0x44]protoPtr [+0x50]ammoExtra
[+0x44?→0x48] — offsets as dword×4 from CalculateDamage/ToHit raw: [2]=+8,[4]=+0x10,[7]=+0x1C,
[9]=+0x24,[10]=+0x28,[12]=+0x30,[14]=+0x38,[15]=+0x3C,[20]=+0x50,[24]=+0x60.

## 4. Canonical action/type tables

GameActionType: Die0 Idle1 MoveTo2 PickUp3 Wander4 Wait5 (6,7 engine) FloatText8 Equip9
Unequip10 SetActiveWeapon11 — sizes/ctors sess 31.
GameEffectType factory(vft+40): 0=Effect(0x70) 1=GameEffectDamage(0xB0) else FatalError.
QueueAnimEvent ids: 1 idle-override(→26/74/…/511 table) 7 combat 9 default 11 crouch
21 equip 22 unequip 29 limp 32 walk **43..52 death (clips 233..241/484/499 → state 10)**
53..104 attack clips (per-stance sets; e.g. 57→{22,72,103,134,166,202,247}).

## 5. Canonical special globals

0x70BD98 = &GameWorld(=JE::World prefix at +0; views: +0 IWorld 0x6685A4, +4 IWorldServer
0x66856C, +C 0x668548, vb 0x390/0x394-0x398). 0x70BE0C = live-entity list head (ctor-reg).
0x6F4170 = 517 clip-name strings (runtime). 0x64F858 = called-shot mods [10,40,40,40,40,60,
120,60]. Settings registry class = cls_0x56af40 ("VFX_Parameter" is a mislabel).

## 6. Audit actions taken this session
- FIXED ×4: `weapon+0x374/0x378` → `+0x3B4/+0x3B8` (attack-state-machine doc + tohit doc).
- FIXED (same audit pass, later update): +0x1E0 is a BYTE field (script/instance slot ≤255),
  not a wide id — Entity::PackState byte-writes it (0x54A610 @0x54a660); GetScript's dword
  read is loading the byte-slot. +0x1DC remains the only wide wire id.
- CONFIRMED: id-block roles §2; ALL prior retractions consolidated (packet/msg-factory,
  view-arithmetic, cls_0x616328, HP-slot, evasion-stub).
- Corpus verdict: docs internally consistent modulo "provisional" flags; Entity::PackState
  base serializer (23 steps incl. 3 strings + ext-chain flag @+0x168) == session 22 §2.

## 7. Save/warp wire format (assembled from sess 15/16/17/24/30 + Map::UnpackState 0x572F10)

```
World pack (this = ws view +4, canonical §2 rule!) : [8 ints][entity section][map section][tail]
 entity section: repeat { u32 +0x1DC(id), u32 +0x15C(classId), Entity::PackState(0x54A610):
    str@1C4, byte@1E0, int@1E4, int@1EC, int x/y/z @4/8/C, int 1F0/1F4/1F8,
    str@21C, str@238, str@254, int 28C/290/180/184, byte 296/297, int 298/29C,
    u8 ext-flag(@+0x168?1:0) then ext->vft[+32](packet) } until u32 0
 map section: repeat { u32 mapId, Map::PackState } until u32 0
Unpack (0x547E60, mirror): ENTITY ids 24-bit (3 bytes) !; maps 32-bit; per-entity flow =
 create-by-id via world-vb vft+24 (CreateGameEntity) → Entity::UnpackState(0x54E690).
 Map::UnpackState(0x572F10): guard map+0xB8==0 ("invalid maps") → name/strs/ints → member
 24-bit id list (0-term) → octree re-insert each → map+0xB8=3 ("normal").
```
⚠ FLAGGED (raws 0x56d82d vs 0x572f81): Map string field base differs by exactly 4 between
the pack path (cls_0x56d7f0 wrapper view: +0x10) and unpack's direct this (+0x0C). Both
self-consistent; unresolved statically. Canonical position: trust UNPACK (it constructs).
One save→load→diff test settles it — logged as reimpl experiment ④.

## 8. Native-call facts (session 33)

- All gameplay natives = `arity-check → resolve entity-id params → new <Action>(sizes:
  canonical §4) → set fields → Entity::AddAction`. One generic spawn helper covers ~41 of them.
- `A_JustAttack` = Attack with +0x68.byte2=0 (skip weapon resolve) + 0x6D=0 (no auto-move).
- Dialogue origin flag: action +0x1D — packet-73 path sets 1 (remote), StartDialogue native
  sets 0 (local). Touch(entity 0x54C440) precedes dialogue-spawn.
- **Quest/journal script natives (OpenQuest/CloseQuest/AddQuestEntry/AddJournalEntry) are
  `return 1` no-op stubs (0x420990)** — quest state flows only via packets 97–99 +
  GameJournal; do NOT port script-side quest effects.
- `JE::Gregorian` identified (ctor 0x544950, vft 0x69166C): calendar/clock object — 0x18-byte
  Y/M/D/H/M/S block qmemcpy'd from a param struct + counters; the type behind the World clock
  fields (sess 23 InitClock) and the effect-driver seen at 0x550f05 region (sess 19). The
  "world+0x2C/0x34 writer" grep hit (E2) turned out to BE THIS CLASS's field init — E2 stays
  runtime-only (false positive formally closed).
- SetMoveMode: not an action — player-controller direct (meth_0x46EC70), gated on player
  existence (area-map-icons-obj+0x18).

## 9. Open items (all owner-assigned)

1. Reimpl runtime: ① CreateGameAction(0) breakpoint ② World +0x2C/0x34 writers ③ clip
   prefix dump ④ Map string-base round-trip.
2. Me, on demand: remaining native bodies (DisplayStringRef/PlayMusic/AddCNPCToParty/
   Lock-UnlockDoor are the interesting five), container-UI family (GameClient+0xA0).

## 10. Sess 38 addendum (cross-agent verification pass)
- Reimpl's SetMap story (dialogue-doc §3) verified against raw; bonus finding: SetMap tail
  writes `~mapId` into the placement-record pair (Entity +0x38C/+0x390) — **that** is the
  "−1 unset pair" from session 18 §1 (unset ⇒ mapId 0 ⇒ ~0 = −1 ✓ the layout self-consistent).
- .dlg resource suffix present in binary as claimed (byte-verified); recursion guard global
  0x70CE00 in .data as claimed. Child-migration loop = entity's child list @+0x2F8 (session 18's
  "gap2F4[4]" head!) — children carry their own mapId slot updated at child+44 (coords: list-payload
  view — flagged for the door/child doc owners to pin).

## 11. Unarmed-attack branch audit (sess 39, answering combat-lane query)
- `GameCreature::CalculateUnarmedDamage_0x409F90` @0x40a045 raw = `03 35 24 00 00 00` =
  **`add esi, ds:0x24`** — an ABSOLUTE read of VA 0x24, not a lost register.
  **⚠ READ THIS BEFORE "FIXING" IT: the decompiler's `MEMORY[0x24]` is LITERAL. There is no
  hidden base register. VA 0x24 sits in the null page (imagebase 0x400000; nothing is mapped
  low), so this instruction FAULTS ON EXECUTION (access violation) if the branch is ever
  taken: (param_5≠null AND mode+0x18∈{1,3} AND weapon+0x3B0∈{9,10}). The demo never reaches
  it ⇒ shipped latent defect, provably unreachable-by-data. Do NOT port the add; do NOT
  guess a base register; do not treat this as a TODO. (combat-lane decision, 2 agents:
  permanently out of CalculateUnarmedDamage, citing this section.)**
- param_5 (3rd arg at 0x4040d4) = **action+0x2C = resolved equipped GameWeapon\*** (written
  at 0x40446b via RTTI cast; the weapon branch passes the SAME expression as CalculateDamage's
  `this`). Can be non-null for "unarmed" modes (punching while carrying a gun).
- weapon+0x3B0: first dword of the item-own tail block (0x3B0..0x3D0, zeroed in both ammo
  (edi=0) and weapon (ebx=0) ctors, filled by item-chunk readers). Compared with 9/10 at
  EXACTLY two sites (this one + unarmed-to-hit twin 0x409eaa/0x409eb8), always paired with
  mode+0x18 ∈ {1,3}. **Hypothesis (plausible, NOT proven): +0x3B0 = body-part weapon class
  (9=hand-type e.g. knuckles, 10=foot-type e.g. boots), modeRec+0x18 = limb attack style
  (1=punch,3=kick)** — GetWeaponCombo/SetWeaponCombo natives corroborate a limb-combo system.
  Verification route: GCHR/GIAM chunk doc (combat lane has the field-order decode — if
  boots/knuckles items carry +0x3B0=10/9, hypothesis confirmed).
- modeRec field map gains: **[+0x18] = attack style/limb (1 punch, 3 kick per above)**;
  [+0x10]=damage type (4 = hand-to-hand per combat lane's data-doc — second opinion: their
  GCHR decode).
- **RETRACTION (session 29 §4 called-shot note)**: 0x409ffd's `param_1[4]==4` reads mode
  +0x10 = DAMAGE TYPE, not a body part. The "part 4 = arm/hand" corroboration claimed in
  sess 29 §2 is VOID — called-shot index meanings (dword_64F858 = [10,40,40,40,40,60,120,60])
  remain officially UNRESOLVED, single-source.

## 12. Pending cross-lane items (registered sess 40)
- +0x3B0 limb-weapon-class hypothesis (this sheet §11): OWNED by combat lane (NiceFalcon) —
  test = GCRE/GIAM/GITAR + unarmed GWAM rows: knuckles→9 / boots→10 / punch→1 / kick→3.
  Verdict will come back as CONFIRMED (with record names → promote in §11) or KILLED
  (→ delete from §11). Until then §11's wording stays "hypothesis, unproven".
- called-shot dword_64F858 index→part: OPEN on BOTH sides by policy — neither lane names
  parts without a second source.
- damageType==4 → +MELEE_DAMAGE mapping: combat-lane decode, labeled theirs; not mirrored
  into this sheet as established fact.

## 13. GW+0x40 resolved (sess 41, closes routed accMs question)
Writers: World ctor (=6) + InitClock atol("Seconds Per Round") only — no continuous writer.
Reader: ws-slot0 getter (0x436240, view+0x3C) whose consumer adds `1000×GW+0x40`
**per dispatched queue event** (round-clock advance @0x5a95ac; explicit-time mode when
owner-byte +0x319 set). Both coordinator label and research slot0 claim verified correct;
T-011 clock guidance in that doc's §6.
