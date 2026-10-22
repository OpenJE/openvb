---
status: landed (gates green on this payload; transient arm + walk-continue + register folds)
assignee: KeenStorm (PULL per contract from tasks/PARK.md top row; dispatched relay to LoudDragon)
type: implementation (product code, submodule-first, full gate law incl. msvc — headers move)
depends: T-082 §1-§8 (the transient verdict), T-080 (shared-pool stub, Message kind 7), T-070/T-071 (the port being revised), WildZenith gen-5 confirmation
---

# P-REV — Port the transient-idle semantics: T-071's resident becomes a one-tick arm

## Scope (from the PARK row + T-082 §1/§6/§7 queues — evidence lives in those docs, cite don't re-derive)

1. **Idle completes on its first processed tick.** Raw law: 0x5a9070 = `c6 41 18 01` — the slot-5
   update of BOTH GameActionIdle (0x6497c4) and ActionMessage (0x6a5f34) is the UNCONDITIONAL
   done-write of the very byte predicate 0x404b60 reads (+0x18; ctor 0x56018d zeroes it). Start
   (activation observables: world-ms stamp, QueueAnimEvent 9-then-1) still fires once BEFORE the
   update — T-082 §13's order preserved; only the lifetime was wrong.
2. **Walk-continue on completion.** The shipped walk (0x54c7f6 predicate -> erase -> deldtor ->
   ADVANCE, loop) processes the next entry in the SAME frame after an instant finish; break only
   on unfinished-head. Our FIFO (`&& !acted`) currently stops after one slot per tick — a one-tick
   skew against shipped that only becomes observable with a transient occupant. Mirror the shape.
3. **The +0x319 raiser**: with transient idle the kind-6 exclusion becomes vacuous (the branch
   always clears its slot) — comment re-cites 0x5a9070; the exclusion guard retires honestly
   rather than silently.
4. **Errata + register folds (the §6/§7 pending items):**
   - Factory case-0 alloc size: `0x20` -> **0x2C** (raw push @0x46f2b9; engine-side Die is 0x1C —
     two factories, two sizes, now a named fact).
   - Shared-pool vft note: idx1/2/3/5/6 identical across Idle/ActionMessage/Wait-family
     (0x404b60/0x404b70/0x4010d0/0x5a9070-or-timed/0x404bb0); F3.json under-registers shared
     stubs under the first-analyzed class — the lossy-mirror warning goes WHERE the mapping
     table lives.
   - Message kind 7: engine-side ActionMessage via default passthrough (0x46f4dc, kind
     UNMODIFIED), same stub, one-tick finisher — a named future kind, not a ported one.
5. **Digest-count implications**: steady-state q excludes idle (0 after first tick); R3
   state-agreement is an INSERT-WINDOW reading (T-056 sampled at the funnel bp = same window).
   Driver gains a `peek` verb (write current frame + advance number, NO tick) so the tool can
   sample the insert window honestly; t071-r3-pair.txt re-armed to peek BEFORE run.
6. **SpawnMove assert extensions**: transient asserts (done byte set before erase; idle absent
   frame-after; push-behind-idle now FIRES at ≤ the next tick — replacing the stall asserts),
   walk-continue same-frame assert, re-arm-after-rebase assert. The T-071 block's (c) flips from
   HOL-stall to transient-pass-through — the correction lands IN THE TEST, not in prose.

## Not in scope (queued elsewhere, unchanged)
RE-REOPEN (run-H stall phenomenon mechanism — the byte verdict says NOT idle-blocking; candidates
stay named in T-082 §queue); T-082 itself (parked at its byte, not touched by this row); the
banked docs' prose (T-079 batch lane).

## Gates (full law — src/ headers move)
run-all 9/9 with raised floors, cppcheck 0, msvc rc0, gate-integrity rc0, sim-driver-verify PASS,
submodule-first (SpawnMove test in openje), both repos clean at close.
