---
status: open
assignee: implementer-C (UltraBear, 01a15aff)
depends: nothing
---

# T-023 — Make cross-TU verification a command, not an invention

## Why
While `d3d8.lib` was missing nothing could link an exe, so "cannot test today" was the default answer.
C got a real verdict anyway by compiling a tiny probe that references the renamed member through its
real header, plus grep against callers. That technique worked and was improvised; productise it so the
next blocked-on-link lane does not have to think of it.

## Scope
1. `tools/verify-symbol.sh <symbol>`: greps the symbol across `src` + all submodule `src`/`tests`
   (old name must survive only in explicitly historical comments), then compiles a generated probe TU
   including the owning header and taking the member's address, with the include paths the host test
   rows already use (`tracing/include`, `openje/include`, `openvb/include`).
2. Compiler is whatever is present — **do not require MSVC**; `cc`/`clang` is the point, since the
   check is cross-TU visibility, not codegen parity.
3. Exit 0 = symbol verified; non-zero with the missing site list.

## Acceptance
- Proves `dtParts56` is gone and `dtRowB` resolves through `CombatMath.hpp` today, with no `F3.exe`.
- Deliberately renaming a member in one header and running the probe **fails** and names the stale
   caller — the tool must catch the class of bug LNK2019 would have caught if the linker worked.
- Documented in `docs/notes` as the fallback when a link step is unavailable, with its limit stated:
   it proves declarations and cross-TU resolution, **not** runtime behaviour.

## Refs
- `tasks/T-021-...` (verification without a linker, as a boundary not a green)
- `docs/notes/bugs-found.md` BUG-007 (forward-decl keyword vs definition — the probe should also
   catch struct/class mismatch; try, and say if it cannot)
