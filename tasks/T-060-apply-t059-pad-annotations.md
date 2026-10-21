---
status: done
assignee: CalmRaven (dispatched by LoudDragon `01a15f82`, 2026-10-21 — T-058 close, T-059 queue unblocked)
type: writer wave, ANNOTATION-TIER (comments only; sole IDB writer; T2 impact = ZERO)
depends: T-059 queue (discoveries doc `1de0403`+AMENDMENT, checkpointed `b3d5578`); author's md5-pinned doc = CITATION
---

# T-060 — Apply the T-059 pad/cleanup owner queue as IDB annotations

## Objective
Annotate SEH-cleanup owners and the corrected function-granularity reading, comments-only, NO renames:
1. 21 GUI-interface-ctor rows: `SEH-cleans@GUI::Interface (+0x4 embedded member) via cleanup helper 0x4F6D40` on each ctor SCOPE head.
2. Trio-releaser 0x401D60 scopes: `SEH/atexit-cleans via trio-releaser 0x401D60 (free([this+4]), zero +4/+8/+C)`.
3. 6 CRT locale/ios statics rows: proven-foreign boilerplate label.
4. +0xC4 atexit/embed cross-note (T-057 §3 embed slot == T-059 atexit destroy slot).
5. SPLIT-COMMENTS at 0x4B2DD0 and 0x56EE40 (both live-confirmed `JE::StringTree::meth_*`):
   `std::string SSO dtor, T-036 artifact-floor aggregation — not a StringTree member`. NO rename, T2-neutral. Cite t059 AMENDMENT.

## Item 43 rule 3 (NEW for this wave)
Re-verify each target/scope EA is a live FUNCTION HEAD before annotating. Any non-head locator in the
author's table -> HOLD that row + report; do NOT adjust to a neighbor head. Author is correcting locators.

## Acceptance
Ledger `discoveries/2025-10-21-t060-pad-annotation-apply-ledger.md`: per-row table (scope EA / target EA /
head-verified? / comment text / get_cmt readback), T2 before==after=0 stated, held rows listed if any.
Mid/end save + dump-verify. Same reconciliation rules as T-058 (`bec4ebc`).

## Acceptance evidence (delivered 2025-10-21)
- **Comments-only; T2 = 2,012 before AND after → DELTA 0.** 37 function heads commented, each `get_cmt`
  readback-verified post end-save (37/37). Renames 0.
- **Item 43 rule 3 head-verification: all 30 scopes + 2 targets are live heads; ZERO non-head locators, ZERO
  holds** (EpicRaven's up-front locator correction held).
- 21 GUI SEH-cleans@+0x4 (target 0x4F6D40 = `JE::GUI::Interface::meth_0x4f6d40`, verified calls 0x4B9D60+free)
  + 9 trio-releaser 0x401D60 scopes (body `free([+4]); zero +4/+8/+C`) + CRT 6-pads→**4 heads** labelled
  proven-foreign (head-granularity note) + +0xC4 embed/cleanup cross-note (atexit peak +0xC4×20 == T-057
  CEngineInterface embed slot `lea ecx,[esi+0C4h]@0x558775`; cleanup side reproduces embed geometry).
- **SPLIT-COMMENTS 0x4B2DD0 + 0x56EE40** (both live `JE::StringTree::meth_*`): body-verified generic std::string
  SSO dtors (0x4B2DD0 `cmp cap[+0x18],0x10/free([+4])/cap->0xF,len->0`; 0x56EE40 frees TWO SSO blocks +0x20/+0x4).
  Commented as T-036 artifact-floor aggregation, NOT StringTree members; **NO rename; b3 StringTree MINT
  UNAFFECTED** (its proof is `_Tree`/`_Tmap_traits` node layout). Cite t059 AMENDMENT.
- Ledger `discoveries/2025-10-21-t060-pad-annotation-apply-ledger.md`; claim `78ce017`; saves cp 16:04:32 +
  END 16:04:57, fresh-dump disk==live. Discrepancy surfaced+logged (not adjusted): 6 CRT pad-rows → 4 unique heads.

## QUEUE v2 supersedure (coordinator `4a81ef0`, applied mid-wave)
- v1 §2/§5 PAD application targets superseded; EpicRaven's locator audit: **20/21 PAD ctor locators are
  pad-reached cold CHUNKS, not call-heads** — annotate at the CONTAINING head, chunk EA in comment text.
- **My 37 v1 comments all confirmed direct-call heads** by the doc's own audit ("all 71 distinct locators are
  direct-call heads, queue-safe as printed" incl. 0x4B2DD0/0x56EE40); I had commented NO PAD chunk EA, so
  nothing to note-or-revert — the coordinator's safety net + my T-058 non-head refusal (`0x507550`→`0x5074D0`
  is literally a v2 re-anchor pair, vindicated).
- v2 NEW: **18 containment heads annotated** (17 chunk-containers incl sole call-head 0x4BB6B0, + in-game
  jmp-pad container 0x493CD0), each head live-verified function-start, chunk+pad+dataref cited.
- **item-43 HOLDS (3, routed not adjusted):** jmp-pad container `0x47A5CD`→`cls_0x668cd8` D3DX9 **vendor band**
  (binding exclusion, no stamping inside third-party); `0x60809F __Stof` + `0x60C817` **≥0x5FD000 out-of-region
  CRT/STL**. Buckets (T4 5/5, 21 PAD/0-orphan) are locator-independent — nothing reclassified under me.
- **FINAL: 55 function heads commented (37 v1 + 18 v2), 0 renames, T2 = 2,012 delta 0.** Saves v2-cp 16:10:07 +
  END 16:10:42; post-dump `discoveries/wavedumps/t060-postdump.json` re-verified T2=2012/T3=2169 → disk==live.
