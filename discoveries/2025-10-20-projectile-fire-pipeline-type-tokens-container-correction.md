# 2025-10-20 — Session 19: ledger supplements — projectile fire pipeline, +0x30 type-token virtual, CORRECTION (container activate)

Target: ORIGINAL_REFERENCE (IDA + raw). Small pass closing the session-18 §6 list, with one
correction to the ledger.

## 1. ⚠ CORRECTION: GameContainer +0xB8 is a GE ACTIVATE RE-THUNK, not class-own

`0x408a60` = `sub_467640(ecx − *(ecx−4) − 92, …)` — pure rebase to the GameEntity activate
body (92 = 0x5C = container's §1 rebase constant). Session 18 §3 row "Container activate own"
→ **wrong; container has NO activate specialization** (its +0x1c/+0x34 etc. re-thunks stand).
Ledger class-own list for Container is therefore: dtor, +0x28 CopyFrom (0x409050), +0x34
(0x409160), quad +0xb4 (via usable re-thunk chain), loaders only. (Apologies — flagged and
fixed here; session-18 file amended too.)

## 2. ⭐ Projectile activate (+0xB8 → 0x411dd0) = THE FIRE PIPELINE — spawn-to-action bridge

```
1. GE activate on rebased handle (0x467640)                        ; base spawn
2. if ([Projectile+0x24])                                          ; "firing" byte (own region;
                                                                   ;  ctor zeroes — session 4)
3.    action = server-intf vft[+20](2)                             ; CREATE ACTION type-2 =
                                                                   ;  projectile-flight action
4.    if (Entity+0x98 ≠ 0):                                        ; PATH-ID field (!= ctor
       path = 0x54be10(Entity+0x98)      ; default — engine fills at fire time)     ;  mbr_0x98=2)
       action->0x55b350(path)                                      ; SET PATH on the action
       (ActionMoveTo-family ctor path — 0x55b350 is in the
        ActionMoveTo vft neighborhood 0x55b3xx ⇒ type-2 action IS a MoveTo variant)
5.    else: action+0x24/28/2C = [Projectile+0x1C/20/24] floats     ; direct target position
6.    encounter-ring push 0x550720(entity, action)                 ; same mechanism as
                                                                   ;  activate→state-4 path
```
⇒ **A projectile fires by converting itself into a flight ACTION via the server factory** —
type-2 ≈ ActionMoveTo with path-or-direct-target; ties the projectile entity into the same
action/pump machinery as movers (A: this is the "attack launches projectile" handoff — decode
0x55b350 + action type-2 fields when wiring combat; `Entity+0x98` here is the *engine* slot
holding a path id at fire time, distinct from GameEntity+0x98=2 ctor default (unit:
[Entity+0x98] vs [GE+0x98] — the −0xb8 base makes this Entity-relative ✓ so it IS the
engine band, verify)).

Projectile band siblings: +0x78 (0x411d90) = rebase→engine 0x54beb0(arg)→**0x551b60 =
JE::Entity::Destroy** — despawn virtual; +0x7c (0x412130) = same pair + extra helper
0x412090 (projectile cleanup) before Destroy; +0xc8 (0x412180) = base
**UpdateActive 0x54c570** + Entity+0x98 path-tracking tail ⇒ in-flight update override.
⇒ Projectile lifecycle: activate(fire) → +0xc8 (per-frame tracked flight) → +0x78/7c (destroy).

## 3. Slot +0x30 = "type token" constant virtual (family pattern)

| class | +0x30 body | returns |
|-------|-----------|---------|
| GameEntity (GE default) | 0x404bb0 | 0 |
| GameUsableObject | 0x412530 | **3** |
| GameDoor / GameContainer | 0x408980 | **9** |
| GameItem | 0x4100b0 | (own — un-decoded, likely const) |
| Creature | 0x4092a0 | 2-or-param (conditional on [Entity+0x1E4] — combat-mode?) |

GE-slot +0x30 in the engine table is the `retn 4` getter band; F3 repurposed slot index 12 as
a per-class type constant (values 0/3/9 ≠ factory ids — engine-side enum: probably
`JE::Entity::GetType` space). Consumers: grep `call [reg+30h]` at next opportunity — B's
interface code likely uses it (inventory slot filters). *(Not GOBJ consumers — verified.)*

## 4. Usable +0x34 (0x412810) = capability/skill-list builder (creature §3-pattern)

`cls_0x568c20` list + repeated `sub_4090D0/sub_408F40` inserts with type-tags 1, 3, … —
identical shape to GameCreature +0x34 (0x40d6b0: pushes tags 0/1/2/3/5 + state-gated 5) ⇒
**slot +0x34 = "GetCapabilities/Usables" list provider** feeding the GetUsableSkills/
interaction menus (vft +0xa8 band). Door +0x34 (0x40fc30) = larger SEH body (unequip/lock
chain?), left raw-pending — one function, claim on request.

## 5. GOBJ +0x18 twin-verdict forming
After this pass, GOBJ's 7 ints have: loaders ✓, savers ✓, CopyFrom ✓, but STILL no gameplay
reader (same profile as GameEntity+0x18). If door/usable +0x30 constants + this pass were the
last candidates, the honest conclusion after one more check (item +0x30 body + `call
[reg+30h]` consumer grep) is: **GOBJ ints and GE+0x18 = reserved/write-only in the shipped
build**. Marking for a final confirmation next pass.

## 6. Evidence index
Projectile: activate 0x411dd0 (GE call 0x411de3, flag test 0x411de8, factory vft[+20](2)
0x411e0c, path arm 0x411e1a..0x411e49, direct-target floats 0x411e50..0x411e66, ring push
0x411e7a, tail store 0x411e80); band 0x411d90/0x412130/0x412180 (bodies §2). Container
correction: 0x408a60 (single rebased tail-call). Type-token bodies §3 EAs. Usable list
builder 0x412810 (inserts 0x412866/0x412895…).
