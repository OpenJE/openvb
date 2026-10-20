# 2025-10-20 — T-010c: called-shot CALLEDTARGET index domain — second source found; NAMES remain unresolved (verdict per policy)

Author: FastArrow (research). Target: ORIGINAL_REFERENCE, read-only. Task:
`tasks/T-010-research-queue-three-blocking-gaps.md` §10c. No IDB/source edits.

## Verdict (acceptance line first)

**FOUND: a genuine second (and third) consumer that locks the INDEX DOMAIN and the {5,6}
DT-row grouping — all evidence below is binary, no inference chains.**
**NOT FOUND: any static index→PART-NAME mapping. Explicit: "not statically resolvable" from
this binary — the UI is icon-driven (no name table for this enum exists in exe or data).**
Lanes keep indexing numerically (§12 policy stands, now with a proven grouping fact added).

## 1. The second source: two identical `switch(target)` guards — one in EACH damage path

`GameWeapon::CalculateDamage_413310` @0x413413 and
`GameCreature::CalculateUnarmedDamage_409f90` @0x40a06b (independent TUs, byte-identical
structure):

```
switch (calledTarget):                 ; param_5 / param_4 = SAME value that indexes
                                       ; dword_64F858 / flt_64D4C0 penalty tables
  case 0,1,2,3,4,7:  dr = *(target + 4*DTidx + 0x7B0)     ; creature DT row A
  case 5,6:          dr = *(target + 4*DTidx + 0x7C8)     ; creature DT row B
  default:           LogDebugString("... - Unknown CALLEDTARGET encountered.\n")
                     dr = *(target + 4*DTidx + 0x7B0)     ; fallback = row A (matches
                                       ; canonical §11 "unknown part falls back, computes")
```
(preceded in both by `if (target == 9 && !usableObject-target)` early-return semantics —
9 = "nothing targeted"; armed path logs `"GameWeapon::CalculateDamage - No body part was
targeted."` @0x41333d first.)
Usable-object targets (GameUsableObject branch of the unarmed fn) read `+0xA4` DT instead —
object armor row, part-independent.

⇒ **What this adds to the corpus** (beyond §11's single anchor): the FULL partition
{0,1,2,3,4,7} vs {5,6} is now established in TWO consumers (was: penalty-table proximity
inference only), the fallback row is pinned, and `9` semantics re-confirmed independently.
The two 8-dword creature arrays at +0x7B0/+0x7C8 are the "two DT rows" the attack-state
doc §3 used — their SELECTION RULE is now this switch, closed.

## 2. The third consumer: the uncalled-shot hit-location SELECTOR shares the domain

`sub_409960(creature, roll1..100)` = cumulative-weight picker over
`creature +0x710..+0x72C` (8 dwords): buckets → returns **0..7**; overshoot past
`total + [cre+0x72C]` → returns 0; the 9th/10th fields fold into cases 6/7.
Caller: `sub_401280(action, target)` (attack doc §"called-shot(9): sub_401280") —
`roll = rand(1,100); action->[+20] = selector(target, roll)` — i.e. when the player did NOT
call a shot (part==9), the engine PICKS a target from the same 0..7 space using the
`baseUncalledChances` weights. ⇒ penalty index space == DT-switch index space ==
uncalled-selector index space == **one 0..7(+9) domain** (three independent anchors).

⚠ Cross-note for the gcre-versions doc (their §1.3 dword-mismatch query): the selector
reads start at **creature+0x710** (dword 452), so the READER base +0x710 is confirmed;
their sanity-sum base 0x70C keeps a leading field — matches that doc's leading
hypothesis (array @+0x70C+4 stride… their call to mirror, file it under their §).

## 3. Names: searched, absent (this is the "not statically resolvable" evidence)

- exe string inventory around the feature: `"CalledShotIcon_up/_down"`,
  `"picWeapon01/02CalledShot_up/_down"` (action-bar ICON pairs only) — no
  `CALLEDTARGET_*` enum strings anywhere; both "Unknown CALLEDTARGET" logs carry no value dump.
- the only named part list in the binary is the MODEL-ATTACHMENT registry (two flavors,
  both DIFFERENT index spaces from CALLEDTARGET):
  (a) `sub_40AE60` defaults `Chest=0,Feet=1,Hands=2,Head=3,Legs=4,UpperArms=5,Waist=6,
      ForeArms=7,Shins=8` (+12 file overrides) — body-part doc §1 (model/armor bones);
  (b) static pointer array @**0x6FF654**: `"", Head, Chest, UpperArms, Hands, Legs, Feet,
      Waist, ForeArms, Shins` (idx 0..8, idx0 = empty string) — resolved by name via the
      stricmp loop at 0x564B30 window (`add edi,4; cmp edi,0x6FF678` over 0x6FF650..0x6FF678);
      it is the model socket table (sibling of `sub_564F20`), NOT the gameplay enum.
  No bridge exists: neither damage fn, the selector, nor the producers touch (a)/(b) with a
  CALLEDTARGET index; no `part→attachment` translation in the hit path.
- English.stf part words ("Groin", "skull", "eye", 36× "leg") are combat-text flavor;
  nothing in the exe indexes stf by this enum.
- producer side: field is `GameActionAttack+0x78` (RAW; attack-state doc §"called-shot target
  part (9=unset) @+0x73+3" mislabeled — 0x4025df proves `cmp [esi+0x78],9`), written only
  from the packet-91 unpack (client UI → command record → sender 91; already mapped by the
  cmd-semantics doc). UI selection ordinal → byte is a pure copy; the name binding lives in
  widget layout (icons), not data.

⇒ residual risk (stated per policy): the ONLY way to name the eight is runtime: (i) click
each called-shot icon client-side and watch the packet-91 byte / `action+0x78`, or
(ii) dump `creature+0x710..0x72C` weights for a known model (torso-weighted by definition →
biggest weight = torso) and cross with `dword_64F858` difficulty ordering. BOTH belong to
A's instrumented-boot lane, not static RE. Suggested one-liner for A: bp 0x4025db per
selection, log [esi+0x78].

## 4. Numeric-only summary the lanes may rely on (safe to port)

| idx | penalty | DT row | selector weight field |
|-----|---------|--------|------------------------|
| 0 | 10 | A(+0x7B0) | cre+0x710 |
| 1 | 40 | A | cre+0x714 |
| 2 | 40 | A | cre+0x718 |
| 3 | 40 | A | cre+0x71C |
| 4 | 40 | A | cre+0x720 |
| 5 | 60 | **B(+0x7C8)** | cre+0x724 |
| 6 | 120 | **B(+0x7C8)** | cre+0x728 |
| 7 | 60 | A | cre+0x72C (overflow → 0) |
| 8 | 0x7F7FFFFE (sentinel in image — never legally indexed) | — | — |
| 9 | unset (skip penalty, early-return guards in both fns) | — | — |
| >9| falls back row A + "Unknown CALLEDTARGET" log | A | — |

(weapon table 0x64F858 == unarmed 0x64D4C0 dwords; keep ONE table per session-17 §6.)
