# AGENTS.md – OpenVB / OpenJE

This file is for agentic coding tools (like AI code assistants) working in this repository. Follow these rules when making changes.

---

## 1. Project Overview

- This repo contains the reverse‑engineered Fallout: Van Buren game (`openvb`) and the Jefferson engine (`openje`) plus a small `tracing` helper.
- The target environment is **Windows + MSVC (Visual C++ .NET 2003 era)** with C++98‑style code.
- CI builds use **nmake + MSVC** via GitHub Actions; see `.github/workflows/*.yml` for details.
- Code style is defined primarily by `STYLE.md`; agents must treat that file as authoritative.
- There are currently **no Cursor or Copilot rule files** in this repo.

---

## 2. Build Commands

From the repo root (`openvb`):

- Default build (Release‑like):
  - `nmake`
- Explicitly using the root `Makefile`:
  - `nmake /f Makefile`
- Debug build (matches CI nightly job):
  - `CFG=Debug nmake /f Makefile`
- Clean all objects and executables:
  - `nmake clean`

CI also demonstrates Linux + Docker usage (MSVC 2002 container):

- Prepare directories (mirrors `release.yml` / `nightly.yml`):
  - `mkdir -p obj openje/obj tracing/obj build`
- Build inside MSVC container (conceptual sequence):
  - `docker run ... ghcr.io/openje/msvc2002:latest -lc 'nmake /f Makefile'`

Agents should not change build scripting (Makefile or workflows) without an explicit user request.

---

## 3. Tests and Single‑Test Workflow

- There is **no unified automated test harness** yet (no gtest/ctest/etc.).
- The `openje/tests/je/*` directory currently holds **tiny sanity‑check programs**, each with its own `main` function, e.g.:
  - `openje/tests/je/Action/Action.cpp`
  - `openje/tests/je/ActionAnim/Action.cpp`
- These programs are **not wired into the root Makefile**.

To run a "single test" today, treat a test file as a small console program:

1. Open a **Developer Command Prompt for VS** (or MSVC 2003 equivalent).
2. From repo root, compile the test against `openje` includes and objects.
   - Example pattern (adjust include/lib paths to your local setup):
     - `cl /EHsc /Iopenje/src /Iopenje/tests /FeActionTest.exe openje/tests/je/Action/Action.cpp openje/src/JE.cpp`
3. Run the resulting `.exe` directly.

Agents **must not invent a test runner name** (like `ctest`, `pytest`, etc.)—if you need better automation, propose it to the user rather than assuming it exists.

---

## 4. Linting and Static Analysis

- There is **no dedicated linter configuration** (no clang‑tidy, no CPPCHECK config, etc.).
- Linting is effectively done via **MSVC warnings**.
- When adding or changing build flags, prefer higher warning levels (e.g. `/W4`) but **do not change the existing flags in `Makefile` without explicit user approval**.
- You may locally run extra analysis (e.g., clang‑tidy) for your own validation, but do **not** add new config files or enforce new lint rules unless the user asks.

---

## 5. File and Include Conventions

Follow `STYLE.md` and mimic existing code:

- Use **paired `.h`/`.cpp` files** for major classes or subsystems.
- Headers should start with `#pragma once`.
- Every `.cpp` must include the precompiled header first:
  - `#include "stdafx.h"`
- After `stdafx.h`, include the matching header, then other local headers, then standard library headers.
- Use `"..."` for project and engine headers; use `<...>` for system / STL headers.
- Do **not** introduce CMake or other build systems without explicit instruction; the current MSVC + nmake layout is intentional.

---

## 6. Formatting

- Indentation: **tabs**, not spaces.
- Brace style: **Allman** (opening brace on a new line).
- Aim for **≤ 100 characters** per line.
- Spacing:
  - Space after keywords: `if (`, `while (`, `for (`.
  - Space around binary operators: `a + b`, `x == y`.
  - No extra spaces just inside parentheses: `Func(a, b)` not `Func( a, b )`.
