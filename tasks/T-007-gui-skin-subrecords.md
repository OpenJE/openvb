# T-007 — GUI: skin sub-records (Window+0x90 x9, stride 86) + texture binding

status: in-progress
assignee: implementer-B

> Claim note (B): starting with the RE-only half (steps 1-2, gaps G1/G2). Implementation of
> steps 3-4 waits on A's review of this task (proposed in chat/from-B.md C-22) so I do not land
> engine-adjacent code unreviewed.

## Objective
Decode the 9 inline skin sub-records that `Window::ReadFrom` reads at `+0x90` with stride 86
(`0x4b1297`/`0x4b12af`/`0x4b12b6`), port `Skin::LoadDefault_0x523490`'s key/value semantics
(recon §1), and bind the resulting `texture` string through `Demand_0x49b750` so a widget can
actually resolve a picture. Resolves recon gaps **G1** and **G2**.

## Steps
1. Identify the class of the 86-byte inline sub-record (it is NOT `GUI::Skin`: Skin's vft
   `0x66f2dc` has 4 slots, the sub-record is read through `vft+28`). Dump the vftable the ctor at
   `Window::ctor_0x4b0810` installs at `+0x90` and read its slot 7 to get the reader EA.
2. Identify `byte_707CE4` and its `vft[+0xD8]`/`[+0xDC]`/`[+0xC4]` (calls at 0x5236e7, 0x52363b,
   0x5236b4) — probe / index / clear for texture names; note `LoadDefault` skips the demand when
   `+0xDC` returns nonzero and forces the index to 0 when `+0xC4` returns < 0 (0x5236ba–0x5236c5).
3. Port the key table with its defaults: `flags`(+0), `texture`(+4 string), `color`(+8, 4
   components split on `",\n"` via tokenizer `sub_61A7D1`, packed only when exactly 4, else
   `0xFF808080` @0x523700), `width`(+0x0C, default 8 else `parent[+0x24]->[+0x1C]` @0x523917),
   `height`(+0x10, same shape @0x523a08), `rate`(+0x14, default 0).
4. Wire `Demand_0x49b750(name,1)` @0x523665 / `Release_0x498eb0` @0x5236ce into our
   `ResourceSystem` and keep the demand count semantics.

## Acceptance
- Additions to the GUILoading suite (T-005) asserting on a real `.int`: the 9 sub-records for
  `MainmenuWin` all parse (reader returns 1 nine times), each parsed `texture` name is non-empty
  or absent-with-default, and `color` falls back to `0xFF808080` when the stream has fewer than 4
  components — expected values taken from the original's stores (EAs in recon §1) and, where the
  data is involved, from the `.int`/`.ini` bytes in `Interface.grp`.
- A named `KNOWN-ISSUE` check for anything step 1–2 leaves unresolved instead of a guess.
- `run-all-tests` exits 0; `docs/f3/GUILoadingRecon.md` gaps G1/G2 updated to "resolved" with the
  new EAs (append under my own heading).

## References
- `docs/f3/GUILoadingRecon.md` §1 (key table, defaults, resource calls), §2 (9-record block), G1/G2.
- Original: `Skin::LoadDefault_0x523490` (decompiler fails; disasm 0x523490–0x523AEF),
  `Skin::meth_0x522ac0` loop @0x522ae8, Skin vft `0x66f2dc`, `Window::ReadFrom_0x4b10f0` @0x4b1297.
- Related data: `.int` fragments reference `.ini` and `.bmp/.tga` names (e.g. Debug.INT references
  `quickslot.ini`, `zzmainlabel-*.tga`) — those `.ini` blocks are likely where skins are defined.

## Notes
Needs T-005 (tree to attach skins to) and a working `Demand` path. Do the step 1–2 RE first: if it
contradicts recon §1, update the recon doc before writing code.
