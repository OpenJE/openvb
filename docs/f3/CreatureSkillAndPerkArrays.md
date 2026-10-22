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
index and the shipped code will read out of the array happily, which is itself worth knowing for parity).
[Later, in B14: the count is 13 and 13, from the perk band's start offset, the switch's `cmp edx,0Ch`, and the
ctor's two `memset` sizes - see RESOLVED above. The worry expressed here that `GetTotalSkill` implied >= 18
skill slots was wrong: the switch has exactly 13 cases and its default arm handles anything above, including
the perk band's memory.]

## RESOLVED in B14 - the blocker was a framing error, and the sizes came out of the code

The section below said these rows needed a LAYOUT DECISION from the header's owner lane, because making
`mbr_0xe0`/`mbr_0x114` real arrays would move every later member's port offset. That was true only for a
mid-list edit. B13 established the safe shape - **append the band at the end of the struct** - so no existing
member shifts, and the port models the band as a real array whose shipped offset lives in the comment, not in
`offsetof`. Both natives are LIVE with 115 asserts, and the two band sizes came from the code rather than from
a guess:

| band | size | evidence (two independent counts each) |
|---|---|---|
| base skills `+0xE0` | **13** | the perk band starts at `+0x114`, and `(0x114-0xE0)/4 = 13`; `GetTotalSkill`'s switch is `cmp edx,0Ch` = cases 0..12 |
| perk ranks `+0x114` | **13** | `GameCreature::ctor_0x40d010` clears it with `memset(<mbr_0x114>, 0, 0x34)` at 0x40d277 = 13 dwords |

The bands are **adjacent**, and that has a consequence worth more than the sizes: `GetTotalSkill` calls the
base-skill accessor BEFORE its switch (0x409837), so skill index 13 does not read a nonexistent slot and fall
through to the default - it reads **perk rank 0** and returns it unchanged. Index 14 returns perk rank 1. That
is a reproducible out-of-range ALIAS, not a crash, and modelling the two bands as separate C++ arrays would
have made it undefined behavior and therefore untestable. They are one 26-int array
(`statBands_0xE0[26]`, `[0..12]` skills, `[13..25]` perks) precisely so the alias stays defined and asserted:

    GetTotalSkill(13) == 23   // == perk rank 0 of the test creature
    GetTotalSkill(14) == 24   // == perk rank 1
    GetBaseSkill(13)  == 23   // the same alias with no formula involved - pure band overlap

Two shipped quirks from the same three reads, preserved and asserted rather than corrected:

1. **`GetPerkRank`'s resolve-failure message is a copy-paste from another native**: it prints
   `"GetPosition() was unable to find the requested entity."` at 0x45ab8e while its arity gate says
   `GetPerkRank`. Wrong in the shipped exe, wrong in the port, byte-exact in both.
2. **The `Entity ID` / `EntityID` space splits the family 2-with / 3-without**: `GetPrimaryAttribute` and
   `GetPerkRank` use the space; `GetDerivedAttribute`, `GetBaseSkill`, `GetTotalSkill` do not. Five natives,
   one sentence, two spellings - which is why the assertion is on the tail of the formatted message and not on
   a normalized string.

## Why these two rows are NOT ported yet (named blocker, not a punt)  [SUPERSEDED - see RESOLVED above]

`src/F3/GameCreature.hpp` carries the first dword of each band as a plain member (`mbr_0xe0`, `mbr_0x114`), and
per item 69 those members' POSITIONS in the port struct are not their shipped offsets (offsetof(mbr_0xbc)=0x4B4
while shipped is 0xBC). So the honest array read cannot be expressed as `(&obj->mbr_0x114)[i]` - that would
index into whatever the name-sorted header put next, silently, and compile clean. Landing the array natives
therefore requires making `mbr_0xe0` and `mbr_0x114` real `int[]` members (or giving the port a shipped-ordered
GameCreature), which is a LAYOUT DECISION for the header's owner lane, not a private edit inside a natives TU.
Filed as B4's prerequisite with the two candidate shapes; B3 ports the scalar HP pair instead and says so.

## GetTotalSkill's four named claims - ALL ANSWERED, and the fourth one was a real constant

[Amended in B14. The section below was written from pseudocode with its numbers explicitly unproven and a
four-item verification list. Every item is now closed against instruction text:]

1. **Each case's arithmetic** - byte-verified, and the arms are exactly what the pseudocode rendered. With EAs:
   cases 0/1 `mov esi,32h` (+50, 0x40988f); case 2 `[ecx+768h] + 2*[ecx+760h]` (0x4098f2); cases 3/4
   `[ecx+76Ch]` with `lea esi,[ecx+ecx*2]` (3x, 0x409850); case 5 `[ecx+76Ch] + 2*[ecx+770h]`... read as
   `mov eax,[ecx+76Ch]; mov ecx,[ecx+770h]; lea esi,[ecx+eax*2]` = intelligence + 2*charisma (0x409861);
   cases 6/7 `[ecx+764h] + 2*[ecx+770h]` = perception + 2*intelligence (0x409878); case 8 `[ecx+768h] +
   2*[ecx+764h]` = endurance + 2*perception (0x40989c); case 9 `3*[ecx+770h]` (0x4098b3); cases 10/11
   `[ecx+774h] + 2*[ecx+764h]` = agility + 2*perception (0x4098c4); case 12 `[ecx+764h] + 2*[ecx+774h]` =
   perception + 2*agility (0x4098db). Note case 5's register reuse: the second `mov ecx,...` OVERWRITES the
   base register, so the pseudocode's operand order is easy to misread as `charisma + 2*intelligence` - the
   `lea` decides it, which is why the arm is asserted with values that break that symmetry (cha 9, int 5:
   `5 + 18 = 23`, not `9 + 10 = 19`).