- One major class per header; one main namespace per file when practical.
- Do not add or run auto-formatters that change indentation style or brace placement.
- Line endings: save source files with Unix `LF` endings. When working from Linux, normalize any CRLF→LF conversions (e.g., `dos2unix` or your editor's "LF" option) so `^M` characters do not reappear. Windows editors should disable auto-CRLF for this repo or convert to LF before committing.

---


## 7. Naming Conventions

High‑level rules (details in `STYLE.md`):

- **Namespaces / prefixes**
  - `Game::` + `GameSomething` (gameplay: `GameEntity`, `GameWeapon`, ...).
  - `Gfx_` for rendering (`Gfx_Texture`, `Gfx_Engine`, `Gfx_RenderManager`).
  - `G3D_` for 3D scene/mesh.
  - `VFX_` for visual effects.
  - `GUI::` for UI classes, often with `C` prefix (e.g. `CMainmenuInterface`).
  - `SS::` for string system, `Common::` for utilities, `net::` for networking.

- **Types**
  - Classes/structs/enums: **PascalCase**, often with subsystem prefixes.
  - Enums: types like `VFX_EffectType`; values ALL_CAPS with subsystem prefixes.

- **Functions / methods**
  - Use **PascalCase** verbs: `OnCreate`, `OnServerUpdate`, `Queue_Limp`.
  - Keep names close to original binary strings when possible.

- **Members and globals**
  - Members: `m_` + CamelCase (e.g. `m_nCaliber`, `m_bUsesPrimaryAmmo`).
  - Static members: `s_` prefix.
  - Globals / singletons: `g_` prefix.

- **Locals and parameters (Hungarian‑lite)**
  - `p` (pointer), `n` (int), `b` (bool), `f` (float), `dw` (DWORD), `sz` / `psz` (C strings).

- **GUI identifiers**
  - `btn*`, `lbl*`, `pic*`, `win*`, etc., matching existing naming seen in strings.

Agents should **mirror existing patterns** rather than inventing new naming schemes.

---

## 8. Error Handling and Logging

- Avoid exceptions in engine/gameplay hot paths.
- Prefer **return codes** (`bool`, `HRESULT`, etc.) plus **logging**.
- Use assertions for internal invariants, ideally via a project macro (e.g. `ASSERT`).
- Log messages should:
  - Include `ClassName::MethodName()` or a subsystem prefix at the start.
  - End with `\n`.
  - Provide clear context about what failed.
- Example style (from `STYLE.md`):
  - `LogError("GameWeapon::ReloadPrimaryAmmo() - This weapon doesn't even use ammo.\n");`
  - `LogError("Gfx_Texture_Manager::AddTexture - CreateTexture Failed (%x %s)\n", hr, pszName);`
- When calling DirectX / Win32 APIs, always check the `HRESULT` / return value and log failures.

---

## 9. Types and Libraries

- Target dialect is **C++98**; avoid C++11+ features (`auto`, `nullptr`, lambdas, range‑for, `override`, etc.).
- Use Win32 / DirectX types where appropriate: `DWORD`, `WORD`, `BYTE`, `BOOL`, `HRESULT`.
- `NULL` is preferred over `nullptr` for pointers.
- STL containers are allowed and expected:
  - `std::vector`, `std::map`, `std::set`, `std::list`, `std::string`, etc.
- Prefer fully qualified names in headers (`std::vector`), and avoid `using namespace std;` in headers.

---

## 10. Memory Management

- Use manual `new` / `delete`; no modern smart pointers.
- Set pointers to `NULL` after deletion.
- If you introduce custom allocation logic, keep it stylistically consistent with existing systems (e.g. `ResourceSystem`).
- Avoid adding third‑party allocators or intrusive smart pointers unless the user explicitly wants them.

---

## 11. Comments and Documentation

- Prefer `//` single‑line comments over large `/* ... */` blocks.
- Keep comments concise, describing intent rather than restating the code.
- Do not introduce heavy Doxygen‑style markup without user request.
- When editing `STYLE.md`, treat it as a spec document—avoid re‑formatting large sections.

---

## 12. Repository‑Wide Agent Rules

- Scope: This `AGENTS.md` applies to the entire repository.
- Do **not** create additional `AGENTS.md` files in subdirectories unless a human maintainer requests finer‑grained rules.
- Before changing build logic, logging infrastructure, or error‑handling patterns, prefer a minimal patch and keep behavior aligned with the reverse‑engineered binary.
- When in doubt, inspect existing usages in `src/` or `openje/src/` and copy the style exactly.

---

## 13. RE Harness Rules

- Promotion is agent‑driven into real module directories: `src/`, `openje/src/`, `tracing/src/`.
- The promoter may update build scripts automatically but must only add sources, never remove.
- Git automation is allowed only on branch `automated-reversing`: agents may commit; push only when the build succeeded.
- GitHub automation is allowed: create/update issues and update OpenJE Project #1 only; never delete items or auto‑close issues.
