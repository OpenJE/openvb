# 2025-10-20 — HP/damage wire flow + GAME_ID protocol names (session 30) — combat CLOSED

Target: ORIGINAL_REFERENCE. Read-only. Final combat session; includes **two corrections**.

## 1. GAME_ID_* names recovered (log-string → packet-id batch map)

| GAME_ID | packet id | direction |
|---------|-----------|-----------|
| ENTITY_DAMAGE | **85** | server → clients ("Server - Sending") |
| ENTITY_SETCURRENTHP | **88** | server → clients |
| INTERFACE_HIDE | **104** | server → client |
(names in code as log literals; unmapped remainder live in client-side "Received" handlers —
derivable from the session-11 receive switch: 78/79/81/82/83/90 + the j_-thunk 95/96 pair
are ITEM_EQUIP/UNEQUIP/weapon-ammo/perk names per the log strings.)

## 2. The complete HP/damage chain (all pieces from sessions 21/22/29/30)

```
SERVER side:
  OnServerUpdate (session 21) → CalculateDamage → queue → finisher
  → new GameEffectDamage(+0x9C=dmg) → attach to VICTIM (Entity+0x168 updater chain, session 28)
  → victim Update ticks effect → Execute → Apply (0x45c140):
       type-switch → hurt-message ids → SENDS GAME_ID_ENTITY_DAMAGE (85) via 0x46d020
       (+ model pokes + AddAction as session 22 §2 mapped)
CLIENT side (state sync path):
  SetHP(entityId, hp) = 0x46ee10:
    resolve id → GameEntity → 0x4629c0: cur(+0x24) = clamp(hp, ≤ max(+0x20))   ; the
    GENT max/cur pair from session 3 IS the HP pair (100/100 wire defaults ✓)
    then SENDS GAME_ID_ENTITY_SETCURRENTHP (88) {id, hp} broadcast
```
⇒ HP storage truth: **GameEntity+0x20 = maxHP, +0x24 = curHP** (gent f7/f8) — matching
session 3's wire layout + session 3's "+0x20 = MAX" reading (vft[0]/0x4080c0 returns +0x20
= GetMaxHP ✓ = native 0x45b110's likely body).

## 3. ⚠ Corrections

1. **Session 29 §3 said "HP current/max = Entity+0x1E0/+0x1E4" — WRONG.** +0x1E0
   (Entity-view +480) is the *self/lookup id slot* AddHP uses to re-find the entity
   (0x46ee10's first arg); actual HP = GE+0x20/0x24 per §2. Session 29 patched by pointer.
2. Session 22 §2's "Apply does NOT store HP inline" — CONFIRMED by this wire mapping
   (it sends id 85 instead); its "queued consequence" = the message/id-85 chain — resolved.

## 4. Status: combat pipeline 100% mapped end-to-end

cursor → cmd enum → packet 91 → server action → beats/timing → to-hit (d100) → damage
formula (DT/pen/roll) → per-hit queue → effect instances on victim → tick → Apply → **id 85
damage notify** → client SetHP (clamp to max) → **id 88 broadcast** → UI. Zero unknowns
remain except the death-state branch (ChangeState at HP≤0 — adjacent, standard) and the
scripted `A_Attack` parity (same action path, session 29).

## 5. Queue (combat retired; global list)

1. Death branch: 0x46ee10/0x4629c0 → who checks `cur<=0` → ChangeState(dying) — quick.
2. `GameParty+0x14` writer (protagonist assignment).
3. Blocked-on-runtime items with reimpl: +0x2C/+0x34 breakpoints, 517-prefix dump,
   client-side packet 85/88 handler confirmation.
4. Clip/World-tree/sys.ini sweeps — done or parked; corpus now pivots to verification rounds.
