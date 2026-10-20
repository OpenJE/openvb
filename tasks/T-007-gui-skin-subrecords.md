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

## Notes (implementer-B, 2026-10-20 — sub-records DONE, binding tail split into two facts)

**Landed** (`JE/GUI/SkinFragments.{hpp,cpp}` + 20 checks in `tests/je/GUILoading`): the 9 inline
records are a **3x3 nine-slice border set** and every rule is measured over all 25 shipped files
(1596 widgets / 14364 records / 13932 named, 432 unused):
slot order UL,UM,UR,ML,C,MR,LL,LM,LR (name suffixes agree in every named record); `d28` is the piece
role and is a deterministic function of the slot except the centre (0 as-is or 3 fill) — corners
6384/6384 are 0, edges 3192/3192 are 1 (horizontal) and 2 (vertical); `d2C`/`d30` are the SOURCE
piece size and the rect obeys: corners rect==piece, horizontal edges height==piece height, vertical
edges width==piece width (0 violations in 14364); `d34`/`d38` zero everywhere; `d154` is colour-like
and 10593 records hold `0xFF808080`, which independently confirms the `SkinDefaults` fallback;
`d8a` is a flag (277 exceptions, all Actionbar `picWeapon02CalledShot_*`); `d8b` uninterpreted.

**Texture binding: the shipped data answers most of it.** `resource.rht` carries 1653 ftype-**200**
resources (422 in pack `Interface`) whose NAME FIELD IS THE STEM — fragment
`zzmain-upperleft.tga` ↔ resource `zzmain-upperleft`. `DeriveTextureKey` + the engine's own
`ResourceExists_0x49abd0` resolve **157 of 165** distinct fragment names; the 8 misses are exactly
the superseded `main-*.bmp` family (asserted as a table, so drift is loud). All shipped names have
one dot, so the last-dot rule is not observable in this corpus.

**Correction to my own earlier note** (was §7 G1 here): `Gfx_Engine +0xDC` (0x5c0970 →
`cls_0x55dc80::meth_0x51f570` on `byte_70B200`) is **acquire-by-name**, not get-or-insert: it walks
the list comparing names, outputs `node[3]` as the id, bumps `node+44` (use count) and returns 1; an
empty list or a miss writes **id 0** and returns 0 (0x51f643/0x51f64d). So a name that is not
registered fails quietly — which is why the 8 missing `main-*.bmp` fragments are not an error to
port around.

**Still open, deliberately not guessed:**
1. the exact string passed to +0xDC at runtime (stem vs `name.tga`) — one lockstep observation of
   `meth_0x51f570`'s `param_2` on a Picture load settles it; my key derivation is marked inferred.
2. the registry itself (`cls_0x55dc80` @`byte_70B200`: node layout `name@+0x10?`, `id=node[3]`,
   `useCount@+44`, list head `mbr_0x28`) and `+0xE0`(0x5c0940)/`+0xD8`(0x5c0930)/`+0xC4`(0x5c0a90,
   14-arg SEH forwarder, decompiler fails). That is engine core shared with VFX/Octree/fog — routed
   to the coordinator as its own task (proposed T-012) instead of half-ported here.
3. the `.ini`-section enumeration for `Skin` records (T-009 accessors, still owed by C).
