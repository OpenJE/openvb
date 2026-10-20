---
status: done
assignee: implementer-C (UltraBear, 01a15aff)
depends: nothing
---

# T-022 — Assert every "verbatim shipped" string is byte-identical in F3.exe

## Why
T-021 nearly corrupted a shipped log string to satisfy a lint grep, and C proved the literal was
byte-identical at PE file offset `0x24fb48` **by hand**. "We preserved it" and "it matches the
original" are different claims and only the second is testable. Automate the hand method.

## Scope
1. `tools/verify-shipped-literals.py` (python3, dev-shell available).
2. Discover our literals: every string marked `// shipped string verbatim, do not "fix"`, plus a
   `--literal` manifest file for ones not yet marked, so marking is opt-in and incremental.
3. Locate each in the shipped `F3.exe` by byte search; report offset, count of matches, and
   **fail on ambiguity** (0 matches = drift; >1 = cannot claim a site without an EA).
4. Exit non-zero with a per-string table. Wire as `run-shipped-literals` **only via a coordinator
   changefile edit** — this lane must not touch `devenv/scripts/default.nix`.

## Acceptance
- Runs in seconds, no build, no wine.
- Catches a deliberately corrupted literal (prove it by breaking one in a scratch copy, not in place).
- Reports the EA-style offset for each site so a citation can reference it.
- `docs/notes` sentence added: every string we call shipped carries a do-not-"fix" marker **and** a
  passing byte assertion.

## Evidence (implementer-C, 2026-10-20)

`tools/verify-shipped-literals.py` + `tools/shipped-literals.txt` + `docs/notes/shipped-literals.md`.

- **9/9 claims pass**, no build, no wine, <2s: 1 marker-discovered (the T-021 literal) + 8 manifest
  claims (6 shipped `.veg` asset names, 2 shipped log strings).
- **Falsification ships with the tool** (`--self-test`, 6 scenarios, all green): control passes;
  a **one-byte** corruption -> DRIFT; a **prefix** of a longer shipped string -> FRAGMENT; a
  **duplicated** site -> AMBIGUOUS; a literal quoted only **inside a comment** is not claimed;
  and an **empty claim set is not a pass** (the row's "runs green while asserting nothing" shape).
  The scratch copy lives in a tempdir; the self-test cannot write into the repo.
- **EA reporting is derived, not assumed**: `ImageBase` 0x400000 read from optional-header `+0x1C`
  for magic 0x10B, then mapped through the section table. The T-021 hand result reproduces exactly:
  raw `0x24fb48` -> EA `0x64FB48`. (My first read used `+0x0C` and printed base `0x0`, then
  `0xCF000` - a wrong base silently mislabels every site, so the field is now documented in-code.)
- `FRAGMENT` is the verdict a hand search cannot produce: shortening a quoted string still "finds"
  it and looks correct. Whole-string means the following byte is `0x00`.

### Two bugs I found in my own tool, recorded because both are the silent kind

1. Discovery counted a phrase **quoted inside a comment** as a shipped literal. Comments are now
   stripped before reading: prose is not a claim.
2. The first stripper **shortened the text**, so every reported line number silently drifted
   (it claimed line 139 for a marker at 138 - right answer for the wrong reason). Comment removal
   now blanks in place and preserves length.

### Boundary, printed by the tool itself, not just here

Bytes + offsets only. It does not prove we call the string from where the original does, that
behaviour matches, or that the exe runs. `--survey` is advisory: 89 unclaimed literals currently
match one whole shipped string; a match is not a claim and marking stays a human decision.

### One thing I did NOT do, on purpose

I first added the two `shipped string verbatim` markers in `src/F3/CombatMath.cpp`, then removed
them: that file carries **live uncommitted T-027 work** (implementer-B's LCG `ShippedRand` seam,
plus `CombatMath.hpp` and B's suite). Committing it would have smuggled their hunks into my
message mid-flight. The two claims moved to the manifest instead - identical assertion, zero lane
collision. **When T-027 lands, those markers belong in the file**; ping me and I'll move them back.

### Wiring, as the wave rule requires

NOT wired as `run-shipped-literals`. `devenv/scripts/default.nix` is off-limits to this lane, so the
coordinator adds the wrapper if it earns one. Suggested gate position: after host rows, before
app-boot - it is cheap and it needs no exe, unlike app-boot.

## Refs
- `discoveries/2025-10-20-t021-combatmath-semantic-cleanup.md` (the marker sites)
- `tasks/T-021-...` acceptance section (grep must not target shipped bytes)
