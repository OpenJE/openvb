# 2025-10-20 — Session 10: CORRECTION (door rebases are structural), GameDoor stub map, Entity::ChangeState state machine

Target: ORIGINAL_REFERENCE. Read-only. Corrects one inference in
[gobj-gdor-gcon-door-geometry-inventory-additem](2025-10-20-gobj-gdor-gcon-door-geometry-inventory-additem.md)
§9 and answers `session7-effect-pipeline-and-damage-apply` §6 ("Entity+0x1E0 == 7 writer").

## 1. ⚠ RETRACTION: Door OnAnimEvent −76 rebase ≠ GOBJ f1 host-pointer

Session 9 §9 inferred GOBJ u32 #1 might be an attached-host `Entity*` because Door's
OnAnimEvent thunk passes `Entity − 76`. Raw scan of the door stub block (0x40e910..0x40ea13,
pattern `2b 49 fc | 83 e9 XX | e9 rel32`) shows **EVERY door stub to a shared GE body uses
`sub ecx,0x4Ch`** — and door's Entity vb sits at Door+0xF0, so:

```
Entity − 0x4C = Door+0xA4 = the handle the SHARED GE bodies expect
(shared body computes gameEntityBase = this − 164 → Door+0x00 = its GameEntity subobject ✓)
```

It is pure vb-layout address arithmetic (the door equivalent of the `this−164` idiom), **not**
a field dereference. GOBJ f1..f6 remain plain unnamed ints with **no consumer found**; the
session-9 §9 implication is retracted (edits applied to that doc + HANDOFF). The two
`sub ecx,0x30` stubs (0x40e8f0 →0x408d30 `return 1`, 0x40e9e0 →0x4128e0) rebases to the
Usable(+0xC0) handle — door's slot +0x1C ("can-see-target gate") is hard-overridden to TRUE.

## 2. GameDoor stub → shared-body map (door = GE behaviour + geometry only)

Decoded jmp targets (add 8 to the stub address before applying rel32):

| door stub | GE Entity-vft slot | shared body | meaning for door |
|-----------|--------------------|-------------|------------------|
| 0x40e910 | +0x3C | 0x467140 | read-notify chain |
| 0x40e920 | +0x44 | 0x467be0 | save/items chain |
| 0x40e930 | +0x48 | 0x467830 | anim-event step |
| 0x40e940 | +0x4C | 0x466920 | IsHumanoid → false path (never humanoid) |
| 0x40e950 | +0x50 | 0x462970 | alive/dead gate |
| 0x40e960 | +0x58 | 0x4629e0 | QueryDefaultAnimation |
| 0x40e970 | +0x80 | 0x4671a0 | read+items |
| 0x40e980 | +0x90 | 0x4629a0 | LineOfSight (door never casts? still shared) |
| 0x40e990 | +0x9C | 0x468650 | equip-model list |
| 0x40e9a0 | +0xA0 | 0x463970 | GENT writer |
| 0x40e9b0 | +0xA4 | 0x467e90 | OnAnimEvent dispatcher (see §1) |
| 0x40e9c0 | +0xA8 | 0x467cd0 | GetUsableSkills |
| 0x40e9d0 | +0xB0 | 0x4637f0 | GameInventory factory |
| 0x40e9f0 | +0xBC | 0x463850 | OnDeath hook |
| 0x40ea00 | +0xD0 | 0x466f40 | save-items |
| 0x40ea10 | +0xD8 | 0x404b70 | no-op |

Unique door code = ONLY: geometry +0x70 (0x4100a0 add-to-grid) / +0x74 (0x40fc20
remove-update) / +0xAC (0x40ea20 accessor returning `Door−0x10` when `*(Door+0xF4-ish flag)`
else 0), dtor 0x40e8e0 (frees the engine block at Entity+0x2E8), +0x1C→const-1, loader.
**Practical for A**: wiring the two geometry slots onto your SpatialGrid is ~the entire
door-specific behaviour at entity level; open/close state itself is NOT in these stubs.

## 3. `JE::Entity::ChangeState` @0x5507f0 — the guarded state machine at Entity+0x1E0

**Superseded/confirmed by `2025-10-20-session8-entity-lifecycle-state-die.md` §1–2** (same
lattice, plus the driver mapping: 0x551880 Create→1, 0x550970 OnCreate→4, 0x550a90 OnLoad→4,
0x550a50 OnDestroy→8, 0x551b60 Destroy→2, 0x551ca0 **Kill→6** = sole pre-death, and 7 set
via the death path). Read this §3 only for the raw transition table + caller scan; use the
lifecycle doc for semantics. My independent additions that survived: the raw-setter call-site
count (1: 0x46797e ctor) and the ⚠ fidelity rule **death from state 5 throws (6→7 only)**.

