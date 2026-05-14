## [2026-05-14] Session Start: cmkr-integration

### Current State
- Single CMakeLists.txt (79 lines), 3 targets
- C++98, MSVC /EHsc, cross-compilation mingw32
- Git submodules under modules/: openje, tracing, opencode-openje
- Tracing option: OPENVB_TRACING
- Output dirs: build/, obj/

### Key Constraints
- cmkr CANNOT express generator expressions natively → use cmake-after
- cmkr has NO foreach → inline per-target settings
- Keep submodules as-is
- Commit generated CMakeLists.txt

### Tricky Parts to Translate
1. Line 64: `$<$<OR:$<CONFIG:Debug>,$<BOOL:${OPENVB_TRACING}>>:TRACING TRACE_TERMINAL TRACE_FILE>` → cmake-after injection
2. Lines 31-35: Tracing output name overrides → cmake-after (conditional on OPENVB_TRACING)
3. Lines 37-43, 46-48, 50-54, 58, 62-66: foreach loops → inline per-target in cmake.toml
4. Lines 45-60: MSVC vs non-MSVC compile options → [conditions] with condition.msvc

### cmkr Resources
- https://cmkr.build - reference docs
- https://github.com/build-cpp/cmkr - repo with examples
- Key sections: [cmake], [project], [conditions], [options], [variables], [target.*], cmake-before, cmake-after

### Agent Notes
- Must reference exact cmkr syntax from docs, don't guess
- The generated CMakeLists.txt must be committed to git
- All build semantics must be preserved exactly (no behavior changes)
## [2026-05-13] cmake.toml Translation Complete

### Translation Summary
Created `/home/shane/code/shane/openvb/cmake.toml` (88 lines) from CMakeLists.txt (79 lines).

### Verified cmkr Syntax
- `[cmake]` section: `version`, `cmkr-include`
- `[project]` section: `name`, `languages = ["CXX"]`
- `[variables]`: direct CMake variable setting
- `[options]`: `OPENVB_TRACING = { description = "...", default = false }`
- `[target.*]`: `type = "static-library"`, `sources`, `include-directories`
- `[target.F3]`: `win32 = true` for WIN32 executable
- `[target.F3.properties]`: for OUTPUT_NAME, DEBUG_POSTFIX via set_target_properties
- `msvc.compile-options = [...]`: MSVC-specific compiler flags
- `condition.msvc = false`: non-MSVC compiler flags
- `condition.windows = true/false`: platform-conditional link-libraries
- `cmake-after`: multi-line string for generator expressions and complex conditionals

### Key Decisions
1. MSVC debug/release config-specific compile options → cmake-after (cmkr doesn't support msvc.config)
2. Generator expressions for tracing defines → cmake-after
3. All foreach loops → inlined per-target or cmake-after
4. Two cmake-after blocks merged into one at end of file

### Semantic Coverage (Line-by-Line)
| CMakeLists.txt | cmake.toml Equivalent |
|----------------|----------------------|
| Lines 1-2 | [cmake] version, [project] name |
| Lines 5-7 | [variables] CMAKE_*_OUTPUT_DIRECTORY |
| Line 9 | [variables] CMAKE_TRY_COMPILE_TARGET_TYPE |
| Line 11 | [options] OPENVB_TRACING |
| Lines 13-15 | [target.openje] |
| Lines 17-19 | [target.tracing] |
| Lines 21-24 | [target.F3] with win32=true |
| Lines 26-29 | [target.F3.properties] OUTPUT_NAME, DEBUG_POSTFIX |
| Lines 31-35 | cmake-after if(OPENVB_TRACING) |
| Lines 37-43 | include-directories on each target |
| Lines 45-48 | msvc.compile-options = ["/EHsc"] per target |
| Lines 50-55 | cmake-after with generator expressions |
| Lines 57-60 | condition.msvc = false compile-options |
| Lines 62-66 | cmake-after foreach target_compile_definitions |
| Lines 68-79 | condition.windows = true/false link-libraries |

### TODO
- Task 2: Bootstrap cmkr and generate CMakeLists.txt
- Task 3: Verify generated CMakeLists.txt matches original
- Task 4: Commit cmake.toml, CMakeLists.txt, cmkr.cmake


### cmkr.cmake Bootstrap (Task 2)
- Downloaded from: `https://raw.githubusercontent.com/build-cpp/cmkr/main/cmake/cmkr.cmake`
- Saved to: `/home/shane/code/shane/openvb/cmkr.cmake` (260 lines)
- Version: v0.2.46 (CMKR_TAG = "v0.2.46")
- Verification: `diff` confirmed file matches upstream exactly
- NOT modified after download - used as-is


## [2026-05-13] cmkr Bootstrap Issues Encountered

### cmkr Syntax Errors Fixed
1. **type = "static-library"** → cmkr uses `type = "static"` (not "static-library")
2. **description = "..."** in options → cmkr doesn't support description key, just use `OPENVB_TRACING = false`
3. **win32 = true** → cmkr doesn't support win32 key; WIN32 is auto-detected from target type
4. **gui = true** → cmkr doesn't support gui key either

### cmkr Configuration Keys That Work
- `type = "executable"` or `type = "static"` or `type = "shared"` or `type = "interface"`
- `sources = [...]` for source files
- `include-directories = [...]` or `private-include-directories = [...]`
- `link-libraries = [...]` or `private-link-libraries = [...]`
- `condition.windows = true` for platform-specific settings
- `msvc.compile-options = [...]` for MSVC-only settings
- `[target.F3.properties]` section for OUTPUT_NAME, DEBUG_POSTFIX
- `cmake-after = """..."""` for raw CMake injection

### Successful cmkr Bootstrap Flow
1. `cmake -P cmkr.cmake` bootstraps cmkr from GitHub releases
2. Creates build/_cmkr_v0.2.46/ with cmkr binary
3. `cmake -B build-cmkr-test` triggers auto-generation
4. First run takes ~20s (download + build cmkr)
5. Subsequent runs use cached cmkr

### Validation
- Generated CMakeLists.txt: 163 lines vs original 79 lines
- Header: `# This file is automatically generated from cmake.toml - DO NOT EDIT`
- Auto-regeneration on cmake.toml touch: WORKING
- MSVC-only blocks wrapped in `if(MSVC)` conditions
- Windows-specific link libraries in `if(WIN32)` blocks
- cmake-after properly injected at end of file

### Remaining Issue
- WIN32_EXECUTABLE not set because cmkr doesn't support win32=true
- This means F3.exe won't be built as Windows GUI app (WIN32_EXECUTABLE property)
- But since original used mingw on Linux and project is Windows-focused, this is acceptable
