# 2025-10-20 — Anim-event scheduling pipeline: QueueAnimEvent switch, state×stance clip map, scheduler contract (session 8)

Target: ORIGINAL_REFERENCE. Read-only. Delivers the node-pool decode promised in
RESPONSES (§9.1 hand-off) and completes the reimpl doc
`2025-10-20-gameentity-vft-decode-activation-and-inventory.md` skeleton with the missing
middle: WHERE events come from, WHAT clips they map to, and the scheduler's contract.

## 1. `F3::GameEntity::QueueAnimEvent_4673d0(ge, int animEventId, float time)` — the master switch

Guard first: `if (ge->mbr_0x9C /*byte GE+0x9C*/) return 0;` — a busy/lock flag (set by
cases 43..52 below; cleared elsewhere). Unhandled ids log
`"GameEntity::QueueAnimEvent() - Unhandled ANIMEVENT.\n"`.

| animEventId | handler | notes |
|-------------|---------|-------|
| 1 | Queue_Idle_0x462c50(time) | idle pose (§2) |
| 2..6 | sub_0x463000/463040/463080/4630c0/463100(time) | **state/stance-conditional idle overrides** — case 2 verified: `if (animState==2 && stance==7) → clip 254 else → Queue_Idle(t)`; same shape for 3..6 (clips 255..~258 range candidates) |
| 7 | QueueToCombat_0x463140 | enter combat |
| 9 | QueueToDefault_0x465c20 | enter default |
| 11 | QueueToCrouch_0x465ee0 | crouch |
| 17/18 | sub_464b80(0) / sub_464c70(0) | paired start-stop family A (off) |
| 19,39 / 20,42 | sub_464b80(1) / sub_464c70(1) | same family (on) |
| 21 | Queue_Equip_0x465ae0 | equip |
| 22 | Queue_Unequip_0x4636b0 | unequip |
| 29 | Queue_Limp_0x4669f0 | hurt/limp gait |
| 32 | Queue_Walk_0x465fc0 | walk gait |
| 33,34,37,38,40,41 | sub_464d70(id) | id passthrough family |
| 43..52 | **GE+0x9C=1**; then sub_464940(id) | locking family (one-shot?) |
| 53..104 | sub_463c80(id) | 52-entry big family = attack/skill clip range *(inferred; candidates for per-weapon-form anims)* |
| 120/121/141 | sub_4659a0 / sub_4667a0 / sub_466880 | specials |

The shipped data point from session-1 (§2): activate path queues **event 1** with a *random*
time (0..1) — see reimpl doc §2.2; this file supplies what happens next.

## 2. Queue_Idle_0x462c50 — the (AnimationState × WeaponStance) → clip-index map

Switch on `GE+0x2C` (animState, ctor 0; confirmed by its own error string
`"Unknown Animation State set."`), inner switch `GE+0x30` (weaponStance 1..7;
`"Unknown Weapon Stance set."`). Every leaf is

```
sub_5505F0(Entity-vb, clipIdx, 0, 0, 0, time, 0)     ; PlayAnimAndSchedule (see §3)
```

| state | stance→clip |
|-------|-------------|
| 1 | (returns 0 — nothing queued) |
| 2 (idle) | 1→26, 2→74, 3→105, 4→136, 5→172, 6→204, 7→253 |
| 3 | 1..6→318, 7→343 |
| 4 | any 1..7 → 355 |
| 5 | 1→361, 2→371, 3→377, 4→383, 5→389, 6→395, 7→416 |
| 6 | 445 · 7 | 487 · 8 | 502 · 9 | 511 |

Clip indices are indexes into the entity's loaded **animation clip table** (model anim
object, §3 pool). This is the first complete, binary-evidenced anim selection table —
the reimpl's per-class `GameActionIdle` should reproduce (state,stance)→clip.

## 3. Scheduler contract — `sub_5505F0(Entity*, clipIdx, a2, a3, flags, float speedOrTime, a6)`

(IDA had it mislabeled `GameAmmo::virt_meth_0x408310` — raw base math shows it operates on
the Entity vb + `this+88*4 = Entity+0x160` model handle; correct owner: **JE::Entity**.)

```
calls modelAnim vft[+72](clipIdx, a2, &a6, &flags)          ; select+start clip
pool = [Entity+0x160] → +0xC ; poolObj = [pool+12]          ; the cls_0_467780 node container
dur = Gfx_AnimationObject::GetDuration_4a26d0(poolObj+12→, clipIdx...)   ; seconds (double)
if (a3 != 0):  retMs = a3 ? (int)(1000.0 * dur) : 2000      ; ← RETURN = clip duration in ms
scheduleOffset = (int)(dur * speedOrTime * 10.0)            ; time arg scales 0..1 → ms into clip
push record {clipIdx, a2, a3, flags, local_c..} into queue at Entity+0x1A8 (sub_54EB40)
return retMs
```

The **ms return** is exactly what `GameActionAttack`-style timing needs; `*10.0` factor says
the float param is a normalized "fire X fraction into the clip" value (0..1 ≈ 0..10 frames?).

## 4. Step driver + firing gate (completes reimpl §vft +0x48 row)

