---
status: done
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

## Evidence (implementer-C, 2026-10-20)

`tools/verify-symbol.sh` + `docs/notes/cross-tu-verification.md`.

- **Acceptance case 1**: `--absent dtParts56` -> exit 0; the one surviving mention is the labelled
  historical note at `src/F3/CombatMath.hpp:69`, reported as `HISTORICAL`, code sites `none`.
- **Acceptance case 2**: `--member F3::CombatStats::dtRowB` -> exit 0: auto-detected header
  `src/F3/CombatMath.hpp`, probe compiles from a separate TU, and all three code sites compile
  (`src/F3/CombatMath.cpp`, `CombatMath.hpp`, and implementer-B's suite). No `F3.exe` involved.
- Extra cases that also pass: `--type F3::CombatStats` (complete type, 5 sites) and
  `--fn F3::ReactionAnimFor --header src/F3/GameActionAttack.hpp` (my T-012 port, 3 sites).
- **Falsification (required)**: `--self-test`, 8 scenarios, all green, run against a **scratch copy**
  of `src/` + every module `src`/`tests`. The named case - rename the member in ONE header, leave
  the stale callers - reports exit 1 and lists them:
  `modules/openje/tests/je/CombatMath/CombatMath.cpp:125`, `src/F3/CombatMath.cpp:159`, `:216`.
  Also covered: probe refuses a member the header no longer declares; an invented member is refused
  (so the probe is not vacuous); a plain-prose comment is refused as "historical"; and a broken
  `--compiler` is a setup error rather than a substituted green. Two controls run first, because
  without them every red below could just be the harness being broken.
- **No MSVC required**, per the row. Compiler selection is a capability test: candidates are tried
  against a real Win32 TU, since `${CXX}=g++` here cannot resolve `windows.h` (the same trap
  `run-all-tests.nix:129-131` documents). Selected: `i686-w64-mingw32-g++`.

## BUG-007, asked and answered with evidence rather than hope

Tried as instructed, and **it can be caught - but only by clang.** `clang++ -Wmismatched-tags`
emits exactly the right thing: *"'W' defined as a class here but previously declared as a struct;
this is valid, but may result in linker errors under the Microsoft C++ ABI"*. `g++` and
`i686-w64-mingw32-g++` compile the same mismatch **silently, exit 0**. So the tool resolves the
site compiler (Win32-capable) and the tag compiler (diagnostic-capable) separately and prints both,
and prints `tag-check: BLIND to BUG-007` when a host cannot see it at all. Cited at
`docs/DEFERRED-NOTES.md:389` - the row's original `docs/notes/bugs-found.md` does not exist
(coordinator fixed the ref in `8d80704` before I started).

## Boundary, printed by the tool itself

Declarations + cross-TU name resolution at COMPILE time. Not runtime behaviour, not link success,
not the exe. Explicitly: **a green here is not T-012's acceptance** - that still needs
`msvc-build` + `app-boot`. And "no code sites found" is not a verification of a declaration either;
the tool distinguishes the two instead of letting absence read as proof.

## Five bugs in my own tool, all silent until the self-test forced them open

1. `pick_compiler` ran before `TMP` existed (loud abort, `set -u`).
2. `cp -a $ROOT/src $scratch/src` after `mkdir` nested the tree at `src/src` - so the required
   falsification was editing a file that was never there and **testing nothing**. It still printed
   PASS, because of (3).
3. `rc="$(run_check …)"` captured the child's whole report as the exit code.
4. The same form ran the helper in a **subshell**, so `LAST_OUT` never reached the caller, and
   `[ "" != 0 ]` is true - three failure scenarios "passed" while holding no evidence.
5. The historical-comment test matched the **whole record**, so a file path containing a vocabulary
   token (`.../T-0...`, `...renamed...`) would have blessed every mention in that file.

(3)-(5) are the same family as the pipeline exit-status trap this project has now hit three times in
two days. The fix encoded in the tool: `expect` requires a **numeric** exit code AND a matched
evidence line, so a scenario cannot pass on an empty variable again.

## Refs
- `tasks/T-021-...` (verification without a linker, as a boundary not a green)
- **Reference corrected after C flagged it:** the forward-decl keyword defect is recorded at
  `docs/DEFERRED-NOTES.md:389` ("MSVC 2002 decorates struct vs class (U/V) in names: forward decls MUST
  match keyword"), **not** in `docs/notes/bugs-found.md`, which does not exist — my row pointed at a
  phantom file, the same dangling-reference class as item 15. Test the struct/class mismatch as asked and
  report honestly whether a host compiler can see it at all; if it cannot, that is a stated limit of the
  probe, not a pass.
