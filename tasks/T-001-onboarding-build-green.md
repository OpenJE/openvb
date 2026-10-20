# T-001 — Onboarding: build and run every suite green

status: done
assignee: implementer-B

## Objective
Prove your environment can build the reimpl and run the whole test matrix. This is the
baseline every later task relies on; it also teaches you the two build paths.

## Steps
1. Read, in this order: `docs/f3.md`, `docs/DEFERRED-NOTES.md`, `.pi/skills/openvb-reimpl-workflow/SKILL.md`,
   `.pi/skills/openvb-ida-mcp/SKILL.md`.
2. `msvc-build` (MSVC 2002 in Docker/container via devenv). Expect `error C…` count 0.
3. Run each host suite under wine (they link `modules/tracing` stubs; cwd for data-based ones
   does not matter, they take the staged root as argv[1]):

```sh
i686-w64-mingw32-g++ -std=c++98 -static -w -Isrc -Imodules/openje/src -Imodules/tracing/src \
  modules/openje/tests/je/<Suite>/<Suite>.cpp <impl sources...> -o modules/openje/tests/je/<Suite>/<Suite>.exe
WINEPREFIX=$PWD/.devenv/state/f3-reimpl-wineprefix WINEDEBUG=-all \
  wine modules/openje/tests/je/<Suite>/<Suite>.exe "C:\\f3data"
```

   Suites: `cls_0x4d8d70`, `PathManager`, `ResourceSystem`, `StringSystem`, `ActionMoveTo`,
   `SpawnMove` (needs `src/F3/MapEntitySpawn.cpp src/F3/GameActionMoveTo.cpp` + the JE
   sources; see the header comment in each test file for its exact link line).
4. Boot the app: `f3-reimpl-run-data` (or run `build/F3.exe` under wine from
   `.devenv/state/f3-reimpl-data`). It must STAY ALIVE (timeout kill → exit 124) — that is
   "boots and runs"; there is no visible window yet.

## Acceptance
- Append the 6 `OK:` lines + `app=124` to this file under `## Notes

### Corrections to this task's steps (kept truthful per chat/from-A.md C-2)
- "see the header comment in each test file for its exact link line" is only true for
  `ResourceSystem`, `StringSystem`, `ActionMoveTo` — and even those are **incomplete** (they do
  not link as written; see below). `PathManager` and `SpawnMove` have **no** build line at all,
  and `cls_0x4d8d70`'s line names a nonexistent `test.cpp`. The verified link lines are below
  and are now the source for `run-all-tests` (T-002).
- Header-line gaps found: `ResourceSystem` needs `Resources.cpp Map.cpp VFX_Parameter.cpp
  Common/Chunk.cpp Subsystem.cpp JE.cpp`; `StringSystem` needs `Common/Chunk.cpp PathManager.cpp
  Map.cpp` added (World.cpp pulls `JE::PathManager::ProcessJobs`, PathManager pulls Map helpers);
  `ActionMoveTo` needs `JE/Action.cpp` (base `JE::Action` vft) and `JE/World.cpp`.
- Suites take the data root as argv[1]; `C:\f3data` works via the wine prefix's Z: drive only
  because the tests resolve it through the staged dir — passing `Z:<abs>/.devenv/state/f3-reimpl-data`
  works too. `ActionMoveTo`/`cls_0x4d8d70` ignore argv.

### Evidence (2026-10-20, implementer-B, clean-tree state: parent `525bbf5`, openje `3052d72`)
All six rebuilt from scratch with `rm -f` first, as `<Suite>.B.exe` (exe-contention protocol,
chat/from-A.md C-4). Driver script: `docs/notes/implementer-B.md` §T-001 (superseded by
the `run-all-tests` devenv script delivered in T-002).

```
cls_0x4d8d70    compile=0 OK: cls_0x4d8d70 registry tests passed              exit=0
PathManager     compile=0 OK: path manager tests passed                       exit=0
ResourceSystem  compile=0 OK: ResourceSystem RHT parse tests passed (7044 entries)  exit=0
StringSystem    compile=0 OK: StringSystem/ESS/World-clock tests passed (3281 strings) exit=0
ActionMoveTo    compile=0 OK: ActionMoveTo tests passed                        exit=0
SpawnMove       compile=0 OK: spawn+move vertical slice passed                 exit=0
```

MSVC build + app boot, back-to-back with binary attribution:

```
$ rm -f build/F3.exe && msvc-build   -> exit=0, `grep -c 'error C'` = 0
$ stat -c '%y %s' build/F3.exe       -> 2026-10-20 02:47:54.649768262 -0400 29696
$ timeout 8 f3-reimpl-run-data       -> app=124   (stayed alive; killed by timeout = healthy)
$ stat after                        -> 2026-10-20 02:47:54.649768262 -0400 29696 (unchanged, B-attributed)
```` and set status: done.
- If anything fails: DO NOT fix engine code (that's lane A). Open a `chat/log.md` entry with
  the exact error and the task stays in-progress.

## References
- `.devenv/state/f3-reimpl-wineprefix`, `.devenv/state/f3-reimpl-data` must exist
  (created by the devenv scripts; if missing, run `f3-reimpl-run-data` once).
