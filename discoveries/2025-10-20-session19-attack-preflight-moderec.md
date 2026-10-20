# Session 19 — GameActionAttack preflight step (0x4023c0) + modeRec layout addenda

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Follows `2025-10-20-attack-state-machine-damage-formula.md` (reimpl-authored) — decodes the
"next target 0x4023c0 family" they named. Cross-checks their modeRec offsets.

## 1. `GameActionAttack::ResolvePreflight_0x4023c0` (bool, no args)

Called **only from `OnServerUpdate_0x404320` @0x40456d** — i.e. the preflight runs inside the
per-frame server update; if it fails the action aborts (`meth_0x401a10(this,1)` = cancel).
Steps:

1. `ResolveWeaponAndAttackMode_0x40b380(...)` (session 6 §1: classId==6 weapon gate) fills:
   - `action+0x28` = weapon entity ptr (prim), `action+0x34` = **modeRec ptr**,
     `action+0x71` = unarmed bool. On failure: cancel + log
   `"GameActionAttack - ResolveWeaponAndAttackMode failed."`.
2. modeRec attack-type byte (**modeRec+0x08**):
   - `0` or `3` → melee-ish: `action+0x72..75 (dword) = modeRec[+0x2C]` (the auto-move range);
     log `"Resolved as melee or unarmed with a range of %d"`.
   - `1` or `2` → ranged: `action+0x70 = 1`, range dword = **−1** ("never auto-move");
     log `"Resolved as ranged so no auto-move will be queued no matter what."`
   - anything else → cancel + `"GameActionAttack - Invalid attack type found in the attack mode.
     Cancelling."`
3. **Called-shot contract** (modeRec byte **+0x41**):
   if set: the action's called-shot part (action byte +0x7C) must have been set (≠9) —
   else cancel + `"GameActionAttack - Attack mode is called shot, but SetCalledShot has not
   been called."`; else sets action byte +0x57 = 1.
4. Target snapshot: `action+0x28-slot-pair` — raw: `*[action+0x20] = [weapon/victim Entity-vb
   +0x1E0]` display → **victim engine netId (Entity+0x1DC)** stored on the action.
5. Return `rangeDword(action+0x72) != 0`.

## 2. modeRec (GWAM attack-mode record) — byte layout, raw-verified

Convention note: everywhere `modeRec[i]` appears in the reimpl doc or in decompilers, `i` is a
DWORD INDEX (byte = 4·i). All rows below re-anchored to raw byte displs this session:

| byte | dword-idx | field | evidence (aligned raw) |
|---|---|---|---|
| +0x08 | [2] | attack type: 0/3 = melee-ish (auto-move path), 1/2 = ranged, other = invalid→cancel | `mov [ecx+8]; cmp 3` @0x402499-0x4024a7 |
| +0x10 | [4] | called-shot requirement flag (0 = none; ≠0 gates on part ≠9) | `cmp byte [edi+10h],0; jz→cancel "called shot...SetCalledShot"` @0x4025d1-0x4025ee |
| +0x14 | [5] | DR-array selector: 0-4,7 → creature DR @+0x7B0; 5,6 → +0x7C8 (else-branch logs "Unknown CALLEDTARGET") | `mov [esp+14h],edi` then `0x7b0(,%edi,4)` vs `0x7c8` @0x41341e/0x41342b |
| +0x1C | [7] | armor-piercing amount (DT reduction sink before halving) | `mov %edx,0x1c(%esp)` from ammo@0x3C0 @0x4133dc — note: the ammo pierce rides INTO the 0x1C slot at use-site; modeRec[7] itself participates in the DT math chain per the formula doc |
| +0x24 | [9] | flat damage adder | `mov 0x24(%edi),%edx` @0x4133f6 |
| +0x2C | [11] | melee reach (auto-move distance; −1 semantics for ranged set by caller) | `mov [ecx+2Ch]` @0x402567 |
| +0x40 | [16] | DT base (formula "−(DT[modeRec[16]]−pierce)/2") | reimpl doc + unit match |
| +0x58 | [22] | ammo cost (sub-attack series) | reimpl doc [22] |
| +0x70,+0x80 | [28],[32] | ammo deduct costs (primary/secondary) | reimpl doc |
| +0xE0..+0xF0 | [56..60] | damage dice (count/sides pair family) | reimpl doc |

(One earlier draft here claimed the called-shot flag at +0x41 — that came from a misaligned
decompiler line; the aligned dump @0x4025d1 is definitive: **+0x10**.)

## 3. New parity-logging strings (client-visible debug channel, cls_0x558740)

- `"GameActionAttack - ResolveWeaponAndAttackMode failed."`
- `"Resolved as melee or unarmed with a range of %d"`
- `"Resolved as ranged so no auto-move will be queued no matter what."`
- `"GameActionAttack - Invalid attack type found in the attack mode. Cancelling."`
- `"GameActionAttack - Attack mode is called shot, but SetCalledShot has not been called."`
Together with "Hit/Miss: Chance/Random" (session 17), "Damage for %d queued." (session 6) and
"Done" (session 14) these form the demo's combat log signature — useful for lockstep diffs.

## 4. For the reimpl

- modeRec offsets are BYTES here; convert to the reimpl doc's bracket-idx by ÷4 when cross-reading.
- Preflight = cheap per-frame retry inside OnServerUpdate — implement as a guard returning
  cancel/resolved states, NOT a one-shot create step (weapon could be equipped mid-action).
- The ranged/melee classification uses the mode record's type byte, and the "auto-move to
  range" number is the mode record +0x2C — your MoveTo-range seeding should read the same.
- Called-shot gating (SetCalledShot before playing the anim) prevents the DM beat firing with
  part 9; mirror the check or your hit-apply will silently produce 0-damage hits (per
  session 6 §5, calledTarget==9 ⇒ damage 0).
