# cmkr Integration for OpenVB

## TL;DR

> **Quick Summary**: Migrate OpenVB from raw CMakeLists.txt to cmkr's declarative cmake.toml format, preserving all existing build semantics exactly — including the tricky generator expressions via `cmake-after` escape hatches.
> 
> **Deliverables**:
> - `cmake.toml` — declarative build configuration replacing manual CMake scripting
> - `cmkr.cmake` — bootstrap script for cmkr auto-download
> - Regenerated `CMakeLists.txt` — committed to git, auto-regenerated on cmake.toml changes
> - Verified equivalent build output (compile + link + run)
> 
> **Estimated Effort**: Short
> **Parallel Execution**: YES - 2 waves
> **Critical Path**: Task 1 → Task 2 → Task 3 → Task 4

---

## Context

### Original Request
Integrate cmkr (https://github.com/build-cpp/cmkr) into the OpenVB repo for declarative TOML-based build configuration.

### Interview Summary
**Key Discussions**:
- **Motivation**: Declarative config (TOML) — want cleaner, easier-to-maintain build definition
- **Submodules**: Keep git submodules as-is — no vcpkg/dependency migration
- **Git strategy**: Commit generated CMakeLists.txt — standard cmkr practice, CI-friendly

**Research Findings**:
- cmkr uses cmake.toml → generates CMakeLists.txt; bootstrap via cmkr.cmake
- cmkr supports conditions, options, variables, target properties — but NOT generator expressions natively
- Compound conditions (Debug OR OPENVB_TRACING) need `cmake-after` escape hatches
- foreach loops in current CMake must be inlined per-target in cmake.toml

### Metis Review
**Identified Gaps** (addressed):
- **Generator expressions**: cmkr can't express `$<$<OR:$<CONFIG:Debug>,$<BOOL:${OPENVB_TRACING}>>` natively — use `cmake-after` injection
- **foreach loops**: `foreach(target_name ...)` must be inlined per-target in cmake.toml — acceptable verbosity tradeoff
- **WIN32_EXECUTABLE**: cmkr supports this as a target property — no issue
- **CMAKE_TRY_COMPILE_TARGET_TYPE**: Set via `[variables]` section — no issue
- **Output directories**: Set via `[variables]` — no issue
- **Toolchain file**: Unchanged, passed via CLI as before — no migration needed

---

## Work Objectives

### Core Objective
Replace the manual CMakeLists.txt with a cmkr-managed build system using cmake.toml, preserving all existing build semantics exactly.

### Concrete Deliverables
- `cmake.toml` at project root
- `cmkr.cmake` at project root (bootstrap script)
- Regenerated `CMakeLists.txt` (committed)
- Build verification: project compiles, links, and runs identically

### Definition of Done
- [ ] `cmake.toml` captures all 3 targets (openje, tracing, F3) with correct properties
- [ ] `cmkr.cmake` bootstraps cmkr successfully
- [ ] Generated `CMakeLists.txt` produces identical build output
- [ ] OPENVB_TRACING option works correctly
- [ ] Cross-compilation with mingw32 toolchain still works
- [ ] MSVC build still works (compile flags, WIN32 macro, link libraries)

### Must Have
- All existing build semantics preserved exactly (no behavior changes)
- cmake.toml is the source of truth (CMakeLists.txt is auto-generated)
- Both Debug and Release configurations work
- OPENVB_TRACING option toggles trace definitions correctly

### Must NOT Have (Guardrails)
- **No vcpkg migration** — keep git submodules as-is
- **No new targets** — just the existing 3 (openje, tracing, F3)
- **No build logic changes** — same compiler flags, same link libraries, same definitions
- **No dependency additions** — cmkr is the only new addition
- **No aesthetic refactoring** — translate what exists, don't "improve" it

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: NO (no test framework in project)
- **Automated tests**: None — this is a build system migration, verified by build output
- **Framework**: N/A

### QA Policy
Every task includes agent-executed QA scenarios using terminal commands (cmake, make, diff).
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation — sequential, each depends on prior):
├── Task 1: Create cmake.toml [quick]
├── Task 2: Add cmkr.cmake bootstrap [quick]
└── Task 3: Generate CMakeLists.txt via cmkr [quick]

