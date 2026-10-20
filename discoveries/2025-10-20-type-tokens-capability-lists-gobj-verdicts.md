# 2025-10-20 — Session 20 (capstone): type-token table FINAL, GOBJ/GE+0x18 VERDICTS, capability-virtual confirmed

Target: ORIGINAL_REFERENCE. Read-only. Closes the last standing items from sessions 18/19.

## 1. Slot +0x30 "type token" — complete table

| class | body | returns |
|-------|------|---------|
| GameEntity, Projectile | GE default 0x404bb0 | 0 |
| GameCreature | 0x4092a0 | 2 or `[Entity+0x1E4]`-gated value (combat-mode conditional) |
| GamePlayer | creature re-thunk | same |
| GameUsableObject | 0x412530 | 3 |
| GameDoor/GameContainer | 0x408980 | 9 |
| GameItem / Weapon / Armor / Ammo | 0x4100b0 (item body, re-thunked by all three subclasses) | **13** |

Values {0,2/…?,3,9,13} form an engine-side kind enum (not factory ids). Consumer hunt:
pattern-scan for `call [reg+30h]` + adjacent `cmp eax,{0..9}` = **0 hits** in .text ⇒ callers,
if any, consume via switches/pass-through (script/serialization band), not inline compares.
Treat as "reported, not branch-on" in the reimpl. *(No consumer found — but the constant
bodies are certainly part of the scripting ABI (scripting-abi-native-tables doc) — A, check
your native-table walk for a "GetEntityType"-shaped entry.)*

## 2. Slot +0x34 = capability-list provider (CONFIRMED family-wide)

Same body shape (`cls_0x568c20` list + `sub_4090D0/0x408F40` tagged inserts) across:
creature 0x40d6b0 (tags 0,1,2,3,5 + mode-conditional), usable 0x412810 (tags 1,3,…),
item 0x4110e0 (tag 1, …), door 0x40fc30 (larger, SEH-wrapped — the only +0x34 body with
extra machinery; un-decoded, likely lock/key capability entries). ⇒ the shipped
interaction/skill menus (GetUsableSkills band, vft +0xa8) are fed by this virtual; the
creature tag list is where "can-use-weapon/can-crouch/…" style bits live. Exact tag
meanings = one decode of the CONSUMER (0x467cd0 band, session-5 doc) away if wanted.

## 3. GOBJ (7 ints @Usable+0xA0..+0xB8) — VERDICT: clone-copied, never interpreted

Usable CopyFrom `0x4125a0` RTTI-casts source→GameUsableObject and copies **exactly the 7
GOBJ dwords** (`*(v3+40..46)` dwords = +0xA0..+0xB8 ✓) then chains GE CopyFrom. Across
sessions 9/16/18/19 every decoded reader/writer has been inventoried:
* loaders: GOBJ parses them; savers: mirror (0x4128dd/0x412a50 family); CopyFrom: copies ✓
* gameplay readers: **none found** — not consumed by any of the 58 GE bodies (all decoded),
  the class-own body list (sessions 18/19), the usable+0x30/34 bodies (constants/list — they
  do NOT read GOBJ), door/geometry/flip machinery, or the cmp-scan.
⇒ **GOBJ ints are persisted + cloned but uninterpreted in the shipped build** — same class of
field as `GENT f1..f4`. Reimpl: store+copy+round-trip, never branch on them. (Reopen if any
EEOV/attachment or scripting ABI consumer surfaces — the +0x30 note in §1 is the one place
consumers could hide behind dynamic dispatch.)

## 4. GameEntity+0x18 — VERDICT holds: write-only (ctor default + GENT round-trip only)
Confirmed by: 58-slot body census (no read), class-own ledger (none), cmp-scan, and the
session-13 save-band ambiguity resolved in-session. Same reimpl guidance as §3.

## 5. Campaign status — the GameEntity family thread is CLOSED
From session 1 (factory/sizes) → this: every class's full 58-slot virtual surface accounted
for (inherited re-thunk | class-own body | open-remaining below), chunk formats (GENT/EFTR/
EEN2/EEOV/EENT/GCRE/GCHR/GOBJ/GDOR/GCON/GITM/GIWP/GIAR/GIAM/GWAM) read AND written, field
maps for entity/creature/item/usable/door/container, the state machine + pump, collision
grid lifecycle, door flip, fire pipeline, regen tick, body parts, encounter ring,
save/clone/copy machinery.

**Still open (curated, all small unless noted):**
1. door +0x34 body 0x40fc30 (SEH-heavy, ~0x193B) — lock/key capability entries? (small, needs patience)
2. `0x40a4b0` quad coverage + shape queries vft+16/+24 (A's grid parity — bigger, engine lane)
3. type-2 flight action fields + 0x55b350 (A's combat lane)
4. `sub_409700` chance-array ±4 base verify + chance consumers (the session-13 ⚠)
5. +0x1E4 reader naming (feeds §1 creature token + regen gates)
6. EEOV attachment trio (meth_0x5512b0/sub_552A20/meth_0x551880 semantics beyond Create)
7. GCRE v4 tree-head absolute base confirm ±4 (session-13 ⚠; lockstep)
8. 12-bool array (+0x6E4) consumers (traits? flags — probably script-side, see scripting doc)

## 6. Evidence index
Usable CopyFrom 0x4125a0 (7-dword copy 0x4125c9..0x4125ff, chain 0x41260b). Item +0x30 body
0x4100b0 (ret 13). Item +0x34 0x4110e0 (list pattern 0x411108..). cmp-scan recipe §1
(FF /2 mod=01 disp8=30 window-32B 3D/83 F8). Type-token bodies per §1 table (session-18
ledger has stub EAs).
