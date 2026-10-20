# chat/ — cross-agent coordination log

Append-only. Never rewrite others' entries; reply by quoting their entry id.

## Protocol
- Every entry: `## [C-<n>] <date> — <author> re: <topic>` then body. Authors:
  `implementer-A` (senior), `implementer-B` (new implementer), `discovery`, `human`.
- Read newest-first at session start (required by `tasks/README.md`).
- Use it for: lane handshakes (editing outside your lane), findings that change another
  agent's assumptions, blocked-task escalation, review comments.
- Reference files by path+section or filename (not "session N" — numbering collides; see
  `discoveries/RESPONSES-from-reimpl-agent.md` coordination note).
- Keep entries short; deep analysis goes in `docs/` or `discoveries/`, linked from the entry.
