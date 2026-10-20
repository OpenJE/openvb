# T-002 — Tooling: fix `cppcheck-openje`, add `run-all-tests`

status: done
assignee: implementer-B

## Objective
Two known tooling papercuts, both self-contained:

1. `cppcheck-openje` (devenv script) references an `openje/` symlink that no longer exists.
   Working invocation (verified): run from `modules/` with
   `cppcheck --language=c++ --std=c++03 --enable=warning,style --inline-suppr
   --suppressions-list=openje/.cppcheck-suppress --quiet -I openje/src -I tracing/src
   openje/src/JE/<file>.cpp`. Fix the script to match (and let it take file args, default =
   all `src/JE/**.cpp` changed vs HEAD).
2. Add a `run-all-tests` script that builds + runs all six host suites and the app boot check
   (exit 124 = pass), printing one line per suite and a nonzero final exit on any failure.
   T-001's link lines are your source; put the per-suite source lists INTO the script (copy
   them from the test file headers).

## Notes (evidence, implementer-B, 2026-10-20)

### Deliverable 1: `cppcheck-openje` — fixed (script), findings remain (lane A)
Broken invocation reproduced first: `cppcheck: error: couldn't open the file:
"openje/.cppcheck-suppress"` (exit code was masked by the pipeline). New script runs from
`modules/` with the verified flags, takes file args in either spelling
(`modules/openje/src/JE/X.cpp` or `openje/src/JE/X.cpp`), and defaults to every existing
`src/JE/**.cpp` that differs from HEAD (tracked-modified + untracked-new).
Tested: (a) explicit arg, (b) short arg, (c) no-arg on a dirty tree -> `2 file(s) changed vs HEAD`,
(d) no-arg on a clean tree -> `nothing changed...` + exit 0 (this needed an explicit `|| true`,
because `set -euo pipefail` aborted on the empty grep match — recorded in my journal).

**Acceptance item "0 findings on `openje/src/JE/Map.cpp`" was not met at the time and was
mine neither to fix nor to hide (resolved later — see "Resolution of the last line"):**
16 findings at parent `5a953bd` / openje `b437a1a` -> 11 `cstyleCast`, 2 `memsetClassFloat`,
2 `constVariablePointer`, 1 `invalidPointerCast` (`Map.hpp:128`). Whole changed-set sweep of A's
in-flight files: 30 (PathManager.cpp 25, ActionMoveTo.cpp 4, Map.hpp 1). Known-clean contrast:
`modules/openje/src/JE/Action.cpp` -> 0 findings, exit 0. I did not edit engine sources and did
not hide these in `.cppcheck-suppress`; filed as `HANDSHAKE:` in `chat/from-B.md` (either fix in
lane, or authorize a per-file baseline and I flip this task to done).

### Deliverable 3 (added from your C-11 action list): trap encoding + opt-in rebuild
`run-all-tests` now also takes `--msvc-build` (rm -f the shared exe first, so a silent no-op can
never masquerade as my build) and classifies failures by *kind* instead of counting only
`error C`: `kind=LNK2019|error C2065|undefined reference` shows up in the suite line and in the
`summary:`. `set -o pipefail`, `rm -f` before every compile and before the boot `cp`, and a
source-mtime/size diff around the copy (prints `WARNING: source exe changed during copy`) are all
in place. Validated with a stubbed `msvc-build` on `PATH` inside a throwaway worktree (never the
shared tree): clean -> `msvc=OK`, `LNK2019` -> `msvc=FAIL (error C=0, LNK=LNK2019)` exit 1,
`error C2065` -> `msvc=FAIL (error C=1, LNK=none)` exit 1; hiding `JE/Action.cpp` ->
`note: ... source not present, skipped` + `ActionMoveTo BUILD-FAIL (exit 1)
kind=undefined reference` exit 1. That stub run also caught a real bug in my first draft: the
`--msvc-build` block ran before `fails/passed/summary` were initialised, so `set -u` aborted on
the first use — fixed by hoisting the counters above the block.

### Deliverable 2: `run-all-tests` — DONE, green, and proven to fail loudly
`.B`-suffix/manual driver replaced by the real devenv script; registered in
`devenv/scripts/default.nix`. Manual reload needed in this harness (`devenv print-dev-env` +
source — hot reload never reaches non-interactive child shells; see journal).

Full run, main tree (openje `b437a1a`):
```
cls_0x4d8d70    OK   OK: cls_0x4d8d70 registry tests passed
PathManager     OK   OK: path manager tests passed
ResourceSystem  OK   OK: ResourceSystem RHT parse tests passed (7044 entries)
StringSystem    OK   OK: StringSystem/ESS/World-clock tests passed (3281 strings)
ActionMoveTo    OK   OK: ActionMoveTo tests passed
SpawnMove       OK   OK: spawn+move vertical slice passed
app-boot        OK   exit 124 (alive past 6s) exe: 2026-10-20 03:17:13.500843410 -0400 40960
summary: ... => 7 pass, 0 fail                       ; run-all-tests exit=0
```
Definitive run after A's `29b1036` (includes the opt-in rebuild, so the booted binary is
attributable to this run):
```
msvc-build      OK   exit 0, error C=0, no LNK
...6 suites OK...
app-boot        OK   exit 124 (alive past 6s) exe: 2026-10-20 03:26:41.525109900 -0400 40960
summary:  msvc=OK cls_0x4d8d70=OK PathManager=OK ResourceSystem=OK StringSystem=OK
          ActionMoveTo=OK SpawnMove=OK  app=124  => 8 pass, 0 fail        ; exit=0
```
Same 7 greens reproduced independently in a throwaway `git worktree` of HEAD
(parent `d4a9e30`, openje `18a8e00`) via `OPENVB_TEST_WINEPREFIX/DATA_DIR/EXE` overrides, so the
result is attributable to committed code rather than to A's mid-edit tree.

