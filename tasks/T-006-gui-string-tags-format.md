# T-006 — GUI/SS: `EngineStringSystem::Format` `<tag>` markup + Label text wiring

status: done
assignee: implementer-B

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


## Notes (implementer-B, 2026-10-20) — steps 1-2 DONE, steps 3-4 need a handshake

Delivered `docs/f3/GUIStringTags.md` (indexed in `docs/f3.md`) and corrected my own wrong guess in
`docs/f3/GUILoadingRecon.md` §4/G4: the `<...>` tags are **substitution**, not style markup.
Decoded the whole 0x42C-byte function from disassembly (decompiler still fails): two branches
(`gv_` case-insensitive @0x56bfb2 -> JE::World `0x70BD98` vft+0x6C; otherwise the token registry at
ESS+0x16C via `cls_0x4d8d70::meth_0x5ace40` @0x56c117 + `sub_4C5900` @0x56c14f), three exact log
strings, and the `<Bad Token>` fallback. Shipped `English.stf` vocabulary enumerated from the file:
`CHARNAME`, `DEVILDOG`, `HANGDOGNAME`, `test`, `test2`, `GV_Jake` (+ expletives that are data).
Also found: the ftype-2200 "F3" lump is a *sound* token list, not this registry; and `CHARNAME` is
referenced by `F3::GameDialogueWindow::virt_meth_0x437f00` @0x437fac (runtime registration).
Side finding for A: this is a caller of `meth_0x5ace40`, which `docs/DEFERRED-NOTES.md` recorded as
"callers not yet RE'd".

Steps 3-4 (implement `Format`, wire Label text) touch `modules/openje/src/JE/SS/EngineStringSystem.*`
= engine core (A's lane). Asked in chat/from-B.md: either A implements it there, or authorizes a new
file `JE/SS/StringFormat.cpp` (+ one call-site line) for me. Task stays `in-progress`; the port is
fully specified in the doc §0 so the implementation is mechanical either way.


## Notes (implementation, implementer-B, 2026-10-20) — openje `638e07b`

Steps 1–2 (RE + `docs/f3/GUIStringTags.md`) were recorded above. Step 3 landed as **new files
only** (`modules/openje/src/JE/SS/StringFormat.{hpp,cpp}`), exactly the option announced in
chat/from-B.md C-27, so nothing of A's `JE/SS/EngineStringSystem.*` was edited:
- `JE::SS::Format(env, text, capacity)` reproduces the original's observable behaviour with EAs at
  each decision (header docblock), and takes its two lookups as an injected `FormatEnv` because the
  game-variable seam is still unresolved (C-29) and belongs to engine core.
- `MakeEnv(ess)` binds the real token map (`GetToken`) and leaves variables "not found" — which is
  precisely the demo's behaviour, since no `tokens.tok` ships (StringSystem suite asserts
  `TokenCount() == 0`), so every shipped `<NAME>` tag falls into the silent-drop path.
- **Not wired** into `Get_0x56c260`: that one call-site line is engine core, offered to A as a
  reviewable diff (and it is step 4's remaining piece, together with the resolver answer).

Acceptance status:
- [x] `docs/f3/GUIStringTags.md` exists, every tag/branch carries an EA, unresolved items named
      (P1 resolver seam, P2 token-container ownership, P3 nesting — P3 answered: single pass, no
      re-scan, because the original snapshots the input at 0x56BE89 and only appends).
- [x] tests: 11 new checks in the `StringSystem` suite covering gv_ hit/miss, the case-insensitive
      prefix (`<GV_Jake>` — a tag that really ships), token hit/miss, unclosed `<`, multi-tag
      single pass, and the fixed-capacity clean-failure deviation. Expected values cite EAs/the
      STF file, not our implementation.
- [x] `run-all-tests` exits 0 with the new source in the StringSystem row:
      `8 suites OK + app=124 => 9 pass, 0 fail`.
- [x] `cppcheck-openje` on the new .cpp: 0 findings (no baseline entry needed; one inline
      `constParameterCallback` suppression with a stated reason, since the seam signature is fixed).
- [ ] step 4 (Label text end-to-end) still needs the call-site line + the resolver seam → tracked
      as a follow-on, not a hidden gap: T-006 closes the *string* half; GUI text display belongs
      with T-005's tree once `InterfaceFile` enters the app build (the `CMakeLists.txt` line).