State slot = Entity+0x1E0 (decompiler shows `gap1AC[52]`; write site 0x550945; every read of
+0x1E0 goes through the getter pair used by `IsDead` in the effect doc). Shipped rules —
transition `cur → want`, **else throws** `JE::Common::Exception
("Invalid state change in Entity::ChangeState().")`:

```
cur 0 (none)   → want ∈ {1, 3}            (0→1|3)
cur 1 or 3     → want = 4                 (1→4 only)      @0x55083a
cur 2          → want = 8                 (2→8)           @0x550863
cur 4          → want ∈ {5, 6, 2}         @0x550896
cur 5          → want ∈ {4, 6, 2}         @0x5508c9
cur 6          → want ∈ {7, 2}            @0x5508f7
cur 7 (DEAD)   → want = 2                 @0x550920
after set: if (vft[+108](this)) meth_0x54ef30(this) else meth_0x54bf00(this)   @0x55094b..0x550960
```

⇒ state lattice: `0 → {1,3} → 4 ⇄ {5,6} ; 6 → 7 ; 7 → 2 → 8`. Dead=7 reachable ONLY from 6;
7→2 (revive/reset?); 2→8 (removal). The **GameEntity ctor bypasses the machine** with the raw
setter `JE::Entity::virt_meth_0x550750(entity, 5)` (single caller in all of .text:
0x46797e — raw scan) — so entities are *born in state 5*, and combat/death flow is
5→6→7 while 4 is an alternate active state (5⇄4, both→6/2).

Callers of ChangeState (raw E8 scan, 11 total): **corrected by
`2025-10-20-session12-serverupdate-ondeath-flip.md` §3** — 0x550998/0x550a78/0x550ab8 sit in
the lifecycle WRAPPERS (Activate→4, OnDestroy→8, OnLoad→4), NOT in a state-changing pump;
`0x550ad0` (vft idx52, state-8 REMOVED tick) contains NO ChangeState calls — my earlier
"the pump itself changes state" wording was wrong. Remaining sites 0x551b36/0x551b8e/
0x551c8b/0x551cb6 (engine add/remove world paths), 0x5a941a/0x5a9485/0x5a987b (**ActionAnim/
JE::Action layer** — incl. a new `push 5` combat-init case @0x5a987b). Plus the F3-side death
path: OnDeath wrapper 0x5509b0 → `ChangeState(7)` @0x550a38 = the ONLY 7-site in the binary.

Naming proposal: 1=CREATING 3=?, 4=ACTIVE?, 5=IDLE(spawn default), 6=ENGAGED?, 7=DEAD,
2=DYING-or-RESET (since 7→2→8 is the removal corridor), 8=REMOVED. *(inferred — pick from
consumers as they surface; the LATTICE itself is byte-level fact.)*

## 4. Small extras found in passing

- `JE::Entity::virt_meth_0x550ad0(this, dtMs)` = engine per-entity update (huge, 0x550ad0..
  ~0x551284): model anim progression (`int_0x160` meth_0x563f10(dt)), water-tile probe
  (octree query → `sub_5AB100` ripple), timed-script queues (`mbr_0x3c4`/`mbr_0x33c`/
  `gap340[20]` RbTree walks advancing `+28/+7` float timers by dt*0.001), effect list
  ticking. Reference for B's "pump" work — engine-side, no F3 semantics.
- Door dtor frees `Entity+0x2E8` block — allocation site unchased (candidate: per-door
  occlusion/portal record made by the engine when the door geometry is added).

## 5. Next (unchanged queue, reshuffled)
(a) GCRE v4 `sub_40B9E0` full decode; (b) GameEntity+0x18 consumers (still nobody found one —
it's written by GENT dword[4], read by NOTHING in F3 per our scans so far → dead data in the
demo? if raw scan of `[reg+18h]`-after-GE-base yields nothing, declare it write-only);
(c) state-slot consumer pass: who reads +0x1E0 besides IsDead (29 sites listed in §3 scan —
0x4e4cdc GUI? 0x40xxxx action layer: 0x404142/0x40533d/0x406711/0x40721d/0x40e3f5 creature/
0x412b74 usable) — mapping those 29 readers to the state names would settle §3's naming;
(d) GOBJ consumer pass, if A's door tests need the 7 ints.

## 6. Evidence index
Door stub block bytes 0x40e8e0..0x40ea1f (rel32 decode recipe in §2); ChangeState body
0x5507f0 (rules @0x55080c..0x55093c, write @0x550945, hooks @0x55094b/0x550954/0x550960);
caller scan method: linear E8-rel32 resolve over .text (list in §3); raw-setter single caller
0x46797e; entity-update pump 0x550ad0 (water probe @0x550d51, anim advance @0x5511d8).
