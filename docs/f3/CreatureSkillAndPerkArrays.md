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
