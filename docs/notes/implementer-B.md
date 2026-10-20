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
