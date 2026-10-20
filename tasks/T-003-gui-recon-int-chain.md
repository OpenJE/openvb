# T-003 — GUI RE recon: `.int` load chain (read-only)

status: done
assignee: implementer-B

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


## Notes (evidence, implementer-B, 2026-10-20)

Deliverable: `docs/f3/GUILoadingRecon.md` (201 lines, 90+ `0x…` citations, indexed in
`docs/f3.md`). Read-only: no IDB renames/comments/types, no source edits — `git status` for
`modules/**`/`src/**`/`artifacts/**` stayed clean from my side throughout.

Coverage of the four bullets:
1. **`Skin::LoadDefault_0x523490`** — task premise corrected with evidence: it is a key→value
   skin-record parser (`flags`/`texture`/`color`/`width`/`height`/`rate`, defaults + EAs), and the
   only resource call in it is `Demand_0x49b750(textureName,1)` @0x523665 / `Release_0x498eb0`
   @0x5236ce. It does **not** demand `.int`.
2. **`EngineStringSystem::Format_0x56be30`** — decompiler fails as warned; decoded by disassembly
   (0x56be89 copy, 0x56befd `'<'`, 0x56bf1c `'>'`, 0x56bf40 payload extraction): inline `<tag>`
   markup pass, no printf substitution. Learned `f3_address_lookup`/`xrefs_to` on this (the DB has
   it as `SS::EngineStringSystem::meth_0x56be30`; the IDB name is `Format_0x56be30`).
3. **Picture/Fragment construction + STF binding** — factory `0x4b2910` type→class table
   (0 Window/1 Button/2 Picture/3 Label/4 Edit, alloc sizes + ctors), per-class read slots
   (slot7 `+0x1C` from the loader vs slot8 `+0x20` in the child loop) for Base/Fragment/Window/
   Picture/Label/Button, and the string→widget chain `Get_0x56c260` → `StringSystem::Get_0x4c5a30`
   + `Format_0x56be30` with all consumer EAs.
4. **Startup caller** — `F3::GameClient::virt_meth_0x45e8e0` → `F3::GameInterfaces::ctor_0x4398e0`
   → per-interface ctor → `LoadInterface_0x495b50` → `FileStreamOpen_0x49b9f0` @0x495b9a → header →
   fragment loop; `Mainmenu.INT`→`cls_0x440990::ctor_0x440990` mapping table for 14 of the 24
   `.INT` string constants (rest is a one-call recipe).

Ground truth came from the **files**, not our code: walked all 25 ftype-1600 lumps out of
`Interface.grp` via RHT (magic `GUI `, version `V1.1/V1.2/V1.3` → loader `a2` 2/3/4, flag, nRoot,
root type/rect/name). That walk corrected two of my first-pass guesses (`Dialog` has **2** roots;
`PipBoy`'s root is a **Picture** named `picPipBoy`; `winActionbar` not `winActionBar`) — table in
§2 of the doc.

Gaps G1–G8 are written as direct questions to you (§6), including the naming decision (G7) and the
three proposed follow-on implementation tasks (G8) you asked for at the end of the task.
