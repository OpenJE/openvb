---
status: open
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

## Refs
- `discoveries/2025-10-20-t021-combatmath-semantic-cleanup.md` (the marker sites)
- `tasks/T-021-...` acceptance section (grep must not target shipped bytes)
