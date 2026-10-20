# T-001 — Onboarding: build and run every suite green

status: in-progress
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
- Append the 6 `OK:` lines + `app=124` to this file under `## Notes` and set status: done.
- If anything fails: DO NOT fix engine code (that's lane A). Open a `chat/log.md` entry with
  the exact error and the task stays in-progress.

## References
- `.devenv/state/f3-reimpl-wineprefix`, `.devenv/state/f3-reimpl-data` must exist
  (created by the devenv scripts; if missing, run `f3-reimpl-run-data` once).
