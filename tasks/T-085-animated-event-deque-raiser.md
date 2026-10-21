---
status: done
assignee: WildZenith (dispatched by LoudDragon `01a15f82`, 2026-10-21 — the fourth cell of YOUR queue map)
type: research (read-only; discoveries DIRTY-until-checkpoint)
depends: T-074 (three-queue table — +0x2E0 done, +0x304 done, +0x37C done via T-080; the ANIMATED-EVENT deque at +0x1A8 got structure-only)
---

# T-085 — The +0x1A8 animated-event deque and the +0x319 raiser chain: anatomy of the last queue surface

The raiser every combat row cites (T-048/T-050/T-051/T-071 — 'sits on the ANIM-event deque @0x54c833')
has NEVER been anatomized. Deliver:
1. Record layout (24B by-value; kind? the +0xC duration field your T-074 noted; payload fields) + the
   Queue_* feeder families mapped by CALLER CLASS (100+ sites is a census, not a list — group by host).
2. RAISER CHAIN byte-verified BOTH branches: 0x54c833 (event remaining < event+0xC => not-finished) and
   0x54c864 (exhausted with dt budget); what exactly writes/clears Entity+0x319 and who reads it besides
   the walk consumer 0x5a955d (item-45 halves named per site).
3. The explicit-time path (+0x14, T-043 handle composed) — what 'explicit-time events' are, and the
   re-entrancy question: could this deque's pump ever RE-enter the manager (T-050's live-refutation shape,
   pre-armed for T-077: exact bp sites + what each would mean).
4. Anim-start sink 0x548d80 relationship; drain-vs-pop semantics vs the other three queues.

Acceptance: discoveries file (DIRTY) with layout table, raiser truth-table, feeder census, and a
'T-077 bp-prearm' box the boot lane can lift verbatim. Same contracts; your four-queue map is the frame.


## Notes (closure)

**DONE 2026-10-21 (WildZenith `01a16092`).** Deliverable: `discoveries/2025-10-21-t085-animated-event-deque.md` (left DIRTY for checkpoint). Headlines, EA-pinned in doc:
(1) RECORD: 24B by-value blocks (sub_54EB40 new(0x18)+qmemcpy @0x54eb80-96; sole push = sub_5505F0 @0x5506b0; xrefs to 0x54EB40 = 2 total); layout {+0/+4 anim name pair (resolver [vft+0x48] @0x550630/0x548dc4), +8 MODE/count (0=loop -> [ent+0x188]=1 @0x548eab; >=1 explicit-time), +C sync-flag -> mesh+0x90, +10 EXPLICIT-MS ([ent+0x180]=worldms-a6 @0x548e3a; duration x -1000 @0x548e79), +14 pop-flag -> clears [ent+0x1BC] @0x54c93d/9f0}.
(2) FEEDER CENSUS 267 sites complete, one primitive, grouped by host (87 meth_0x463c80 / 53 meth_0x464d70 / 24 Walk / 24 Limp / 21 ToCombat / 11 Idle / 10 ToDefault / rest wrappers); front door = GameEntity::QueueAnimEvent_4673d0 (100+ ANIMEVENT ids, verbatim \"Unhandled ANIMEVENT.\\n\" @0x467536, [ent+0x9C] suppress gate) — RE-TIMING of T-070's reading: it is the anim-event QUEUE front door, not a 141-case state setter. Kind-6 ActionAnim start = BYPASS (direct sink @0x5a92a1, clears [ent+0x288] queue gate, own deadline [act+0x24] @0x5a92d7); full 19-site sink census recorded.
(3) RAISER TRUTH TABLE both branches byte-verified: R1 @0x54c833 = head-not-finished AND [ent+0x174] < [head+0xC] (raw cmp @0x54c828-31); exhausted pair: arm @0x54c86d ([+0x318]=1) -> SECOND occurrence raises @0x54c864; CLEARER = virt_meth_0x549090 ([vft+0x60] of base vft 0x692084; blind-clear + [ent+0x300]=ms stamp) called by manager maintenance for [ent+0x1E0]==5 (@0x5a94fd) and GameCreature turn accounting sub_409BE0 @0x409bfa; CONSUMER = 0x5a955d in cls_0x5a9330::meth_0x5a94a0 (sole caller World::virt_meth_0x545d60 @0x545e0d; branch gated on manager mbr_0x4 round-MODE != 0 — the T-051 dead-consumer half). Item-45 halves per site: R1/E3/C1 producer-side shipped-LIVE; X1 consumer-side dead-by-config. Pump dispatch = [vft+0xC8] via F3::ServerUpdate @0x549130 states 4/5/7 — state 7 gated by [ent+0x1BC] (THE record pop-flag) @0x5491c0.
(4) DRAIN-VS-POP: one-per-frame pop; Path A inline advance @0x54c8df-909 / Path B helper meth_0x54b590 @0x54c9c1 (byte-identical semantics); NO drain-all for this deque — unfired events die with block-free @0x550553 (pure value semantics). Default-anim fallback @0x54c9fe ([+0x295] never written post-ctor = permanently open) explains boot-time ActionAnim sightings (R4b pairing context).
(5) T-077 BP-PREARM BOX doc section 6: bp7 0x54c833 / bp8 0x54c864 / bp9 0x549090 / bp10 0x5a955d (THE refutation event) / bp11 0x5a94a0 mbr_0x8 re-entrancy depth / bp12 0x54c9c1+0x54c92a path discriminator / bp13 0x548d80 sink values; meanings state plainly: raiser hits WITHOUT bp10 = 'latch raised, consumer gated-off by mode' = consistent with T-051; bp11 depth staying 1 = re-entrancy closed-as-unobserved, NOT disproven.
(6) CROSS-NOTES: KeenStorm's T-082 transient-idle correction independently raw-confirmed from my own dump (0x5a9070 = c6 41 18 01 shared idle/Message update stub); relay-parenthetical SWAP of 0x54c390/0x54b3f0 flagged with re-dumped bytes (doc header) — T-074/T-080 attributions as committed stand.
(7) Port verdict NAMED: no existing seam needs the anim deque; atlas four-queue table closes the structure-only cell.
Posture: read-only honored; ecx-flow selftest 6/6 before verdicts; raw-over-decompiler with export band-noise catches (0x5ab3e0 false-usage flagged).
