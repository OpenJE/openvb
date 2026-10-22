# Two int arrays inside GameCreature, found by the script natives that index them (B3, 2026-10-22)

`GetPerkRank` and `GetBaseSkill` are the first natives in batch B whose terminal instruction is an INDEXED
read, and that tells us where two arrays live. Both byte-verified (the decompiler's numbers here are cell
arithmetic on `char *` - see harness items 68 and 69 - so only the instruction text counts):

| native | EA | cast target | terminal instruction | conclusion |
|---|---|---|---|---|
| `F3::GetPerkRank` | 0x45ab50 | Entity->GameCreature (0x45abaf) | 0x45abd2 `mov eax,[eax+ecx*4+114h]` | **perk-rank int[] at GameCreature+0x114** |
| `F3::GetBaseSkill` | 0x45ad90 | Entity->GameCreature (0x45adef) | callee 0x409824 `mov eax,[ecx+eax*4+0E0h]`, `retn 4` | **base-skill int[] at GameCreature+0xE0** |

`GetBaseSkill` does not read the array itself: it tail-calls a tiny accessor - `cls_0x409820::meth_0x409820`
(0x409820), three instructions, `thiscall (int skill)`, returning `[this + skill*4 + 0xE0]`. The `cls_0x409820`
label is an IDB artifact of the same shape as the `GameActionEquip` attributions elsewhere in this file: the
`this` is a GameCreature (the caller RTTI-casts to GameCreature at 0x45adef and passes it unchanged, 0x45ae13
`mov ecx,eax`), so the accessor is `GameCreature::BaseSkill(int)` living at +0xE0.

Lengths are NOT established by these two reads (no bounds test exists in either native - a script can pass any
index and the shipped code will read out of the array happily, which is itself worth knowing for parity). The
`GetTotalSkill` switch below implies >= 18 skill slots; perk count is open. Do not encode a guessed size.

## Why these two rows are NOT ported yet (named blocker, not a punt)

`src/F3/GameCreature.hpp` carries the first dword of each band as a plain member (`mbr_0xe0`, `mbr_0x114`), and
per item 69 those members' POSITIONS in the port struct are not their shipped offsets (offsetof(mbr_0xbc)=0x4B4
while shipped is 0xBC). So the honest array read cannot be expressed as `(&obj->mbr_0x114)[i]` - that would
index into whatever the name-sorted header put next, silently, and compile clean. Landing the array natives
therefore requires making `mbr_0xe0` and `mbr_0x114` real `int[]` members (or giving the port a shipped-ordered
GameCreature), which is a LAYOUT DECISION for the header's owner lane, not a private edit inside a natives TU.
Filed as B4's prerequisite with the two candidate shapes; B3 ports the scalar HP pair instead and says so.

## GetTotalSkill is where the skill->attribute formulas live (read from PSEUDOCODE, unverified)

