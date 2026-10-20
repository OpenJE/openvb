# Responses from implementer-B (GUI / strings / tooling lanes)

Writer: implementer-B (parallel implementer; lanes per `tasks/README.md` — tooling & tests, and the
GUI/skin RE lane). Discoveries agent: fold what is useful into `2025-10-20-canonical-fact-sheet.md`
and mark the corresponding sessions CLOSED — I have already paid for these EAs, so please do not
re-derive them. Format matches your convention: `0x…` = original F3.exe VA; anything marked
*(inferred)* is not verified.

## 1. GUI interface (`.int`) loading is fully decoded — no session needed
- Chain: `F3::GameClient::virt_meth_0x45e8e0` -> `F3::GameInterfaces::ctor_0x4398e0` ->
  per-interface ctor (e.g. `cls_0x440990::ctor_0x440990`) -> `JE::String::Assign("<Name>.INT")`
  @0x440a35 -> **`JE::CLoadMapInterface::LoadInterface_0x495b50`** @0x440a4b, then
  `Assign("<Name>Win")` + `FindWidget_0x495740` @0x440ab5 (result cached at obj+4).
- `.int` header: u32 magic `'GUI '` (0x20495547) @0x495bca -> u32 version literal `V1.0`..`V1.3`
  -> internal `a2` 1..4 (switch @0x495be8; magic absent -> rewind + `a2=0` @0x495c30) -> u32-len
  name (own filename) via `sub_50B880` @0x495c3a -> u8 flag @0x495c78 -> u32 root count -> per root
  `{u32 type, vft+28 read}`.
- Fragment factory `GUI_CreateFragment_0x4b2910`: 0 Window / 1 Button / 2 Picture / 3 Label /
  4 Edit; unknown -> NULL and the loader aborts (0x495cfa). Reader variants: slot7 `+0x1C`
  (window 0x4b10f0, picture 0x4ab190, **label and edit share 0x4aa640**, button 0x4a6b60) vs slot8
  `+0x20` used by the child loop.
- **Widget records are recursive**, and the child-link table is `{string,u32 next}` entries at
  Window+0xCAC terminated by -1 (0x4b12e3..0x4b134b) — a flat walk desyncs past the first root
  (`Dialog.INT` is the only 2-root file; `PipBoy.INT` is the only Picture root, `picPipBoy`).
- All 25 shipped `.int` files (RHT ftype 1600, all in `Interface.grp`) and their root
  type/rect/name are tabulated in `docs/f3/GUILoadingRecon.md` §2, plus 14 of the 24 `<Name>.INT`
  literals mapped to their ctors.

## 2. Skins: one skin == one `.ini` resource (this is the answer to "where do the .ini names in .int come from")
- Widgets request skins **by name at runtime**: `Window::virt_meth_0x4b06c0` @0x4b06e4 ->
  name-keyed registry `cls_0x4b5c30::meth_0x4b5c30` (RB-tree at +0x3C; miss -> `new Skin`(0x28),
  `Skin::ctor_0x4b5b10`) -> `Skin::vft[+8]` = **`LoadByName_0x522c30`**, which calls
  **`F3::LoadIniResource_0x4e0960`** @0x522cb0 and feeds the parsed tree to
  `Skin::meth_0x522ac0` (iterates the record list -> `LoadDefault` per record).
- `Skin::LoadDefault_0x523490` (decompiler fails; disasm only) keys, with defaults and shipped
  quirks: `flags`(+0), `texture`(+4), `color`(+8), `width`(+0x0C), `height`(+0x10), `rate`(+0x14).
  Colour = 4 components split on `",\n"` (`asc_677A40`, tokenizer `sub_61A7D1` @0x5237cb/0x5237e2),
  stored **only** at exactly 4 (else `0xFF808080` @0x523700); packing masks components 1..3 but
  **not** component 0 (@0x523806..0x523832). Absent width/height -> 8 with no texture, else
  `parent[+0x24]->+0x1C/+0x20` or 0 (@0x523912/@0x523917/@0x523926).
- The 9 "skin slots" per window are inline **`GUI::Fragment` objects, stride 0x158 (344 bytes,
  i.e. +86 DWORDS)** created by `eh vector constructor iterator` @0x4b08c2; their reader is
  `Fragment::ReadFrom_0x4b20a0` = 6 x u32 + texture-name string + u32 + u32 + 16-byte rect.
- **`byte_707CE4` is the `JE::Gfx_Engine` singleton** (class size 2212, primary vft `0x6ae7ec`,
  120 slots), written by `Gfx_Engine::ctor_0x5bfe10` @0x5bfe33. Slot roles seen at the GUI call
  sites: `+0xC4` name->index (negative = absent, callers clamp to 0), `+0xDC` registered?-probe
  (nonzero -> skip the Demand), `+0xE0` add/lock by name, `+0xD8` release/forget. Shared by GUI
  Picture/Label, AreaMap icons, cursor manager, container item icons, VFX sprite/model/trail,
  Octree2Gfx and Gfx_HeightFog — i.e. one texture registry for GUI *and* 3D.

## 3. `<...>` inside engine strings is substitution, not style markup
`EngineStringSystem::Get_0x56c260` = `StringSystem::Get_0x4c5a30` then **`Format_0x56be30`**
(0x42C bytes, decompiler fails; fully decoded from disasm). Payload after `'<'`…`'>'`:
- case-insensitive `"gv_"` prefix (@0x56bfb2, literal 0x6995b8) -> game variable: the World global
  `0x70BD98` is taken through its **virtual base at obj+0x168** (vbtable `0x691A94 = [-4,352,…]`,
  installed by `F3::GameWorld::ctor_0x470340` @0x470388), slot `+0x6C` = `0x545430`, and the
  consumer `sub_5A9F80` resolves it as a **VFX_Parameter ini key with section defaulting to
  `"Globals"`** (@0x5a9fc5, tree at object+28, `meth_0x4339b0` @0x5a9fd5, copy-out @0x5aa037).
  Found -> append value; missing -> log `"Couldn't find variable %s"` + append `<Bad Token>`.
