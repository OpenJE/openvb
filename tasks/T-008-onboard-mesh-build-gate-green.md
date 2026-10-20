---
status: in-progress
assignee: implementer-C (UltraBear, mesh sess 01a15aff)
---

# T-008 — Onboarding: enter the mesh, build, and prove the whole gate green

## Objective

Get implementer-C productive without touching shared code: join the mesh, learn the toolchain,
and independently confirm the current committed state is green. Everything after this task
assumes you can reproduce this result in one command.

## Steps

1. `pi_messenger({action:"status"})` then `{action:"list"}` — you should see 7 agents. Note your
   **sessionId**, not your name: names are re-randomized on every `/reload` or restart, so the
   sessionId (registry `~/.pi/agent/messenger/registry/<name>.json`) is the only stable identity.
2. Read, in this order: `chat/README.md` (protocol v3 — mesh/Engram/discoveries split),
   `tasks/README.md` (lanes + WIP=1), `.pi/skills/openvb-reimpl-workflow/SKILL.md` (build/run),
   `.pi/skills/openvb-ida-mcp/SKILL.md` (only if you will do IDA work).
3. Reserve before writing anything: `pi_messenger({action:"reserve", paths:[...], reason:"T-008"})`.
   A reservation blocks a peer's `edit`/`write` mechanically. Release when done.
4. Build: `rm -f build/F3.exe && msvc-build; echo "exit=$?"`. Check the exit code — never judge
   a build through a pipe, and remember link failures are `LNK`/`error LNK`, not `error C`.
5. Run the gate. **The installed `run-all-tests` may be stale** if the devenv shell predates a
   table change; the source of truth is the `suites=(...)` table in
   `devenv/scripts/run-all-tests.nix`. If a row prints `kind=undefined-reference`, the fix is to
   add the defining `.cpp` to that row — never to weaken a test.
6. Boot check: `f3-reimpl-run-data` (or a `/tmp` private copy under wine). Exit 124 = alive =
   pass; 126/1 = wine prefix contention, retry once after 3s before calling it a regression.
7. Post the result on the mesh to `NiceFalcon` with the exact command output.

## Acceptance

- `msvc-build` exits 0 with no `error C` and no `LNK` matches in its output.
- `run-all-tests` (or the replayed table) reports **9 suites** and `=> 9 pass, 0 fail`, plus
  `app-boot OK exit 124`.
- A mesh message to the coordinator containing the summary line verbatim.
- No file outside your own task file modified in this step.

## References

- `chat/README.md` §"Startup (every session, in order)", §"File ownership → reservations"
- `devenv/scripts/run-all-tests.nix` (suite table + why rows grow)
- `docs/DEFERRED-NOTES.md` "Tooling traps"

## Notes

- Expect `discoveries/` to show ~60 uncommitted files. **Do not commit them** — that is live
  research-agent churn; only the coordinator checkpoints them, and only when a research agent
  says the set is ready.
- If `engram` is missing from your PATH, see `chat/README.md` §"Engram binary" — the fix does not
  require a restart.