2. **Which skill value maps to which case** - the switch is `cmp edx,0Ch` over 0..12 with ten jump targets, and
   four of them serve TWO values each (0/1, 3/4, 6/7, 10/11). So the mapping is established for every case, but
   WITHIN a pair the two names are indistinguishable from the code - both get one body. Recorded as an enum in
   `ScriptNatives.hpp` with that limitation stated, because "which of the pair is which" is not knowable here
   and pretending otherwise would be a fake precision.
3. **The attribute offsets in bytes** - `+0x760` strength, `+0x764` perception, `+0x768` endurance, `+0x76c`
   charisma, `+0x770` intelligence, `+0x774` agility, all as literal disp32s in the arms above. This is also the
   second, independent confirmation of B13's primary band (same offsets, different function).
4. **Whether the Firearms/Melee arm is really a constant** - YES: `mov esi,32h` is an immediate 50 with NO
   attribute load in that arm at all, unlike every other case which loads from `0x760..0x774`. So combat skills
   get a flat +50 rather than an attribute bonus, and that is a design fact, not a decompiler artifact. It was
   the one claim here that could have been a mis-scaled term; it is not.

Original text, kept because the reasoning was the point:

`GetTotalSkill is where the skill->attribute formulas live (read from PSEUDOCODE, unverified)`

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

### RETRACTED: the "systematic off-by-one-low at divisors" was MY constant, not the engine's

The claim above, written in B13 and repeated in a commit message and two ping messages, said derived HP comes
out one LOW at every exact divisor of 6000 (AP=20 -> 299, 30 -> 199). It is **wrong about the engine**. The
chain of error, kept in full because each link is a distinct trap:

1. I read the constant from IDA's pseudocode text: `fmul ds:dbl_64D590` rendered as `0.0001666666666666667`,
   and I typed that decimal into the port.
2. The stored double at VA `0x64D590` (.rdata) is qword **`3F25D867C3ECE2A5`** = `0x1.5d867c3ece2a5p-13` = the
   NEAREST double to `1/6000`, which lies **below** `1/6000`. IDA's 17-digit print parses **one ulp ABOVE** it.
   So my literal was high by one ulp while the engine's is low - the error's SIGN was inverted, and with it the
   direction of every divisor result.
3. My table was then reproduced EXACTLY by simulating the engine's arithmetic with IDA's printed decimal
   (`{1:5999, 6:999, 12:499, 20:299, 30:199}`), which is why it looked like a discovered law: it was a
   measurement of my own transcription. JadeBear caught this by reading the bits, not the print.

So: **constants are adjudicated against stored BITS, exactly like offsets and addresses.** This is the third
distinct pseudocode-rendering hazard this file has recorded (cell-vs-byte offsets, `mbr_0x0` as a positional
name, and now a decimal print off by one ulp), and it is the worst of the three because it survives almost all
testing: 15 of 16 AP values agree perfectly between the two constants, and only the exact divisors of 6000
expose the difference. A test suite that probes "interesting" values is exactly the thing that cannot see it.

### What is actually provable statically, and what only lockstep can settle

With the correct bits, `AP * const <= AP/6000`, so the reciprocal is `>= 6000/AP` and the int conversion
(`__ftol2`, 0x409b3c) yields `6000/AP` or higher - **never one below it**. The old claim violated that bound.

What static bytes do NOT settle is the LAST bit at exact divisors, because that depends on how wide an
intermediate the compiler keeps between `FMUL`, `FDIV` and the conversion. This was measured in the port, not
argued: **in one binary, the shipped-shaped path returns 199 for AP=30 while the identical-value expression
`1.0 / (30.0 * k)` returns 200.** Same compiler, same constant, different expression shape. So the suite asserts
(i) the bound `>= 6000/AP - 1`, (ii) this build's value as a pin, and (iii) the ulp relation between the print
and the bits - and it does NOT claim to know the engine's last bit.

The lockstep experiment, ready for whoever has the trace: a creature with `agility_0x774 == 0` has AP = 20
(ACTION_POINTS is itself `agility + 20`, and HIT_POINTS re-enters for it). Read `GetDerivedAttribute(HIT_POINTS)`
in the original:

| observed | meaning |
|---|---|
| 300 | MSVC rounded the intermediates to double, like the direct expression - the port's shipped-shaped path needs a store-point tweak |
| 299 | MSVC kept 80-bit intermediates, like the port's x87 path - the port is already right and the bound assert is the whole story |

