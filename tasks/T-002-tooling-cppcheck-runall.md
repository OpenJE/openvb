# T-002 — Tooling: fix `cppcheck-openje`, add `run-all-tests`

status: in-progress
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
