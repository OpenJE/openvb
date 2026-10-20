# 2025-10-20 — Session 9: GOBJ/GDOR/GCON field maps, door geometry vslots, GameInventory::AddItem (answers C-7)

Target: ORIGINAL_REFERENCE. Read-only; no IDB/source edits. Follows
[class-vft-tables-fourcc-matrix](2025-10-20-class-vft-tables-fourcc-matrix-gcre-gchr.md) §2 and
[een2-eeov-eent-engine-entity-blocks](2025-10-20-een2-eeov-eent-engine-entity-blocks.md) §2.
Implements implementer-A's request **C-7.1** (pin WorldServer name-lookup + inventory AddItem
shapes) and delivers the usable-family chunk maps.

## 1. GOBJ — GameUsableObject chunk (`cls_0x412540::meth_0x412540` @0x412540)

Plain payload, **7×u32** in stream order → `GameUsableObject+0xA0, +0xA4, +0xA8, +0xB0, +0xB4,
+0xB8` (+0xAC) (evidence: `&this->mbr_0xa0` read @0x412558, loop of 6 dwords 0x412569..0x41258a,
`this` = Usable base). This is the ENTIRE usable own-block: the ctor zeroes the same range
(rep stosd +0xA4..+0xBB, full-class-geometry §4). Door and Container inherit this (their loaders
tail-call it for 'GOBJ', class-vft-matrix §2). Semantics of the 7 ints: *(unnamed; candidates —
skill requirements / difficulty / key id / open-close anim speed)* — consumers not chased this
session.

## 2. GDOR — GameDoor chunk (`F3::ReadGDOR_0x40e6e0`), versioned on the CHUNK header ver

```
v0 always (sub_40E5E0):
  byte → Door+0xBC   (bool: !=0 stored)        @0x40e616 (raw mov [esi+0BCh],cl)
  byte → Door+0xBD   (bool)                     @0x40e635
  string → Door+0xC4?/0xC8?                     @0x40e63c (`add esi,0C4h` then StreamReadString)
if v >= 1 (sub_40E660):
  u32 → Door+0xC0 (this+48 dwords)              @0x40e696
  u32 rnd; if (rnd) *(Door+0xC0) += rand() % (2*rnd) / -2      @0x40e6ca
       ; signed-divide trick = uniform ±[0..rnd-1] jitter on the lock/strength value
```
⚠ string-base ambiguity to settle before coding: the ctor initializes {byte@0xC8, size@0xD8=0,
cap@0xDC=15} ⇒ string base **0xC8** (matches ctor, full-class-geometry §4), while the reader's
`add esi,0C4h` says **0xC4**. One instruction chain is `base+4` off (decompiler/our read).
Recommend trusting the ctor (0xC8) and treating 0xC4 as the string-object `this` in the
StreamReadString ABI which the 2002 CRT advances by 4 internally — verify with the writer pair.
Door+0xC0 value with ±jitter + two bools + a key/name string = **locked-door skill difficulty
(+0xC0, randomized per-instance), is-locked (+0xBC?), is-jammed/trap (+0xBD?)**, string = key
item name *(candidates — consumers = GameActionOperateDoor, chase below)*.

## 3. GCON — GameContainer chunk (`cls_0x4d8d70::meth_0x408b80` @0x408b80 — ignore class label,
`this` = Container base), versioned

```
v0 (sub_408A70):
  byte → Container+0xBC (bool)    @0x408aa6
  byte → Container+0xBD (bool)    @0x408ac5
  string → dest computed as `this+4` in IDA's broken units — same ±4 caution as door; ctor
          shows string own-region fields (cap@+0xF8=15 etc., full-class-geometry §4)
if v >= 1 (sub_408AF0):
  u32 → Container+0xC0 (lock difficulty) + same ±rand jitter @0x408b5e
  u32 more... actually: after jittered u32, ONE string (StreamReadString this+49 dwords
        @0x408b1b) = key name; then done
```
(Container/door share the exact shape: locked byte, second byte, lock-strength u32 ± jitter,
key string — v1 adds the last three.)

## 4. Door geometry virtuals (Entity-vft +0x70/+0x74/+0xAC — Door-only slots)

Confirmed against A's spatial-grid thread (the `cls_0x57bc90` = map grid object from
"collision/geometry pairing" notes):

