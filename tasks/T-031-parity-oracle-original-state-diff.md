---
status: in progress — synthetic shipped side DELIVERED; the VALUES are still gated on T-034
assignee: HappyKnight (01a15be7), inherited from implementer-B/ZenRaven (01a15802)
depends: T-026 (digest format exists), T-027 (seeded stream exists)
---

# T-031 — Parity oracle: read the ORIGINAL's state and diff it against ours

## Status note (HappyKnight 01a15be7, 2026-10-21): the synthetic half is BUILT - `tools/orig-digest.sh`

`tools/orig-digest.sh` + `tools/orig-digest.py` render a **capture file** into the ours side's exact
line grammar, and `tools/orig-digest-verify.sh` proves it in 47 checks. Per the coordinator's two
hardenings:

1. **PROVENANCE is required input** (`--provenance` or a `record=provenance` line): none means exit 3
   and NOTHING written. Anything short of an earned `lockstep` claim emits
   `# SYNTHETIC - exercises the tool, proves nothing about F3.exe` in the artifact itself. A
   `lockstep` label is **checked, not believed**: it must name its reads, carry a `record=read` line
   per read (address / value / when / breakpoint - T-034's own columns), have those values appear in
   its own frames (r3 in some `q=`, r4b in some `vt=`, r1 in some `lcgstate=`), and cite
   `exe=<md5>` matching `artifacts/F3.json`. Stated limitation: **sloppy-proof, not forgery-proof**
   (the md5 is public); what it stops is a fixture being compared as evidence.
2. **Mixed provenance is refused by the comparator**: `tools/diff-digest.sh` prints
   `MIXED-PROVENANCE: no comparison performed` and exits 3 BEFORE comparing a line. Sanctioned
   pairings: `synthetic`/`synthetic`, `lockstep`/`unrecorded`, `unrecorded`/`unrecorded`.
3. **The fixture violates invariants so the refusals are proven to fire**: an out-of-order frame
   (`frame=99` after `frame=100`) gives `INVARIANT-VIOLATION` exit 4 with nothing written; an action
   whose vftptr is in no class prints `UNKNOWN_VT=0xdeadbeef` AND counts it in
   `# AK-RESOLUTION=4/5`, and `--require-resolved-ak` turns that into a refusal. Also refused:
   unknown record type or field, non-dense entity slots, `vt` not exactly four entries, `q` above
   what the capture's own pointers imply, and a capture with no frames at all - an empty digest would
   otherwise compare IDENTICAL to any other empty digest.

Grammar decisions worth knowing from here: vftptr to class resolution is offline through the DB (the
half of R4b that needs no debugger); `draws` prints `NA:no-counter-on-shipped-generator` and the
comparator now separates a **COVERAGE** difference (NA vs a value, or two NA reasons) from a state
divergence - without that, every ours-vs-original run reports the same fake finding; the pinned R1
state rides in CONTEXT as `lcgState=`; and the contract block is a deliberate copy, asserted
byte-for-byte against a real binary-produced digest by `verify` - that check found the CRLF/LF bug
the day it was written, where an LF-only emitter makes EVERY frame of a real comparison diverge by one
invisible byte.

**What is still not here, unchanged from the note below:** the values.
`tools/scripts/orig-capture-synthetic.txt` is typed, not read, and says so. The acceptance line
`first divergence at line N, frame=M` **against the original** still needs T-034's R1/R3/R4b - and
that row's field list is now corrected (`+0x314` = COUNT, `+0x310` = head index), so the capture side
will not be handed bad labels. The oracle's boundary is printed in every file it emits.

## Status note (coordinator, 2026-10-21): stalled at the read boundary

**Grammar side is landed** (`d43033b`): digest fields split into COMPARED vs CONTEXT, action records compared by
**class** via vftptr through `F3.json` instead of pointers or kind tags. **The shipped side is not**, and that is not a
capacity problem — it is a dependency: `tools/orig-digest.sh` needs the T-034 values (R1 pinned LCG state, R3 deque
depth, R4 pre/post-call owner value), which are NiceHawk's and currently unreported. ZenRaven has been away since the
20:15 commits, so this row is **blocked on reads, not on will**. Do not treat the oracle as progressing until either
T-034 reports or a lane picks up `orig-digest.sh` against synthetic input and says plainly that it is testing the
comparator, not parity.

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
   `original vs ours` instead of `ours vs ours` — **done as the renderer** (`tools/orig-digest.sh`);
   the values are not.
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

## Design decided before code (docs/f3/ParityOracleDesign.md)

- **Seeding = capture-and-replay (read once, pin from the captured state), not forced reseed.** One read
  instead of two writes plus proof the writes landed; `byte_707C64` gates re-seeding, so writing only the
  state dword can be clobbered by the lazy `time32` path at 0x48ccb. The pinned value is then a measured
  constant, reproducible by later sessions with no debugger at all.
  **Boundary to print in the output: this proves SAME-STATE parity, not same-seed-from-clock parity.**
  "Identical under a pinned state, divergent in real time" is the honest claim.
- **Acceptance part 3 (the time-base question) is CLOSED by statics, no debugger spent** —
  `docs/f3/WorldTickPump.md`: the pump is `JE::Server::virt_meth_0x552b60`, which loads
  `JE_TickMilliseconds_0x56af50()` (0x552bad) and calls World **vftable +24** with it (0x552bb5), so `ms`
  is absolute machine uptime; and `JE::World::virt_meth_0x546690` zeroes `+0x48` (0x5466b5) while seeding
  `+0x4C`/`+0x54` (0x5466bd/0x5466c8). The original takes the same frame-0 leap our port takes ⇒ **mirror
  it, do not "fix" it**, and `clock` is therefore a valid comparison field.
- **Reads still owed (R1–R4)**, written as address/value/when in the design doc, for one clean session:
  R1 the LCG state at the first draw, R2 `Gregorian::totalSeconds` at the same instant and at frame 1,
  **R3 the live entity's `+0x304` deque (block ptr / capacity / element index) mid-combat-round** — the
  only check of the `actions[4]` "first block" assumption against real play, and if the index exceeds 4
  our cap is a dropped order rather than a stylistic deviation — **R4 the enqueued action's `+0x14`**,
  which must equal the entity pointer, confirming the seam fires on the *script-native* path we do not
  reach yet.
- Two fields the oracle must handle explicitly: `ak` and `tickMs`.
  **`ak` is DECIDED** (docs/f3/ParityOracleDesign.md §4b): exclude it from the comparison by default AND
  print the exclusion in the file, while capturing the action's **vftptr in a separate non-hashed CONTEXT
  column** — so a verified kind<->vftptr table obtained later (from the shipped vftables, not from our kind
  numbering) promotes the column to a comparison field WITHOUT re-running any debugger session. I had
  framed this as map-or-exclude and escalated it; the coordinator sent it back with the rule that became
  harness item 23: *when a field cannot be compared, decide whether the goal is a comparison now or a
  measurement later — and do not invent a mapping to make it comparable.* My two options would have made
  the action queue permanently unverified on the shipped side (exclude) or fabricated a mapping (map);
  exclude-and-capture makes it temporary AND honest.
  `tickMs` uses the same CONTEXT convention (uptime is incomparable across processes).

## Also landed while here: the render-device probe (`tools/render-device-probe.sh`)

Takes the M-001 verifiability question off prose. Three cases (no DISPLAY / Xvfb+device off / Xvfb+device
on), prints wine exit codes, the device trace line when present, and the wined3d diagnostics, with the
boundary in its own output. Current verdict on this host:

```
case 1 no-DISPLAY:    exit=1    diag: nodrv_CreateWindow
case 2 xvfb, OFF:     exit=124  trace: RenderDevice: device path off ...
case 3 xvfb, ON:      exit=1    diag: Failed to find a suitable pixel format
                                  diag: Failed to get a GL context
```

So **even on a virtual X server with software GL asked for explicitly, wined3d cannot obtain a GL context
here** ⇒ M-001 slice 1's *visual* acceptance is a harness limit, not our code, and case 3 needs an exe
built with the render device (currently the scratch worktree only) — the tool reports UNKNOWN rather than
FAIL when the opt-in is absent, which is why that distinction is in the script and not just here.