Negative tests (deliberate break, in the worktree only, then reverted -> `git status` clean):
```
compile typo (main renamed away):  StringSystem BUILD-FAIL (exit 1) ; run-all-tests exit=1
wrong expected value (2253->2254): StringSystem FAIL (exit 1) "FAIL: start year 2254" ; exit=1
```
Deviations from the task text (documented, not silent): exes go to `.devenv/state/tests/` instead
of `tests/je/**` (C-4 contention), the app check boots an agent-tagged copy inside the data dir
(C-10 race + DLL adjacency), and per-suite source lists had to grow (`JE/SpatialGrid.cpp`,
`src/F3/MapEntitySpawn.cpp`) because `Map.cpp`/`GamePathManager.cpp` now reference them.

## Gotchas (learn these the easy way)
- ALWAYS `rm -f` the .exe before compiling and check the compiler exit status before running:
  a stale exe from a previous build has repeatedly masqueraded as a "passing/regressing" test.
- Test exes are gitignored (`modules/openje/.gitignore`); scripts belong in `devenv/scripts/`
  (registered via `devenv/scripts/default.nix` or the scripts set) — follow
  `devenv/scripts/f3-reimpl-run-data.nix` as the template.

## Acceptance
- `cppcheck-openje` runs clean (0 errors/warnings output) on `openje/src/JE/Map.cpp`.
- `run-all-tests` prints 6 `OK:` lines + `app OK` and exits 0; introduce a deliberate typo in
  one test temporarily and show it exits nonzero (then revert).
- Evidence + exit codes pasted under `## Notes`; status: done.

### Acceptance checklist status
- [x] `run-all-tests` prints 6 `OK:` lines + `app OK` and exits 0 — evidence above (main tree and
      a clean worktree of HEAD).
- [x] deliberate typo -> nonzero exit, then reverted — evidence above (both compile-break and
      wrong-expected-value variants, exit=1 each).
- [x] `cppcheck-openje` script fixed: runs the verified invocation, takes file args, defaults to
      `src/JE/**.cpp` changed vs HEAD.
### Resolution of the last line (2026-10-20, after chat/from-A.md C-20)
A fixed 7 of the 16 (their Map edits) and authorized option (b) for the rest. I added a
**file-scoped, auditable baseline** to `modules/openje/.cppcheck-suppress` (openje `7b5fe96`
equivalent, commit "cppcheck: T-002 baseline ..."): `cstyleCast`+`memsetClassFloat`+
`constVariablePointer` on `openje/src/JE/Map.cpp` and `invalidPointerCast` on
`openje/src/JE/Map.hpp`, each block commented with the in-lane fix. Verified:
`cppcheck-openje modules/openje/src/JE/Map.cpp` prints **nothing**, exit 0; and
`cppcheck-openje modules/openje/src/JE/PathManager.cpp` still prints **25** findings, so the
baseline masks nothing else.

**Why A saw "0 findings" while I saw 16/7/3** (reproducible, worth guarding everywhere):
- the pre-fix `cppcheck-openje` aborted with `cppcheck: error: couldn't open the file:
  "openje/.cppcheck-suppress"` and **still returned exit 0** — a pipeline like
  `cppcheck-openje | grep -c ...` therefore looks perfectly clean;
- cppcheck writes findings to **stderr**, so any `2>/dev/null` (or a count regex needing a
  leading colon) reports 0. Correct count form: `cppcheck-openje <file> 2>&1 | grep -cE
  ' (style|warning|portability|performance): '`.
I reported the numbers rather than assuming my tool was right, and A's fix-then-baseline split
settled it. T-002 is now `done`.

Follow-on tooling (same lane, after this task closed): `run-all-tests` gained
`OPENVB_TEST_EXTRA_SOURCES` (so a lane-A file that is still untracked, e.g. `JE/Gregorian.cpp`,
cannot turn the shared gate red) and `BUILD-FAIL` lines now print the missing symbols next to
`kind=` — both verified live against A's in-flight clock work.

### Original blocking note (kept for the record)
- [x] `cppcheck-openje` **0 findings on `Map.cpp`** — RESOLVED (see "Resolution of the last
      line" above): A fixed 7 of the 16 in lane; the remaining 3 classes are baselined
      file-scoped in `modules/openje/.cppcheck-suppress` with their in-lane fixes named.
      Verified: 0 output / exit 0 on `Map.cpp`, and `PathManager.cpp` still reports 25 findings,
      so nothing beyond the three named classes is masked.
