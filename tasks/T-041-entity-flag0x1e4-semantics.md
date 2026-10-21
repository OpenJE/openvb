---
status: open
assignee: unassigned (statics only; NOT a prerequisite for T-011 step 3)
depends: T-039 (found it), T-011 (uses it in a predicate)
---

# T-041 — What is `Entity+0x1E4`? Our name asserted a meaning the evidence does not support

## Known
- **Size 4** in the DB; the port declared a byte (widened during T-039's sweep, name demoted from `noTurn0x1E4` to
  `flag0x1E4`).
- ctor-zeroed at `0x55147b`; **unpacked from the entity record at `0x54e739`** — persisted state, not runtime-only.
- Readers gate **map membership**, not turns: `0x54bc3a` guards `--*(oldMap+280)` / `++*(newMap+280)`; `0x54947e` bails
  out of a movement update when it is 0.
- Appears in the turn predicate `+0x1E4 == 0 || +0x314 != 0` at `0x5a95c1`, whose second term is now established as the
  queued-action count.

## Question
Who **writes** `+0x1E4`, in whose caller chain, and which record field does it unpack from? Plausible readings include a
map-residency / in-world flag, a movement-enabled flag, or a spawn-phase marker. "No turn" is not among the supported ones.

## Method — and the trap that already bit once here
T-039's writer hunt failed because **the DB attributes stores made by embedded-container code to the container type**,
leaving the owner struct's `usages` list with readers only. So: start from the **unpack at `0x54e739`** and walk its
caller chain outward, and check the record layout on the `SpawnedEntity` side, rather than trusting an
`f3_member_index Entity 0x1E4` usage set (harness item 27). Statics before debugger (item 13).

## Acceptance
A cited writer set, the record field identified, and a name the evidence supports — or an explicit decision to keep
`flag0x1E4` as an honest placeholder. **Do not** alter the predicate at `0x5a95c1`: mirroring the shipped test at the
known offset and width is already correct.

## Boundary
Answering this does not gate T-011. If the semantics stay unresolved, the port mirrors the shipped test and the question
stays on the board — an unknown meaning is not a licence to invent one.