Wave 2 (Verification — parallel):
├── Task 4: Verify equivalent build output (Linux/GCC) [quick]
├── Task 5: Verify OPENVB_TRACING option [quick]
└── Task 6: Verify mingw32 cross-compilation [quick]

Critical Path: Task 1 → Task 2 → Task 3 → Task 4-6 (parallel)
```

### Dependency Matrix

| Task | Depends On | Blocks |
|------|-----------|--------|
| 1    | —         | 2, 3, 4, 5, 6 |
| 2    | 1         | 3, 4, 5, 6 |
| 3    | 1, 2      | 4, 5, 6 |
| 4    | 3         | — |
| 5    | 3         | — |
| 6    | 3         | — |

### Agent Dispatch Summary

- **Wave 1**: 3 tasks — T1 `quick`, T2 `quick`, T3 `quick`
- **Wave 2**: 3 tasks — T4 `quick`, T5 `quick`, T6 `quick`
- **FINAL**: 4 tasks — F1 `oracle`, F2 `unspecified-high`, F3 `unspecified-high`, F4 `deep`

---

## TODOs

- [x] 1. Create cmake.toml with all build semantics

  **What to do**:
  - Create `cmake.toml` at project root
  - Translate ALL semantics from existing `CMakeLists.txt` into cmkr's TOML format
  - Use cmkr's `[conditions]` for platform-specific logic (MSVC vs non-MSVC, Windows vs non-Windows)
  - Use `[options]` for OPENVB_TRACING
  - Use `[variables]` for output directories and CMAKE_TRY_COMPILE_TARGET_TYPE
  - Use `cmake-after` for the compound generator expression `$<$<OR:$<CONFIG:Debug>,$<BOOL:${OPENVB_TRACING}>>:TRACING TRACE_TERMINAL TRACE_FILE>` — cmkr cannot express this natively
  - Inline per-target settings (cmkr has no `foreach` — repeat for each target)
  - Include `cmkr-include = "cmkr.cmake"` in `[cmake]` section
  - Include `condition.msvc = "MSVC"` in `[conditions]` section
  - Set `WIN32_EXECUTABLE = true` for F3 target
  - Set `DEBUG_POSTFIX = "-dbg"` for F3 target
  - Keep `CMAKE_RUNTIME_OUTPUT_DIRECTORY`, `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`, `CMAKE_LIBRARY_OUTPUT_DIRECTORY` as `[variables]`
  - Set `sources = ["src/main.cpp", "src/F3.cpp"]` for F3, `["openje/src/JE.cpp"]` for openje, `["tracing/src/tracing.cpp"]` for tracing
  - Set include directories for all three targets: src/, openje/src/, tracing/src/
  - Set link libraries: F3 links openje + tracing + system libs (conditional on Windows)
  - Set compile definitions conditional on Debug config OR OPENVB_TRACING option
  - Keep mingw32 toolchain file UNCHANGED — it's passed via CLI, not part of cmake.toml

  **Must NOT do**:
  - Do NOT add vcpkg dependencies
  - Do NOT add new targets beyond the existing 3
  - Do NOT change build behavior — this is a 1:1 translation
  - Do NOT refactor or "improve" the build logic
  - Do NOT remove the tracing option or simplify the conditional logic

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Single file creation with clear specification; no architectural decisions needed
  - **Skills**: [`implementation-alignment`]
    - `implementation-alignment`: Ensures changes are minimal and aligned with existing conventions
  - **Skills Evaluated but Omitted**:
    - `engine-game-boundary`: Not relevant — this is build system work, not game/engine boundary
    - `function-reconstruction`: Not relevant — not reconstructing game functions

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 1 (sequential — other tasks depend on this)
  - **Blocks**: Tasks 2, 3, 4, 5, 6
  - **Blocked By**: None

  **References** (CRITICAL — Be Exhaustive):

  **Pattern References** (existing code to follow):
  - `CMakeLists.txt:1-79` — The ENTIRE existing build definition. Every line must be accounted for in cmake.toml.
  - `CMakeLists.txt:9` — `CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY` → `[variables]` section
  - `CMakeLists.txt:5-7` — Output directory variables → `[variables]` section
  - `CMakeLists.txt:11` — `OPENVB_TRACING` option → `[options]` section
  - `CMakeLists.txt:13-15` — openje static library target definition
  - `CMakeLists.txt:17-19` — tracing static library target definition
  - `CMakeLists.txt:21-24` — F3 executable target definition (note: WIN32 flag)
  - `CMakeLists.txt:26-29` — F3 target properties (OUTPUT_NAME, DEBUG_POSTFIX)
  - `CMakeLists.txt:31-35` — Tracing-specific output name overrides → `cmake-after` (conditional on OPENVB_TRACING)
  - `CMakeLists.txt:37-43` — Include directories applied to all 3 targets
  - `CMakeLists.txt:45-60` — MSVC vs non-MSVC compile options → `[conditions]` with condition.msvc
  - `CMakeLists.txt:62-66` — Compile definitions with compound generator expression → `cmake-after` (cmkr can't express `$<$<OR:...>>`)
  - `CMakeLists.txt:68-79` — Platform-conditional link libraries (Windows vs non-Windows)
  - `cmake/toolchains/mingw32.cmake:1-6` — Cross-compilation toolchain (NOT modified, just referenced for understanding)

  **External References** (libraries and frameworks):
  - cmkr docs: `https://cmkr.build` — cmake.toml reference and examples
  - cmkr GitHub: `https://github.com/build-cpp/cmkr` — README, test examples, and issues
  - Key cmkr features: `[cmake]`, `[project]`, `[conditions]`, `[options]`, `[variables]`, target types, `cmake-before`/`cmake-after`, `condition.` prefix

  **WHY Each Reference Matters**:
  - Every line of CMakeLists.txt must have a corresponding entry in cmake.toml — missing semantics = broken build
  - The compound generator expression on line 64 is the trickiest part — it must go in `cmake-after` because cmkr conditions can't express `$<$<OR:...>>`
  - The WIN32 flag on line 21 maps to `WIN32_EXECUTABLE = true` in cmkr target properties
  - The foreach loops (lines 37, 38, 46, 50, 58, 62) must be inlined per-target since cmkr has no loop construct

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY):**

  ```
  Scenario: cmake.toml schema validity
    Tool: Bash
    Preconditions: cmake.toml created at project root
    Steps:
      1. Run `python3 -c "import tomllib; tomllib.load(open('cmake.toml', 'rb'))"` to validate TOML syntax
      2. Verify file contains sections: [cmake], [project], [conditions], [options], [variables], [target.openje], [target.tracing], [target.F3]
      3. Verify [cmake] has version = "3.24" and cmkr-include = "cmkr.cmake"
      4. Verify [project] has name = "openvb" and languages = ["CXX"]
      5. Verify [target.F3] has win32 = true (or WIN32_EXECUTABLE = true depending on cmkr schema version)
      6. Verify [options] has OPENVB_TRACING with default = false
    Expected Result: TOML parses without error, all required sections present with correct values
    Failure Indicators: TOML parse error, missing sections, wrong values
    Evidence: .sisyphus/evidence/task-1-toml-valid.txt

  Scenario: cmake.toml captures all CMakeLists.txt semantics
    Tool: Bash
    Preconditions: cmake.toml created
    Steps:
      1. Grep original CMakeLists.txt for all target definitions, compile options, definitions, include dirs, link libs
      2. Grep cmake.toml for corresponding entries
      3. Verify: output directories (build/, obj/) are in [variables]
      4. Verify: CMAKE_TRY_COMPILE_TARGET_TYPE is in [variables]
      5. Verify: MSVC compile options (/EHsc, /Zi, /Od, /GS, /O2) are present (likely in cmake-after or condition.msvc)
      6. Verify: -std=c++98 is present for non-MSVC
      7. Verify: Debug postfix "-dbg" on F3
      8. Verify: Conditional output name overrides for tracing builds are in cmake-after
    Expected Result: Every semantic from CMakeLists.txt has a corresponding entry in cmake.toml
    Failure Indicators: Missing target, missing compile option, missing definition, missing link library
    Evidence: .sisyphus/evidence/task-1-semantics-check.txt
  ```

  **Commit**: YES (groups with Tasks 2-3)
  - Message: `build: migrate from CMakeLists.txt to cmkr (cmake.toml)`
  - Files: cmake.toml
  - Pre-commit: TOML validation check

