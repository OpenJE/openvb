---
status: in-progress
assignee: WildZenith `01a16092` (PULL per lane contract, PARK id R-VEG-E proposed by SwiftMoon in R-VFM-2 sec 3-successor, dispatched by LoudDragon `01a15f82` 2026-10-21 after R-AMX accept 84a9dad)
type: research (read-only; discoveries DIRTY-until-checkpoint)
depends: R-VFM, R-VFM-2 (both accepted — ParameterInfo grid, applicator band, the two FINAL-unknowns this row closes)
---

# R-VEG-E — VFX_Parameter value-cache roles + Holder 0x28-row anatomy + value-emit family

## Objective
Close the two FINAL-unknowns R-VFM-2 left with reasons printed, plus the family attribution:
1. **Value cache +0x20/+0x24/+0x28** (VFX_Parameter 52B base subobjects): the three -1 sentinels'
   roles — which is last-value, which is min/max clamp (R-VFM-2 proved live block {Loop byte,
   Min f32, Max f32} at HOLDER+0x30 — is the Parameter cache the same triple in different dress?),
   settle by READER/Writer containment per field, not type-guess.
2. **Holder 0x28-row element anatomy**: find-walks showed stride-0x28 rows; deliver the row field
   map via the row-ctor / row-allocator site (name string @+0x00? + what fills +0x04..+0x27).
3. **0x500Bxx-0x500F9x value-emit family**: the emit/eval functions that READ the cache + write
   instance values — attribution matrix: which type (0x6C/A4/DC subobject class + the encoded
   0xDC bool from R-VFM-2's self-errata) dispatches to which emit fn, EA-pinned.
   Completeness METHOD: containment (this lane's R-AMX method) — the family = exactly the call
   graph reachable from the applicator band's dispatch slots; state the anchor set + why closed.

## Deliver
`discoveries/2025-10-21-r-veg-e-value-cache-emit-family.md` (DIRTY): cache-field role table
(each dword: writers[], readers[], sentinel semantics, verdict), row anatomy table, emit-family
attribution matrix + closure argument, named port relevance (feeds audio/cursor port era per
PARK note — state what the port needs, invent nothing).

## Acceptance
Every claim EA-anchored; raw bytes over decompiler for all field roles (item 43); family closure
method stated with limits (item 41); R-VFM-2's yardsticks re-confirmed not re-guesses (0x28
stride, 52B Parameter, -1 sentinels); zero IDB writes; collisions honored (KeenStorm product,
HappyEagle harness, writer-seat tools/ files untouched — incl. the current tools/ecx-flow.py
unstaged foreign edit: NOT touched, NOT committed by this lane).

## Steps
1. Claim (this file + relay). 2. Field-role sweep: member_index + raw mov-sweep on 0x20/24/28(%reg)
in the 0x4F9000-0x502300 band; classify writer/reader chains. 3. Row-ctor hunt via allocator
(new(0x28)) xrefs in band. 4. Emit family: xrefs graph closure from vft slots of Holder/VFX
class; attribute per-type dispatch. 5. Write doc DIRTY, flip done, commit task-file-only, relay.

## References
discoveries/2025-10-21-r-vfm2-parameterinfo-applicator.md (banked), R-VFM doc, T-073, harness
items 40-47 (+52 honest-subtraction ethos), openvb-ida-mcp skill.
