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