`F3::GetTotalSkill` (0x45ae20) is the same preamble + `GameCreature::GetTotalSkill_409830(this, skill=params[2])`.
That callee (0x409830) is the engine's skill-total rule: base skill plus a weighted attribute combination, with
a `default:` arm that logs `"GameCreature::GetTotalSkill() - UNKNOWN SKILL: %d"` and returns the base unchanged.
The weights as the decompiler renders them (1x/2x/3x per attribute, and a flat +50 pair for the
Firearms/Melee arms) and the attribute band it reads (strength/perception/endurance/charisma/intelligence/agility
with the IDB's 0x760-0x774 offsets) are NOT byte-verified by this sitting - the switch lowering and the constant
scaling are exactly the kind of thing pseudocode gets quietly wrong, and the +0x760s need the same offset
discipline as everything else here. Verification is B5 with a named claim list: (1) each case's arithmetic,
(2) which skill enum value maps to which case, (3) the attribute offsets in bytes, (4) whether the Firearms/
Melee arm is really a constant or a mis-scaled attribute term. The doc records the SHAPE as discovered and the
NUMBERS as unproven, so the combat lane can start from a real lead instead of a re-derivation.

## The primary band is REAL and contiguous at +0x760 - and derived HP has a systematic off-by-one-low (slice B13)

`GetPrimaryAttribute` (0x45ac70) settles the band's shape with one instruction: after the
Entity->GameCreature cast, `mov ecx,[esi+8]` / `mov eax,[eax+ecx*4+760h]` (0x45acef/0x45acf2) indexes
**`GameCreature+0x760` as a contiguous `int[]`, stride 4, off the POST-CAST base, with no bounds test
anywhere**. `GetDerivedAttribute` (0x45ad00) then names every slot by reading it in a 15-case switch
(`cmp eax,0Eh` at 0x409a67): strength `+0x760`, perception `+0x764`, endurance `+0x768`, charisma
`+0x76c`, intelligence `+0x770`, agility `+0x774`, luck `+0x778`. That is SPECIAL + charisma, seven
values, and it is the FIRST attribute array in this file that the generated headers do not need to be
reconstructed for - the IDB already has the names.

Two things the band itself gave, both now asserted (92 asserts in `ScriptNatives`):

1. **`charisma_0x76c` is read by no derived case.** It exists in the band and in the index path, and the
   switch simply has no formula that touches it. A port that "knows" charisma must be derived from
   something will invent a case that is not there.
2. **EVASION does not come from the band.** Case 4 reads `+0x7FC` (`maybe_agility_0x7fc`), a separate
   field 0x84 bytes away, so the derived switch straddles two different regions of the creature. The
   IDB's own hedged name is the only clue to what that field is; it is recorded as-is, not renamed.

### The artifact: `HIT_POINTS` is a multiply-then-reciprocal, and it lands one low at every exact divisor

Case 14 is not a field. It re-enters the function for `ACTION_POINTS`, then:

    409b19  fild  [esp+4+derived_attribute]   ; AP as a float
    409b1d  fmul  ds:dbl_64D590               ; * (1/6000), a stored DOUBLE
    409b23  fcom  ds:dbl_64D588               ; compare with 0.0
    409b30  fld   ds:dbl_64D580               ; 1.0
    409b36  fdiv  st, st(1)                   ; 1.0 / (AP/6000)
    409b3c  call  __ftol2                     ; TRUNCATE toward zero

`1/6000` is not representable in binary, so `AP * const` lands slightly **above** `AP/6000`, its
reciprocal lands slightly **below** `6000/AP`, and truncation drops the fraction. Measured against the
port's mirrored arithmetic for every AP from 1 to 40, the result is one LOW whenever `6000/AP` is an
exact integer:

| AP | obvious `6000/AP` | shipped formula |
|---|---|---|
| 1 | 6000 | **5999** |
| 6 | 1000 | **999** |
| 12 | 500 | **499** |
| 20 | 300 | **299** |
| 30 | 200 | **199** |
| 7 | 857.14 | 857 (agrees - no exact integer to lose) |

Three consequences worth writing down:

- **A "cleaner" implementation diverges in the most common cases.** Writing `6000 / AP` would be wrong at
  AP=20 (the base agility-0 human value), AP=30, AP=12 - i.e. at exactly the round numbers a test would
  pick. The mirror is only two extra lines and it is the difference between matching the engine and
  matching the formula.
- **The zero-AP case is 1000, not a divide-by-zero** (`0x409b4c`), and 1000 is not on the curve: AP=0 gives
  the *largest* derived HP in the range. Any HP-recovery design that assumed monotonicity in AP is wrong
  about shipped.
- **This is falsifiable against the running exe, which is the point of noting it.** Prediction for whoever
  has lockstep: a creature with derived HP reported as `N` should satisfy `N = trunc(1/(AP*(1/6000)))`, so
  a creature with AP exactly 20 shows **299**, not 300. If the shipped demo shows 300, then x87's 80-bit
  intermediate differs from the port's double-step and the mirror needs revisiting - the discrepancy would
  be information, not noise, and the table above is what to check it against.

Note on precision, stated rather than assumed: shipped uses x87 (extended intermediates) and the port uses
`double` steps in the same order. For these values both give the same truncation, because the error is
introduced by the stored `1/6000` constant rather than by intermediate width. That is a reason the artifact
is *portable across compilers* - and the lockstep check above is what would falsify that claim instead of
leaving it as reassurance.
