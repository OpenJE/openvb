---
status: landed (gates green; R3 state-agreement achieved IN THE TOOL)
assignee: KeenStorm (self-proposed §5(a) sketch; coordinator-approved, same author-knowledge lane as T-066)
type: implementation (product code, submodule-first, full gate law incl. msvc when headers move)
depends: T-070 (the mechanism is RE'd — this row ports it), T-067/R5-R6+4 live confirms (behavioral backing optional, statics sufficient to start)
---

# T-071 — Port the GameActionIdle resident: never-finishing baseline + drain-then-feed at command boundaries

## Mechanism to port (all EA-pinned in discoveries/2025-10-21-t070-idle-resident-rootcause.md — cite, don't re-derive)
1. **Idle action kind exists**: one construction analogue of factory case 1 (0x46f290), never-finishing
   (completion byte +0x18 stays 0; own update consumes 0ms, sets +0x16), activation stamps entity+0x184
   with world-ms + applies states 9-then-1 — port the OBSERVABLES, not the 141-case detour unless
   state 9/1 matters to existing consumers (check first, log choice).
2. **Rebase semantics**: the drain-whole-deque-then-feed-idle pattern (meth_0x551ba0 @0x551bbe->0x551be4)
   at spawn/rebase — both net contexts. Port as ONE seam ('RebaseOntoIdle') called where our equivalents
   exist (spawn + the world step-5 path); if our architecture has no rebase trigger, the seam ships
   UNCALLED and documented (mirror-and-cite, no invented caller — T-019 discipline).
3. **Head-of-line consequence**: pump breaks on unfinished head — verify OUR pump does the same (it
   mirrors 0x54c570); then queue-depth cap D-2's '>4' re-expressed as 3+resident under this protocol.
4. **Fix the kind NUMBERING** (ours 1=MoveTo vs shipped 2): align ActionKindName enum values to the
   shipped factory table (0=Die 1=Idle 2=MoveTo 3=PickUp 4=Wander 5=Wait 6/7=passthrough 8=FloatText
   9=Equip 10=Unequip 11=SetActiveWeapon; Attack NOT a factory kind) OR map explicitly — pick one,
   comment the choice with the table's EAs.

## Acceptance
- New/changed tests: idle resident occupies head and BLOCKS later orders until a drain; drain-then-feed
  idempotence; numbering round-trip assert; R3-style digest now shows state (resident vs real order),
  not just count — closing the T-068 §4 correction in the DIGEST itself.
- Gates: run-all-tests >= floors, cppcheck 0, msvc rc0 if headers move cross-TU (item 37), submodule-first,
  both repos clean, sim-driver-verify stays PASS (0 new SetRoundMode sites).
- Parity note: rerun the T-068 diff table after landing — the R3 row should upgrade from count-agreement
  to STATE agreement, or the residual delta is logged.
