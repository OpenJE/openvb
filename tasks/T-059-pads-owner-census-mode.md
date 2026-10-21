---
status: done
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

## ACCEPTANCE EVIDENCE (EpicRaven, 2026-10-21)

- `pads` mode committed in tools/ecx-flow.py; offline-only (raw EXE + F3.json; no 8745/IDB),
  md5-pinned cache unchanged (EXE md5 a68c22f3dd1c256ec9539a70e87cb434).
- Gate: selftest 5/5 PASS including NEW T4 = T-057 21-row PAD snapshot reproduced EXACTLY
  (15 @cls_0x50dbb0, 6 @cls_0x4542d0, 0 @cls_0x50db20) + 0-orphan invariant, via the executable
  `bucket_entries()` rule; evidence bit printed on every PAD/DELEG decision
  (`evidence=dataref@0x...` / delegation-chain note; pads-registration REQUIRES the .rdata/.data
  dword-ref, tail-jmp-only thunks can only reach DELEG).
- Regression: T-054 cases external counts unchanged (16/142/4); T-057 buckets identical
  (71/50/15/0, 73/10/0/0, 23/25/6/0) -- taxonomy now machine-checkable and checked.
- Pads census totals: 3,253 pads = 49 EH-call + 2,474 jmp + 730 adjusted-jmp; full 49-row
  per-pad owner table + aggregate findings in discoveries/2025-10-21-t059-pad-owner-census.md;
  README row added. py_compile clean.
