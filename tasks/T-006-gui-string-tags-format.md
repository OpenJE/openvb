# T-006 — GUI/SS: `EngineStringSystem::Format` `<tag>` markup + Label text wiring

status: open
assignee: —

## Objective
Enumerate the inline `<tag>` markup language that `Format_0x56be30` applies to strings right after
`StringSystem::Get_0x4c5a30` (recon §4), port it, and wire `Get_0x56c260` -> widget text so labels
show real English.stf strings. First half is RE (disassembly only — the decompiler fails on this
function), second half is implementation in new files.

## Steps
1. Disassemble `0x56be30` end to end (frame 0x118, SEH prologue at 0x56be30). Known skeleton:
   `String::CopyAssign_0x401b20` @0x56be89 snapshot, scan `'<'` (0x3C) @0x56befd, matching `'>'`
   (0x3E) @0x56bf1c, payload via `meth_0x458ef0` @0x56bf40, loop bound `jbe` @0x56bee8.
   Enumerate the tag vocabulary by walking every compare chain after 0x56bf40 and collecting the
   literal operands (record each tag's EA).
2. Document the result as `docs/f3/GUIStringTags.md` (discoveries-style: EA per claim, gaps for
   anything not proven), then implement `Format` in the reimpl's `JE::SS` GUI-adjacent path.
3. Identify what each tag does to the render state (colour / font / case / alignment) and where it
   is stored on the widget — recon §4 saw F3 call `vft[+360]` (colour literals -16777088 /
   1082163328) and `vft[+232]` (enable) around label updates at 0x45065b–0x4509bd.
4. Wire one end-to-end path: a Label in a `.int` tree (T-005) whose message id comes from
   `English.stf`, text set through `Get_0x56c260`.

## Acceptance
- `docs/f3/GUIStringTags.md` exists, lists every tag with at least one EA each, and states which
  tags remain unresolved.
- Format tests in the existing `StringSystem` suite (or a new suite added to `run-all-tests`) that
  feed real `English.stf` strings containing each discovered tag and assert the transformed output
  / state — expected values derived from the original binary (single-step it in lockstep with
  `f3-lockstep-debug` if needed), never from our implementation.
- `run-all-tests` exits 0.

## References
- `docs/f3/GUILoadingRecon.md` §4 + gap G4; `docs/DEFERRED-NOTES.md` §Strings (STF format,
  ordinal ids, male fallback order); `modules/openje/src/JE/SS/EngineStringSystem.hpp` (vft[1]
  `Get_0x56c260` note).
- Consumers to keep compatible: `0x40EC90`, `JE::CLoadMapInterface::meth_0x4580c0`,
  `sub_56C280`, `JE::Client::meth_0x58aad0`, `JE::DialogueWindow::virt_meth_0x595010`.

## Notes
Depends on T-005 for the widget side, but step 1–2 (the RE + doc) is independent and can start
immediately; A's dialogue/quest work will want the tag list before long.
