# chat/ — cross-agent coordination (one-writer-per-file)

## Protocol v2 (why: shared append files + anchored edits both caused silent clobber/
no-op incidents; every mutable file now has exactly ONE writer)

- **Write only your own file**: `chat/from-A.md` (implementer-A), `chat/from-B.md`
  (implementer-B). Never edit another agent's file; never "reply inside" it.
- **Read everyone's** at session start, newest entry downward, and act on `ACTION:` lines.
- Entries: `## [C-<n>] <date> — <author> re: <topic>` (number continues from
  `ARCHIVE-log.md`, which is frozen history — do not edit).
- `discovery` agent: they don't run git; their output lands in `discoveries/` and is
  committed by implementer-A as checkpoints. Reimpl-directed questions live in their docs'
  "Questions for the reimpl agent" sections; answers go in
  `discoveries/RESPONSES-from-reimpl-agent.md` (writer: implementer-A; B gets
  `discoveries/RESPONSES-from-implementer-B.md` if/when needed).

## File ownership (mutable-at-any-time = owner only; everyone else reads)
| path | writer |
|------|--------|
| chat/from-A.md | A |
| chat/from-B.md | B |
| tasks/README.md (board rules) | A; task files: the current claimant (status/assignee/Notes) |
| docs/DEFERRED-NOTES.md | A (living backlog) |
| docs/notes/implementer-B.md | B (their journal, create when ready) |
| discoveries/* | discovery agent writes; A commits checkpoints |
| docs/f3/*.md | claim via task before editing; appends under own heading allowed |
| `modules/openje/tests/**`, tooling scripts | B lane; engine/game sources | A lane |

Stale-exe / submodule-first / ground-truth invariants unchanged (see C-1 in ARCHIVE-log.md).
