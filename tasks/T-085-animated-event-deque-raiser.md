---
status: in-progress
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
