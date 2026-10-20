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