Either answer is useful and neither is a surprise; what would be a surprise is anything outside
`{299, 300}`, which would mean the constant was read wrong after all.

Consequences that survived the retraction:

- **Mirror the shipped SHAPE, and pin the constant's BITS.** The two-step multiply-then-recipical is right
  (the bytes show it); the input was wrong (the print, not the image). Both halves are needed - a faithful
  shape with a transcribed constant reproduces a law that never existed.
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

## Closed: the engine computes `floor(6000/AP)`, so the faithful port divides in ints (B17)

The retraction above left one question open - the last bit at exact divisors - and it is now answered from
instruction bytes plus a sweep, not from a guess. JadeBear decoded the caller and `__ftol2`; I verified the
claim myself and, in doing so, found the port's own defect.

**The engine's chain (bytes).** `0x409b15 mov [esp+8],eax` (an INTEGER store of AP) / `0x409b19 fild` /
`0x409b1d fmul ds:dbl_64D590` / `0x409b23 fcom ds:dbl_64D588` / `fnstsw` / `test ah,44h` / `jnp -> 1000` /
`0x409b30 fld ds:dbl_64D580` / `0x409b36 fdiv st,st(1)` / `0x409b38 call __ftol2` / `0x409b3d fstp st(0)`.
Between `FMUL` and the call there is **no store of a real** - the only memory operations are constant loads.
`__ftol2` itself is the register-direct variant (`fistp qword` reads the 80-bit value; the `fst DWORD` inside
it is a float32 *probe* used by the correction chain, not a spill of the value being converted).

**The consequence.** With the constant's true bits (`3F25D867C3ECE2A5` = nearest double to 1/6000, BELOW it)
and no rounding until truncation: `AP * const` is exact and slightly below `AP/6000`, the reciprocal lands
slightly ABOVE `6000/AP`, truncation gives `floor(6000/AP)`. Non-divisors are safe because the distance from
an integer is at least `1/AP`, which dwarfs any ulp. Swept over AP 1..4000 against a double-rounded chain and
an extended-precision chain: **zero disagreements with `floor(6000/AP)`**. So the derived-HP case is integer
division plus the `AP == 0 -> 1000` branch, and lockstep now grades a CERTAIN value rather than a coin flip.

**The port's defect, named precisely.** Mirroring the formula as FP - `double scaled = ap * k; return
(int)(1.0 / scaled);` - returned **199** at AP=30 while the engine returns 200. Bit-level measurement of my
own build (sequenced prints, per the standing rule):

    k            bits=3f25d867c3ece2a5   (compiler's 1.0/6000.0 == the image's constant - the input was right)
    s = 30*k     bits=3f747ae147ae147b   (stored as DOUBLE: this is ABOVE 0.005 - the store INVERTED the direction)
    r = 1/s      bits=4069000000000000   (exactly 200.0 once the quotient is also rounded to double)

So 199 required a **mixed** chain: the product rounded to double (landing above the true value) and then the
reciprocal consumed at extended precision (199.9999999999999958, truncated down). Neither pure width produces
it - all-double rounds the reciprocal back to exactly 200.0, all-extended lands above 200. My earlier framing
("double-rounded = 300, 80-bit = 299") had both horns wrong, which is why I measured instead of choosing.

**Why not `long double`.** On MSVC `long double` IS `double`, so keeping it wide is not portable across the two
toolchains this file must build in, and the result would depend on each compiler's spill decisions. Integer
division is deterministic in both and provably equal over the whole reachable domain - and because C++
truncates toward zero, it also matches `__ftol2` for negative AP, which the suite now pins: `agility = -27`
(AP = -7) gives **-857**, where a floor-based implementation would give -858, an off-by-one in the opposite
direction from the claim I retracted.

The arc is worth keeping as one sentence: I asserted a law from a formula, the suite rejected it, I asserted
the inverse law from a transcribed constant, my peer corrected the constant by reading bits, the engine's
bytes then showed the whole computation is an integer division, and the port's remaining error turned out to be
a precision mixture no formula-matching could have exposed. **Every step was a source-of-truth question: the
print, the bits, the instruction sequence, and finally the arithmetic itself.**

## The string seam and the first inventory native (B16), for the same register

`ScriptNativeFetchString` stands in for the three-leg AMX fetch and preserves exactly the observables the legs
produce - unknown cell fails, `strlen >= bufSize` fails (0x59ab40's comparison is STRICT, so a 13-character
name is REJECTED by a 13-byte buffer), and the destination is untouched on failure - with no decode-error
outcome modelled, because `amx_GetString` returns 0 on every path and inventing a third failure mode would be
inventing behavior. `ScriptInventory` models what the matcher and vft slot 10 prove: case-insensitive name
match (0x46a777 `_stricmp`) and equippedness as membership in SIX slots at `GameInventory+0x14`
(0x46a8cc `while (v3 < 6)`), so an equip into slot 6 is invisible in the port exactly as it would be in the
engine. `IsItemEquipped` (0x45b750) lands on top of both, with its five shipped failure strings verbatim and
its check ORDER - resolve, fetch, cast - asserted by giving a non-creature an unfetchable name and watching it
report the NAME.
