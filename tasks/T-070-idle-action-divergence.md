---
status: open
assignee: KeenStorm (self-queued at T-068 close; coordinator-approved)
type: research (read-only on ida_original_f3, discoveries/ dirty-until-checkpoint)
depends: T-068 finding #2 (the divergence this row root-causes)
---

# T-070 — The shipped GameActionIdle baseline resident: who enqueues it, and what advances on it

## Established by T-068 (do not re-derive)
Shipped first-insert class = GameActionIdle (vft 0x6497C4, class .?AUGameActionIdle@@) on 12/12 sampled
entities at frame-0; later inserts ActionAnim. Our enum has NO idle kind — port models idle as ABSENCE.

## Questions (statics-first; live confirmation deferred to T-067/R5 if needed)
1. WHO enqueues the Idle action at frame 0 — the same funnel 0x550720? Which caller (entity ctor?
   world-spawn? script-binding?) and with what fields (owner back-link +0x14, target slots).
2. What CONSUMES it: the animated-event queue pump, the combat raiser (+0x319 — pin the CONSUMER half,
   item 45), or TurnPrep? If Idle's OnUpdate is the shipped "advance one frame doing nothing" step,
   idle-as-absence is NOT equivalent and the divergence is behavioral, not cosmetic.
3. Class anatomy: GameActionIdle size/ctor/OnUpdate/OnCreate bodies (vft slot 0 0x405690, slot 4 0x405620
   per the export) — one table, EA-anchored.
4. Verdict: does the port NEED an Idle action kind, or a documented deviation that PROVES absence-
   equivalence (a test that our no-action frame produces identical downstream effects on the R3/R4
   observable surface)? Both outcomes are acceptable; a guess is not.

## Acceptance
discoveries/ file (dirty): caller census + consumer map + anatomy table + verdict with the named
follow-up (port row OR deviation-test) — coordinator checkpoints; any queue feeds first-in-writer.