- [x] 2. Add cmkr.cmake bootstrap script

  **What to do**:
  - Download the official `cmkr.cmake` bootstrap script from the cmkr repository (https://github.com/build-cpp/cmkr/blob/main/cmkr.cmake)
  - Place it at project root as `cmkr.cmake`
  - Verify it matches the upstream version exactly (don't modify it)
  - Ensure the `[cmake]` section in cmake.toml references it via `cmkr-include = "cmkr.cmake"`

  **Must NOT do**:
  - Do NOT modify the cmkr.cmake script — use it as-is from upstream
  - Do NOT write a custom bootstrap script

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Single file download/creation, well-defined source
  - **Skills**: []
    - No specialized skills needed — just downloading and placing a file
  - **Skills Evaluated but Omitted**:
    - `implementation-alignment`: Not reconstructing anything, just placing a file

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 1 for cmake.toml structure)
  - **Parallel Group**: Wave 1 (sequential after Task 1)
  - **Blocks**: Tasks 3, 4, 5, 6
  - **Blocked By**: Task 1

  **References** (CRITICAL — Be Exhaustive):

  **Pattern References**:
  - `cmake.toml` (Task 1 output) — Must contain `cmkr-include = "cmkr.cmake"` in `[cmake]` section

  **External References**:
  - `https://github.com/build-cpp/cmkr/blob/main/cmkr.cmake` — Official bootstrap script to download
  - `https://cmkr.build` — Documentation on how cmkr.cmake works (auto-downloads cmkr binary, bootstraps build)

  **WHY Each Reference Matters**:
  - cmkr.cmake must match upstream exactly for reliable bootstrapping
  - The cmkr-include path in cmake.toml must match the file location

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY):**

  ```
  Scenario: cmkr.cmake bootstrap works
    Tool: Bash
    Preconditions: cmake.toml and cmkr.cmake at project root
    Steps:
      1. Run `cmake -B build-cmkr-test` to trigger bootstrapping
      2. Verify cmkr downloads and executes without error
      3. Verify CMakeLists.txt is generated at project root
      4. Verify command exits with code 0
    Expected Result: cmake -B completes successfully, CMakeLists.txt generated
    Failure Indicators: CMake error about missing cmkr, download failure, generation failure
    Evidence: .sisyphus/evidence/task-2-bootstrap.txt

  Scenario: cmkr.cmake matches upstream
    Tool: Bash
    Preconditions: cmkr.cmake downloaded
    Steps:
      1. Download latest cmkr.cmake from https://github.com/build-cpp/cmkr/raw/main/cmkr.cmake
      2. Diff local file against downloaded version: `diff cmkr.cmake <(curl -sL https://github.com/build-cpp/cmkr/raw/main/cmkr.cmake)`
      3. Verify no differences (or only version hash difference which is expected)
    Expected Result: Files match or differ only in expected ways
    Failure Indicators: Significant structural differences, missing content
    Evidence: .sisyphus/evidence/task-2-upstream-diff.txt
  ```

  **Commit**: YES (groups with Tasks 1, 3)
  - Message: `build: migrate from CMakeLists.txt to cmkr (cmake.toml)`
  - Files: cmkr.cmake

- [x] 3. Generate CMakeLists.txt and verify build equivalence

  **What to do**:
  - Run `cmake -B build-cmkr` to let cmkr bootstrap and generate CMakeLists.txt
  - Review the generated CMakeLists.txt to verify it captures all semantics
  - Compare the generated CMakeLists.txt against the original — all targets, compile options, definitions, include dirs, link libs must be present
  - Build the project: `cmake --build build-cmkr`
  - Verify the F3 binary is produced
  - Run `cmake --build build-cmkr --clean-first` to ensure clean build works
  - Check that the generated CMakeLists.txt has the `# Auto-generated by cmkr` header comment
  - Ensure cmake.toml changes trigger regeneration (modify a comment in cmake.toml, reconfigure, verify CMakeLists.txt is updated)

  **Must NOT do**:
  - Do NOT manually edit the generated CMakeLists.txt — it's auto-generated
  - Do NOT change build output behavior
  - Do NOT remove the original CMakeLists.txt semantics

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Standard build verification, well-defined steps
  - **Skills**: [`implementation-alignment`]
    - `implementation-alignment`: Ensure the generated output matches the original semantics exactly
  - **Skills Evaluated but Omitted**:
    - `function-reconstruction`: Not relevant — this is build system work

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 1 (sequential after Tasks 1, 2)
  - **Blocks**: Tasks 4, 5, 6
  - **Blocked By**: Tasks 1, 2

  **References** (CRITICAL — Be Exhaustive):

  **Pattern References**:
  - `CMakeLists.txt` (original, lines 1-79) — The gold standard to compare generated output against
  - `cmake.toml` (Task 1 output) — The source definition
  - `cmkr.cmake` (Task 2 output) — The bootstrap script

  **External References**:
  - `https://cmkr.build` — Documentation on auto-regeneration behavior

  **WHY Each Reference Matters**:
  - The generated CMakeLists.txt must produce identical build output to the original
  - Auto-regeneration on cmake.toml changes is a key cmkr feature — must be verified

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY):**

  ```
  Scenario: Build succeeds with generated CMakeLists.txt
    Tool: Bash
    Preconditions: cmake.toml and cmkr.cmake in place
    Steps:
      1. Run `cmake -B build-equivalence` to generate CMakeLists.txt and configure
      2. Run `cmake --build build-equivalence`
      3. Verify F3 binary exists in build-equivalence/ directory
      4. Verify no build errors or warnings
    Expected Result: Clean build produces F3 binary
    Failure Indicators: CMake configuration error, build failure, missing binary
    Evidence: .sisyphus/evidence/task-3-build-equiv.txt

  Scenario: Auto-regeneration triggers on cmake.toml changes
    Tool: Bash
    Preconditions: CMakeLists.txt generated, build succeeded
    Steps:
      1. Record timestamp of CMakeLists.txt: `stat -c %Y CMakeLists.txt`
      2. Add a comment to cmake.toml (e.g., modify a field slightly or add whitespace)
      3. Run `cmake -B build-equivalence` again
      4. Record new timestamp: `stat -c %Y CMakeLists.txt`
      5. Verify timestamps differ (file was regenerated)
    Expected Result: CMakeLists.txt timestamp changes after cmake.toml modification
    Failure Indicators: Timestamp unchanged — regen not working
    Evidence: .sisyphus/evidence/task-3-regen-check.txt
  ```

  **Commit**: YES (groups with Tasks 1, 2)
  - Message: `build: migrate from CMakeLists.txt to cmkr (cmake.toml)`
  - Files: CMakeLists.txt (generated), cmake.toml, cmkr.cmake

