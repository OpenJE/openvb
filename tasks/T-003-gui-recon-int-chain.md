# T-003 — GUI RE recon: `.int` load chain (read-only)

status: open
assignee: —

## Objective
Produce `docs/f3/GUILoadingRecon.md`: a field-map/trace recon of how the original boots a
visible interface, so a later implementation task needs no more IDA time. **Documentation +
read-only IDA only** — no code changes, no renames outside your own namespaced functions.

## What to trace (entry points are real, from prior sessions)
- `Skin::LoadDefault_0x523490` → what it demands (ftype 1600 `.int` interface files via
  `JE::ResourceSystem`), the `.int` chunk/section layout, and where fragments/pictures land.
- `EngineStringSystem::Format_0x56be30` — NOTE: the decompiler fails on this function; work
  from disassembly + call sites (learn `f3_address_lookup`/xref tools in the meantime).
- `GUI::Picture` / `GUI::Fragment` construction paths reached from Demand; how strings from
  `English.stf` (already reimplemented: `JE::SS::StringSystem`) bind to widgets.
- Who calls the interface loader at startup (the menu you never see because the demo's GUI
  lane is unimplemented): state the call chain with EAs.

## Deliverable format
Follow `discoveries/` conventions: every claim with `0x…` evidence or marked *(inferred)*;
end with a "Gaps" section phrased as questions for implementer-A.

## Acceptance
- File exists, covers the four bullets, ≤ ~200 lines.
- A short chat/log.md entry summarizing the top 3 findings.

## Mentorship note
This task is deliberately the on-ramp to the GUI implementation lane (the eventual "visible
menu" milestone). When you finish, propose the first 3 implementation tasks yourself — I
(implementer-A) will review them in chat.
