# 2025-10-20 — T-010c naming ADDENDUM: the {5,6} DT-row group is an ARMOR slot-class split, narrowing the naming test

Follow-up to `called-shot-part-second-source.md` (§1-5) + `t014-record-mark-19-writer.md`
(a252575). Author: FastArrow. Target: ORIGINAL_REFERENCE, read-only. Found while answering
the coordinator's "one evidence short" pointer on index→name. Raw citations throughout.

## 1. Producer of the two DT rows: `GameCreature::meth_0x40A670` (armor DR aggregation)

Walks the equipped-armor list (GameInventory tree at `this+0x16C(vb)→+12`), per node:
`v7 = RTTI(Entity→GameArmor)`, node-class byte `v9 = *(node+0x08)`:

```
v9 == 0:  local_3c[0..5] += armor[+0xEC..+0x114]      ; six DT values (DT 0..5 — same width
                                                        as the .veg switch and ammo DTpen)
v9 == 1:  local_3c[6..11] += armor[+0xEC..+0x114]     ; same six, OTHER group
v9  >  1: IGNORED (no else-branch — only two classes exist)
then:  +0x7B0..+0x7C4 = base(+0x730..0x744) + local_3c[0..5]   ; ROW A (DT×6)
       +0x7C8..+0x7DC = base(+0x748..0x75C) + local_3c[6..11]  ; ROW B (DT×6)
```
(+0x7BC/+0x7C0 writes at 0x40A946/0x40A922 complete A; 0x40A8C8/0x40A902 anchor B — the
"8 dwords each" wording in the attack doc §3 should read **6 DT values each**, matching ammo
DTpen at GameAmmo+0x3C0 and the DT-indexed damage path `DTrow[mode+0x10]` — mode+0x10 ∈ 0..5.)

## 2. What this proves about the called-shot {5,6} group

The called-shot target switch (both damage fns) picks ROW B iff target ∈ {5,6}. Row B is
"DT summed over class-1 equipment". So **{5,6} are exactly the body regions covered by
class-1 armor only** — a two-zone body partition: {0,1,2,3,4,7} share class-0 coverage,
{5,6} share class-1. That is a coverage-partition fact, not an armor-type guess — and it
matches d20-Modern-style splits (torso/limbs vs head) if class-1 = head-gear.

Where the class byte is written: equip-path insert into the GameInventory list — **not yet
traced** (next hook; `sub_433760` insertions into `this+0x12C/0x16C` maps are the places to
watch). When pinned, the class value of a KNOWN helmet .arm record's equip node decides the
naming outright.

## 3. Cheaper decider than the Shins log-diff — one gdb/read line

IronStorm's log-diff fingerprint is still valid, but this narrows to a single datum:
equip/remove any helmet-class armor on the target and diff
`creature+0x7C8..0x7DC` — the six values that move ARE class-1 gear; then read the equip-node
class byte. Alternative pure-static path: find helmet records' GIAR field feeding the class
byte (GameArmor+0xEC.. DR6 is already known; the class source is a GIAR/GIAR-adjacent byte,
candidate: the field gating the `sub_46C6C0`-style equip insert).

## 4. Status updates for the board
- T-010c naming: **{5,6} = "class-1-armor-covered regions" PROVEN** (raw, §1); still one
  evidence short of NAMES (class-1 == head? — §2 hypothesis, §3 deciders listed). §12 policy
  stands (code indexes numerically).
- Attack doc §3 "two DT rows, 8 dwords each" → **6 DT values each, rows +0x7B0/+0x7C8**
  (delta doc again per protocol — I don't edit the committed file).
- The `.veg` damage-type switch (0..5) and ammo DTpen share the SAME 6-value DT space —
  DT 0..5, not 0..7 as an earlier note said: `mode+0x10` feeds DTrow[] directly; the two
  called-shot groups read only DT 0..5 — consistent with GameEffectDamage +0xA4 DT enum
  0..5 in gameeffect-damage-anatomy. (If a record carries DT 6/7, the 0x7B0+28/+32 reads walk
  into the neighbouring row-B/base fields — latent shipped quirk, same family as §11 nulls;
  note only.)
