# Cross-TU verification without a link step (`tools/verify-symbol.sh`)

This exists because a missing `d3d8.lib` made "cannot test today" the default answer. A rename that
breaks a caller is a *name-resolution* fact, and name resolution is visible to a compiler without
producing a binary. So the check is: compile a generated probe TU that takes the symbol's address
through its **real header**, then compile every site that references the name.

```sh
tools/verify-symbol.sh --member  F3::CombatStats::dtRowB     # data member, arrays included
tools/verify-symbol.sh --fn      F3::ReactionAnimFor --header src/F3/GameActionAttack.hpp
tools/verify-symbol.sh --type    F3::CombatStats             # requires a COMPLETE type (sizeof)
tools/verify-symbol.sh --absent  dtParts56                   # retired name: prose-only, and labelled
tools/verify-symbol.sh --self-test                           # prove the checker can fail
```

Exit: **0** verified · **1** failed, with the site list · **2** setup error (no usable compiler,
no image, no header). `2` is never a pass and the tool says so.

## Why the probe works without an object or a definition

`template <typename T> static void vs_take(T) {}` called as `vs_take(&F3::CombatStats::dtRowB)`
deduces a pointer-to-member in C++98, so an array data member needs no instance, no definition and
no link. That is what makes this runnable in a tree that cannot produce an exe.

## Compiler choice is a capability test, not a name lookup

Our sources are Win32 — nearly every TU reaches `windows.h` through `JE.hpp`. Candidates are tried
against a real Win32 TU, not `command -v`, because a compiler that cannot resolve platform headers
reports **itself**, not the code. Two consequences worth knowing:

- `${CXX}` on this host is `g++`, which cannot; `i686-w64-mingw32-g++` can. The same trap is noted
  in `devenv/scripts/run-all-tests.nix:129-131`.
- `--compiler X` does **not** fall back when X is broken. A forced compiler is a promise about which
  toolchain produced the verdict; silently substituting another prints a green nobody asked for.

## BUG-007 (struct vs class forward-decl) — answered with evidence

`docs/DEFERRED-NOTES.md:389`: MSVC 2002 decorates `struct` and `class` differently (U/V), so a
mismatched forward declaration is a real ABI bug here. Tested directly:

| Compiler | Verdict |
|---|---|
| `clang++` | **detects it** — `-Wmismatched-tags`: *"valid, but may result in linker errors under the Microsoft C++ ABI"* |
| `g++` / `i686-w64-mingw32-g++` | **blind** — compiles the mismatch silently, exit 0 |

So the tool resolves the site compiler (Win32-capable) and the tag check (diagnostic-capable)
**separately**, and prints which compiler produced each. If neither on a host can see it, the tool
prints `tag-check: BLIND to BUG-007` with the citation — a stated limit, not a silent pass.

## The verdicts and what they mean

| Verdict | Meaning |
|---|---|
| `OK` | resolves / compiles at that site |
| `STALE CALLER` | the retired name still appears in **code** — file and line named |
| `UNEXPLAINED` | a comment mention with no historical wording (`formerly`, `renamed`, `disproved`, `T-0…`) |
| `FAILED` | probe or site genuinely does not compile |
| `SKIP (unverifiable)` | the **host** could not compile that file at all — an unproven case, not a clean one |

## Boundary — the tool prints this itself, do not paraphrase it into a green

It proves **declarations and cross-TU name resolution at compile time**. It does **not** prove
runtime behaviour, that a link would succeed, or that the exe works. Concretely: a green here is
**not** T-012's acceptance, which still needs `msvc-build` + `app-boot` against the real binary.
Absence of code sites is also not verification of a declaration — the probe is what does that, and
the tool says which of the two it just did.

## Provenance: five bugs found in this tool while building it

All were silent, and all are recorded because silent failures in a checker are worse than missing
ones — a self-test that cannot read its own evidence reports PASS exactly as confidently as FAIL:

1. `pick_compiler` ran before `TMP` existed → `set -u` abort (loud, at least).
2. `cp -a $ROOT/src $scratch/src` after `mkdir` nested the tree at `src/src`, so the "rename under
   one header" scenario edited a file that was never there — the falsification was testing nothing.
3. `rc="$(run_check …)"` captured the child's whole **report** as the exit code.
4. The same form ran the helper in a **subshell**, so `LAST_OUT` never reached the caller; three
   scenarios then "passed" on an empty `$RC`, because `[ "" != 0 ]` is true.
5. Comment-historical matching tested the **whole record**, so a file path containing a vocabulary
   token would have blessed every mention in that file.

(3) and (4) are the same trap as judging a pipeline's exit status through the pipe, which this
project has now hit three times in two days. `expect` requires a numeric exit **and** a matched
evidence line for that reason.