- **Door::virt_meth_0x4100a0 (slot +0x70) = ADD-DOOR-GEOMETRY-to-grid**:
  if own flag (Entity+0x84-ish, `v3[132]`) not set: builds transformed record (sub_40FF50), sets
  flag=1; `WorldServer[2]-intf->vft[+84](Entity+0x100)` = get MAP/world instance by the door's
  map-id field at **Entity+0x100**; then grid `meth_0x57eb40/setActive` + `meth_0x57d4f0` with
  geom id `*(owner+0x1DC)`; either model-geom add (`meth_0x580260`) or **9-float OBB/corners
  add** (`meth_0x57de50`, 3×(x,y,z) at owner+0x10..0x24 + base pos); on success
  `intf->vft[+104]` = **GetParty()** (→ GameParty @World+0xF4; corrected by session 35 —
  NOT a cell-occupant iterator) ⇒ iterates party members (begin@+8/end@+C) calling
  `JE::Entity::meth_0x549450(member, 1.0f)` — i.e. door geometry changes notify the WHOLE
  PARTY (re-path under the changing door)
  (notify/push overlapping entities). Owner = the entity whose geom is used: `self` when
  `self==240` sentinel else `Entity−236B` i.e. **the door's parent/attachment target**
  (`&v2[vbdisp−236]` = Door+0x?? — this is the door's `attachedTo`/blocker object pointer
  field; exact offset: Entity−0xEC = Door base → reads *(param_1a+476)=owner+0x1DC geom id,
  owner+0x04..+24 = bbox floats ✓ Entity-local coords).
- **Door::virt_meth_0x40fc20 (slot +0x74) = REMOVE/UPDATE-GEOMETRY** (open/close flip):
  if flag set, re-transforms param_1 (per-part loop `sub_40F070(param_1,i,…)`) clears flag,
  then re-adds via the same grid calls (no OBB branch) + occupant notify.
  ⇒ **door open = flip geometry between closed-AABB and open-AABB via slots +0x70/+0x74.**
- **Door::virt_meth_0x40ea20 (slot +0xAC)** = tiny accessor: returns `Door−0x10`
  (i.e. `&v1[-8]` dword = a back-pointer/self-tag at Door+0xF0-ish region — likely
  "get attached blocker/panel interface ptr" *(inferred)*) when flag at *(this−8B)!=0 else 0.

## 5. GameActionOperateDoor skeleton (meth_0x406730, 0x406730)

```
target = RTTI Entity→GameEntity cast of action->target (this+0x11 slot);
JE::Entity::meth_0x548f90(targetEntity)   ; = Entity+0x184 = WorldTimer->vft[0]() ; timestamp
                                          ; "last interaction" — NOT the open toggle
GameEntity::QueueAnimEvent_4673d0(actor, 121, 0.0)   ; actor plays "operate" anim
```
QueueAnimEvent case 121 body = `meth_0x4667a0` @0x4667a0: state-machine on the ACTOR's
`+0x2C animState` → engine queue `sub_5505F0(entity, 359|431|442|449, 0, 1, 0, 0.0, 0)`
(clip ids per state 4/5/6; stance 1..6 picks 442 vs 449; state 10 = no-op; default state →
QueueToDefault then set state 5 and recurse). So OperateDoor = *timestamp + "operate"
animation*; ~~the actual door state flip arrives later via the anim-event token dispatch~~
**SOLVED (session 17): not a token — the door's OWN activate-slot (+0xB8) override
`0x40ffe0` plays anim 514, backdates Entity+0x94 by clip duration, and flips collision via
vslots +0x70/+0x74 (octree Map+0x88) — see
`2025-10-20-entity-shape-lifecycle-door-flip-solved.md` §3.** ACTION(for A/B): ~~grep the
shipped action-update loop~~ retired — nothing to grep; wire per session 17.

## 6. C-7.1 ANSWER — the EEOV v≥2 item-creation chain, pinned

