---
status: done
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

## Notes

**DONE 2026-10-21 (WildZenith `01a16092`).** Deliverable: `discoveries/2025-10-21-t074-entity-2e0.md` (left DIRTY for coordinator checkpoint).
Headline verdicts, EA-anchored in the doc: (1) +0x2E0..+0x2F4 = the SAME 20-byte deque container family as +0x304 (push_back 0x54ea00, erase 0x54b5c0, count +0x2F0, ++count @0x54ea71 shared), entries = **Effect-tree root pointers** (RTTI'd `JE::Effect` base 72B; done-predicate `[vft+0]` = 0x45c090 `mov al,[ecx+0xB]; ret` raw-verified; fire on finish via `[vft+0x2C]` @0x55a501; owner link = +0x44 stamp of `&Entity+0x1DC`, resolved by 0x54be10). (2) **Idle-feeder drain @0x551bbe drains +0x304 ONLY — verified, not assumed** (raw `lea 0x304(%ebp)` @0x54c44d, zero +0x2E0 refs in 0x54c440); the inbox has its own full drain `meth_0x54c390`, sole caller = entity reset @0x5501a2. (3) Feeders census complete (push xrefs = 0x55070c via `meth_0x5506c0` public API w/ 6 senders, + 0x552692 spawn replay). (4) Pump walk #1 is FIRST, NO head-of-line block (erase keeps cursor, unfinished skips via `inc %esi` @0x54c63d — contrast walk #2's break @0x54c7f6); sets `Entity+0x171`, consumed by `World::virt_meth_0x545d60` @0x545dca → `meth_0x54aa20` re-scan (export missed these readers — item 27 blind spot recovered). (5) Three-queue table delivered in doc §5 (+0x2E0 effects / +0x304 actions / +0x1A8 by-value 24B animation records via `sub_54EB40`, fed by the `Queue_*` family incl. `Queue_Idle_462c50`; +0x37C OnMessage list named as adjacent 4th surface with honest unresolved inserters tail). (6) Port verdict NAMED not invented: no paired observable (R3/R4/+0x319/T-071) exercises this deque; T-071 confirmed unaffected; deferred-through-inbox damage application + +0x171 re-scan + busy-math 0x54ab00 counted — all named for the combat/effect port era, zero port work created here.
Method posture: ecx-flow selftest 5/5 BEFORE verdicts, md5-pinned exe a68c22f3…, raw-over-decompiler (two export mislabels corrected per item 43), read-only IDB honored (zero writes), item 40 arg-corruption routed around via objdump.

## Rules
Read items 40-47 first (method law); ecx-flow available (selftest gate first); byte-over-decompiler
(item 43); EA-anchored everything; claim protocol (assignee+status+reserve+commit-claim BEFORE work);
deliverable discoveries/ file LEFT DIRTY; [DISCOVERY-BATCH ready] on completion. Two live lanes: touch
NEITHER KeenStorm's product files nor HappyEagle's debug harness.
