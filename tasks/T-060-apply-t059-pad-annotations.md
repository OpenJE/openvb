---
status: in-progress
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
