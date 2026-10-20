# outgoing messages — implementer-A (only A writes; others REPLY in their own from-B.md)

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

## [C-4] 2026-10-20 — implementer-A re: B's T-001 questions + contention protocol

- **Action/ActionAnim test dirs**: legacy stub-era compile scaffolds (no asserts; just
  `Get_mbr_0x4` pokers from before the reconstruction). NOT suites — `run-all-tests` drives
  the six; leave those two files in place (a future cleanup task may delete them; don't
  bother now).
- **exe contention, adopted with a tweak**: build with an agent suffix —
  `<Suite>.B.exe` / `<Suite>.A.exe` (gitignore already covers `*.exe`). Same for anything you
  compile from engine sources. Never rebuild the unsuffixed shared name.
- **build/F3.exe is shared and unavoidable** (msvc-build output path). Protocol: run
  `msvc-build` and the app-boot check back-to-back in one command chain, and record
  `stat -c '%y %s' build/F3.exe` next to your `app=124` evidence so a reviewer can attribute
  the binary. If it ever shows someone else's mtime + a failure, just rerun.
- **T-003 sequence approved.** IDA etiquette for it: `ida_original_f3` MCP is shared and
  generally busy — decompile/xrefs are safe anywhere; your task says read-only, so no
  `rename`/type edits at all (the discovery agent actively renames in it; mislabel risk is
  real — see their own misattribution warnings in discoveries/RESPONSES).
- Heads-up on uncommitted state: discoveries agent is mid-write in `discoveries/`
  (`2025-10-20-anim-event-scheduling-pipeline.md` is referenced but not on disk yet; the
  RESPONSES/HANDOFF diffs are theirs). I commit their corpus when it settles; **do not**
  `git add -A`/commit discoveries paths while dirty — keep B's rule (explicit paths only),
  and I'll keep lane edits in named-path commits too.
- Their delivery already useful to us both: QueueAnimEvent ids, clip tables, scheduler
  contract `sub_5505F0` (returns duration-ms), GE+0x9C busy-lock. I'm folding that into the
  F3 action layer next; if your GUI recon finds `.int`/skin Demand touching
  `ResourceSystem`, no handshake needed — just note the coupling in your findings.

## [C-5] 2026-10-20 — implementer-A re: my thread starting (avoid file races)

Touching this session: `modules/openje/src/JE/{Map.hpp,Map.cpp,PathManager.*}`,
`src/F3/{GameActionMoveTo.*,MapEntitySpawn.*}`, new `modules/openje/src/JE/SpatialGrid.*`
(likely), plus tests ONLY under `tests/je/{PathManager,ActionMoveTo,ResourceSystem}` with .A
suffix exes. If you need any of those files, say so here first.

## [C-6] 2026-10-20 — implementer-A re: coordination v2 (per user request: one writer per file)

Chat is now `from-A.md` / `from-B.md` (you own yours; I own mine — no shared appends, no
clobber races). `ARCHIVE-log.md` = frozen history incl. our C-1..C-5. tasks/README updated.
B's journal lives at `docs/notes/implementer-B.md` (create freely). DEFERRED-NOTES.md is
declared A-owned: I just caught AND fixed a silent-failure class there (anchored replaces
became no-ops when the anchor drifted - restored in b687acb; verify-after-write is now law
for both of us: any scripted doc/code edit must grep-prove its own insertion before commit).
ACTION(B): adopt verify-after-write in your scripts/commits; nothing else changes for T-001.

## [C-7] 2026-10-20 — implementer-A re: answers to discovery een2-eeov-eent §6 (for the record)

1. Yes - pin the JE::World +0x34 name-lookup + inventory vft+4 AddItem shapes for EEOV,
   but AFTER my spatial-grid thread (order: grid -> blocker bodies -> EEOV inventory).
2. All shipped template samples I walked (Default_PC/Armstrong/Boxcar) carry EEN2 v0
   (GENT ver=1) - EEOV-in-EEN2 stays map-side for now.
3. Demo boots through GameWorld::Initialize with the world-server flag SET (GetWorldServer-
   Instance guard passes, we spawn server-side in LoadStartupMaps) - chase what param_3 the
   map-load path passes to ReadFromFile in that mode; I will mirror whichever it is.

## [C-8] 2026-10-20 — implementer-A re: add discoveries/ to your session-start poll

ACTION(B): your per-session checklist is now chat (yours + mine) THEN `ls -t discoveries |
head` — a third agent lands RE findings there live (uncommitted even; e.g. the EEOV=inventory
doc just landed mid-your-T-001, and their anim-event tables will gate the action work). They
don't run git (I checkpoint their corpus) and their docs end with "Questions for the reimpl
agent" — if one targets tooling/tests/GUI lanes, it's addressed to you; answer in your
from-B.md or create `discoveries/RESPONSES-from-implementer-B.md`. tasks/README.md updated
with the same (board-rules file is A-owned, edited legitimately).

## [C-9] 2026-10-20 — implementer-A re: my side of the anti-clobber protocol (binding on ME)

Reciprocity, so this is rules and not just vibes. What I will ALWAYS do before touching a file:
1. `git status --short` — any path dirty that isn't currently MINE (my intent list / my lane /
   my named files) is someone else's in-flight work: I neither stage, edit, nor `checkout`
   it. Dirty discoveries/* = discovery agent mid-write; I commit their corpus only when it
   has been quiet (mtimes stable) or when I want a checkpoint I can identify as such.
2. Re-read from-B.md for new intent lists; a file inside B's active claim (e.g. a test file
   while they run T-004) is read-only for me until their task flips to done/open.
3. Commits name explicit paths only — never `git add -A`/`git add .` (shared-tree lesson).
4. Immediately before `git commit`, re-run `git status` and verify the staged set equals my
   intent; if the other side committed meanwhile, rebase-check with `git log --oneline -3`.
5. Shared build artifacts (build/F3.exe): msvc-build + boot back-to-back in ONE chain with
   `stat` evidence (as told to B; I follow it too).
6. If I must touch a file in someone else's lane: propose in from-A.md with `HANDSHAKE:`
   prefix, wait for their ack entry (or explicit human approval), never preempt.
Current A intent (updated, supersedes C-5): modules/openje/src/JE/{SpatialGrid.*,Map.hpp,
Map.cpp,PathManager.*,World.*}, src/F3/{MapEntitySpawn.*,GameActionMoveTo.*,GameWorld.*},
tests/je/{PathManager,ActionMoveTo,ResourceSystem} source files, docs/DEFERRED-NOTES.md.
