# 2025-10-20 — naming-addendum follow-up: the {5,6} class byte is an EQUIP SLOT INDEX (6-slot array at GameInventory+0x14); slots 2..5 contribute NO armor DR

Author: FastArrow. Closes the §2 open hook of `t010c-naming-addendum-armor-rows.md` (6fbb2cc).
Target: ORIGINAL_REFERENCE, read-only. Raw citations inline.

## 1. Chain, end to end (all three sites decompiled this pass)

1. **Aggregation** `GameCreature::meth_0x40A670` (B6 §1): for each equipped GameArmor, the
   class byte `v9` comes from nodes produced by:
2. **Slot enumerator** `GameInventory::vft[+40] = 0x46A860`:
   `for (i=0..5) if (*(inventory + 0x14 + i*4 == itemPtr)) outList.push_back(i)` —
   the "small records +0x14..0x2B" of gobj-doc §7 are a **6-dword equip-slot array**, and the
   pushed node payload (list elem+0x08, consumed as `v8[2]`) is the **slot index 0..5**.
3. So `meth_0x40A670`'s branches mean, literally:
   - armor in **slot 0** → sums its DT6 (armor+0xEC..+0x114) into **row A** (+0x7B0, used for
     called targets {0,1,2,3,4,7});
   - armor in **slot 1** → into **row B** (+0x7C8, used for called targets {5,6});
   - armor in **slots 2..5 → IGNORED by DR entirely** (v9>1 falls through; no else-chain).

## 2. Shipped-defect candidate (register with §11 family)

Slots 2,3,4,5 exist, are equip-valid, and are invisible to damage calculation — if the design
intended e.g. arm/leg gear to DR anything, this is a silent bug. Same species as the §11
null-page branch and the `+0x19` alias traps: **a hard-coded two-zone read of a six-wide
table**. *(inferred design intent; defect status pending which slots shipped gear ever
occupied — a data question for the GIAR records, next.)*

## 3. What {5,6} naming now reduces to

One datum: **which paper-doll slot receives helmets.** slot0=body & slot1=head ⇒
{5,6} = head-and-eyes family (6 with penalty 120 = the eyes/head-hard variant); the socket
table order ("",Head,Chest,…) hints slot1≈Head but is NOT the same index space (socket
indices 1..9 vs equip slots 0..5) — do not conflate without the equip-side constant.
Deciders, cheapest first:
a. `GameActionEquip::OnServerUpdate` slot-selection code — one decompile — shows which
   armor field maps to slot 0 vs 1 (GIAR byte) → names the slots by gear type.
b. Runtime: equip helmet → re-dump +0x7C8 row delta (B6 §3) — now guaranteed to show the
   slot-1 item specifically.
§12 policy stands until (a) or (b) lands.

## 4. Board/coordination
- B6 §2/§3 wording is superseded by this file (committed-file left as-is per protocol).
- Next static hop if assigned: `GameActionEquip::OnServerUpdate` slot-choice (3's item a).
  It also feeds T-012's equip-side tests, so one pass serves two rows.
