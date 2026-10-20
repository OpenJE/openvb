# T-005 — GUI: `.int` reader + widget tree with name lookup

status: in-progress
assignee: implementer-B

## Objective
Port the interface-file loader that `docs/f3/GUILoadingRecon.md` §2/§3 decoded, so a shipped
`.int` becomes a live widget tree that F3 code can query by name. New files only
(`modules/openje/src/JE/GUI/*`) — no edits to A-owned existing sources; A reviews (GUI lane rule,
`tasks/README.md`).

## Steps
1. `GUI::Base` / `GUI::Fragment` / `GUI::Window` / `GUI::Picture` / `GUI::Label` minimal objects
   with only the fields the reader actually sets: rect at Base+0x08 (16-byte move, `0x4b2864`),
   `Base+4=1`, name at Window+0x1C, `+0x38/+0x39/+0x3A` bytes, `+0x40` string, `+0x5C`, and the
   `a2`-gated extras (`a2>=2`: `+0x60`, `+0x64`; `a2>=3`: `+0xCB8`, `+0xCD4`, `+0xCD8`, `+0xCF4`,
   `+0xCF8`). Keep everything else as gap arrays — do NOT invent field names.
2. Factory `GUI::CreateFragment(type)` per `0x4b2910`: 0 Window / 1 Button / 2 Picture / 3 Label /
   4 Edit, unknown -> NULL, and the caller must then abort (matching `0x495cfa`'s failure path).
3. `LoadInterface(name)` port of `0x495b50`: `FileStreamOpen_0x49b9f0(name,1)` -> magic `'GUI '`
   (0x20495547) else rewind + `a2=0` -> version literal `V1.0..V1.3` -> `a2` 1..4 -> u32-len name
   -> u8 flag -> u32 nRoot -> per root {u32 type, `vft+28` read} -> name index with
   `FindWidget_0x495740` semantics (child loop `0x4b13a0`/`0x4b13bc`).
4. Store the 9 inline skin sub-records as opaque stride-86 blobs at Window+0x90 (decode = T-007)
   and the child-link table as opaque 28-byte entries.

## Acceptance
- New suite `modules/openje/tests/je/GUILoading/GUILoading.cpp`, added to
  `devenv/scripts/run-all-tests.nix`, with `run-all-tests` still exiting 0, asserting from the
  real files: `Mainmenu.INT` -> a2=4, root type 0, name `MainmenuWin`, rect `0,0,1024,768`,
  nChildren > 0, `FindWidget("MainmenuWin")` == root; `Dialog.INT` -> 2 roots; `PipBoy.INT` ->
  root type 2 named `picPipBoy`, rect `0,-1,1024,609`; `Loading.INT` (V1.1) -> a2=2 and the
  `a2>=3` fields left unread (assert the stream consumed exactly the a2=2 field count).
- Every expected value cited to `docs/f3/GUILoadingRecon.md` §2 (the file walk), not to our code.

## References
- `docs/f3/GUILoadingRecon.md` §0/§2/§3 (loader chain, header table, per-file ground truth, slots).
- Original EAs: `LoadInterface_0x495b50`, `CreateFragment_0x4b2910`, `Window::ReadFrom_0x4b10f0`
  (+ sibling `0x4b0e10`), `Base::ReadFrom_0x4b2850`, `FindWidget_0x495740`, string read `0x50b880`.
- Data: `Interface.grp` lumps — Mainmenu 80, Dialog 155, PipBoy 162, Loading 167.

## Notes
Order matters: T-006 (string tags) and T-007 (skins) both consume this tree. Depends on nothing in
A's queue except `ResourceSystem` file streams, which already exist.