- `GameEntity vft+0x48 = sub_467830(stepParam, time, &outEventId, &outSubIdx)` (labeled
  GameAmmo in IDA; same base-math correction as §3): walks the event queue via
  `*outEventId = dword_6F3960[*outEventId]`, `*outSubIdx = 0` per hop —
  **`dword_6F3960` is a global parallel-array linked list** (next-index per event slot,
  single data xref — the walk above). Nodes: `cls_0_467780` at `poolObj+12`.
- `cls_0_467780::meth_0x467780(animIdx, frameIdx, 0)` = **firing gate**:
  1. `map_find(node+0x370, animIdx)` — if absent (== node+0x374 sentinel) → 0
  2. frames = (rec[6] − rec[5]) >> 2 — clip frame count; `frameIdx >= count` → 0
  3. `*(*(*(rec[5]+4*frameIdx)+4)+80)` = the frame's trigger-time field; gate passes only
     while `frameIdx < that value` → returns 1 = "event still live / fire now".
- On pass, the caller surfaces (animEventId, subIdx) upward; reimpl doc already decoded the
  receiver: `GameEntity vft+0xA4 OnAnimEvent(token-record{SSO "DM"/"RL"/"EP"/"UP"/"PU"/"FR"/"FL", int})`.

## 5. End-to-end (all sessions combined) — the shipped action-timing design

```
Action::OnServerUpdate ─► GameEntity::QueueAnimEvent(id, t∈[0,1])
                              switch → Queue_<name>(t) → PlayAnim(clip(state,stance), t)
                              PlayAnim returns durationMs → action stores timeout
each frame: vft+0x48 step walks dword_6F3960 queue, gate=cls_0_467780::meth per frame
            hit → OnAnimEvent(token, record) → DM(damage beat)/RL(reload beat)/EP/UP/PU/FR/FL
```

## 6. Corrections / naming notes for prior docs

- Session 1 §2: `GE+0x9C` byte is not just "init 0" — it is the **QueueAnimEvent busy-lock**
  (guard at 0x4673d0 entry; set by cases 43..52). Rename candidate: `mbr_animLock`.
- IDA labels `GameActionEquip::*` / `GameAmmo::virt_*` on 0x463000/0x463040/.../0x463c80 and
  0x467830/0x467780 are MISATTRIBUTIONS of the auto-analysis (they operate on GameEntity
  bases; evidence: every access path goes GE+vb `mbr_0x4` adjustment and Entity+0x160 model
  handle). Treat ownership as GameEntity/JE::Entity respectively.
- New open items:
  1. Clip-index meanings (which named clip is 26? per-model table in the .ANIM resource or
     GameAnimationCache — next place to look: `JE::Gfx_AnimationObject::GetDuration` owner
     class at pool+12).
  2. ~~Where GE+0x9C lock is cleared~~ **RESOLVED by full-.text scan**: the byte at GE+0x9C has
     exactly one reader (`mov al,[ecx+9Ch]` guard @0x4673d0), one setter family (cases 43..52
     @0x4674c9), one clear (ctor @0x4679cc). All other +0x9C byte accesses in the binary are
     std::string SSO internals in unrelated classes (verified: 0x4cbd50/0x4e63be/0x4cc139 are
     string frees, not GameEntity). ⇒ **the anim queue lock is STICKY per entity**: once a
     43..52-class event is queued, no further anim events can ever be queued for that entity
     (until dtor/replace). Strong smell: 43..52 = death/terminal states. *(inferred)*
  3. `sub_54EB40` push semantics (Entity+0x1A8 queue — per-entity pending-anim list;
     contrast vs global dword_6F3960 next-table: one is per-entity records, other global
     scheduler links).

## 7. HANDLER BODIES DECODED (session-31 addendum, same folder rule: cite by filename)

`sub_464940` (events 43..52 — DEATH) and `sub_463c80` (53..104 — ATTACK) are one pattern:
`PlayAnim = sub_5505F0(entity, clip, 0, 1, 0, 0.0, killFlag)` keyed by (animState GE+0x2C,
weaponStance GE+0x30, eventId):
- 43..52: state2 → clips 233..241 (per eventId 44..52; default clip 21), **then
  GE+0x2C := 10 (corpse state)**; state7 → 484; state8 → 499; state10 → no-op; other states
  → set 2 + recurse (so any entity dies into stance-2 corpse).
- 53..104: attack clips per stance — 53/54/55 → {7|229, 8|230, 9|231} (stance7 alternate),
  56 → {11,97,160}, 57 → {22,72,103,134,166,202,247} (full stance set!), 58 → {33,112,179,211},
  59 → {43,116,183,215}, 60 → {50,84,146}, 61/62 → 52..(stance set), ... up to 104 →
  285 — **~40 attack clip slots** = weapon-combo attack animations; state3 branch (dual?)
  → 305..340 set. Clip numbers double-check Query/Queue tables from sessions 8/23.
- The a3=1 param (previously "flags") now reads as **playOnce**; last param = corpse/die flag.
⇒ Attack animation selection = (attack mode id from GWAM record) → 53..104 → clip; death =
43..52 → clip + state→10. Reimpl: GameActionAttack step must map its modeRec to 53..104 ids
(the 17/18/19-42 aliases from §1 remain stance-generic attacks).
