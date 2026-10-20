# Session 32 — T-010c SECOND SOURCE FOUND: static part-name table at 0x6FF650 (10 entries: "", Head, Chest, UpperArms, Hands, Legs, Feet, Waist, ForeArms, Shins); alignment to the 8-entry penalty table needs exactly ONE runtime observation — not more static work

Target: ORIGINAL_REFERENCE. Read-only. Dispatch response: T-010c research input;
also records the VOID of my "part 4 = arm" inference per coordinator ruling (done in
sessions 6 §5 + 29 §2).

## 1. What was found (raw evidence)

1. **`.rdata` name cluster 0x64D608..0x64D654**: `Shins, ForeArms, Waist, UpperArms, Legs,
   Head, Hands, Feet, Chest` (8-byte-aligned strings). Adjacent to the
   `ResolveWeaponAndAttackMode` assert string (same page) — combat-vocabulary region.
2. **Pointer array 0x6FF650 (10 dwords, statically initialized)** =
   `[0x64d7d8(""), 0x64d638(Head), 0x64d650(Chest), 0x64d624(UpperArms), 0x64d640(Hands),
     0x64d630(Legs), 0x64d648(Feet), 0x64d61c(Waist), 0x64d610(ForeArms), 0x64d608(Shins)]`
   — dump verified from the flat image (VA−0x400000). NOTE the `.rdata` *layout* is the
   reverse of the traversal in the builder below — layout order is a push_front artifact;
   **do not** read anatomy order off string adjacency.
3. **Single construction site**: `0x564AB4 mov edi, 0x6FF650` inside `0x564A50` (a
   range-walk that push_backs the C-array into a `vector<string>`); `0x564A50` has exactly
   one caller, `0x565676`, inside a larger init routine 0x5652xx–0x5656xx (also contains
   3× `fadd 4294967296.0` = 2^32 wrap helpers — unrelated). That routine was guessed to be the combat-UI picker
   init — **SUPERSEDED (session 33): sole owner is `JE::ModelInstance::SetSocketModel_564f90`
   (call @0x5650C5); the list is the ART-SOCKET name table. Session 33 derives the part enum
   from it (0=HEAD..8=SHINS, 9=UNSET) and closes T-010c statically.**
4. **Penalty table** dword_64F858 confirmed 9 entries [10,40,40,40,40,60,120,60,FLT_MAX]
   (session 17) — 8 usable + sentinel; part 9 = "unset" in every code consumer
   (sessions 6/19/21).

## 2. The alignment question — cleanly stated for one runtime tiebreak

Name vector: 10 slots (""+9 parts). Penalty: 8 slots. Code sentinel: 9=unset.
Two candidate alignments, both static-legal:

| id | A-array (0="",1=Head..9=Shins) | B-shifted (0=Head..8=Shins, 9=unset) |
|---|---|---|
| penalty[0]=10 | "" (!) | Head |
| penalty[4]=40 | Hands | Legs |
| penalty[5]=60 | Legs | Feet |
| penalty[6]=120 | Feet (!) | Waist |
| penalty[7]=60 | Waist | ForeArms |
| 9 = unset | Shins (collision!) | ✓ consistent |

(A ids: [1]Head [2]Chest [3]UpperArms [4]Hands [5]Legs [6]Feet [7]Waist [8]ForeArms [9]Shins;
B ids: same list minus the placeholder, i.e. B_id = A_id − 1.)

B-shifted is self-consistent with the "9 = unset" sentinel: 10th array slot "" is a display
placeholder, ids 0..8 = the 9 names, 9 = unset; 8 penalties cover ids 0..7 (Head..ForeArms),
id 8 (Shins) falls to the FLT_MAX/default path ⇒ **Shins never penalized** — plausible.
A-array leaves sentinel semantics broken (Shins at 9 collides with "unset") and puts Feet at
the 120 max-penalty slot. **B-shifted is the better static candidate** — under it the max
penalty is Waist (120) and the min is Head (10): unusual vs other CRPGs, which is exactly
why the tiebreak stays runtime.
DR-row group {5,6}: A ⇒ {Legs, Feet}, B ⇒ {Feet, Waist}. Robust across both: **the +0x7C8
DT row = Feet + one lower-body neighbor (Legs or Waist)**; supersedes my old legs/torso guess.

## 3. Tiebreak recipe (one debugger stop, or one lockstep line)

Break at `0x402393` (RollToHit after penalty fetch, `dword_64F858[part]`), pick a known part
in the UI (the picker labels come from the vector above) → observed part byte vs penalty
index. One stop settles ids; or in lockstep, `SetCalledShot('Head')` from AMX/debug and diff.
If UI labels render Head first with index 0 → B-shifted confirmed; shipped enum then:
0 HEAD, 1 CHEST, 2 UPPER_ARMS, 3 HANDS, 4 LEGS, 5 FEET, 6 WAIST, 7 FORE_ARMS,
8 SHINS (no penalty entry — default path), 9 UNSET.

## 4. For IronStorm (T-010b, adjacent bonus)

The 0x671Bxx cluster dumped this session is the **equip-slot name list** (OFFHANDITEM,
ONHANDITEM, OFFHANDWEAPON, ONHANDWEAPON, VANITY, EYES, HAIR_*, FEET, HANDS, BACK, HEAD,
SHOULDER, BODY, SKIN) — candidate label source for GWAM/equip slots; also present near
cls_0x442bf0 refs used by the cutscene-STOP fan-out (session 31 §2). Not the called-shot
answer; useful for their +0x3B0 item-record cross-checks.
