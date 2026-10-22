# Shipped-string fidelity (`tools/verify-shipped-literals.py`)

Rule this enforces: **every string we describe as shipped carries a `// shipped string verbatim,
do not "fix"` marker in its own file _and_ a passing byte assertion against the original image.**
"We preserved it" and "it matches the original" are different claims; only the second is testable,
so the test is what makes the claim mean anything.

Run it (no build, no wine, seconds):

```sh
python3 tools/verify-shipped-literals.py              # assert all claims
python3 tools/verify-shipped-literals.py --self-test  # prove the checker can fail
python3 tools/verify-shipped-literals.py --survey     # propose unclaimed candidates
```

The shipped image comes from `$OPENVB_ORIG_EXE`. If it is missing the tool exits **2** and says so:
absent image means unverifiable, never verified.

## Where a claim may live

| Where | For | Why |
|---|---|---|
| `// shipped string verbatim, do not "fix"` above the call | log/assert text, one site | the marker is the reason the literal reads oddly to a later lint pass, so the warning belongs next to it |
| `tools/shipped-literals.txt` | shipped **data** identifiers (asset names, keys) used at several sites | one fidelity claim, not six marker comments; a wrong byte here fails silently at runtime, not in a suite |
| `--literal "..."` | one-off question in a shell | does not create a durable claim |

Manifest lines may be TAB-pinned to an EA when the same bytes legitimately occur more than once:
`"lit"<TAB>0x66079C`. Pinning is how an AMBIGUOUS verdict becomes a claim about a *specific* site.

## Verdicts

| Verdict | Meaning | Why it is a failure |
|---|---|---|
| `OK` | one whole match | — |
| `DRIFT` | 0 matches | our bytes are not the original's; someone "tidied" a shipped string |
| `AMBIGUOUS` | >1 matches | no site can be cited without an EA, so the claim proves nothing |
| `FRAGMENT` | matches as the **prefix** of a longer shipped string | our literal is not the string we think we ship |
| `PIN-MISS` | pinned EA holds no match | the citation, not the bytes, is wrong |

`FRAGMENT` is the case a hand search misses: a shortened quote still "found" the text and looked
correct. Whole-string here means the byte after the match is `0x00`.

## What a green run does NOT mean

It proves bytes and offsets. It does **not** prove we call the string from where the original does,
that behaviour matches, or that the exe runs — it is a string-fidelity gate, never an acceptance
result for behaviour. `--survey` output is advisory only: a hit says the bytes exist once in the
image, not that the string is load-bearing or ours to claim. Marking is a human decision.

## Provenance

T-021 nearly corrupted `"GameWeapon::CalculateDamage - No body part was targeted.\n"` to satisfy a
grep; it was confirmed byte-identical by hand at file offset `0x24fb48` (EA `0x64FB48`), which is
the same EA a decompiler reports. This tool automates that check and derives the EA from the PE
section table (`ImageBase` 0x400000 at optional-header `+0x1C` for magic 0x10B) instead of assuming
identity, because an assumed base mislabels every site it prints.

Two bugs found while building it, both silent and both worth remembering:
- discovery counted a literal **quoted inside a comment** as a shipped string, so comments are now
  stripped before reading literals — prose is not a claim;
- stripping changed the text length and **mis-numbered every reported line**, so comment removal
  preserves length and offsets now.


## Script-native error strings (B1/B3, 2026-10-22) - the shipped text is inconsistent ON PURPOSE

Byte-verbatim, all asserted by the port suite (T-022 identity):

- arity gate, two lines in order: `Invalid number of paramters passed to function %s().` (0x59af63) then
  `Expected %d parameters but recieved %d.` (0x59af70) - **`paramters` and `recieved` are shipped
  misspellings**, do not "fix" them.
- per-native resolve failures: `GetLevel() - Unable to resolve the requested entity.` (with `the`) versus
  `GetRace()/GetGender()/GetEthnicity()/GetSubRace() - Unable to resolve requested entity.` (without it) -
  GetLevel is the odd one out, measured at 0x45ac1e vs 0x45af7e/0x45aeee/0x45b016/0x45b0b6.
- per-native cast failures: `... - EntityID does not belong to a game creature.` (GetRace 0x45af21,
  GetGender 0x45afb1, GetEthnicity 0x45b049) versus `... - Entity ID does not belong to a game creature.`
  (GetLevel 0x45ac51, GetSubRace 0x45b0e9) - `EntityID` vs `Entity ID`, split across the same family.
- HP pair cast failures say `GameEntity`, not `game creature`: `GetCurrentHP() - Entity ID does not belong
  to a GameEntity.` (0x45b229), `GetMaxHP() - Entity ID does not belong to a GameEntity.` (0x45b189).
- **a shipped copy-paste bug, banked as-is**: `F3::GetPerkRank` (0x45ab50) reports
  `GetPosition() was unable to find the requested entity.` (0x45ab8e) when *its own* entity resolve fails -
  the message names a different function. Its cast failure line is correct
  (`GetPerkRank() - Entity ID does not belong to a game creature.`, 0x45abc1). The port reproduces the wrong
  string verbatim: fixing engine telemetry is not the reimplementation's job, and a parity diff that hides a
  shipped bug is worse than the bug.

## HP setter telemetry (B4, 2026-10-22)

- `meth_0x46d620` logs exactly `Server - Sending GAME_ID_ENTITY_SETCURRENTHP` (0x46d6a6) - no format args, and
  the packet id it fills is the literal `88` (0x46d675, `packet.mbr_0x30 = 88`).
- `meth_0x46ee10`'s failure paths (handle unresolvable at 0x46ee23, cast null at 0x46ee3e) return 0 with **no
  string at all** - unlike the script natives, the setter chain is silent on failure. Absence of a message is
  the shipped behavior; do not add one.
- `GameCreature::GetTotalSkill` default arm: `GameCreature::GetTotalSkill() - UNKNOWN SKILL: %d` (0x409830-era) -
  the only skill-name-leaking literal found so far; its numbers are still marked unverified in
  docs/f3/CreatureSkillAndPerkArrays.md.


## Script-native batch B literals, tranche 2 (B6, 2026-10-22) - and one EA in my own earlier entry corrected

- `GetItemType() - Entity is not an Item` (0x45b2b3) - **no trailing period**, unique in the batch.
- `GetUsableObjectType() - Entity is not an usable ubject` (0x45b4b3-era) - shipped TYPO (`ubject`) plus the
  "a/an" slip, both verbatim in the port.
- Resolve-failure phrasing has (at least) THREE shapes across one family:
  `Unable to resolve requested entity.` (most), `Unable to resolve the requested entity.` (GetLevel), and
  `was unable to find the entity.` (GetLockDC 0x45b5b2-era, IsItemEquipped, GetPerkRank's mangled cousin).
- GetLockDC's dual-cast line names both alternatives: `GetLockDC() entity passed in was neither a container nor
  a door.` (0x45b608-era) - the only native literal so far that mentions two classes.

**CORRECTION to my own entry above**: I cited GetPerkRank's copy-paste push as 0x45ab8e; the opcode starts at
**0x45ab88** (`push offset 0x65e2a0`) - my EA landed on the displacement byte of the same instruction. JadeBear
independently logged 0x5a95b0/0x551c4b as the same EA±1 class, so this is the third sample and the pattern is
loud: a locator read out of pseudocode is ~1-in-10 likely to be inside its own instruction, so quote the
disassembly's left-hand address or none at all.
