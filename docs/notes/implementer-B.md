# implementer-B journal

My durable working notes (tooling/tests lane). Evidence style as in `docs/DEFERRED-NOTES.md`:
addresses `0x…`, member offsets, file paths. Verify-after-write: any scripted doc/code edit must
grep-prove its own insertion before commit (adopted per chat/from-A.md C-6).

## T-001 — verified suite invocations (source for `run-all-tests`, T-002)

Common prefix (from repo root, `C:\f3data` works as argv[1] for the data-based suites):

```sh
i686-w64-mingw32-g++ -std=c++98 -static -w -Isrc -Imodules/openje/src -Imodules/tracing/src \
  <test .cpp> <impl sources...> -o <out .exe>
WINEPREFIX=$PWD/.devenv/state/f3-reimpl-wineprefix WINEDEBUG=-all \
  wine <out .exe> 'C:\f3data'
```

Wine binary is not on `devenv` PATH in my shell: use
`/nix/store/b0d6l5vh4fdawmvq5j504a47pcxfsk8i-wine-11.0/bin/wine` (or `${pkgs.wine}` via a script).

`JE=modules/openje/src/JE`, `JEM=modules/openje/src/JE.cpp`:

| suite | test source | impl sources that link clean |
|-------|-------------|------------------------------|
| cls_0x4d8d70 | `tests/je/cls_0x4d8d70/cls_0x4d8d70.cpp` | `$JE/cls_0x4d8d70.cpp` |
| PathManager | `tests/je/PathManager/PathManager.cpp` | `PathManager Map ResourceSystem Common/Chunk Resources VFX_Parameter Subsystem JE.cpp World Action` + `src/F3/GamePathManager.cpp` |
| ResourceSystem | `tests/je/ResourceSystem/ResourceSystem.cpp` | `ResourceSystem Resources Map VFX_Parameter Common/Chunk Subsystem JE.cpp` |
| StringSystem | `tests/je/StringSystem/StringSystem.cpp` | `SS/StringSystem SS/EngineStringSystem ResourceSystem Resources Strings VFX_Parameter World Subsystem JE.cpp Common/Chunk PathManager Map Action` |
| ActionMoveTo | `tests/je/ActionMoveTo/ActionMoveTo.cpp` | `Action ActionMoveTo PathManager Map ResourceSystem Common/Chunk VFX_Parameter Resources Subsystem JE.cpp World` + `src/F3/GamePathManager.cpp` |
| SpawnMove | `tests/je/ActionMoveTo/SpawnMove.cpp` | same as ActionMoveTo **+ `src/F3/MapEntitySpawn.cpp src/F3/GameActionMoveTo.cpp`** |

Gotchas learned in T-001:
- Header "build:" comments are NOT sufficient (only 3 of 6 exist, all 3 under-specified). Table
  above is the verified truth; it must move into `run-all-tests` (T-002).
- Link direction is transitive: `JE/World.cpp` needs `JE/PathManager.cpp` + `JE/Map.cpp`
  (`WorldToOctreeCell_0x560680`, `CellToWorld_0x560710`), and `JE/PathManager.cpp` needs the Map
  helpers + `JE::Host::TickMilliseconds` (test TU provides it). Pulling `World.cpp` into a suite
  that does NOT define `JE::Host::TickWorldEntities(int)` fails to link — that's why
  `ResourceSystem`'s source list stays minimal.
- `JE::Action` base vft/dtor lives in `JE/Action.cpp`; both move suites need it.
- `ActionMoveTo`/`cls_0x4d8d70` ignore argv; the other four take the data root.
- exe contention: build as `<Suite>.B.exe`; never the unsuffixed shared name (C-4).

## T-001 — MSVC + boot attribution recipe (C-4/C-9 protocol)

`build/F3.exe` is shared output of `msvc-build`. To get evidence attributable to me, force the
link in the same chain as the boot check:

```sh
rm -f build/F3.exe && msvc-build            # exit 0, grep -c 'error C' == 0
stat -c '%y %s' build/F3.exe                # 2026-10-20 02:47:54.649768262 -0400 29696
timeout 8 f3-reimpl-run-data; echo app=$?   # app=124 == boots and stays alive
stat -c '%y %s' build/F3.exe                # unchanged => the booted binary is mine
```

