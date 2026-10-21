---
status: open
assignee: WildZenith (dispatched by LoudDragon `01a15f82`, 2026-10-21 — onboarding row, tight scope by design)
type: research (read-only on ida_original_f3 + offline tools; NO IDB writes; findings dirty-until-checkpoint)
depends: T-070 (which NAMED this deferral: "the +0x2E0 first-deque walk is a SEPARATE message queue, named deferred, not conflated")
---

# T-074 — The Entity+0x2E0 message queue: what it carries, who feeds it, who drains it

## Objective
T-070's idle-resident work cleanly separated the +0x304 ACTION deque (count@+0x314, head@+0x308-block
cursor@+0x310) from a DIFFERENT structure walked at +0x2E0 in the pump path — explicitly not conflated,
explicitly deferred. One row, done properly.

## Questions
1. Structure: entries/stride/limit at Entity+0x2E0 (vs +0x304's deque anatomy), owner-class confirmation.
2. Feeders census (all stores/enqueues) and drainers census — is the idle-feeder's "drains the WHOLE
   deque" (@0x551bbe) acting on +0x304, +0x2E0, or both? (T-070 assumed; verify.)
3. Content: what IS a message here — pointer to what class? dispatch shape (which pump branch walks
   it first — the 0x54c570 consumer split).
4. Relationship to the animated-event queue the +0x319 raiser reads and to script message flow — map
   the THREE queue surfaces cleanly (this row's deliverable is a disambiguation table).
5. Port relevance: does our Entity model need this queue to reproduce R3/R4-adjacent behavior, or is
   it absorbed by existing seams? Name the answer; do not invent port work in this row.

## Rules
Read items 40-47 first (method law); ecx-flow available (selftest gate first); byte-over-decompiler
(item 43); EA-anchored everything; claim protocol (assignee+status+reserve+commit-claim BEFORE work);
deliverable discoveries/ file LEFT DIRTY; [DISCOVERY-BATCH ready] on completion. Two live lanes: touch
NEITHER KeenStorm's product files nor HappyEagle's debug harness.
