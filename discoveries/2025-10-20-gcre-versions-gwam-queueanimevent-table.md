# 2025-10-20 — Session 7: GCRE version sub-readers, GWAM attack-mode record, QueueAnimEvent id table

Target: ORIGINAL_REFERENCE (`artifacts/F3.exe.i64` + raw `$OPENVB_ORIG_EXE`). Read-only;
no IDB/source edits. Follows
[entity-file-format-gent-eftr](2025-10-20-entity-file-format-gent-eftr.md),
[gameentity-vft-decode](2025-10-20-gameentity-vft-decode-activation-and-inventory.md),
[class-vft-tables-fourcc-matrix](2025-10-20-class-vft-tables-fourcc-matrix-gcre-gchr.md).
Complements (does not overlap)
[session6-classid-readers](2025-10-20-session6-classid-readers-weapon-ammo-combat.md) and
[gameworld-interfaces-effects-classid](2025-10-20-gameworld-interfaces-effects-classid.md).

Closes the HANDOFF offers: GWAM field map + GCRE v1/v2/v3 sub-readers + the anim-event queue
mechanics the reimpl wanted for the live pump.

## 1. GCRE version-gated sub-reader map (all `this` = GameCreature*)

Dispatch is `GameCreature::ReadData_0x40e360` (see fourcc-matrix doc §3). Payload order per
GCRE header version `v`:

### 1.1 v1 block — `sub_409410` (called when `v != 0`; EA evidence 0x409482..0x4096de)

```
string → Creature+0x278                 (StreamReadString @0x409482, this+632)
u32    → Creature+0x294                 (this+660)
string → Creature+0x298
u32    → Creature+0x2B4                 (this+692)
byte   → Creature+0x2B8                 (bool)
×12:   string → Creature+0x2BC + 0x1C*i   (loop cap 0xC @0x409496..0x4094b9)
×13:   string PAIR → Creature+0x444+0x1C*(2i?) ...   (loop cap 0xD @0x4094c0; each iteration
        reads TWO consecutive 0x1C-spaced string slots: (j−28) and (j))
×12:   byte bool → Creature+0x6E4+k      (1764..1775; each `bool = (u8!=0)`)
```
NOTE the "12" cap matches the 12×0x1C array shape from entity-family-subtables doc §6, but this
array is at **GameCreature+0x2BC (own region)** — the Entity-vb 12×0x1C array seeded from the
name strings is a *different* array. Same caution for the "13" pair-loop vs the Entity-vb
13×0x38 array. Contents: 12 strings + 13×2 strings + 12 bools = per-creature dialogue/package/
behavior lists? *(semantics unconfirmed — candidates: voice files, perk/prereq pairs; the
bools @+0x6E4 could be per-skill flags — 12 ≈ ?)*. Say the word if you want consumers chased.

### 1.2 v2 block — `GameCreature::ReadVersion2_0x40d9e0` (called when `v >= 2`)

```
u32 count → Creature+0x6EC     (gap47C[624]; "No Mode counter found" on read failure)
if count == 0:                 ; creature has zero attack modes → FAILSAFE melee mode injected:
    log "GameCreature::ReadVersion2() - Making failsafe attack mode for entity: %s\n"
    record (0x70-byte node via list-insert sub_40B510 into list-head Creature+0x6F4/+0x6F8 —
    the ctor's first 0x70 pool @0x40d123):
        dword[0]=3  [1]=27  [2]=4  [3]=0  [4]=1  [5..7]=0  [8]=3  [9]=3  [10]=0 [11]=0
        [12]=1 [13]=2 ; byte f1=1, f2=0, f3=0 ; int v14=3 ; v15=97 ; v16=888 ;
        string = literal @byte_64D7D8 (the "default/unarmed" mode-name token — same seed the
        ctor uses at 0x40d286) ; trailing bools 1,0,0
```
⇒ the GWAM record is **~0x70 bytes** and a weaponless creature still gets anim id 97 mode.
`v15=97` is in the GWAM→anim-id map space (§2.2) and `v16=888` looks like a duration/AP sentinel.

### 1.3 v3 block — `sub_409700` (called when `v >= 3`) + the sanity check

