# tasks/ — shared task board (parallel implementers)

Two implementers work this repo in parallel. This board is the source of truth for **who is
doing what**; `chat/log.md` is for conversation. The discovery agent is separate
(read-only RE, writes `discoveries/`).

## How to take a task

1. Read `chat/from-A.md` + `chat/from-B.md` (newest entries) first — every session, always.
   One-writer-per-file protocol in `chat/README.md`; `chat/ARCHIVE-log.md` is frozen.
   ALSO poll `discoveries/` (new/changed files: `ls -t discoveries | head`) — the discovery
   agent writes live RE notes there between commits and tasks often get unblocked or
   invalidated by them (field naming, formats, "don't trip on this" notes). Reference by
   filename; treat everything there as evidence-backed but verify claims that gate your edits.
2. Pick a task file with `status: open`, set `assignee`, `status: in-progress`, and commit
   that claim **before** editing any code. Release (back to `open` + a chat note) if blocked
   or idle >1 session.
3. One task at a time; finish or hand off before claiming another (WIP limit = 1).

## Conventions

- File name: `T-<nnn>-<slug>.md`.
- Sections: Objective / Steps / Acceptance / References / (optional) Notes.
- Acceptance must be mechanically checkable (a command, a test name, a diff).
- Evidence style: binary addresses `0x…`, member offsets, and file paths — same as
  `docs/DEFERRED-NOTES.md`.
- When a task is done: `status: done`, paste the acceptance evidence into the file, commit,
  and announce in chat.

## Lane ownership (do not edit outside your lane without a chat handshake)

| lane | owner | areas |
|------|-------|-------|
| Engine core | implementer-A (senior) | `modules/openje/src/JE/*` except GUI, pathfinding, Map/resource/world/entity-spawn, pumps |
| Game layer | implementer-A | `src/F3/*` (world, entities, actions, spawn) |
| Tooling & tests | implementer-B (new) | `.devenv/scripts`, `devenv/scripts`, `run-all-tests`, test harness glue, `modules/openje/tests/**` (existing suites), cppcheck/lint scripts, `docs/notes/implementer-B.md` |
| GUI/skin lane | implementer-B → A review | RE + `JE::GUI`/`F3` interface loading (`.int`, skins, fonts) — new files only until handshake |
| Docs | both (append-only sections) | `docs/`, `discoveries/RESPONSES-from-reimpl-agent.md` (B may append under its own heading) |

Submodule note: `modules/openje` is a git submodule — commit inside it first, then bump the
parent pointer (both repos must end each task clean).

## Active board

| id | title | status | assignee |
|----|-------|--------|----------|
| T-001 | Onboarding: build + run every suite green | in-progress | implementer-B |
| T-002 | Fix `cppcheck-openje`; add `run-all-tests` script | open | — |
| T-003 | GUI RE recon: `.int` load chain field notes (read-only IDA) | open | — |
| T-004 | Test coverage: EMAP v5 fog/ambient byte stores | open | — |
