# Coordination — pi-messenger mesh + pi-intercom (protocol v3)

Live coordination between implementers now runs over **pi plugins**, not append-only chat
files. The file protocol (v2, `chat/from-A.md` / `from-B.md`) is frozen history — read
`chat/ARCHIVE-log.md` and the tail of `chat/from-{A,B}.md` for past decisions; do not start
new threads there.

**One rule survives everything:** the mesh is the *wire*, the repo is the *record*. Anything
that must outlive a session (a corrected fact, a decision, an acceptance proof) gets written
into `discoveries/`, `tasks/`, or `docs/` and committed. "I messaged them about it" is not a
decision trail.

## What runs where

| need | mechanism |
|------|-----------|
| "who is alive, what are they touching, did the gate go red" | `pi_messenger` presence + `feed` |
| broadcast / short reply to a named peer | `pi_messenger` `send` / `broadcast` |
| "I am editing this file now, don't clobber me" | `pi_messenger` `reserve` (blocks peers' write/edit) |
| blocking question to one peer, answer needed *in this turn* | `intercom` `ask` (returns the reply as the tool result) |
| durable RE facts, evidence, answers to `C-<n>` questions | `discoveries/` files (unchanged — repo data, the discovery agent does not join the mesh) |
| task identity, acceptance criteria, proof | `tasks/T-*.md` (unchanged — repo data) |

`chat/README.md` (this file) documents the protocol; `docs/notes/` stays per-agent journaling.

## Startup (every session, in order)

1. `pi_messenger({ action: "status" })` → join check + your name; `list` → who else is here.
2. `pi_messenger({ action: "feed", limit: 20 })` → the event stream replaces "read both chat
   files top-down". Commits, edits, test results, messages, task events land here.
3. `tasks/README.md` board → pick your row; then **reserve before you edit** (below).
4. Tail `chat/from-A.md` / `chat/from-B.md` **only if** your task cites a specific `C-<n>`
   thread — the files are the archive of those numbers.

`discoveries/` polling is unchanged: `ls -t discoveries | head` at session start — the research
sessions write evidence files there and commit them; the mesh is only how you ask.

## File ownership → reservations

v2's "one writer per file" was a social contract; v3 makes it mechanical. Before the first
write in a lane, reserve it, and release when the task commits:

```typescript
pi_messenger({ action: "reserve",
               paths: ["src/F3/GameActionAttack.cpp", "src/F3/GameActionAttack.hpp"],
               reason: "T-011 damage apply (session 7 §3)" })
// ... work, commit ...
pi_messenger({ action: "release", paths: ["src/F3/GameActionAttack.cpp"] })
```

- Reservations are advisory to `git`, not to peers: a peer's `edit`/`write` on a reserved path
  is **blocked** with your name and reason, so reserve *before* editing, same as the claim
  commit was. Reserve the narrowest set of paths that covers the task.
- Release explicitly; `leave`/exit auto-releases, but a stale-looking reservation still makes
  a peer stop and ask.
- The lanes in `tasks/README.md` still decide *who may* own an area. A reservation is a claim
  inside your lane, not a claim on someone else's lane — that still needs a handshake, now by
  `send`.
- Test suites under `modules/openje/tests/**` and `.devenv`/`devenv/scripts`: B's lane. Engine
  and game sources: A's lane. Shared infrastructure (`devenv/scripts/run-all-tests.nix`) takes
  a `send` first — additive rows are fine, reordering someone's rows is not.

## Module map (who to talk to)

| topic | to | reply expectation |
|-------|----|-------------------|
| build/gate red, cppcheck, run-all-tests rows, GUI/skin lane | implementer-B | `send`, non-blocking |
| an RE fact you need *now* (offset, polarity, enum value) | the 4 research sessions (`IronFalcon`, `KeenOwl`, `IronViper`, `KeenJaguar2`) | `pi_messenger send` to ONE of them (never broadcast — 4 agents duplicate the same dig), keep the durable answer in `discoveries/` |
| engine/game semantics I changed under you | implementer-B `send` + the commit that documents it | — |
| cross-task steering, priorities | the human | `intercom ask` to the driving session |

Discovery stays on files on purpose: their output is evidence, it must be reviewable and
committed, and they may not be running pi at all. Do not `ask`-block a lane that owns the file
you are about to write — reserve it instead.

## Completion protocol — every task ends with a report, or it didn't happen

A task is not finished when the code compiles. It is finished when the **coordinator has the
result line**. Silent completion is indistinguishable from being stuck, and I will not guess.

```typescript
pi_messenger({ action: "send", to: "NiceFalcon", message:
  "[T-0nn DONE] verdict=green|red | gate=<summary line verbatim> | commits=<sha list> | " +
  "next=<what you're starting or 'idle, need assignment'>" })
```

Required in every report:
1. **`verdict`** — green/red, and if red, the failing row verbatim (not a paraphrase).
2. **`gate`** — the literal `run-all-tests` summary line, or the honest reason it could not run.
   Numbers must be the REAL tally: 9 suite rows + app-boot = `=> 10 pass, 0 fail`
   (+`msvc-build` = 11). Do not normalize it to 9.
3. **`commits`** — SHAs, submodule-first (openje commit, then parent pointer bump).
4. **`next`** — either the row you picked or `idle, need assignment`. I dispatch on this line;
   no line, no next task, because I assume you're still working.

Also report, do not sit on:
- **`[BLOCKED]`** + what unblocks it (a peer, a fact, a decision from me). Say it early — blocked
  quietly for a session is the most expensive failure mode we have.
- **`[FACT-CHECK]`** when a task's premise looks wrong. This already worked: implementer-C found
  that my own board's acceptance line (`9 pass`) was arithmetically impossible, because app-boot
  shares the tally. Correct catches are a gift; report them the same way.
- **`[DISCOVERY-BATCH-N ready]`** (research lane) — the only trigger for me to commit `discoveries/`.

When I reply with a new row, claim it (edit the row + commit) and reserve your paths. WIP stays
at 1: finish or hand back before taking another.

## Message shape (same discipline as `C-<n>`)

`[T-<n>] <topic> — <one line of what changed> | ASK: <exactly what you need, if anything>`
Cite addresses/offsets/discovery filenames, not vibes. One writer per file never applied to
the mesh: `send` is append-only by nature, so no one edits anyone else's message.

## Engram memory (the recall layer)

Engram is the third channel: **mesh = live coordination, `discoveries/` = committed evidence,
Engram = recall**. Both agents share one store, so a finding saved by A is searchable by B
without a message and without waiting for a commit. The binary comes from
`devenv/packages/engram.nix` (plus the out-of-shell shim below); the store is the platform
default `~/.engram` and **`ENGRAM_DATA_DIR` is deliberately not set** — see the note in
`devenv/environment/openvb.nix`: pinning a repo path would create a second store for sessions
booted outside the devenv, and the extension fails closed on the resulting instance-id mismatch.
Project separation comes from the tracked `.engram/config.json`, not from the directory.

Use `mem_save` for exactly the things that are expensive to re-derive and not yet file material:

| save | type | example |
|------|------|---------|
| a verified RE conclusion | `decision` | "DT-row selector in 0x413310 is body PART, not style" |
| an offset/layout census | `decision` | "base-skill array is GameCreature+0xE0 int[13]" |
| a reproducible gotcha | `task` | "MSVC C2248 needs explicit public: after long inline init-lists" |
| a rejected hypothesis | `decision` | "EME2 flag 0 does NOT freeze NPCs in the demo slice" |

Rules that keep it from rotting into a second source of truth:

- **Evidence stays in `discoveries/`.** A memory cites a discovery filename + address; it never
  replaces the doc. If a memory and a committed doc disagree, the doc is wrong or the memory is
  stale — resolve it by reading the decompile, then update both.
- **Retrieval is BM25 over SQLite FTS, and default `match_mode:"all"` is AND.** Stop-words kill
  queries: `"which DT row does called shot part select"` returns nothing, `"DT row"` and
  `CALLEDTARGET` hit. Query with keywords, or pass `match_mode: "any"` for broad recall, then
  narrow by `type`/`project`. (Verified against core 3.5.1.)
- **Titles carry the recall.** Short, specific, keyword-dense title; body holds the citation.
- `.engram/chunks/` is gitignored: `engram sync` output is memory payload, and committed payload
  is permanent in history. Only `.engram/config.json` (project identity) is tracked.
- **No embedding provider is involved** — nothing to key, nothing to leak, works offline.
  `--semantic` conflict scanning shells out to an agent CLI (`ENGRAM_AGENT_CLI`); it is not an API.
- **Evidence still lands in `discoveries/*.md`.** The research sessions are on the mesh (they
  run pi), so asking them live is now the fast path — but a finding is only *adopted* when it is
  written into `discoveries/` and committed. Engram is an index over that corpus, not a
  replacement for it: don't route a durable finding into a memory and stop there.

### Engram binary: the PATH fact that decides everything

- **Inside the devenv shell** (how these agents actually run: `nvim -> devenv shell -> zsh ->
  pi`), `engram` must come from `devenv/packages/engram.nix`. `~/.pi/agent/bin` is **not** on
  that PATH, so a shim there does nothing for in-shell sessions.
- The directory that *is* on the in-shell PATH but does not exist by default is
  `~/.local/state/nix/profile/bin`. Creating it and putting the binary there is the fix that
  works **without restarting anything** — verified: `mem_doctor` went from
  *"The Engram binary 'engram' could not be found"* to 10/10 ok in the same live session.

```bash
mkdir -p ~/.local/state/nix/profile/bin
cp "$(nix-build -E 'let pkgs = import <nixpkgs> {}; in
      pkgs.callPackage ./devenv/packages/engram.nix {}' --no-out-link | tail -1)/bin/engram" \
  ~/.local/state/nix/profile/bin/engram && chmod +x ~/.local/state/nix/profile/bin/engram
```

**Do not believe the earlier claim in this file that a failed probe is cached for the process
lifetime.** It is not: the extension resolves `engram` through PATH *at spawn time on each
call*, so `mem_doctor` re-probes and recovers. The only genuine restart requirement is for
`ENGRAM_URL`/`ENGRAM_PORT`/`ENGRAM_BIN`, which are read at extension load.

These shim copies are real files (survive nix GC) and therefore **silently pin 3.5.1** — re-run
the command after bumping `devenv/packages/engram.nix`, or re-enter the devenv shell so the nix
module provides the binary instead.

## Bootstrap — one-time, per agent (already done in this repo by implementer-A)

Packages are project-local and `.pi/` is gitignored, so **a fresh worktree has no mesh**:

```bash
pi install npm:pi-messenger -l
pi install npm:pi-intercom  -l      # writes .pi/settings.json, deps into .pi/npm/
```

Config that then applies to every session from this cwd:
- `.pi/pi-messenger.json` — `autoRegister: true` (join on start), `scopeToFolder: true` (only
  this repo's agents are visible), `contextMode: minimal` (no per-turn context spam), overlays
  off (autonomous sessions must not be hijacked), `feedRetention: 200`.
- `~/.pi/agent/intercom/config.json` — `inboundTrigger: always`, `busyDelivery: steer` (a peer
  answer queues into the next safe boundary instead of aborting a 40-minute RE turn).

Name yourself deterministically — both the mesh name and the intercom target come from the
launch, so `send to:` and `ask to:` are stable across restarts (no random themed names):

```bash
PI_AGENT_NAME=implementer-A pi --name implementer-A
PI_AGENT_NAME=implementer-B pi --name implementer-B
```

Without `PI_AGENT_NAME` the mesh assigns a random themed name and peers cannot address you by
a stable name; without `--name` intercom falls back to a runtime alias. Sessions already
running when this landed must **restart** to load the extensions.

## Still true (unchanged by v3)

- Submodule `modules/openje` commits first, then the parent pointer bump.
- Both trees clean at task end; never trust a stale exe (`rm -f` before build, check exit).
- `discoveries/` is evidence-backed but verify claims that gate an edit.
- Uncertain fact → routed question, never a guess in code (that routing is now `C-<n>` in a
  discovery question, not a mesh message).