- [x] 4. Verify equivalent build output (Linux/GCC)
- [x] 5. Verify OPENVB_TRACING option
- [x] 6. Verify mingw32 cross-compilation

  **What to do**:
  - Clean build: `rm -rf build-mingw-test`
  - Configure with mingw32 toolchain: `cmake -B build-mingw-test -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw32.cmake`
  - Build: `cmake --build build-mingw-test`
  - Verify F3.exe is produced (Windows binary)
  - Verify Windows link libraries (user32, gdi32, winmm, shell32) are in link flags
  - Verify the toolchain file is NOT modified and still works as before
  - Note: This test only runs if i686-w64-mingw32-g++ is available. If not, verify cmake.toml has the correct conditional link libraries and skip actual build.

  **Must NOT do**:
  - Do NOT modify the mingw32 toolchain file
  - Do NOT add Windows-specific code

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Standard cross-compilation verification
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 4, 5)
  - **Blocks**: None
  - **Blocked By**: Task 3

  **References**:

  **Pattern References**:
  - `cmake/toolchains/mingw32.cmake` — Existing toolchain file (must NOT be modified)
  - `CMakeLists.txt:68-79` — Windows-conditional link libraries
  - `cmake.toml` (Task 1) — Must have condition.windows for link libraries

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY):**

  ```
  Scenario: mingw32 cross-compilation produces Windows binary
    Tool: Bash
    Preconditions: Task 3 completed, i686-w64-mingw32-g++ available OR skip build and verify config only
    Steps:
      1. `rm -rf build-mingw-test`
      2. `cmake -B build-mingw-test -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw32.cmake`
      3. If mingw available: `cmake --build build-mingw-test`
      4. If mingw available: `file build-mingw-test/F3.exe` — should show PE32 Windows binary
      5. If mingw NOT available: verify CMakeCache.txt shows MINGW=1, check link flags contain user32/gdi32/winmm/shell32
    Expected Result: Windows binary produced (or config verified for cross-compilation)
    Failure Indicators: Configuration fails, wrong binary type, missing Windows libs
    Evidence: .sisyphus/evidence/task-6-cross-compile.txt
  ```

  **Commit**: NO (verification only)

