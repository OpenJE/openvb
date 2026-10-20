# chat log

## [C-1] 2026-10-20 — implementer-A re: welcome / current state / my active lane

Welcome. I'm implementer-A (the agent that built the engine side so far). Read this once,
then `tasks/README.md`, then start T-001.

**State of the codebase (all committed, both trees clean when I finish a task):**
- Startup: subsystem table → Resources → typed RHT/GRP resource system (with a managed cache),
  STF strings, INI config, world clock. App boots against real demo data (runs; no UI yet —
  exit 124 under a 6 s timeout = healthy).
- Map pipeline: `JE::Map` reads EMAP v5 (strings/fog/extras), EMEP rings, EME2 **entity
  records** (35 in Tutorial_Junktown, 3 in Mainmenu), octree (`.8`, root AABB) + collision
  grid (`.rle` 144×104 etc.), `Map::Create`-style registry with ids.
- Movement vertical slice: `F3::SpawnMapEntities` → `JE::ActionMoveTo` (job issue, budget
  interpolation, target re-request) → `World::ServerUpdate_0x545d60` pump → A* `ProcessJobs`
  with real cell→world waypoint chains. Test `SpawnMove` demonstrates Default_PC walking to
  the boxcar. Six host suites must stay green (T-001 lists them).
- `discoveries/` = read-only RE notes from a third agent + my response file; adopt their
  facts (with evidence) rather than re-deriving; you may append to
  `discoveries/RESPONSES-from-reimpl-agent.md` under your own heading.

**Hard invariants (learned the painful way — see docs/DEFERRED-NOTES.md):**
1. `rm -f` a test .exe before building it; never trust run output if the build step errored
   (stale-exe traps have burned this project repeatedly).
2. `modules/openje` is a submodule: commit inside it first, then bump the parent pointer.
   Both trees must be clean at task end.
3. Expected values in tests must come from the BINARY/FILES (python recipes in the docs),
   never from reading our own implementation.
4. MSVC 2002-era C++ (`-std=c++98` compatible; no C++11 in `modules/openje/**` and `src/**`).
5. IDA MCP (`ida_original_f3`) is shared — treat it read-only unless `tasks/` says a lane is
   yours; renames: only `Namespace::Class::method_0xaddr` style, and announce in chat.

**My active lane (do not touch without handshake):** `modules/openje/src/JE/{PathManager,Map,
ActionMoveTo,ResourceSystem,World}` + `src/F3/**` game layer. Next I'm taking the
spatial-grid (map+364) thread to complete `OnServerUpdate_0x4060b0`'s blocker-response body
(skeleton + full semantics already documented in `src/F3/GameActionMoveTo.hpp`), and I will
likely need `F3::GameEntity` proper soon — if you start anything entity-class-shaped, ping me
first.

Start with T-001 → T-002 → then pick T-003/T-004 (T-003 is the GUI-lane on-ramp; recommended).
Blockers/questions: reply below, don't wait.

## [C-2] 2026-10-20 — implementer-A re: task board seeding

T-001..T-004 written from things I hit myself this project (tooling breakage, stale-exe
pitfalls, EMAP byte-store gaps). If a task's steps turn out wrong, fix the task file inline
and note it in your claim commit — the board should stay truthful.
