---
status: in-progress
assignee: EpicRaven (offered during T-057 arbitration, pre-approved by LoudDragon `01a15f82`)
type: tooling (offline, tools/ecx-flow.py extension; NO IDB writes)
depends: T-057 (families mode + installer data-refs already emitted)
---

# T-059 — `pads` report mode: standing EH/vector-cleanup owner census

## Objective
The 49 EH-call pads (21 counted PAD in T-057's buckets; the wider pad-thunk set) name per-class
destructor-chain cleanup owners for the big interface classes (e.g., GameCSSkillMenu +0xD58 member
destruction). Turn pad attribution into a first-class, one-command report so band censuses never
re-count them and cleanup-owner questions are answerable without new join classes.

## Steps
1. Add `python3 tools/ecx-flow.py pads` report mode: enumerate pad thunks (SEH __except_handler
   shapes, atexit registrations, vector-iterator dtor-call pads), attach the data-ref evidence
   (.rdata/.data dword refs to pad VA per T-057 rule), map each to the owning class via the
   enclosing band + installed vft, and emit the PAD-vs-DELEG bucket decision with the evidence bit
   that drove it.
2. Selftest: T-057's 21 PAD rows must reproduce exactly under the mode (gate before verdicts as
   always); the 0-ORPHAN invariant must hold.
3. Run against the 49-set: per-pad owner census table into discoveries/.

## Acceptance
- Mode committed in tools/ecx-flow.py (offline-only, md5-pinned cache discipline unchanged), T-057
  21-row selftest reproduction, full pad-owner table in a discoveries file, coordinator checkpoint.
- Board effect: harness item 41 gains a tool citation for pad/deleg classification; future band
  censuses quote `pads` output instead of re-deriving.
