---
status: in-progress
assignee: EpicRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — their own offer, pre-loaded data)
type: offline analysis (tools/ecx-flow.py against the md5-pinned cache; NO IDB writes ever from this lane)
depends: T-054 (the tool), T-055 in flight (any MINT-READY verdicts queue BEHIND CalmRaven's wave — writer slot is theirs)
---

# T-057 — Re-adjudicate the registered escalation families with the four-join tool

## Objective

The standing escalations are the bulk of the remaining T2 mass (50dbb0 120, 55dc80's 83 clones —
already ruled foreign-TU _Tree, 50db20 79, 4542d0 50). They were escalated under pre-T-054 evidence
rules; item 41 now makes "no owner" notes falsifiable in three independent queries. Answer them with
the tool, not re-scans.

## Scope (per EpicRaven's pre-load)

1. **cls_0x50dbb0** — clone census by stride/size-shape (repeat counts 0x329/0x1d1/0xbe/0xb7/0xd3/0x3e
   already measured) + `band --cls` owner answer + slot-dispatch census. Verdict per family: confirmed
   std::_Tree clones in named foreign owners (list the owners) vs any genuinely-unknown host.
2. **cls_0x50db20** — the CEngineInterface-embeds question (T2-12): re-run with the embed-tier proof
   (+0x84/+0x8c/+0x8d member stores keyed by owner in the flow cache) instead of the member-name guess.
3. **cls_0x4542d0** — one run, same shape.
4. Any candidate that comes out MINT-READY goes to the coordinator's QUEUE (writer slot belongs to
   T-055 until CalmRaven closes; nothing gets applied from this row directly).

## Acceptance

- discoveries file: per-family verdicts answering item-41's three queries explicitly for every
  negative; owner lists for the clone families; MINT-READY queue separated; gate-selftest-first
  wording (selftest green @ pinned md5 before any verdict).
- Board effect: T-033's "remaining T2" decomposition gets updated wording (confirmed-foreign vs
  owner-now-known), which is the census honesty the chain was closed on.
