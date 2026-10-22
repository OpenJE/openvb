---
status: in-progress
assignee: HappyStorm `01a16092-41cf-7127-936b-377a304178a4` (PULL per coordinator order 20:29, dispatched by LoudDragon `01a15f82` — writer seat, sole IDB writer for the wave duration)
type: WRITER WAVE (annotation tier — comments ONLY, zero renames, T2 delta 0 by construction)
depends: Q-W2a close (90a653e — gate lifted), T-059 split queue, T-086 §4.1/§4.2 decodes, R-CS §9 store-site notes
---

# R-ST — Split-annotation wave: T-059 StringTree artifacts + T-086 multi-host decodes + R-CS §9 store-sites

## Objective
Land the three named open annotation queues as ONE wave, comments only:

**[A] T-059 split-comment queue (the named pair)** — the two functions still wearing
`JE::StringTree::meth_*` live names that the T-059 RETRACTION proved are app-wide machinery:
- `0x4B2DD0` — the std::string SSO destructor (cap≥16→free, cap→0xF/len→0 rearm; 2,322-call fan;
  1,958 of the 2,062 "cls_0x4d8d70 pad owner" census — a T-036 artifact-floor mis-aggregation).
- `0x56EE40` — second confirmed artifact member (same live-name state, T-059 coordinator check).
Comment each: artifact-tier identity + the split lesson (item 43 function-granularity), citing the
T-059 amendment. NO rename (annotation wave; any rename is a future MINT decision with share-check).

**[B] T-086 decode notes (multi-host independent classes)**:
- `JE::GfxCameraHolder::ctor_0x4C0190` — six-host referencer census note (the six new(0x280)
  holder slots engine+0x18/+0x1C, GUI::Manager+0x8C, CharGen+0xB8, Inventory+0x290,
  DialogueWindow+0xD78; band-level share-check SHARING = referencer census, membership never
  claimed; §4.1 cite).
- `JE::Gfx_AnimMeshInstance::ctor_0x4A5BE0` — multi-host construction fan note (heap=4, ctor-sites
  4: VFX_Model ×3 + ModelInstance ×1; identity is RTTI-anchored, §4.2 cite).

**[C] R-CS §9 [E2] store-site pair (the two named in the wave brief)**:
- `0x58F0CF` (and the 0x58BE40 factory context) — "Client+0x8C: CDebugInterface instance from
  factory slot *0xA0 = virt_meth_0x58BE40 (sole ctor site 0x58BEC1, name 'Debug'); two Debug
  windows of ONE class — slots-vs-class identity (T-086 [E2] S1)".
- `0x58F114` — same note for Client+0x90 (second instance, differing window size 0x1A4×0x12C).

## Pre-flight (T-087 pattern, no self-waiver)
1. gate: ecx-flow selftest 6/6 @ pinned md5 `a68c22f3…` (offline leg) — FIRST.
2. writer-seat verification: mesh state + reservations clear; no other IDB writer seated.
3. baseline: IDB mtime snapshot + full-function census dump → `discoveries/wavedumps/rst-predump.json`
   (ea+name pairs, same schema as t0xx dumps); live==disk PROVEN by save-sync BEFORE first comment.
4. T2 baseline count (cls_ prefix census) recorded — expected unchanged at close (delta 0 by
   construction; any drift = revert candidate flagged).
5. per-target read-back: every comment lands at a LIVE function HEAD (item 43 writer rule;
   0x58F0CF/0x58F114 are INSTRUCTION sites inside Client::virt_meth_0x58F010 — those two land as
   set_cmt at the containing function head with the instruction EA IN the comment text, per the
   ratified T-060 rule).

## Apply + close
- set_cmt per site (existing comments: APPEND with ' | ' + provenance, never overwrite — writer
  law from T-055/T-058 waves); get_cmt read-back per site.
- mid-wave checkpoint save; END save via `ida_loader.save_database` + mtime + `rst-postdump.json`
  fresh dump proving disk==live + T2 delta check (must be 0) + 3-EA spot re-query.
- ledger: discoveries/2025-10-21-r-st-split-annotations-ledger.md (rows: target EA, head, current
  name, comment text, citation, read-back, save EAs) — writer-ledger tier, committed at wave close.
- successor line: what this wave's saves revealed (if any) + PARK relay.

## Exclusions (named, not silent)
- `JE::CSound::meth_0x539370` rename (R-CS §7.1) = MINT-tier, NOT annotation — stays queued.
- R-CS §7.2-7.6 comments = CSound epoch wave, not one of the three named queues — relayed as
  candidate wave-2 annotation batch, not applied here.
- T-059's 'residual ~104 pads over 19 band members' caution — census-wording level, no IDB site.