Note: an up-to-date tree makes `msvc-build` a silent no-op (0 `Building`/`Linking` lines), so the
mtime stays someone else's — always `rm -f build/F3.exe` first when you need attribution.

## T-002 — `run-all-tests` + `cppcheck-openje` (tooling lane)

Files: `devenv/scripts/run-all-tests.nix` (new, registered in `devenv/scripts/default.nix`),
`devenv/scripts/cppcheck-openje.nix` (rewritten).

`run-all-tests` behaviour:
- builds + runs the six suites and the app-boot check; one line per suite; `summary:` line;
  exit 1 on any failure. Exes go to `.devenv/state/tests/<Suite>.exe` (never a shared
  `tests/je/**` name, so it cannot collide with A's or my manual `.A`/`.B` binaries).
- every suite is `rm -f`'d then compiled, and compile status + file existence are both checked
  before wine is invoked (stale-exe invariant).
- flags: `--only Suite[,Suite]`, `--no-app`, `--keep`, `--timeout N`, `-v` (compiler/wine output).
- env overrides (added so the tool can be pointed at a *different* checkout):
  `OPENVB_TEST_WINEPREFIX`, `OPENVB_TEST_DATA_DIR`, `OPENVB_TEST_EXE`.
- app boot: copies `build/F3.exe` to `<dataDir>/F3.run-all-tests.exe` and runs the copy
  (chat/from-A.md C-10: the shared exe is often mid-link). The copy must live in the data dir,
  not `/tmp`, because the engine resolves `granny2.dll`/`binkw32.dll` next to the exe. It also
  stats the source before/after the copy and prints a WARNING if it changed = concurrent build.
- listed sources that don't exist yet are skipped with a `note:` line, because `JE/SpatialGrid.cpp`
  is A-lane and untracked at the time of writing while `JE/Map.cpp` already references it.
- Current source table requires `src/F3/MapEntitySpawn.cpp` for `PathManager` and `ActionMoveTo`
  (`F3::GetActiveMap()` is defined there, openje `18a8e00`) and `JE/SpatialGrid.cpp` for every
  suite that links `Map.cpp`. Expect to update the table whenever A adds a new cross-module call.

### devenv reload reality (this harness)
`devenv shell` hot reload does NOT reach my tool shells: each `bash` call is a fresh child of the
harness process, whose PATH is the frozen env snapshot from shell start (reload files in
`/tmp/devenv-reload-<pid>.sh` were rewritten, but only the interactive shell sources them).
Manual reload that works, ~6s:

```sh
cd <repo root> && devenv print-dev-env > /tmp/env.sh 2>/dev/null && source /tmp/env.sh
command -v run-all-tests   # -> /nix/store/<new hash>-run-all-tests/bin/run-all-tests
```
Verify the *payload* not the wrapper when checking that a reload took:
`.../bin/<script>` is a 2-line stub that execs `/nix/store/...-<script>`; grep that file.

### Proving green while the other implementer is mid-edit
`git worktree add --detach /tmp/B-head HEAD` +
`git -C /tmp/B-head submodule update --init modules/openje` (then, because A's commits are local
only: `git -C /tmp/B-head/modules/openje fetch <main>/modules/openje <sha> && git checkout FETCH_HEAD`;
same trick for `modules/tracing`) + `run-all-tests` with the `OPENVB_TEST_*` overrides. Cleanup:
`git worktree remove --force /tmp/B-head`. This is how I ran the deliberate-typo negative test
without touching A's files.

### cppcheck findings owned by lane A (as of parent d4a9e30 / openje 18a8e00)
`cppcheck-openje modules/openje/src/JE/Map.cpp` = 16 findings: 11 `cstyleCast`, 2 `memsetClassFloat`,
2 `constVariablePointer`, 1 `invalidPointerCast` (from `Map.hpp:128`'s reinterpret_cast).
Changed-set sweep over A's in-flight files = 30 (PathManager.cpp 25, ActionMoveTo.cpp 4, Map.hpp 1).
I do not fix engine sources and I did not suppress them; filed in chat instead.

### Counting pitfalls (bit me once, and A's C-10 warns about the same class)
- cppcheck prints findings to **stderr**; `cmd 2>/dev/null | grep -c ...` reports 0 findings and
  exit 0. Use `2>&1`.
- `grep -c` exits 1 on zero matches, so never put `&& cp`/`&& run` behind a count grep (same
  class of bug A hit per chat/from-A.md C-10; it silently skipped my verification once too).
- a finding count is therefore: `cppcheck ... 2>&1 | grep -cE ' (style|warning|portability|performance): '`.

### How to test a devenv script's failure paths without touching shared state
Devenv scripts are plain files on `PATH` (not shell functions), so a stub on a prepended `PATH`
overrides anything the script calls:

```sh
mkdir -p /tmp/B-stubbin && printf '#!/usr/bin/env bash\ncase "${STUB_MODE:-ok}" in ... esac\n' > /tmp/B-stubbin/msvc-build
chmod +x /tmp/B-stubbin/msvc-build
cd /tmp/B-head2                       # git worktree of HEAD
PATH=/tmp/B-stubbin:$PATH bash "$(payload-of-run-all-tests)" --msvc-build --no-app --only cls_0x4d8d70
```
Notes: NixOS has no `/bin/bash` (a `#!/bin/bash` stub exits 126 — that is the *stub* failing, not
the script); the runnable payload is the `/nix/store/...-<name>-script` file the `bin/<name>`
wrapper execs, and that's the file to `bash -n` / grep after every reload.

What this caught that reading did not: my first `--msvc-build` block ran **before** the
`fails/passed/summary` initialisation, so `set -u` aborted on first use ("summary: unbound
variable"). Any new flag that emits into `summary` must sit below the counter init. Also verified:
`kind=` extraction reports `LNK2019` / `error C2065` / `undefined reference`, and a hidden source
degrades to a `note:` line + a classified `BUILD-FAIL`, exit 1.

Standard gate from now on (replaces hand-written loops):
`run-all-tests --msvc-build` -> `msvc=OK` + 6 `OK:` + `app=124`, exit 0.

## T-004 prep — EMAP ground truth from the shipped maps (read-only, no test edits yet)

Walker: `/tmp/B-emapwalk.py` (RHT ftype 1300 -> `<pack>.grp` lump -> chunk walk
`{tag,version,size}`, size includes the 12-byte header -> EMAP body decode in
`Map::ReadEMAP`/0x56C9D0 field order). Values below are from the **files**.

| map (grp/lump) | EMAP ver/size | fog(v>=1) | +0x1D8 | ambient(v>=3) | +0x1D9 | k0 | extras (f32) | FINAL +0x113..116 |
|---|---|---|---|---|---|---|---|---|
| Default_StartMap (Engine/17) | 5 / 110 | `00 ff 80` | 0 | `00 00 00 64` | 1 | 0 | 250, 500, 1.0 | `00,00,00,64` |
| Mainmenu (Maps/152) | 5 / 86 | `73 6f 62` | 0 | `29 24 1f 60` | 0 | 0 | 0, 0, 1.0 | `29,24,1f,60` |
| 00_03_Tutorial_Junktown (Maps/339) | 5 / 122 | `53 58 6c` | 0 | `00 00 00 60` | 1 | 0 | 250, 500, 0.0 | `00,00,00,60` |

Consequences for T-004 (and for the Map.cpp comments):
1. **T-004 item 1 is confirmed for Mainmenu: `29,24,1f,60`** — and the alternative `...,80` is
   **unreachable from any shipped map**: all three v5 files carry the v>=3 ambient quad, which
   overwrites the v1 fog store including the `+0x116 = 0x80` sentinel (Map.cpp 0x56cac7 path). So
   the `0x80` state must be produced by a **synthetic v1/v2 stream**, not by a real file. Do not
   write `check(fog==29&&..==80 || ...)` as an OR-of-two-realities; write the real one for the
   file and the `0x80` one for the fake stream.
2. `Mainmenu`'s extras are `0.0, 0.0, 1.0`, not `250/500/x`. The existing
   `ResourceSystem.cpp` assert (`mbr_0x1B8==250.0f && 1BC==500.0f && 1C0==1.0f`) is on
   **Default_StartMap** and stays correct; the Map.cpp comment "shipped files carry 250.0/500.0/
   {0,1}.0 (probe on 4 real maps)" is over-broad — Mainmenu is a counterexample. Worth a comment
   fix in A's lane (I am not editing Map.cpp).
3. `+0x28` = 0 in all three; the 4th EMAP string is empty in all three; strings are
   `<map>.8` / `<map>.rle` / `<map>_AM.dds` (so the ambient-texture name is the `_AM.dds`, whose
   trailing `64`/`60` byte in the ambient quad is presumably a strength/alpha — semantics unknown).
4. `k0..k3` are **all zero** in the shipped maps, and `mbr_0x1B4 = 0xFF000000` there — the `ff`
   comes from the expression's own `0xFFFFFF00` sentinel, not from data
   (`t1=((k1|0xFFFFFF00)<<8)&m = 0xFFFF0000; t2=((k2|t1)<<8)&m = 0xFF000000; r = k3|t2`).
   Careful with this one: every shift must be masked to 32 bits, and unmasked python bignums give
   a *different* answer, so mirror it in C or mask each step. A synthetic `k1=0xff` body gives
   `0xFFFF0000` (that is what the new test asserts). Combined with the
   `+0x1C8/+0x1CC/+0x1D0/+0x1D4` mirror copies that means the v5 tail is a duplicated
   (flags, x, y, z) block, which matches the "3 float extras" assert already in the suite.
5. Chunk chains observed: Default_StartMap `EMAP+EMEP+ECAM+EMNP`; Mainmenu
   `EMAP+3xEME2+ECAM+EMNP+EMEF`; Junktown `EMAP+EME2...`. Only Default_StartMap still routes EMEP
   (the non-routed 0x573B00 reader) — consistent with Map.hpp's note that EMEP is not dispatched
   by the shipping router.

Blocked before editing: T-004 touches `modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp`,
which is on implementer-A's C-5 intent list (they also own `Map.cpp`). Ownership asked in
`chat/from-B.md` C-14; submodule currently clean at `b437a1a`, so a claim+edit is safe the moment
they confirm.

## Infra note (2026-10-20 ~03:30)
`ida_original_f3` / `ida_reimplementation_openvb` MCP **tools disappeared from the pi harness
registry** ("Tool ... not found", then `mcp_tools_search` returns other servers), while
`devenv list_processes` shows both `ida-*-headless` as `ready` and `ss` shows 8745/8746 LISTEN
(pids 12324/12326). So the servers are healthy; the harness's client connection dropped — needs a
`/mcp` reconnect from the operator, not a restart (a restart would cut A off mid-analysis).
`ida-orig-gdb` shows `exited` (A's stale Oct-19 stub; harmless).

## T-004 delivered — what the byte-store tests found

Tests added to `modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp` (T-004): real-file
Mainmenu/Default_StartMap byte asserts + synthetic v0/v2/v3/v5 `ReadEMAP` bodies through a
hand-built `MemStream` (only `{start,end,cursor}` are touched, so no `Res` is needed).

Findings, all verified against the ORIGINAL (objdump on `$OPENVB_ORIG_EXE` + IDA `decompile`,
never against our code):

1. **`Map::ReadEMAP` mirror block converts instead of copying.** Original is three plain 4-byte
   moves (`mov [esi+0x1cc],eax` @0x56CCD4, `[esi+0x1d0],ecx` @0x56CCDA, `[esi+0x1d4],edx`
   @0x56CCE0) = raw float bit patterns. Our port assigns `dword = float` (Map.cpp 0x56CCD4
   comment block), producing 250/500/1 instead of 0x437A0000/0x43FA0000/0x3F800000. Pinned as a
   KNOWN-ISSUE check so the fix fails-then-passes visibly.
2. **`Map::Map()` ctor defaults diverge from `ctor_0x57b330`** (size 0x54b, verified as
   `JE::Map::ctor_0x57b330` by `lookup_funcs`): original writes 0xC0 to +0x110..+0x115
   (`mov al,0xc0` @0x57B4CE, stores @0x57B4DD..0x57B4FB), 0x80 to +0x116 (@0x57B501), 250 to
   +0x11C (@0x57B50E), 0xFF808080 to +0x1B4 (@0x57B5F0), **250.0f/500.0f** to +0x1B8/+0x1BC
   (@0x57B5FA/0x57B604 = 0x437A0000/0x43FA0000), and 1/1 to +0x1D8/+0x1D9 (@0x57B618/0x57B61F).
   We zero them and use 16384.0f/524288.0f — and `Map.hpp`'s member comments attribute those
   wrong numbers to this very ctor, so the comment is not just stale, it is mis-sourced.
3. **The v1 fog branch's temp read leaves the raw fog bytes at +0x110..+0x112 in the original**
   (reads at 0x56CA4F..0x56CA9D target `&mbr_0x110+0/1/2`, then shifts up to +0x113..+0x115).
   Our port reads into locals, so those three bytes stay at default. Pinned (KNOWN-ISSUE).
   Harmless today (nothing reads +0x110..+0x112) but it is an observable state difference.
4. Version gating re-confirmed from the decompile: `if (v3)` = version>=1 (fog), `>=2`
   (+0x1D8 from `!=0`), `>=3` (ambient quad overwrites +0x113..+0x116 including the 0x80 slot),
   `>=4` (+0x1D9), `>=5` (k0/k1k2k3 + 3 float extras + mirrors). So the `0x80` sentinel is only
   observable for version 1-2 files, and none shipped — hence the synthetic streams.

## run-all-tests: cross-lane coupling hooks (post-T-002)

- `OPENVB_TEST_EXTRA_SOURCES="path1.cpp path2.cpp"` appends engine sources to every suite. Use it
  when a lane-A `.cpp` exists but is not committed yet (e.g. `modules/openje/src/JE/Gregorian.cpp`
  while the clock lands) so the gate can be green without editing the table or their files. Say so
  in chat when you rely on it — the table should absorb the file once it is committed.
- On `BUILD-FAIL` the line now prints `kind=<error Cxxx|LNKxxxx|undefined reference>` plus
  `missing: <up to 3 symbols>` extracted from the link log, so the reader knows immediately whether
  it is a syntax error, a link error, or a missing source. Verified live against A's in-flight
  Gregorian breakage (`missing: JE::Gregorian::AddDays(unsigned int) JE::Gregorian::AddSeconds(...)`).
- `set -u` + optional env vars: any new `$VAR` reference in the script must be `${VAR:-}`; I shipped
  an "unbound variable" abort on first use of the hook and caught it in one run.

## T-007 RE checkpoint (read-only; implementation still awaiting A's review)

- **G2 solved as a side effect of T-005:** the 9 inline records at `Window+0x90` are
  `GUI::Fragment` (`Window::ctor_0x4b0810` @0x4b08c2 vector-ctor with obj_size 0x158, count 9,
  `Fragment::ctor_0x4b24a0`). My earlier "stride 86 bytes" note here was wrong — the read loop
  `add edi, 56h` / `p += 86` is in **DWORDs** = 344 = 0x158 ✓ matches the vector ctor size.
- **G1 (partially):** `byte_707CE4` is a texture-manager singleton pointer. Observed slot roles:
  `+0xC4`(196) name->index (negative = absent; `Skin::LoadDefault` clamps it to 0 at 0x5236c5 and
  the Fragment reader stores it at `*(this+74)+304`), `+0xDC`(220) "is it registered" probe
  (nonzero -> skip the Demand: 0x523641 / 0x4b21c1), `+0xE0`(224) add/lock by name (0x505c3f),
  `+0xD8`(216) release/forget by name (0x5236e7, right after `Release_0x498eb0`).
  Consumers span GUI Picture/Label, AreaMap icons, cursor manager, container item icons,
  VFX sprite/model/trail, Octree2Gfx and Gfx_HeightFog — i.e. one global texture registry shared by
  GUI and 3D, which is why the class name matters: candidate `JE::Gfx_Texture_Manager`
  (`LoadTextureFromFile_521b10` / `LoadTextureFromBuffer_521ca0` live in that already-named class),
  **not yet confirmed** by finding the writer of 0x707CE4 (the xref list is long and every hit I
  sampled was a consumer). Left as an open item rather than guessed.
- Consumer shapes worth remembering for the port: `GUI::Picture::dtor_0x4aaaa0` @0x4aaae6 releases
  by name, and `Picture::meth_0x4aaf10` @0x4ab0bc (plus `virt_meth_0x4ab1f0` @0x4ab35f/3a4/3de)
  are where a Picture resolves/re-resolves its texture — that is the code path T-007 step 3/4
  must reproduce, and it is why the skin record keeps the NAME, not the handle.
