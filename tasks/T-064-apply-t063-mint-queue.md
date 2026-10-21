---
status: open
assignee: CalmRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — writer slot, first-in per standing rule)
type: writer wave (sole WRITER on ida_original_f3; T-055 rules verbatim)
depends: T-063 queue (doc `b3ba8e7`-era, coordinator checkpointed)
---

# T-064 — Apply the T-063 MINT-READY queue (15 fns, census-era follow-through)

## Scope (source: discoveries/2025-10-21-t063-conservative-half-attribution.md MINT queue)
- [A] `cls_0x48fb60`+`cls_0x48fce0` band (10 fns) -> member-class-at-G3D_Node+0xA0 naming. Construction-tier
  proof: LIVE-named `JE::G3D_Node::ctor_0x4d7600` site 0x4D7689 `lea ecx,[esi+0xA0]; call 0x48FF20`.
  Pick the name per your writer judgment (no shipped string => your call, collision-check like
  Gfx_BufferManager; cite EpicRaven's doc + this site).
- [B] `cls_0x558740` residual 5 -> CDebugInterface extension (live-named meth callers 0x558190/0x558510
  + CEngineInterface-methods calling with this-0xB0/0xC4 view-adjust = item-42 geometry).
  0x434760 is WEAK-TIER: writer discretion — verify containment yourself or hold-and-note.

## Rules (unchanged, restated because this is renames not comments)
Re-verify every claim at write time (item-42 RTTI-size checks before naming; item-41 negatives
answer three queries if anything fails); per-fn ledger table with tier labels; T2 BEFORE/AFTER
itemized by [A]/[B]/held; mid-wave + end-of-wave save_database + mtime + fresh-dump disk==live;
registered exclusions untouched. Any rename that fails YOUR re-check: label-and-skip, itemize.

## Acceptance
Ledger `discoveries/2025-10-21-t064-*.md` + row flip + [DISCOVERY-BATCH ready]. Census note for the
board on close: T2 with the 15 applied => expected ~1,997 (state the real number, not the forecast).