- otherwise -> token registry at ESS+0x16C (`cls_0x4d8d70::meth_0x5ace40` @0x56c117 then
  `sub_4C5900` @0x56c14f); absent -> log `"No token %s found\r\n"` and append **nothing**
  (asymmetric with the variable path); present-but-unmaterializable -> log
  `"Couldn't find string %s"` + `<Bad Token>`.
- unclosed `'<'` is dropped and scanning continues (0x56bf27 -> 0x56C1E7); single pass, no rescan.
- Shipped `English.stf` has 25 distinct `<...>` runs, 19 of them identifier-like
  (`CHARNAME`, `DEVILDOG`, `HANGDOGNAME`, `test`, `test2`, `GV_Jake`, plus dialogue expletives
  `<ASS>`, `<DAMN>`, `<Shit>`, `<fucking>`, …). Exactly one is `gv_`-prefixed (`GV_Jake` - also the
  proof that the prefix match is case-insensitive). The parser cannot tell an intended token from an
  expletive, and no `tokens.tok` resource ships, so **every** non-`gv_` run takes the silent-drop
  path at runtime (that is an observable behaviour to match, not a no-op).
- Open, needs a debugger (see §5): is the payload the **key** in section `Globals` or the
  **section** name? The `__userpurge`/`ecx`-return arg order at 0x5a9fd5 vs 0x5aa01e does not say.

## 4. Skill / DerivedAttribute enum constants (in case your session wants them)
`GetTotalSkill_0x409830` -> 13 cases 0..12: 0 FIREARMS, 1 MELEE (both `base+50`), 2 UNARMED
(`END+2*STR`), 3 BARTER / 4 PERSUASION (`3*CHA`), 5 DECEPTION (`INT+2*CHA`), 6 MECHANICS / 7 MEDIC
(`PER+2*INT`), 8 OUTDOORSMAN (`END+2*PER`), 9 SCIENCE (`3*INT`), 10 SNEAK / 11 SECURITY
(`AGI+2*PER`), 12 STEAL (`PER+2*AGI`); SPECIAL at +0x760/764/768/76C/770/774; base-skill array =
`int[skill]` at **+0xD8** (`cls_0x409820::meth_0x409820`). Unknown index logs
`"GameCreature::GetTotalSkill() - UNKNOWN SKILL: %d"`.
`GetDerivedAttribute_0x409a60` -> 15 cases 0..14 (0:`AGI+20`, 1:`(STR+1)*25`, 2:`PER*2`,
3:`[+0x778]`, 4:`[+0x7FC]`, 5:`END`, 6:`STR`, 7:`PER*3+40`, 8:`3`, 9/10/11:`0`, 12:`INT*2+5`,
13:`STR*3`, 14: recurse(0) scaled by a double @0x64D590, cap 1000). Unknown index logs
`"GameCreature::GetDerivedAttribute() - Unknown attribute requested: %d"`.
Combat helpers: `GameActionAttack::meth_0x402260` hit test = roll `sub_48CD00(0,100)`
(**0..100 inclusive**, 101 outcomes) with `chance >= roll` @0x402393, and it feeds an on-screen
`"Hit/Miss: Chance = %d, Random = %d"` readout @0x40236f; `CalculateDamage_0x413310` selects the DT
**row set by body part** (`+0x7B0` for parts 0..4 and 7, `+0x7C8` for parts 5 and 6) while the
damage *type* (`param_1[4]`, 0..5) indexes inside the row, and its `param_5 == 9 -> return 0` guard
@0x41333d **is** unconditional (the conditional one is at 0x409f90).

## 5. For the EXPERIMENTS sheet: my lane vs the debugger lane
E1, E2, E4, E5, E6, E7 and the writer-watchpoint half of E3 all need breakpoints/hardware watchpoints
against the **original** exe — that is `f3-lockstep-debug` and implementer-A's declared lane; routing
them to A rather than leaving them unassigned. What I own and can provide instead:
- a deterministic boot of our rebuilt exe (`run-all-tests` app-boot, `f3-reimpl-run-data`) if a dump
  needs the reimplementation side running to the same point;
- the **static** half of E3 is already done without a debugger: the skill/derived constant tables in
  §4 came out of jump tables, and the clip table question can be attempted the same way if you want
  me to;
- string-substitution observation (§3 open item) is exactly one in-game `<gv_…>` render — if A's
  lockstep session can log the value written by `sub_5A9F80`, I will land the resolver accordingly.

Two reporting traps that would bite experiment logs: findings/`LogDebugString` output goes to
**stderr**, and an exit status read through a pipe is the pipe's, not the program's — so
`cmd > f 2>&1; echo $?` for anything you cite as pass/fail.

Primary documents: `docs/f3/GUILoadingRecon.md`, `docs/f3/GUIStringTags.md`,
`docs/notes/implementer-B.md`; code: `modules/openje/src/JE/GUI/{InterfaceFile,SkinDefaults}.{hpp,cpp}`,
`modules/openje/src/JE/SS/StringFormat.{hpp,cpp}` (tests: `tests/je/GUILoading`, `tests/je/StringSystem`).
