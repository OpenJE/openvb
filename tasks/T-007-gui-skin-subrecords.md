# T-007 — GUI: skin sub-records (Window+0x90 x9, stride 86) + texture binding

status: done — steps 1-2 CLOSED (`06ac911`), steps 3-4 LANDED + CHECKPOINTED (openje `a166dfa` /
parent `2b17631`; coordinator-confirmed via GoldYak's independent 10/0 gate run on the fresh 14:34
payload incl. GUILoading 187>=116; tail questions live in T-013's narrowing updates)

> **STEPS 1–2 CLOSED (coordinator checkpoint 2026-10-21, LoudDragon `01a15f82`):**
> `discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md` accepted. Verified independently against
> the frozen export: G1 = `GUI::Fragment` 0x158-byte objects ("86" = 86 DWORDs; push 158h
> @0x4b08b1, add ebx,158h @0x4b12b6, dtor iterator push @0x63212e); ctor `0x4b24a0` installs vft
> `0x66ea1c` @0x4b24d6 (9x via eh-vector-ctor @0x4b08c2); slot 7 (+0x1C) = reader `0x4b20a0` —
> export agrees (vft 0x66ea1c index 7 -> 0x4b20a0). G2 = `0x707CE4` is the `Gfx_Engine` singleton
> (sole store `a3 e4 7c 70 00` @0x5bfe33, vft `0x6ae7ec` @0x5bfe43; clears @0x5bedd6/@0x5c1e24);
> +0xC4=slot49=0x5c0a90 get-or-create index (delegates cls_0x55dc80/0x70B200, negative=load
> failure), +0xD8=slot54=0x5c0930 handle->object, +0xDC=slot55=0x5c0970 lookup-only probe (writes
> id through out ptr on hit, bumps obj+44 usecount, never inserts) — export slot table agrees.
> ERRATUM registered for T-040: F3.json `GUI::Fragment` size 3384 is a frozen-export error; the
> byte-level truth is 0x158 (max member +0x154+4, three independent size encodings).
> Steps 3–4 (the port) UNPAUSED — re-dispatched to KeenStorm under the build-gate rules below.

> Claim note (KeenStorm): doing steps 1–2 as a research row. Target: ORIGINAL_REFERENCE only
> (`ida_original_f3`), READ-ONLY (GoldYak is the sole IDB writer this wave — no renames/comments/
> type edits from me). Deliverable: `discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md` with
> structural evidence from `artifacts/F3.json` first, IDA disasm to verify the cited EAs.

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


## Status update (implementer-B, 2026-10-20 — my side of T-007 is DONE; tail moved to T-013)

`JE/GUI/SkinTable.{hpp,cpp}` (openje `8ce1d3f`) completes the skin chain on the data side:
`LoadSkinList` reads resource `skinlist` -> the five shipped skin inis; `SkinTable::Load` parses one
ini (owning the text buffer, because `ParseIniText` aliases into it) and builds **one record per
section** through `ReadSkinDefaults`, exactly as `Skin::meth_0x522ac0` does per tree node.
30 new checks, every expectation measured from `Interface.grp` (ftype 800), none from our parser:
5 names in order; 54/63/54/54/54 sections; `button.disabled` present only in zzmainmenu; window
`flags` = 0,1,0,2,3,2,0,1,0 in **all five** skins (independent confirmation of the `.int` `d28`
nine-slice table); texture misses 0/0/3/54/0; and all 8 shipped `color` lines being 3-component means
every record keeps `0xFF808080` — the exactly-4 rule doing real work. Suite green via the repo table,
cppcheck 0 findings, GUILoading row gained `SkinTable.cpp`.

What T-007's title still owes is now **T-013** (registry `cls_0x55dc80` @`byte_70B200` + slots
+0xC4/+0xD8/+0xE0 + the one lockstep observation of the acquire key string), assigned after M-001
slice 1 by the coordinator. Acceptance evidence for the binding is already in place: `TextureMisses`
shows which names the registry must reject (areamap's 54, quickslot's 3), so T-013 has a test to hit
rather than a blank slate.

## Status note (NiceHawk, `01a15bd1` — step 4 landed; the row's remaining tail is T-013's)

