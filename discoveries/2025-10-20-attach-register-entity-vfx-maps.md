# 2025-10-20 — Session 25: attach-register body decoded — entity VFX attach maps + UpdateActive synthesis

Target: ORIGINAL_REFERENCE (IDA). Read-only. Last functional item from the map-thread/EEOV
chain (session 14 §5, session 22 §2 "register body pending"). One ⚠ dual-reading flagged in §4.

## 1. `JE::Entity::meth_0x54ef90(ent, vegInstance, p2, p3, attachId, p5)` = attach-register
(0x54ef90..0x54f135)

Entity-side attach bookkeeping, three linked containers:
```
Entity+0x338/0x33C  multiset keyed by VEG-instance ptr   (entries 0x18 bytes)
Entity+0x344        map keyed by attach-id → VisualEffect*
Entity+0x350/0x354  secondary/dormant instance collection (copied FROM on first register)
entry layout:  +0  attachId | +4 VisualEffect* | +8 p5 | +12 0 | +16 VEGInstance*
               +20 p3 | +21 p5 byte
flow: look instance in 0x338; miss → check 0x350, copy entry across (meth_0x54ea80 = insert,
      0x54cad0 = erase from secondary); still missing →
        VEG->meth_0x48dbe0()                       ; instance "adopted" hook
        node = insert(0x338 map); new VisualEffect(0x1C8 bytes) via
             JE::VisualEffect::ctor_0x59fcb0(ownerCtx = Entity+0x1DC, veg, p3?p2:0, 0)
        node fields set (§1 layout); if attachId != 0 → 0x344[attachId] = effect
      hit  → just (re)stamp attachId + p5 bytes (attach-id reassignment path) and return
             existing effect
returns the JE::VisualEffect* in all paths (0 if instance null).
```
⇒ **per-entity attached VFX are `JE::VisualEffect` managers (0x1C8 B) over `VFX_VEGInstance`
assets**, addressable by an optional integer **attach-id** — which is exactly what the EEOV
v0 loop mints (`sub_552A20` global counter, session 22 §2) and what footstep-style runtime
attaches pass. EEOV name string → VEG lookup → `meth_0x5512b0` → THIS register. Chain closed.

## 2. Synthesis correction: the "timed object lists" in `Entity::UpdateActive` ARE these

Session 15 §4 described 0x54c570 sections 4–6 as ticking "timed-object lists
Entity+0x33C / 0x3C4 / gap340[20]". With §1, the **0x338/0x33C container is the attach
multiset** ⇒ UpdateActive's per-entry `sub_55A1C0(entry, dt, budget)` loop = per-attach
VisualEffect time advance (with budget consumption against Entity+0x17C-ish float), and the
`vft+20/+22` calls when it drains = "all attached VFX finished" hooks. So: EEOV-attached
effects tick every frame from birth, and the attach-id map (0x344) is the stop/remove API
surface. One coherent story for map-attached FX + runtime (footstep) attaches + scripted FX.

## 3. Field-additions to the entity map (engine band)
| off | meaning (this session) |
|-----|------------------------|
| Entity+0x338/0x33C | attach multiset head (by VEG ptr) |
| Entity+0x344 | attach-id → VisualEffect map |
| Entity+0x350/0x354 | dormant/secondary instance collection |
| Entity+0x1DC | owner-context passed to VisualEffect::ctor — ⚠ see §4 |

## 4. RESOLVED (same-day, post-publication sweep): `Entity+0x1DC` = numeric reserved ID — resolution (c)

Evidence gathered after the doc's first version flagged the ambiguity:
- Writer @0x55190c (inside `JE::Entity::Create` 0x551880): `call [worldServer_vft+0x24]` →
  `mov [esi+0x1DC], eax` — an int result stored; lifecycle doc's "vft[+36] reserved id" holds.
- VisualEffect ctor stores param_1 at ITS OWN `DynamicObject+0x15C` (0x59fcfe) and consumers
  treat it as a KEY, never a pointer: 0x59fe74 `push [me+0x15C]; call 0x54be10(...)` (the
  same id→cache lookup used by ActionMoveTo's path fetch); 0x59fa89 only null-checks it
  (0 = unowned sentinel).
⇒ VisualEffect's owner-context = the owner entity's RESERVED NET ID routed by id (matching the
VFX-over-net story). Port accordingly: pass the same id the Create chain reserves.
⚠ Slot-collision lesson (now third sighting): DynamicObject+0x15C holds a classId tag on
JE::Entity, a water-tile pointer in the engine water path (session 4's dual-use warning), and
an owner-id on VisualEffect. Treat `+0x15C` as per-class-overlaid, never as one field.

## 5. Open now
- VisualEffect::ctor param-1 usage (§4) — CLOSED same-day, see resolution above.
- The 0x350/0x354 "dormant collection" writers (who pre-populates it — likely load-time batch).
- Entity+0x238 string identity (session 24 §2) — EEOV v0-era reads.
- EMAP header 4-strings consumers.
That exhausts my claimed chains; remaining corridor work is A/B-side (grid coverage 0x40a4b0,
type-2 action fields, chance ±4 verify) or NEW threads (interfaces/GameClient UI, dialogue).

## 6. Evidence
0x54ef90 body EAs: containers 0x54efd8/0x54f004..; adopt hook 0x54f09f; new+ctor
0x54f0b7..0x54f0df (0x1C8 size, ctor 0x59fcb0 args incl. `v7->mbr_0x1dc`); node fields
0x54f0fa..0x54f11b; id-map insert 0x54f11e..0x54f133 / 0x54f07e..0x54f093. UpdateActive
loops per session-15 doc §4 (0x54c77c..0x54ca2b).
