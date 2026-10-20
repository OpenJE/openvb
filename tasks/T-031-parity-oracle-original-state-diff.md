---
status: open
assignee: unassigned (needs a lane with IDA + wine; propose implementer-B or IronStorm)
depends: T-026 (digest format exists), T-027 (seeded stream exists)
---

# T-031 — Parity oracle: read the ORIGINAL's state and diff it against ours

## Why
Every check we have proves **self-consistency**. `SimDigest` says so in its own header:
`DOES NOT PROVE=parity with F3.exe`. That is why a claim like "reaction ids are {33,34,37,38,40,41}"
survived four days of prose and died in twenty minutes of cross-checking — we argue because we cannot
diff. **This row is the single highest-leverage item on the board: it turns RE questions into comparisons.**

## What
Instrument the **original** `F3.exe` (`OPENVB_ORIG_EXE`, `${localPackages.f3demo}`) and dump the same
fields our digest prints, then diff. No GPU is required — state lives in memory, not in a back buffer.

1. Pick the read set from `docs/f3/SimDigest.md`'s field list so both sides print the same names.
2. Get the original's values by **lockstep debugging** (`devenv/scripts/debugIdaF3.nix`, GDB ports
   `31337`/`31338` already exist) or by a read-only memory dump at fixed frame boundaries. Prefer
   whatever needs no code injection into a shipped binary.
3. Emit an original-side digest file, same line grammar, and point `tools/diff-digest.sh` at
   `original vs ours` instead of `ours vs ours`.
4. **Seeding is the whole game**: the shipped generator is one global LCG seeded from `time32(NULL)`
   behind `byte_707C64`. A parity run must pin both sides to a known seed (T-027 gave us
   `SeedShippedRand`) or the streams diverge on frame 1 and nothing else is measurable.

## Acceptance
- One command produces `first divergence at line N, frame=M` between the **original's** state and ours,
  on a scripted sequence that reaches at least one combat round.
- It **must** be demonstrated to fail: deliberately corrupt one operand on our side and show the diff
  names that field, then restore (rule 2).
- The time-base question that T-026 surfaced gets answered here, not in prose: read `World+0x48` and the
  clock across the first `Update` of a new game on the original, and say what `ms` the pump passes
  (`Update_0x544a90` has no direct caller — vftable xrefs `0x668584`/`0x6919fc`).
- `docs/f3/SimDigest.md` gains a `PARITY=` line so the tool's boundary updates, and the digest row stops
  being the only place that says "not parity".

## Boundary (print it in the output)
A memory read sees what the shipped code *stored*, not what it *intended*: fields written for a
presentation layer we do not have will differ harmlessly, and identical values on a path we never reach
prove nothing. Divergence is a **question with a location**, not a verdict on the port.