Steps 1–3 were already landed and asserted when this row moved to me, so I did not redo them. What
the title still owed was **"so a widget can actually resolve a picture"**, and that turned out to be
one function in the binary: `Window::virt_meth_0x4b06c0`. It is now decoded (from objdump — the
decompiler fails on it), ported, and driven by shipped data. Full write-up:
`docs/f3/GUILoadingRecon.md` §8. Code: `JE/GUI/SkinTable.{hpp,cpp}`
(`Find`, `ApplySkinRecord`, `ApplySkinGroup`, `SkinSet`), openje `fa4363c`.

Three things worth the row's verdict rather than a changelog line:

- **Slot order no longer rests on data agreement.** The nine suffixes are a shipped pointer table at
  `0x6faf2c`, bounded by `cmp $0x6faf50` with step 4 — nine *by construction*, each carrying its own
  leading dot, middle row `.left/.middle/.right`. `SKIN_SLOT_SUFFIX` is now the single source for
  both the apply's `group ++ suffix` key and the ini-suffix matcher.
- **The two "reserved, always zero" fields are explained.** The `.int` writes six dwords at
  `+0x1C/+0x20/+0x24/+0x28/+0x2C/+0x30` (`lea` operands `0x4b20d8`…`0x4b215a`); `+0x28` is `rate`,
  which is authored in the skin ini and never in the interface file — that is the *reason* for the
  14364 zeros, not a guess at a meaning. `+0x2C` has no writer on either path and is still
  uninterpreted. The recon's "second u32 at +8" was wrong: it is `+0xC0` (@0x4b2250).
- **The `.int` really carries the binding's key.** Measured in the suite: `MainmenuWin`'s `+0x40` is
  literally `'zzmainmenu.ini'`, and all 9 widgets in `Mainmenu.INT` name one of the five shipped
  skins (cross-checked against `skinlist`, an unrelated resource). The **group** is caller-supplied
  (`'label'` @0x66d428), so `ApplySkinGroup` is exercised per shipped group across all five inis, not
  per widget — both facts print as KNOWN-ISSUE lines instead of being smoothed over.

Gate: `gate-integrity.sh` rc=0 → `run-all-tests` **10 pass / 0 fail**, GATE STAMP
`2026-10-21T03:49:29Z`, HEAD `ef85492` / openje `9567e6e` (my files dirty at measurement), payload
`x95aqfk70mrsq…`, table hash `2945679485`; GUILoading **150 asserts** (was 116), cppcheck 0 findings.
The floor was **not** raised: that needs `devenv/scripts/run-all-tests.nix`, which is build-affecting
and two-lane capped. `msvc-build` was **not** run — no header in `src/F3`, no new TU, and every file
touched is already on the GUILoading row's source list, so the host suites do compile the changed
headers. If a reviewer wants the MSVC link proof anyway, say so and I'll run it.

Left for **T-013**, deliberately not faked here: the texture registry behind the acquire seam
(`cls_0x55dc80` @`byte_70B200`), and the one lockstep observation of the key string form. The seam is
ready — `SkinAcquireFn` takes `{resolved, index, gate, dimW, dimH}` and the port treats dims strictly
as inputs, with no-acquire meaning *defer and count* rather than store a name where an id belongs.
T-013 plugs in by supplying that one function. New fact that narrows its scope: `Gfx_Engine vft+0xD8`
returns the **object** (the binding dereferences `[obj+0x24]` for extents and tests `[obj+0x28]` as a
gate) while `+0xDC` returns the **id** — two slots, two shapes.

## Status note (KeenStorm, 2026-10-21 — STEPS 3–4 LANDED; port half complete, awaiting coordinator closure)

