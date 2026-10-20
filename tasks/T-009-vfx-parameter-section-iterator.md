---
status: open
assignee: implementer-C (UltraBear)
depends: T-008
unblocks: T-007 (implementer-B's skin registry)
---

# T-009 — `VFX_Parameter`: read-only section/entry iteration (6 accessors, additive only)

## Objective

implementer-B cannot enumerate an ini's sections as skin records without a read-only iterator on
`JE::VFX_Parameter`, which today exposes only `SectionCount()` plus by-name getters. This is the
missing half of T-007's skin table. The storage already exists (`m_sections`,
`IniSection{name, entries, count, capacity}`, `IniEntry{key, value}`), so this is accessors, not
new machinery.

## Steps

1. Reserve `modules/openje/src/JE/VFX_Parameter.hpp` and `.cpp`.
2. Add exactly these four accessors, implemented against the existing flat `IniSection` storage
   (the same backing `GetStringRaw_0x4df070` reads). Signatures are B's from `chat/from-B.md`
   C-36 — keep them verbatim so their caller compiles unchanged:
   ```cpp
   const char* SectionNameAt(int i) const;                 // i in [0, SectionCount())
   int  EntryCount(const char* section) const;
   const char* EntryKeyAt(const char* section, int i) const;
   const char* EntryValueAt(const char* section, int i) const;
   ```
3. Out-of-range and missing-section handling: return `NULL` (names/keys/values) and `-1`
   (`EntryCount`). Do **not** assert and do **not** log-and-continue — a skin loader will probe
   absent keys routinely. Document that choice at the function.
4. `const` correctness: all four must be callable on a `const VFX_Parameter&`.
5. Add cases to the existing suite that cover: valid index, index == count (boundary), negative
   index, unknown section name, and a section that exists but has zero entries.

## Acceptance

- New checks in `modules/openje/tests/je/GUILoading/GUILoading.cpp` (or the ResourceSystem suite,
  whichever already constructs a parsed ini) pass, including the boundary cases above.
- `run-all-tests` stays at 9 pass / 0 fail; `cppcheck-openje` reports no **new** finding on the
  two files (compare against `docs/notes/cppcheck-openje-sweep-2026-10-20.txt` baseline).
- `msvc-build` exit 0 — this matters: a mismatched `struct`/`class` keyword in a new forward
  declaration is invisible to mingw and is an `LNK2019` under MSVC 2002.
- Zero behavior change for existing callers (`git diff` shows additions only).

## References

- `chat/from-B.md` C-36 (the ask, with the exact signatures and why `Skin::vft[+8]`
  `LoadByName_0x522c30` → `F3::LoadIniResource_0x4e0960` makes one skin == one `.ini` resource)
- `modules/openje/src/JE/VFX_Parameter.hpp:32-35,56,63` (storage + existing getters)
- `van-buren-docs/File-Format-Specifications.md` (ini/skin section layout)

## Notes (coordinator)

- **Additive only.** Do not refactor `GetStringRaw_0x4df070`, do not rename existing members, and
  do not touch `m_sections` layout — B is live in this area for T-007 and a reshape there starts
  a collision you cannot see in your own test run.
- If you find yourself wanting to change a signature because the storage fights it, stop and send
  the coordinator a message instead — that means the task is wrong, not you.
