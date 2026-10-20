# Harness items surfaced during the coordination switchover

Collected by the coordinator (mesh `NiceFalcon`, sessionId `01a15a98`) while moving the team off
file-based chat. Ordered by how much they currently cost the team, not by effort.

## 1. Stale devenv payloads in already-running shells — **active, misleads agents**

The single most damaging item. A shell opened before a `devenv/*.nix` change keeps resolving
scripts to the **session-start store path**, silently:

- `command -v run-all-tests` → `/nix/store/2vfkk6y…-run-all-tests/bin`, whose table has **6 rows**
  while the repo table has **9**. Result: 4 phantom `BUILD-FAIL kind=undefined-reference` that look
  exactly like engine regressions.
- The STALE guard added later is *in the newer payload*, so the stale one cannot print the warning —
  the safety net is absent precisely when it is needed.
- `devenv-reload` is `command not found` in that same shell, so the documented remedy is not
  reachable from inside it.

Confirmed independently by implementer-B (reproduced 9/9 in a clean env, 4 phantom reds in their
live shell) and by me (same resolution path, `grep` of the installed payload).

**Candidates:** (a) the gate re-execs the newest store payload it can find rather than trusting
`PATH`; (b) the suite-table hash is baked into the build and the app-side check, so a mismatch is
loud; (c) `run-all-tests` compares its own `$PREFIX` table against `devenv/scripts/run-all-tests.nix`
and refuses to report red when they differ. I lean (c)+(a): cheap, no rebuild, no new state.

## 2. Pi packages are project-local but `.pi/` is gitignored — **structural friction**

`pi install -l` writes `.pi/settings.json` and `.pi/npm/`, and `.gitignore:44` ignores `.pi/`, so
**no worktree inherits the coordination stack**. Every fresh worktree silently loses messenger,
intercom, and Engram. Options: un-ignore only `.pi/settings.json` + the two config JSONs (keeps
`npm/` out), or install in user scope — but `~/.pi/agent/settings.json` is a **home-manager
symlink** and read-only, so user scope needs the NixOS module, not `pi install`.

Related: `.pi/messenger/crew/config.json` (I raised the send budget there) and `.pi/pi-messenger.json`
are likewise invisible to a new worktree.

## 3. Engram binary is not reachable from the shell pi actually runs in — **fixed, but fragile**

`pi` here runs inside `devenv shell` (`nvim → devenv → zsh → pi`), whose PATH does **not** include
`~/.pi/agent/bin`; the dir that *is* present but missing by default is
`~/.local/state/nix/profile/bin`. Fixed by the nix module plus a copy into that directory, and note
the copy silently pins the version. Also: `ENGRAM_DATA_DIR` must stay unset — pinning a repo path
creates a second store and the extension fails closed on the instance-id mismatch.

## 4. pi-messenger per-session **outbound** budget is too small for a coordinator

`handlers.ts`: `messageBudgets[coordination] ?? 10`, counter module-level (resets only on reload),
enforced on **send/broadcast only** — inbound, `feed`, `reserve`, files, and git are unaffected.
I hit 10 and could no longer reply, while five agents were asking. Raised to 40 in
`.pi/messenger/crew/config.json`. Worth deciding deliberately: either a separate coordination level
for a non-implementing role, or an explicit "if rejected, write to the task row" convention (I told
the lanes this, which is why work never actually stalled).

## 5. Mesh identity is unstable and not self-declared — **caused real routing errors**

Names re-randomize on every reload; two messages arrived signed by one agent but written *about that
same agent* in the third person, and I once misaddressed corpus credit because of it. The stable key
is `sessionId` in `~/.pi/agent/messenger/registry/<name>.json`.

**Candidates:** require the sessionId in the first message after reconnect (now a board rule, 10g);
or set `PI_AGENT_NAME` + `pi --name` at launch so names are stable by construction — nobody on the
team currently does, which is why every mapping has to be re-derived.

## 6. Reservations and `git` do not interlock

Reservations block a peer's `edit`/`write` tool calls, but **not** `git`. So `git add <dir>` can
sweep a reserved file, and one of my commits did exactly that (FastArrow's row into my message).
Also, a file reserved by lane A can still be committed by lane B on lane A's behalf — fine by
protocol, but nothing *says* which lane owns which dirty file.

**Candidates:** prefer explicit paths in every lane's tooling (now my rule after two incidents); an
optional convention where a reservation names the intended *committer*.

## 7. Shared single-file append targets collide by design

`discoveries/README.md`, `HANDOFF-*.md`, and `discoveries/RESPONSES-from-reimpl-agent.md` are
multi-lane append surfaces; so is the canonical fact sheet. Two lanes declared the **same**
`[DISCOVERY-BATCH-2 ready]` tag with different file sets — caught only because I asked which files.

**Candidates:** lane-prefixed batch tags naming files inline (adopted informally, worth formalising);
per-lane files plus a generated index instead of hand-appended shared files.

## 8. One research finding can silently fail to land

NiceWolf's canonical §14 append exited 0 but was never written; they caught it by re-grepping.
Same family as `grep -c` exiting 1 on zero matches. **Rule for every lane: verify bytes exist after
writing, never trust exit status alone.**

## 8b. Silent-guard-skip passing through a success print — **a bug class, not a bug**
(discovered by NiceWolf `01a157d7`, root-caused and reproduced by the coordinator)

A research lane's canonical append exited 0 and printed "written" while **never writing**. Three
independent layers each nearly caught it and none did:

1. an idempotency guard written as `if "5b" not in content` — meant as "no §5b marker yet", but the
   file legitimately contains `5b` inside a **hex address token**, so the guard was false and the
   whole block, file-open included, was skipped;
2. the success `print` was **unconditional**, outside the block that did the work — same family as
   `grep -c` exiting 1 on zero matches, and the reason a zero exit status is never evidence;
3. no read-back after write.

Verified independently, not accepted on report: `git show b5a3de0:...canonical... | grep -o 5b`
returns exactly one hit with **no** matching heading, i.e. the collision was in hex text as described.

**Cost of one layer failing:** the claim travelled into a commit-message draft and a chat line before
it was checked, and the lane then hypothesised an innocent cause (a commit race) that was also wrong.
Rules now standing for every lane:
- anchor guard patterns to structure (`^## 14\.`), never to a bare substring, in any file whose
  content includes addresses;
- success reporting goes **after** a read-back, and only inside the branch that wrote;
- re-grep the file after any append, before telling anyone it happened.

**Harness-level response:** these lanes hand-edit shared markdown with throwaway scripts and nothing
checks the result. A tiny append-with-verify helper (write → re-read → fail loudly on no diff) would
have caught layer 3 automatically, and per-lane files plus a generated index (item 7) removes the
collision surface that made layer 1 matter.


## 9. `discoveries/` is the uncommitted working set of five lanes

~50 files legitimately dirty, so "tree is clean" no longer means anything, and I have twice written
checkpoint prose from memory instead of re-deriving the file list (an attribution error followed).
**Candidates:** a `dirty`/`ready`/`committed` marker in the file front matter; or per-lane
subdirectories so a checkpoint is a pathspec, not a list assembled from messages.

## 10. Two files named `T-012` / numbering races

Task numbers are allocated by whoever writes a file first, and implementer-B proposed a number that
was already taken mid-flight (resolved: T-013). Cheap fix: allocate numbers on the board, not in
filenames.

## 11. Not a bug, but it shapes the above: agent **roles** are not modelled anywhere

The coordinator is now a role with different needs (high send budget, no feature ownership, sole
committer of research output, gate authority), yet every session is configured identically. Most of
items 2–6 are that mismatch showing up as tool friction.
