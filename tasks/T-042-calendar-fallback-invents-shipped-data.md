---
status: done
assignee: NiceHawk 01a15bd1 (edits half-landed, session died mid-gate) — taken over and gated/committed by UltraMoon 01a15cb7 (01a15cb7-c19d-74ae-9744-8221b99fffb5).
# Small code row, fits alongside T-007; no IDB, no debugger. Order agreed: T-007 first, then this.
depends: T-038 (which found it)
---

# T-042 — Our port hardcodes the shipped data's *contents* as its defaults (`GameWorld.cpp:90-92`)

## The defect
```
src/F3/GameWorld.cpp:90   m_clock = new JE::Gregorian( m_calendarYear  ? m_calendarYear  : 2253,
src/F3/GameWorld.cpp:91                            m_calendarMonth ? m_calendarMonth : 10,
src/F3/GameWorld.cpp:92                            m_calendarDay   ? m_calendarDay   : 18, ...)
```
with the comment above it asserting *"defaults are the sys.ini 2253-10-18 13:13:13"*. **That is the shipped file's
contents wearing the clothes of a default.** Shipped's actual missing-key fallback is the **ctor's**
`0,1,1,0,0,0` (`0x548af6..0x548b08`); `2253` only ever arrives *from* `sys.ini`. Related sibling: `JE::World.hpp:13`
defaults the trio to `0,0,0` where shipped uses `0,1,1`.

## Why it matters even though the key ships
Both sides agree while `Year/Month/Day` are present, so no current test can see this. It diverges the moment the key is
absent — a stripped INI, a compressed-region read that fails, or a save from a build without the block — and then our
port reports a **plausible** date where the original reports a **structurally invalid but defined** one. Plausible-wrong
is worse than obviously-wrong: it will not look like a bug in a digest.

## Execution order is load-bearing (found by WildYak correcting their own doc, `489b240`)

**Two halves, and the wrong order produces a different bug.** My row originally read as "pass `m_calendar*` through
unchanged and delete the literals". WildYak checked their own summary against this row and found their "one-line fix"
asserted that `JE::World.hpp:13` was *already* initialised to `0/1/1` — **it is not: `World()` zeroes year, month and
day.** Therefore:

1. **First** align the ctor to shipped's `0,1,1` (`0x548af6` / `0x548af9` / `0x548afc`).
2. **Then** delete the `?:` literals in `GameWorld.cpp:90-92`.

Delete-only leaves `0/0/0` on an absent key — wrong in a *different* direction, and invisible on the shipping payload,
which is exactly the class of defect this row exists to remove. The absent-key test in Acceptance is what makes the
ordering visible rather than a matter of luck.

## Fix
Mirror, don't improve: fall back to the shipped ctor values (`0,1,1,0,0,0`), citing `0x548af6..0x548b08`, and align
`JE::World.hpp:13` to the same. Remove the comment that states sys.ini contents as if they were defaults.

## Acceptance
- A test with the calendar keys **absent** asserts the shipped-shaped zero/one defaults, and a test with them **present**
  asserts pass-through — so the two paths are separately pinned rather than the shipping case alone.
- `run-all-tests` green with the new assertions visible above their floors, **`msvc-build` exit 0** (header touch),
  quoted with its `GATE STAMP` after `tools/gate-integrity.sh` returns 0.
- Register in `docs/DEFERRED-NOTES.md` as **ours-to-fix**, distinct from the shipped-defect ledger, and say explicitly
  that removing the invented default *changes* behaviour on absent keys — which is the point, not a regression.

## Boundary
This is a default-selection bug in **our** code, not shipped behaviour we must mirror, and it is not a parity risk on
the shipping payload. It becomes one only on data we do not currently ship.

## Acceptance evidence (2026-10-21T04:43:41Z gate, UltraMoon 01a15cb7)

- **Order honored**: ctor aligned to shipped `0,1,1` first (openje `77c18d8`: `JE/World.hpp` + the four absent-key/mixed/authored-zero
  checks in `tests/je/StringSystem/StringSystem.cpp`), then the `?: 2253/10/18` ternaries deleted in
  `src/F3/GameWorld.cpp` (this parent commit). The comment asserting sys.ini contents as defaults is gone; both sides
  now cite `0x548af6..0x548b08`, `0x548abb`, `0x546B50`, and the `rep movsl` seed copy `@0x54497d`.
- **Absent-key path pinned separately from pass-through**: ctor-only / stripped section / mixed per-key / authored-zero,
  7 new asserts — StringSystem `[27 asserts >= 20]`, observed above floor exactly by the +7.
- **Gate, this run**: `tools/gate-integrity.sh` rc **0** — payload `/nix/store/x95aqfk70mrsqicri2x6spbf38cl63ha-run-all-tests-script`,
  9 rows identical to tree, hash **2945679485**. `run-all-tests` rc **0**: 9 suites + app-boot **10 pass, 0 fail**
  (cls_0x4d8d70 11≥11, PathManager 103≥74, ResourceSystem 111≥111, StringSystem 27≥20, ActionMoveTo 13≥13,
  Scripting 6≥6, GUILoading 150≥116, SpawnMove 88≥80, CombatMath 43≥43, app exit 124 = alive past 6s).
  **`msvc-build` exit 0** (header touch is the real gate): World.cpp/F3.cpp/GameWorld.cpp rebuilt, F3.exe linked
  2026-10-21 00:43:33 -0400; only pre-existing C4700 warnings in f3.cpp(415/419).
- **Registered** in `docs/DEFERRED-NOTES.md` under the new “Ours-to-fix defects corrected” section, explicitly stating
  the absent-key behaviour change is the point, not a regression.
- Env note for the ledger: the MSVC leg was initially blocked by a wedged rootless dockerd sandbox DNS (`[::1]:53` refused)
  plus the dead session's 7h-hung `magical_euler` container holding `build/`; container removed, daemon restored via
  `devenv processes restart docker-rootless` (ZenNova).
