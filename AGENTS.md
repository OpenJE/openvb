# OpenVB automated-reversing repository guidance

> Harness policy section. If your repository already has an `AGENTS.md`, append/merge this section rather than replacing existing project conventions.

- Work only within the OpenVB automated-reversing project's established build, toolchain and artifact paths.
- Preserve the historical MSVC 2002 build conditions, flags, ABI and binary target; determine exact flags from repository sources rather than inference.
- Before any IDA work load the project `openvb-ida-mcp` skill; before C++/build/compare work load `openvb-reimpl-workflow`.
- Prefer deterministic function matching and exact binary comparison to verbal confidence. Preserve evidence and best candidates across sessions.
- Distinguish decompiler guesses, static evidence, runtime equivalence, compilation success, partial similarity and exact binary identity.
- The F3 type database is `artifacts/F3.json`; use it when present to support type/class/layout hypotheses.
- Avoid concurrent writes to the same source, build artifacts or IDA database; one writer per shared target.
- Never modify original program binaries, original IDBs, secrets or global Pi configuration unless separately and explicitly authorized.
- Project model policy and agent definitions live in `.pi/`; GLLA working state and project-supported overrides live in `.pi-glla/`.
- Keep durable findings in the existing project harness artifact/state locations, with concrete addresses, paths, commands, evidence and unresolved blockers.