---

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

> 4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.

- [x] F1. **Plan Compliance Audit** — `oracle`
- [x] F2. **Code Quality Review** — `unspecified-high`
- [x] F3. **Real Manual QA** — `unspecified-high`
- [x] F4. **Scope Fidelity Check** — `deep`
  For each task: read "What to do", read actual diff. Verify 1:1 — everything in spec was built (no missing), nothing beyond spec was built (no creep). Check "Must NOT do" compliance. Flag any vcpkg additions, new targets, or gratuitous refactoring.
  Output: `Tasks [N/N compliant] | Contamination [CLEAN/N issues] | Unaccounted [CLEAN/N files] | VERDICT`

---

## Commit Strategy

- **Task 1-3**: Single commit — `build: migrate from CMakeLists.txt to cmkr (cmake.toml)`
  - Files: cmake.toml, cmkr.cmake, CMakeLists.txt (generated)
  - Pre-commit: `cmake -B build-test && cmake --build build-test`

---

## Success Criteria

### Verification Commands
```bash
# Full build works
cmake -B build && cmake --build build
# Expected: F3 binary in build/ directory

# Tracing option works
cmake -B build-trace -DOPENVB_TRACING=ON && cmake --build build-trace
# Expected: F3-trace binary, TRACING/TRACE_TERMINAL/TRACE_FILE defined

# Cross-compilation works
cmake -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw32.cmake && cmake --build build-mingw
# Expected: F3.exe Windows binary

# CMakeLists.txt regenerates on cmake.toml changes
# (touch cmake.toml, reconfigure — should trigger regeneration)
```

### Final Checklist
- [ ] All "Must Have" present
- [ ] All "Must NOT Have" absent
- [ ] Build succeeds on all configurations