What landed (all inside `modules/openje`, this row's own files):

1. **The Fragment wire layout, byte-exact, in the `.int` lane.** `InterfaceFile::ReadFragmentRecord`
   (new public static, the T-004-family seam, driven by `ReadWidgetBlock` itself) reads
   `Fragment::ReadFrom_0x4b20a0`'s footprint with the G1 field table's EAs in comments. The two
   previously-unmodelled port-rule items are now ported **as evidence demanded**: `+0x2C` is
   parse-and-store with no interpretation (assert: `0xDEADBEEF` round-trips), and the
   `+0xC0 -> +0xDC/+0xF8/+0x114` broadcast is reproduced as the literal **COPY** of dword[+0xC0]
   (new `dDC/dF8/d114` fields; assert they equal `d8b`; nothing computes them). The `d8b` comment
   is corrected from "+8" to `frag+0xC0` per recon §8c.
2. **The texture binding as BEHAVIOUR, not ABI** (`GUI::TextureSlots` / `BindFragmentTexture` /
   `ResolveSkinTexture`, new in `JE/GUI/SkinFragments.{hpp,cpp}`): probe (`+0xDC` 0x5c0970) hit
   **skips the demand** (0x4b21c3/0x523643); miss goes through our `ResourceSystem::Demand_0x49b750`
   (flag 1, the shipped a3) → `+0xC4` get-or-create on the **resolved** Res name + payload →
   **negative-clamp to 0** (0x4b2223/0x5236c5) → `Release_0x498eb0` exactly once even on a clamped
   index → store. LoadDefault's variant additionally runs the `+0xD8` **validity gate** (0x5236d7)
   that CLEARS a nonzero id with no object (0x5236f8); the reader chain provably has NO gate —
   asserted as separate shapes, not unified. `InterfaceFile::SetTextureSlots` wires it into the
   loader (chain consumes zero stream bytes: the whole-lump `BytesConsumed` invariants still hold,
   asserted with binding ON). `SkinTable::ResolveTextures` runs the LoadDefault tail over a parsed
   ini into a parallel id array (record+0x04 is the resolved INDEX in the shipped record, recon
   §8d — the parse-time name stays in `SkinDefault::texture`, the id goes to `ResolvedTextureIndex`).
   `ResourceSystem::ResName` became public (3 lines) so the convert sees the resolved name like the
   shipped code does at 0x4b21e8. Demand-COUNT semantics asserted directly: after the chain's
   release the managed Res survives in the cache at count 0 and a re-demand bumps it to 1.
   The REAL registry (cls_0x55dc80 node layout, stem-vs-extension key string) stays **T-013**; the
   seam is what it plugs into.
3. **Tests**: `tests/je/GUILoading` grew from 150 to **187 asserts** — synthetic byte-level
   Fragment record → per-field asserts (T-004 family, incl. truncation refusal), all chain shapes
   (hit-skip / demand-miss-quiet / clamp-still-releases / gate-clears / gate-keeps / key-absent /
   empty-name-not-special-cased), binding through the real `Mainmenu.INT` loader (bound vs
   unbound -1 census), and `areamap.ini` (54 named records) end-to-end through
   `ResolveTextures`. One data discovery worth the coordinator's inventory: with the honest
   all-miss seam, named `.int` textures whose stem resolves in the rht **do** pass through the
   demand (our resolver strips the extension before the probe, mirroring
   `ResolveResourceName_0x499f10`/`ResourceExists_0x49abd0`), so the shipped chain converts them;
   the census prints instead of being hidden.

Gate evidence (measured with these files dirty on top of `0a8c94e` / parent `4c256a5`):
- `tools/gate-integrity.sh`: **rc 0** — "table: OK (9 rows identical to the tree)", "gate payload
  is current and attributable" (build-affecting dirty 3 = other lanes' devenv/tools files, not
  staged here).
- `run-all-tests --no-app`: **9 pass / 0 fail**; GATE STAMP `2026-10-21T18:30:10Z`, table hash
  `483599247`, GUILoading `[187 asserts >= 116]`. Floor NOT raised (row unchanged — no new TU,
  every touched file is already on the GUILoading row).
- `cppcheck-openje`: 0 errors / 0 warnings (pre-existing style notes only; exit 0).
- `msvc-build`: **rc 0**, `[100%] Built target F3` — mandatory this time (public headers
  `ResourceSystem.hpp`/`InterfaceFile.hpp`/`SkinFragments.hpp`/`SkinTable.hpp` touched across TUs;
  item 37 respected: mingw-green was NOT claimed as product-green).
