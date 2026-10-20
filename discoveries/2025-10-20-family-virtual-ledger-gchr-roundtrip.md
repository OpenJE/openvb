# 2025-10-20 — Session 18: COMPLETE FAMILY VIRTUAL LEDGER — every class's slot map, class-own vs GE re-thunk, GCHR save round-trip

Target: ORIGINAL_REFERENCE. Read-only. This closes the class-vft-matrix doc's
"per-class +0xE4 fourcc matrix" era into a full 58-slot ledger: for each of the 11 classes,
which slots are REBROADCASTS of a GameEntity body (pure vb-rebase thunks, zero behavior
change) vs genuinely class-specific. Raw recipe: stub bytes
`2b 49 fc | (81 e9 disp32 | 83 e9 imm8 | —) | e9 rel32`.

## 1. The rebase constant per class = vbdisp − 0xA4 (verified exactly)

| class | stub `sub ecx,N` | = Entity@class − 0xA4 | GE-handle |
|---|---|---|---|
| UsableObject | 0x1C | 0xC0−0xA4 ✓ | class+0xA4−0x1C… |
| Projectile | 0x18 | 0xBC−0xA4 ✓ | |
| Door | 0x4C | 0xF0−0xA4 ✓ | (session 15/17 corollary re-proved) |
| Container | 0x5C | 0x100−0xA4 ✓ | |
| Item | 0x310 | 0x3B4−0xA4 ✓ | |
| Armor | 0x32C | 0x3D0−0xA4 ✓ | |
| Ammo | 0x334 | 0x3D8−0xA4 ✓ | |
| Weapon | 0x38C | 0x430−0xA4 ✓ | |
| Creature | 0x760 | 0x804−0xA4 ✓ | |
| Player | 0x780 | 0x824−0xA4 ✓ | |

⚠ Two conventions in the wild: class→GE thunks subtract N from the ENTITY ptr (this table);
the *item-family* sub-rebases use Item-handle convention (Weapon 0x7C = 0x430−0x3B4, Armor
0x2C, Ammo 0x24) when re-thunking **Item** bodies (slots +0x30/+0x34 → Item bodies
0x4100b0/0x4110e0 — see §3). Both check out arithmetically; don't mix them.

## 2. Slots where a class merely re-thunks a GameEntity body ⇒ INHERITED behavior, no delta

Creature/Player re-thunk 15/17 slots (alive-gate, LOS*, QueryDefaultAnimation,
Inv-factory, OnDeath-hook, canSeeGate, IsHumanoid, save+items ×2, read+notify, anim-step,
items-pack, GetUsableSkills, OnAnimEvent, equip-models, +0xd8→0x4097a0 post-create, +0xb0…).
(*LOS +0x90 is re-thunked by Item-family only; creature/player leave it at the GE value —
same body, table-level.) ⇒ **For the reimpl: all of these need ZERO per-class specialization —
one GameEntity implementation + handle rebase covers the whole family.** That's the flattening
license for `SpawnedEntity`, extended from Mover-fields to the whole virtual surface.

## 3. Genuinely class-specific slots (the actual per-class behavior surface)