```
×8: u32 → Creature+0x710 + 4k           (loop `this+452` @0x40970c)
×6: u32 → Creature+0x730 + 4k  AND mirror at +0x7B0 + 4k   (j[32] copy @0x40974b)
×6: u32 → Creature+0x748 + 4k  AND mirror at +0x7C8 + 4k   (k[32] copy @0x409777)
```
This is the exact field set the ctor zeroed (entity-family-subtables §3 "+0x730/+0x748/+0x7B0/
+0x7C8 zero loops") — two **current/base pairs** (0x80 apart). The post-read sanity at
0x40e3d7 sums 8 dwords starting `&mbr_0x70c` and requires `== 100`
("SANITY ERROR: ... baseUncalledChances don't add up to exactly 100"). ⚠ One-of-us is off by a
dword: reader loop writes at +0x710, sanity loop reads from +0x70C (raw `lea` operands at
0x40e3d7/0x40970c — verify before hardcoding; likely the chance array is +0x70C with a leading
field, or IDA's mbr_0x70c name is +4 skewed). Semantics unchanged: **8-entry percentage
distribution = AI "uncalled chance" table** consumed by the attack-mode picker
(the session6-classid doc's `ResolveWeaponAndAttackMode_0x40b380` is its reader side).

### 1.4 v4/v5 blocks

- `v >= 4 → sub_40B9E0`: count u32, then count×{string + …} referencing WorldServer
  (`global_cls_0x616328_0x70bd98`) per iteration — partial disasm @0x40ba40..; **not fully
  decoded** (SEH-guarded string temporaries defeat quick decompilation; do it with raw scans
  next round if needed).
- `v >= 5`: one u32 → **Entity+0x30** region (per fourcc-matrix doc §3 — engine-side field).
- After everything: `sub_40E180(creature)` post-fixup (unchased).

## 2. GWAM attack-mode record (`GameWeapon::ReadAttackModeData_0x40dba0`, `this`=Creature, a2=Chunk)

Fields in stream order (all MemStreamRead u32 unless noted; EAs in 0x40dc15..0x40defe):

```
[0] modeType       1..34+  (switch §2.2)
[1] a              u32     @0x40dc47
[2] b              u32     @0x40dc79
[3] c..[j]         9 × u32 @0x40dd86 chain
[k]                u32     @0x40dd86 chain
[l] bool           1 byte  → stored as `!=0` (@0x40dda0, v20[4])
── if *(chunk+0x30) != 0  (chunk-object extra flag; likely "has weapon requirements"):
[m] u32            @0x40de49
[n] bool  v21 = "consumed"
[o] u32            v24
[p] bool  v22 = "thrown"
[q] bool  v23
    FatalError if consumed && thrown:
    "GameWeapon::ReadAttackModeData() - A weapon can't be both consumed and thrown."
── gated on chunk-object field *(v3[12]) (the GWAM chunk's own sub-version dword,
   read via `*&a2[12]`, byte offset 0x30 of the Chunk object — NOT the Common::Chunk header
   version; verify which Chunk::mbr maps if you reuse):
≥2: u32  v25       (anim/attack override; if 0 → default from §2.2 map)
≥3: u32  v26
≥4: string v27     (StreamReadString)
≥5: 2 bytes v31,v32
── end
record appended: sub_40C970(&Creature+0x6F8 listhead, &record)   ; 0x70-byte pool node
```

### 2.1 Storage
Same list as §1.2 failsafe: nodes of the 0x70 pool anchored at **Creature+0x6F4/+0x6F8**
(ctor `sub_40A190()` @0x40d123). Count at **Creature+0x6EC**. The GCRE loop iterates
`*(Creature+0x6EC)` GWAM chunks after the version blocks (fourcc-matrix doc §3 had the count
slot misprinted as +0x708 — corrected here: the loop var is `*&this->gap47C[624]` =
0x47C+0x270 = **+0x6EC**, @0x40e44b/0x40e493).

### 2.2 modeType → default anim-id map (when v25==0; 0x40df2f..0x40e0a5)

```
1→56  2→59  4→61  6→60  7→63  8→64  9→65  10→53  11→54  12→55
13→83 14→84 15→85 16→86 17→87 18→88 19→89 20→90 21→91 22→92 23→93 24→94 25→95 26→96 27→97
28→98 29→99 30→100 31→101 32→102 33→103 34→104
```
⇒ **modeType 13..34 ⇔ anim ids 83..104**, and 1..12 ⇔ the 53..65 band. This is the same id
space that QueueAnimEvent cases 53–104 use (§3) and that QueryDefaultAnimation returns from its
own table (56/59/…/104 ⊂ {26..511}). modeType is therefore a **weapon attack action-kind enum**
(one punch/bullet pattern per canonical anim), and v25 overrides its animation.

## 3. `GameEntity::QueueAnimEvent_4673d0` — the anim-event id table (HANDOFF item CLOSED)

Guard: `if (GameEntity+0x9C /*busy byte*/) return 0;` @0x4673d0. Cases (EAs 0x4673fd..0x46753e):

| event id | body | semantics |
|----------|------|-----------|
| 1 | `Queue_Idle_462c50(this, timeFloat)` | state/stance → idle anim (§3.1) |
| 2..6 | `meth_0x463000/0x463040/0x463080/0x4630c0/0x463100(this, t)` | per-weapon attack-anim queues (one fn each) |
| 7 | `QueueToCombat_463140` | combat-stance transition anim |
| 9 | `QueueToDefault_465c20` | return-to-default anim |
| 11 | `Queue_To_Crouch_465ee0` | crouch |
| 17,19,39 | `meth_0x464b80(this, 0/1)` | pair A (0/1 variant) — *(candidate: aim-in / ready)* |
| 18,20,42 | `meth_0x464c70(this, 0/1)` | pair B — *(candidate: aim-out / relax)* |
| 21 | `Queue_Equip_465ae0` | equip anim |
| 22 | `Queue_Unequip_4636b0` | unequip anim |
| 29 | `Queue_Limp_4669f0` | limp/hurt anim |
| 32 | `Queue_Walk_465fc0` | walk anim |
| 33,34,37,38,40,41 | `meth_0x464d70(this, id)` | flourish group (id passed) |
| 43..52 | `meth_0x464940(this, id)`; **sets +0x9C=1 first** | busy-locked group (blocking anim) |
| 53..104 | `meth_0x463c80(this, id)` | **direct anim-id band == §2.2 map output space** (attack anims by id) |
| 120 | `meth_0x4659a0` | |
| 121 | `meth_0x4667a0` | |
| 141 | `meth_0x466880` | |
| other | `"GameEntity::QueueAnimEvent() - Unhandled ANIMEVENT.\n"` → 0 | |

### 3.1 Queue_Idle (id 1) confirms the state×stance matrix
`Queue_Idle_462c50` (0x462c50) switches **+0x2C (animState) 1..9** × **+0x30 (weaponStance)
1..7** and returns the SAME ids as `QueryDefaultAnimation` (26,74,105,136,172,204,253 /
318,343 / 355 / 361,371,377,383,389,395,416 / 438,445 / 487 / 502 / 511; state 1 = no-op),
then calls the engine queue primitive:

```
sub_5505F0(Entity*, animId, 0, 0, 0, timeFloat, 0)     ; JE::Entity::QueueAnim-style
```
Strings `"GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n"` /
`"... Unknown Animation State set.\n"` (0x462f7b / 0x462f93) — same enum names as
QueryDefaultAnimation: **+0x2C = AnimationState, +0x30 = WeaponStance, re-confirmed from the
queue side** (anim-pipeline roadmap in the vft-decode doc §2.1/§2.2 stands).

### 3.2 Stepping + the "event exists" query (terminology correction)
`cls_0x467780::meth_0x467780(animObj, animIdx, frame, ?)` (0x467780) is NOT the queue node:
it queries the **animation object's event track** (list head at +0x370/+0x374, node fields
node[5]=start node[6]=end, entries `*(start + 4*frameIdx)`, key at +80) —
"does this animation have an event at this frame". The per-entity pending-queue pool indexed
through `dword_6F3960[next]` (see vft-decode doc §1.1 slot +0x48) remains undecoded — it is
the engine-side queue fed by `sub_5505F0`. For the live pump you can start from
`sub_5505F0`'s entity queue instead of reverse-engineering the pool from the read side.

## 4. Practical summary for the reimpl (creature combat loop data)

1. Per-creature attack modes = list at `Creature+0x6F4/0x6F8`, count `+0x6EC`, node 0x70 bytes,
   filled by GWAM chunks in GCRE; **zero GWAM ⇒ failsafe unarmed mode (anim 97, value 888)**
   is auto-injected — replicate this or unarmed creatures will have no attack mode.
2. Attack-mode pick consumes the **8×u32 chance table** (sum==100) at ~+0x70C/+0x710 (§1.3);
   the resolution side is `ResolveWeaponAndAttackMode_0x40b380` (session6-classid doc).
3. To wire the pump: actions call `QueueAnimEvent(id, t)` — the id table (§3) is complete for
   the ids the shipped demo's actions actually use (1 idle, 32 walk, 21/22 equip/unequip,
   2..6 attacks, 7/9 stance, 53..104 direct anim). Idle desync = `QueueAnimEvent(1, rand0..100)`
   from the activate virtual (vft-decode doc §2.2).
4. `GameEntity+0x9C` byte = "busy anim queued" gate — cleared where? (unchased; grep
   `mov byte ptr [...+9Ch], 0` if you need it before we do.)

## 5. Open items (mine unless claimed)

- v4 (`sub_40B9E0`) string→WorldServer loop content; `sub_40E180` fixup.
- The 12 strings / 13×2 strings / 12 bools @+0x2BC/+0x444/+0x6E4 consumers (§1.1).
- Chance-array base ±4 discrepancy (§1.3) — raw-verify next round.
- `sub_40C970`/`sub_40B510` node-insert shapes if you need the exact 0x70 layout field-by-field;
  we can map it from the two writers (GWAM 0x40e0ad + failsafe 0x40db28) in one pass — say so.
- `dword_6F3960` pool (only if `sub_5505F0`'s queue turns out inaccessible at runtime).

## 6. Evidence index

GCRE v1 0x409410 (loops @0x409496 12×str, 0x4094c0 13×pair, 0x409504.. bools).
v2 0x40d9e0 (counter @0x40da19, failsafe record 0x40da79..0x40db60, insert sub_40B510 @0x40db53).
v3 0x409700 (8+6+6 with mirrors @0x40970c/0x409730/0x40975c). Sanity sum 0x40e3d7..0x40e3e8.
v4 0x40b9e0 partial (count @0x40ba28, strings+WorldServer @0x40ba5e..). GWAM reader 0x40dba0
(append @0x40e0ad; fatal @0x40de73; modeType map 0x40df2f..0x40e0a5).
QueueAnimEvent 0x4673d0 (all case bodies 0x462c50..0x466920 range); Queue_Idle 0x462c50
(sub_5505F0 calls @0x462c8a..); anim-event query 0x467780; activate desync 0x467640/0x467689.
