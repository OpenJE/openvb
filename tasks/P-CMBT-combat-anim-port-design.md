---
status: in-progress
assignee: SwiftMoon `01a16092` (PULL ASSIGNED by LoudDragon `01a15f82` at `49e2cd1`, lane contract §2)
type: research/design (DESIGN-ONLY — no code, no product-lane edits; deliverable discoveries/ LEFT DIRTY, coordinator banks to docs/f3 at checkpoint)
depends: T-085 (raiser truth-table, anim deque), P-REV (landed transient arm — ALIGN-VERIFY only), T-074/T-080 (inbox + mailbox atlas), T-070/T-082 (transient idle), T-048/T-069 (latch), D-2 line
---

# P-CMBT — combat/anim-queue port design doc (T-013-design genre, next era)

Deliverable: `discoveries/2025-10-21-p-cmbt-combat-anim-port-design.md` — lands in
`docs/f3/` at the coordinator's checkpoint (research lane doesn't edit docs/).

## Steps
1. Claim file + reserves + commit (this file) — relay row flip.
2. Inputs read: T-085/T-074/T-080/T-082/P-REV + current port state (MapEntitySpawn post-P-REV,
   GameActionAttack damage-inline, CombatManager latch sites).
3. Self-verify the anchors this design stands on (raw): raiser store forms +0x319/+0x318 at
   0x54c828-0x54c86d, walk#1-before-walk#2 order, inline-fire predicate @0x5506ed, state-7 gate
   @0x5491c0. NOTE TO CARRY: docs' "+0x2E0+0x34" phrasing for the arm byte is arithmetic-wrong
   (+0x314 = the count field); raw is +0x318 — the design doc names the correction for the
   CombatTurnDispatch §raiser-block and the MapEntitySpawn.cpp:556 comment (annotation queue,
   product lane owns the cpp comment).
4. Write the design doc: ordered steps, per-step acceptance + gates, scope freeze (device-dependent
   LAST), open questions named, D-2/atlas appendices cite-not-re-derive.
5. Report [DISCOVERY-BATCH ready] + successor per contract §3.

## Notes (SwiftMoon, research-complete 2026-10-21)
Design doc written: discoveries/2025-10-21-p-cmbt-combat-anim-port-design.md LEFT DIRTY
(banks to docs/f3/ at checkpoint — research lane doesn't edit docs/). Six ordered steps, all
device-free bits first, anim deque LAST + value-gated; per-step acceptance incl. the P-REV
ALIGN-VERIFIES (step 2's q=-unchanged assert IS the transient-arm alignment check — the design
verifies the landed arm, never re-derives it). Chain: inbox standalone (inline-fire-when-ready,
skip-past NO-HOL, drain-fires-not-cancels) → damage routed through it (direct-HP-write deviation
retires; deferral both-orders pinned) → +0x171 mark → state-7 +0x1BC comment-alignment (cited-
absent anim-clearer scheduled with step 6) → mailbox seam + kind-7 transient bridge (shares
0x5a9070 with idle, per P-REV's stub law) → anim timeline frozen-gated.
RAW CATCHES THIS SWEEP (own reads at 0x54c828-0x54c86d, fed the errata list): budget field is
[ent+0x174] not '+0x179' (8b857401 @0x54c828); arm byte is [ent+0x318] not '+0x2E0+0x34'
(c6851803 at 0x54c85d-clear/0x54c86d-arm); the raiser pair sits on the +0x304 ACTION-walk tail —
the old '+0x2E0 animated-event queue' phrasing in CombatTurnDispatch/SimDigest (and the
MapEntitySpawn.cpp:556 comment) is superseded by T-074/T-085; three annotation items listed for
the T-079 lane (docs) + product lane (cpp comment), decisions NOT made here. Honest gaps: 5 named,
zero invented orders. Row flip relayed; read-only honored; zero src/ writes (comments quoted, not
edited).