| class | slot → body | what it is |
|-------|------------|------------|
| Creature | +0x00 → 0x40e4f0 | dtor |
| Creature | +0x28 → 0x40acc0 | CopyFrom (v1 arrays) — s13 |
| Creature | +0x30 → 0x4092a0 | small stub (with +0x34 0x40d6b0 skill-list) |
| Creature | +0x60 → 0x409be0 | regen-tick — s15 |
| Creature | +0x90 → 0x4093b0 | tiny stub |
| Creature | +0xa0 → 0x40bb80 | GCRE save (ver 5) — s13 |
| Creature | +0xb4 → 0x40d840 | collision quad — s16 |
| Creature | +0xb8 → 0x4093c0 | **ACTIVATE override**: `if ([Creature+0x180]) JE::Server::meth_0x555890([Creature+0x1B0], that)` then GE activate (sub_467640) — server registration of a spawned creature (@0x4093e9; ⚠ dword-unit derivation, verify once) |
| Creature | +0xd8 → 0x4097a0 | post-create (also called explicitly by ctor — engine-side) |
| Creature | +0xdc → 0x40a430 | small (paired with 0x40a4b0 quad-coverage family) |
| Creature | +0xe0 → 0x40ae60 | body-part registration — s14 |
| Creature | +0xe4 → 0x40e590 | GCRE load |
| Player | +0x00 → 0x411b60 | dtor |
| Player | +0x28 → 0x411c00 | CopyFrom (chains creature's) |
| Player | +0xa0 → **0x411690** | **SAVE: GCRE-save then GCHR chunk** — §4 |
| Player | +0xa8 → 0x411ba0 | player GetUsableSkills variant |
| Player | +0xe4 → 0x411a90 | GCHR→GCRE load chain |
| Usable | +0x1c → 0x408d30 | const-1 (shared w/ door/container re-thunk) |
| Usable | +0x28 → 0x4125a0 | CopyFrom (chains GE) — s16 |
| Usable | +0x30 → 0x412530 | own body (item/weapon/armor re-thunk pattern NOT used; own) |
| Usable | +0x34 → 0x412810 | own |
| Usable | +0xb4 → 0x4128e0 | collision quad (usable variant) — s16 |
| Usable | +0xe4 → 0x412760 | GOBJ load |
| Door | +0x00 → 0x40ec50 | dtor (frees Entity+0x2E8 block) |
| Door | +0x28 → 0x40f000 | CopyFrom (chains Usable→GE) |
| Door | +0x30 → 0x408980 | shared w/ Container (+0x30 both) |
| Door | +0x34 → 0x40fc30 | own |
| Door | +0x70 → 0x4100a0 / +0x74 → 0x40fc20 | geometry add/remove — s9/s17 |
| Door | +0xac → 0x40ea30 | host accessor |
| Door | +0xb8 → 0x40fd60 | **activate = door flip** (anim 514 + collision swap) — s17 |
| Door | +0xe4 → 0x40e720 | GDOR load |
| Container | +0x00 0x408f00 dtor; +0x28 0x409050 CopyFrom; +0x34 0x409160 own; +0xe4 0x408bc0 GCON load — **+0xb8 NOT own: corrected in session-19 §1, it is a plain GE-activate re-thunk** | |
| Projectile | +0x00 0x412050 dtor; +0x28 0x411e90 CopyFrom; +0x78 0x411d90, +0x7c 0x412130, +0xc8 0x412180 **projectile-flight band** (only slots +0x78/7c/c8 in family); +0xb8 0x411dd0 activate own (lifetime); +0xe4 = GE-rebase ONLY loader (no fourcc) ✓ s6 |
| Item | +0x00 0x410b40 dtor; +0x28 0x411330 CopyFrom(26 strings); +0x30 0x4100b0 ★, +0x34 0x4110e0 ★ (★ = SHARED by Weapon/Armor/Ammo via item-rebase — "item-class virtuals"); +0xb8 0x467640 GE activate re-thunk; +0xe4 0x411260 GITM |
| Weapon | +0x00 0x4154f0; +0x28 0x415670 CopyFrom; +0xe4 0x415530 GIWP; all else GE/Item re-thunks |
| Armor | +0x00 0x4088d0; +0x28 0x4086e0 CopyFrom; +0xe4 0x408910 GIAR |
| Ammo | +0x00 0x4084d0; +0x28 0x408200 CopyFrom; +0xe4 0x408510 GIAM |

## 4. GCHR save round-trip (answers "who writes the player string")

`GamePlayer::+0xA0 body 0x411690`: `GameCreature::SaveToFile(GCRE)` first, then
`{'GCHR'(1380467527), ver 0, size-patched}` + **1 u32 filler** + `sub_49C730(creature?,
stream)` = StreamWriteString of the GamePlayer+0x808 string. Reader = `sub_411670`
(ver-independent, one StreamReadString → +0x808) — **round trip closed: GCHR = ver 0,
u32(0) + one string**. (The `v3 = this−back−32` handle rebase = Player→Creature at −0x20 —
consistent with §1's conventions.)

## 5. Consequences for the reimpl (consolidated)
1. §2 = flattening license (one GameEntity vtable impl + class handle).
2. Per-class surfaces = §3 lists ONLY — e.g. a faithful `GameDoor` = flip-activate + geometry
   pair + loaders; everything else is GE.
3. Projectile owns the only non-GE flight band (+0x78/+0x7c/+0xc8) — decode when projectiles
   are scheduled.
4. ~~Container +0xB8 activate-own (0x408a60)~~ CORRECTED by session 19 §1: container has no
   activate specializaion (plain GE re-thunk). Projectile fire pipeline = session 19 §2.
5. Creature activate registers with `JE::Server::meth_0x555890` before GE activate —
   relevant to A's spawn pipeline parity (server registration at activate, not create).

## 6. Next (small)
- Bodies: 0x412530/0x412810 (usable +0x30/34), 0x40fc30 (door +0x34), 0x408a60 (container
  activate), 0x4092a0/0x4093b0/0x40a430 stubs, projectile band — ~10 decompiles, one pass.
- GOBJ 7-int consumers (unclaimed for two sessions; now narrowed: usable +0x30/+0x34 bodies
  are prime suspects for reading GOBJ fields).

## 7. Evidence index
Decode recipe §intro; per-class adj table §1 (stub EAs in ledger rows). GCHR save 0x411690
(write 0x4116bb tag/0x411759 string call/size patch 0x411763..0x411787). Creature activate
0x4093c0 (server call 0x4093e9, GE tail 0x4093ff). Item-family shared bodies 0x4100b0/
0x4110e0 (Weapon stubs 0x4146a0/0x4146b0 adj 0x7C; Ammo 0x4082d0/0x4082e0 adj 0x24; Armor
0x408780/0x408790 via 0x1C path). Full decode output preserved in this doc's tables.
