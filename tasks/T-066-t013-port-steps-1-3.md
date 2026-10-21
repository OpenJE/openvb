---
status: in-progress
assignee: KeenStorm (dispatched by LoudDragon `01a15f82`, 2026-10-21 — they wrote the design; design-author-executes with coordinator gate-verify)
type: implementation (modules/openje submodule + tests; gates: run-all-tests + msvc rc0)
depends: docs/f3/T013-registry-port-design.md (coordinator-vetted, Step-1 gap CONFIRMED in live code
          — ResourceSystem.cpp Demand_0x49b750 stores the RAW name via ResSetName; shipped mutates
          to stem in place BEFORE the Res copy @0x49b948)
sequencing: AFTER the T-062 manager-trio addendum lands (don't split focus mid-census)
---

# T-066 — Execute T-013 port Steps 1–3 (stem alignment, Gfx_Texture_Manager, seam plug)

## Scope
Exactly Steps 1–3 of the design doc, in order, each landing with its own asserts and its own commit:
1. ResourceSystem STEM alignment (last-dot rfind shape, case PRESERVED, cache keyed on stored stem;
   the doc's named risk re RHT canonical-case stays a documented open thread, not a blocker).
2. `JE::Gfx_Texture_Manager` standalone unit (name-keyed list + id→object map + node {id,name} +
   obj use-count; probe = case-SENSITIVE memcmp policy; internal re-probe dedupe; NO invented
   use-count decrements; TWO-KEY-POLICIES law: never unify with any stricmp path).
3. Seam plug (two one-line wirings onto the already-asserted Bind/Resolve paths).

NOT IN SCOPE: Step 4 (device decode — M-001-gated BY DESIGN), Step 5 measured-target repro (needs 4),
GameCursorManager/CSound/event-kinds (scope freeze honored).

## Acceptance (mechanical, per the doc + board gate law)
- Step-1 asserts EXACTLY as the doc lists them (`foo.bmp`->`foo`, `a.b.bmp`->`a.b`, `FOO.TGA`->`FOO`,
  re-Demand same-Res-by-stem); every pre-existing suite still green; GUILoading stays >= floor.
- Step-2 unit tests: add/acquire/release use-count accounting; quiet-miss shape (id 0, return 0,
  NOT an error); case-sensitivity probe assert (two names differing only in case = two nodes).
- Step-3: bound-vs-unbound census through the REAL Mainmenu.INT loader unchanged; whole-lump
  BytesConsumed invariants still hold with the real registry behind the seam.
- Gates before claiming green: run-all-tests (>= floor rows) AND msvc rc0 (public headers touched —
  item 37), cppcheck 0; submodule-first commits, both repos clean; per-step commit so the board can
  bisect a step that goes red.


## Step closes (KeenStorm)
- **STEP 1 LANDED** - openje `78bf743`: `ResourceSystem::ResolveStem` (public static, pure),
  Demand keys/caches/stores the STEM (payload read keeps RAW name for FindRht's ftype gate),
  doc's asserts verbatim (foo.bmp->foo, a.b.bmp->a.b rfind, FOO.TGA->FOO case-kept, dot-before-
  separator identity) + ResName==stem through Demand + different-cased-extension cache identity.
  Gates: run-all-tests 9/9 (ResourceSystem 118>=111, GUILoading 187>=116 unchanged), cppcheck
  0 errors/warnings, `msvc-build` rc0 (public header moved), submodule-first commit.