Objects: `sub_401130()` @0x401130 returns `&GameWorld[2] + *(GameWorld[2].vft+4)` + 8 — the
**world "service/registry" interface** (B's docs: factory intf 0x668494). Slots used by EEOV
@0x55238a/0x552399 (decimal names from gameworld-interfaces-effects-classid §9):

```
inst = intf->vft[+52](nameString)      ; lookup-by-name   (B's slot13)
inst = intf->vft[+24](inst)            ; create-from-template (B's slot6)
inst->vft[+32](nameString, 0, 0)       ; = meth_0x551880 — per-instance set-init-name
[Entity+0x168]->vft[+4](inst)          ; GameInventory::AddItem  ★ below
```
EEOV ver gates re-confirmed: v≥1 = 2 bools @Entity+0x178/+0x17A; v≥2 = the loop above
(count u32 @0x552320). `(a2+99)` at 0x46913b passes the *item's* per-instance id string
(Entity-relative +99*4 = +0x18C? caution: char* units) into the registry `sub_5A04B0`.

## 7. GameInventory own map (ctor 0x46b970, size 0x4C, vft 0x666754)

```
+0x00  JE::Inventory vb/base (ctor 0x55fff0; vft rewritten to 0x666754)
+0x14..0x2B  memset 0 (0x18) — small record (counts? gold/cap?) *(unnamed)*
+0x2C u32     += added-item weight-ish field (see AddItem else-branch) → TOTAL WEIGHT?
+0x30 i32 -1  owner/net-id sentinel?
+0x34 byte    local_11 garbage-write in ctor (uninit — shipped bug candidate)
+0x38 ptr     sub_57C2E0() RbTree-style node (self-linked) — contents map root #1
+0x3C u32 0
+0x44 ptr     sub_469530() — contents list/tree head #2
+0x48 u32 0
```
vft table (first 16): +0 0x46ba10, +4 **0x4690a0 AddItem**, +8 0x469550, +0xC 0x4692d0,
+0x10 0x469240, +0x14 0x4691c0, +0x18/0x1C engine, +0x20/+0x24 both 0x469370 (alias stubs),
+0x28 0x46a860, +0x2C 0x469670, +0x30 0x469400, +0x34 0x469410, +0x38 0x46be40, +0x3C 0x46b350.

### 7.1 AddItem (vft+4 @0x4690a0) shape

```
if (item = RTTI(Entity→GameItem)(arg)) && item->(field @+40) == 5
   && intf->vft[+104](?) = GetParty → party-member iter && ...vft[+12](slot, v9)==1:  ; (sess-35)
        ; SPECIAL/quest item path: global registry add (intf->vft[+172] → sub_5A04B0(x,
        ;   item id string @item+99·4))
        itemEntity->vft[+4](itemEntity+4·?, 1)      ; item "in-carried-state" virtual
else:
    JE::Inventory::virt_meth_0x560100(this, arg)    ; BASE add (real container insert)
    inv+0x2C += *(GameEntity-cast(item) + 7·?)      ; weight/price accumulation (⚠ units)
    if RTTI(Entity→GameWeapon)(arg): sub_414EF0(owner)  ; weapon-specific post-add
```
*(Two `+N` unit ambiguities flagged ⚠; raw-verify the instructions at 0x4690c0 (field cmp) and
0x46917e (+= source) before mirroring — the semantics (base-add + running total + weapon hook)
are solid.)*

## 8. What this unlocks / next threads

- **A**: EEOV v2 wiring can follow §6 exactly (lookup52→create24→setName32→addItem+4); §7 gives
  the container fields your inventory port needs. The door geometry slots (§4) are the same
  `cls_0x57bc90` grid API your spatial-grid thread is rebuilding — the +0x70/+0x74 pair is a
  ready-made consumer test once the grid lands.
- **B**: OperateDoor skeleton (§5) + GDOR/GCON lock fields (§2/§3) feed the UI/act layer;
  the door state-flip question → ANSWERED, session 17 (activate-slot override).
  claims it.
- Discovery queue after this: ~~(a) door state-flip token/consumer~~ done session 17,
  (b) GOBJ 7-int consumers,
  (c) +0x18 semantics (=GENT dword[4], still open per session6-classid §8), (d) GCRE v4
  sub_40B9E0 full decode, (e) Item 26-string block map (if A hasn't taken it).

## 9. Addendum (same session): Door::OnAnimEvent rebases the dispatcher

**⚠ RETRACTED — see `2025-10-20-door-rebase-correction-changestate-machine.md` §1.**
`GameDoor::virt_meth_0x40e9b0` (Door Entity-vft +0xA4) =
`sub_467E90(ecx − *(ecx−4) − 76, …)` — but the −76 is **structural vb arithmetic**
(Entity@Door+0xF0 → Door+0xA4 = the handle shared GE bodies expect), used uniformly by ALL
door stubs, NOT a dereference of GOBJ f1. GOBJ f1..f6 remain plain unnamed ints with no
consumer found; the host-object theory is dead. Door's OnAnimEvent = the base dispatcher.

## 10. Evidence Index

GOBJ 0x412540/0x412558/0x412569. GDOR 0x40e6e0→0x40e5e0 (bytes 0x40e616/0x40e635, string
0x40e63c)/0x40e660 (jitter 0x40e6ca). Door ctor string init 0x40e869/0x40e873/0x40e87d.
GCON 0x408b80→0x408a70/0x408af0 (jitter 0x408b5e, string 0x408b1b). Door geom +0x70 0x4100a0
(grid calls 0x56dbb5..0x56dc9f, notify 0x56dcd7), +0x74 0x40fc20, +0xAC 0x40ea20.
OperateDoor 0x406730; timestamp 0x548f90; QueueAnimEvent-121 body 0x4667a0. Registry helper
0x401130; EEOV chain 0x55238a/0x552399/0x5523b4/0x5523c1. GameInventory ctor 0x46b970, vft
0x666754 (dump recipe: .rdata raw off = VA−0x647000+0x247000), AddItem 0x4690a0.
