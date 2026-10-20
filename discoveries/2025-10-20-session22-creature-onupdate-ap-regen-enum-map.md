# Session 22 — GameCreature::OnUpdate located (Entity-vft +0x60 override) = AP-regen tick; derived-attribute enum map; HP-regen verdict

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Answers the gap I flagged for the reimpl in Session 21 ("regen hunt hasn't found its writer")
— found the actual periodic-recovery machinery and what it regenerates.

## 1. `GameCreature::OnUpdate_0x409BE0` (vft +0x60 override — the session-14 pump step)

Session 14 §2 noted the per-entity pump calls `e->vft[+0x60](dt)` (base write Entity+0x300 =
dt, +0x319 latch) before `ServerUpdate`. For creatures that slot resolves to this override:

```
OnUpdate(dt):                                   ; sub_409BE0 = GameCreature vft[+0x60]
  base Entity::Update(dt)                        ; (virt_0x549090 call @0x409bfa)
  if (guardFlagSlot != 0): stampSlot = dt; return            ; ★ exact slots unit-murky — §4
  else if (dt − stampSlot >= GetDerivedAttribute(HIT_POINTS=14)):
      acc = counterSlot
      if acc < GetDerivedAttribute(ACTION_POINTS=0):          ; cap = max AP
          counterSlot = acc + (dt − stampSlot) / hitpointsInterval
          clamp to max AP
      stampSlot = dt
```
⇒ periodic **ACTION-POINT trickle regen**: interval between points = `HIT_POINTS` derived value
(= 6000/(20+AGI) ms per the formula in session 6 §6), accumulator capped at
`ACTION_POINTS` (= 20+AGI). This is the turn-economy clock, NOT HP. The guard flag = "don't
regen while X" *(inferred: in-combat / busy state)*.

## 2. DerivedAttribute enum values (from aligned caller scans)

pushed imm before `call 0x409a60` (13 sites; all F3-side):
| enum | attr | consumer evidence |
|---|---|---|
| 0 | ACTION_POINTS | HP formula (self @0x409b10), AP cap (@0x409c4d), 0x418862/0x4277e4 (UI-ish), |
| 1 | CARRY_WEIGHT | @0x43b882 (inventory over-encumbrance) |
| 4 | EVASION | to-hit trio @0x409ece/0x413094/0x4132c5 + @0x416d31 |
| 6 | MELEE_DAMAGE | CalculateUnarmedDamage @0x40a005 |
| 13 | THROWING_RANGE | ranged to-hit @0x4131b2 |
| 14 | HIT_POINTS | regen interval @0x409c2a (= 6000/AP, self-referential ms) |
Dynamic-enum caller: @0x45ad85 = AMX native (GetDerivedStat) — scripts can request ANY attr.
**HEALING_RATE (endurance), CRITICAL_CHANCE, PERCEPTION_RANGE, PERK_RATE, SKILL_RATE,
COMBAT_SEQUENCE have NO static C++ consumer** ⇒ their use (if any at runtime) is via the
native/script layer — consistent with combat being scripted beyond the C++ core.

## 3. Verdict on HP recovery

No static C++ HP-regen exists: the only periodic regen machinery is AP (§1), and every HP write
funnels through the one-setter trio already canonical (session 20-H/21 §4: Apply / client
recv-packet-88 / AMX chokepoint 0x46ee10). ⇒ HP recovery in the shipped game = **scripts calling
the SetHP/AddHP natives** (they reach the same chokepoint). Reimpl: no regen tick needed for HP;
just expose the SetHP chokepoint to the script layer, and implement the AP trickle above for
turn economy.

## 4. Slots needing a debugger pin (unit murk in 0x409BE0)

The three creature fields — guard flag, stamp (ms), AP counter — appear in the decompiler as
`(this + vbdisp − N*4)` triplets with inconsistent bases (−1118/−1676/−503 dwords). Given the
Entity-at-Creature+0x804 geometry, they land inside GameCreature's own block
(0x49C..0x804) — most plausibly near the GE+0x90 "LOS" cluster from the parallel docs. One
watchpoint on `[creature+0x49C..0x520]` during idle gameplay pins all three; NOT blocking for
the reimpl (any creature-side fields will do if they keep semantics: guard/stamp/acc).

## 5. Cross-links

Session 6 §6 (formula table — HIT_POINTS=6000/AP there re-derived as the regen interval here),
session 14 (pump step order), session 20-H/21 (HP setter canon). For their
`copyfrom-caller-hunt-creature-regen-tick.md` thread: the "regen tick" IS this OnUpdate — but
it regens AP; HP regen is script-side, not static.
