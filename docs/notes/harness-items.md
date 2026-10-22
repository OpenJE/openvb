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


## 8c. The phantom-slot class: absolute-address scans that "find" references that are not there
(method note from HappyWolf `01a157ec`, archived at their suggestion)

While resolving the GameInterfaces late-slot owners, a heuristic scan for raw absolute addresses
(e.g. `0x70BE38`/`0x70BE48`) matched what looked like live references — and was wrong. The matches
were **FPU thunk stores**, not typed uses. The only thing that disproved it was finding the real
allocation site first: the object is a `new(0x64)` @0x45e8f8, so any reasoning about its slot
layout that started from a guessed static base was built on sand.

Why it belongs in this file next to 8b: the failure mode is identical in shape — a plausible signal
that survives every check *except* the one that would have established ground truth. A lane scanning
absolute slots without locating the allocation would have "proven" a phantom `+0x30`/`+0x40`
reference and written it into a doc with a citation, which is the worst kind of wrong: verifiable
evidence, unfounded conclusion.

Standing guidance for every lane: **a slot is not attributed until you have reached it from a typed
use or an allocation site.** An address-string match is a lead, not a finding, and if a census is
the deliverable, say which of the three it rests on (typed use / allocation site / raw scan) so the
next reader knows how much to trust it. Note the honest outcome here: the slots turned out to be
**dead** — a correct negative result is a deliverable, not a failed task.

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

## 12. No check that `tasks/` files and the board table agree — **caused real drift today**

`T-016` was written, committed, and referenced in two messages, yet had **no row** in the
`tasks/README.md` table: a lane reading only the board would conclude the runtime batch does not
exist. `T-010`'s row also still advertised sub-items as open that were answered, reassigned, and
partly resolved-as-defect. Both were found by accident, while inserting unrelated rows — not by any
check.

Cheap, deterministic, and worth a script more than a rule:

```bash
# every task file must have exactly one table row, and every row must name an existing file
for f in tasks/T-*.md tasks/M-*.md; do
  n=$(basename "$f" | sed -E 's/^([TM]-[0-9]+)-.*/\1/')
  grep -q "^| $n |" tasks/README.md || echo "MISSING ROW: $n ($f)"
done
awk -F'|' '/^\| [TM]-[0-9]+ /{gsub(/ /,"",$2); print $2}' tasks/README.md | while read -r id; do
  ls tasks/"$id"-*.md >/dev/null 2>&1 || echo "ORPHAN ROW: $id"
done
```

Two related conventions that would prevent the softer half of this:
- **a broadcast is not an assignment.** Assign by row (file, committed); use the mesh to *point at*
  the row. Two lanes were working on chat-authority alone today until rows were added.
- **status strings age silently.** `open | research (10a X, 10b Y, 10c Z)` was wrong within one
  turn. Prefer one row per sub-item if sub-items are going to be claimed and closed independently —
  which is exactly how 10a–10e drifted.

## 13. "Needs a debugger" often means "nobody opened the data files" — lesson from T-018

Three lanes converged on the view that called-shot part naming required a runtime observation
(which equip slot receives a helmet, plus a log-diff fingerprint at a breakpoint). It required
neither: the answer was **in `build/data/_ARM.grp`**, statically readable in minutes — every head-worn
piece has `GITM.eqslot = 1`, every suit `0`, which closed the question outright and *killed* the
runtime read as unnecessary. The `.grp` layout is now documented for reuse, with a trap noted:
compressed records contain stray `GITM` strings that decode to garbage `equip = 0` values, so sweeps
must filter on `equip == 1`.

Same shape as 8c from the other side: there, a static scan produced a phantom from the wrong source
(absolute-address matching); here, a static *limit* was assumed when a different static source had the
answer. Both are the same rule: **before requesting a runtime read, enumerate what on-disk data could
settle it.** Data is cheaper than a debugger, reproducible by every later session, and does not
depend on anyone holding a debug window.

Corollary for the coordinator: T-016 was created to batch three runtime reads. Two of the three are now
closed without a debugger, and the row shrank on its own. Ask "is this data?" before "is this
observable?".

**Footnote, added by FastArrow from the counter-example (T-019 stage 1), because it is the
part that stops this rule becoming naive:** data first, **then verify the mapping against the code.**
A census over `build/data/_GRP` files answered a question no debugger would have (all 57 GWAM chunks
are v5, so the consumed+thrown `FatalError` is not dead by gating) — and the same census produced a
plausible false positive (three records whose last two body bytes are both set) that only died on
re-reading the checker, where the two operands are asymmetric: one is the last stream byte, the other
byte 2 of a fixed field, not the chunk end. A data sweep is a lead generator; the field layout still
has to be right before a count is a finding.

## 14. One lane's **uncommitted** WIP can break the gate for every lane — reservations don't help

Reservations block a peer's `edit`/`write`. They do **not** protect against a lane whose uncommitted
work leaves the shared tree unbuildable. It happened: an M-001 probe added `d3d8` to
`cmake.toml`'s link list plus new sources, uncommitted; `d3d8.lib` is absent from the MSVC image, so
`msvc-build` now fails with `LNK1181`, `build/F3.exe` cannot exist, and **`app-boot` fails for
everyone** — while two other lanes' committed work was green. A second lane ran the gate, got red,
and correctly spent a message proving the red was not theirs.

The deeper version: the shared working tree makes "the gate" a **global** property with per-lane
staging. Anything uncommitted is invisible to a claim of health, and the coordinator's "gate
unchanged: 9/9" became a stale incantation repeated for several turns *while the tree was red* — the
exact verify-don't-assert failure this file is full of, committed by me against myself.

**Candidates:** (a) build-affecting probes (link lists, new sources, toolchain files) run in a
**scratch worktree**, never the shared tree; (b) the gate is reported with a timestamp and a
`git status --porcelain` summary so a claim carries its own freshness; (c) no lane edits
`cmake.toml`/`CMakeLists.txt`/`msvc-build.nix` without an explicit coordinator ack, since those are
the files that turn one lane's experiment into everyone's outage.

## 15. Two lanes with independent "session NN" counters — same collision family as the batch tags

`discoveries/session21-*.md` from one lane and `session21` prose from another are different
sessions. Seven files found untracked this turn run a counter that does not match the numbering of
the lane that has been writing `session15…session36`, so filenames collide semantically while being
distinct documents, and any cross-lane citation by bare "session 21" is ambiguous.

**Candidates:** prefix lane initials in filenames (`ironstorm-session35-…`) or cite with a lane tag
(`IS:sess35` / `HW:sess-31`). Same root cause as the batch-tag clash and the mesh-name churn: an
identifier that is only unique inside one writer's head is not an identifier.

**Partly resolved, and my first "resolved" claim was over-stated — recording the correction.** The
seven untracked-but-indexed files were claimed in full by one lane and in part by another: four
(`attack-state-machine-damage-formula`, `hp-damage-wire-and-game-ids`, `tohit-apply-tail`,
`tohit-formulas-complete`) to the combat-counter lane, three (`map-chunk-table-eme2-spawn-handler`,
`gameclient-vft-demand-ui-factories`, `gui-root-cls-0x4b28a0-map`) to HappyWolf's counter. I asserted
the ownership question was settled from the documents' own header style; it was not — **both** lanes
write `Session NN` headers, so style discriminates nothing, and the index author column reads
`discovery` for every one of them, which discriminates nothing either. Three counters (this lane's,
the combat lane's, IronStorm's `session15-36`) now coexist.

**Settled three of seven, by evidence HappyWolf supplied.** The discriminator is not style, it is
*internal self-reference across committed boundaries*: `gameclient-vft-demand-ui-factories` corrects
"session-26 §5", which is that lane's own `gameinterfaces-singleton-table`;
`gui-root-cls-0x4b28a0-map` cites "session 27 §2/§3" three times plus "session 28", both its own
documents; `map-chunk-table-eme2-spawn-handler` opens a map-load thread "proposed in HANDOFF", which
is that lane's file. Only an author who wrote the earlier session could make those corrections, so the
three are HappyWolf's. Stated honestly and usefully by them: the same shape is FastArrow's habit too,
so this proves *theirs* rather than disproving anyone else's — the remaining four stay with the lane
that claimed them (FastArrow), and I am not reading a partial discriminator as a total one.

So what is settled: every index row resolves to a tracked **file** (the coordination bug), and
authorship is now attributed per-file with a method that could be re-checked. Standing citation form:
self-cite `lane:sessNN`, cross-cite by **filename**, and treat a bare `session NN` as ambiguous until
proven otherwise. The reusable lesson is that the productive discriminator was always going to be a
*content graph* claim — who corrects whom — not a formatting one; I tried formatting first and got it
wrong twice.

The meta-lesson belongs with 8b and 16: I converted an *unverified* inference (header style implies
author) into a *recorded resolution* within one message, and the second time today. Both times the
check that would have caught it was one command away, and both times I ran it only after stating the
conclusion.

## 16. When the link step is unavailable, say what you *did* prove — and there is a usable substitute

`msvc-build` was red on a missing import lib, so a lane could not get the one cross-TU proof a link
gives: that nothing still references a symbol you removed. Instead of claiming green or staying
silent, implementer-C stated the boundary explicitly ("compile + cross-TU verified; **exe-level
behaviour is NOT verified, because app-boot cannot run") and substituted a method:

```bash
grep -rn "removedSymbol" src modules/*/src modules/*/tests modules/tracing | grep -v <own file>
```

…empty output plus four host rows that compile the file against real callers, plus the decorated
export names read out of the produced `.obj`. That is a legitimate partial proof, and naming its
limit is what made it usable: I could act on the compile claim and knew not to act on the rest.

**Why it belongs here:** a broken gate tempts two bad moves — reporting the old green number
(I did this), or landing a rename/behaviour change nobody can link-verify and describing it as done.
The substitute method plus an explicit verified/unverified line is the third option, and it is worth
recording as a convention rather than one person's good instinct. Suggested rule: **when a lane
cannot run a check, it says which check is missing and what it used instead** — and the coordinator
must not fill that gap by quoting a stale result.


## 17. Never read a status through a subshell or a command substitution (UltraBear, T-023)

Three times in two days this project reported a result it had not observed, and all three were the same
mechanism: **exit codes and captured output travel differently.**

- The gate quoted numbers from an **installed payload** whose repo counterpart had changed (items 8b, 14).
- T-023's own falsification harness did `rc="$(run_check …)"`, which captures the child's whole *report*
  as the exit code, and ran the helper in a **subshell**, so `LAST_OUT` never reached the caller. Three
  failure scenarios printed PASS while holding **zero evidence**, because `[ "" != 0 ]` is true.
- `cp -a $ROOT/src $scratch/src` after a `mkdir` nested the tree to `src/src`, so the headline
  falsification was editing a file that had never existed.

The rule: an assertion helper takes the exit code **and** the evidence line as separate, validated inputs —
`expect` requires a numeric code *and* a matched output line — and a scenario that produces neither is a
failure, not a pass. A test that cannot fail is decoration, and the tool written to check assertions is
exactly where that is most expensive. Corollary for grep-style checks: match the **field under test**, not
the whole record, or a path containing a forbidden token blesses every mention in that file.


## 18. A changefile edit is verified by evaluating what CONSUMES it, not by checking a name exists

I broke `devenv print-dev-env` for every lane for about twenty minutes. The edit was one line in
`devenv/packages/openvb.nix`: a bare string `"xvfb-run"` inside `packages = (with pkgs; [ ... ])`. Inside
that list a quoted string is a **string**, not an attribute lookup, and devenv's `packages` option is
typed `package` — so the whole environment failed to evaluate ("definition ... is not of type `package`").
Implementer-B found it and named the cruel part: **the tool I shipped to detect stale payloads depends on
the very reload my edit had made impossible.** `devenv-reload` could not run, so `run-all-tests` stayed
stale, and the false-red guard I had just written was unreachable.

What I had "verified" beforehand was `builtins.hasAttr "xvfb-run" pkgs` → true. That is a fact about the
attribute set, not about my edit. The check that would have caught it costs ~10 seconds:

    devenv print-dev-env > /tmp/env.sh 2>/tmp/env.err; echo $?   # never 2>&1

Rules that follow:
- **Evaluate the consumer.** For a package list, the consumer is the shell evaluation, not a name lookup.
  Type errors in a changefile are global, immediate, and silent to a name-existence check.
- **Land the integrity check before the change it guards**, or a broken changefile hides behind the reload
  it prevents (B's phrasing, added to item 14's spirit).
- One fix per commit, and quote the gate **after** the fix: the run below is the first green of the day that
  carries a stamp, a verified-current table, and per-suite assertion floors.

## 19. Quote a gate with its stamp or not at all

The stamp block (`GATE STAMP` = UTC time, HEAD, openje SHA, dirty totals, payload path, table rows + hash,
compiler, exe mtime/size) exists so a number cannot be separated from the artifact that produced it.
Preceding it with `tools/gate-integrity.sh` (exit 0) closes the loop from outside, because a script cannot
report on its own staleness. A gate run that prints no stamp is not the tree's gate, even when it says 9/9 —
or, as it turned out, when it says CombatMath is red.


## 20. pi-messenger counter semantics (measured from source, not assumed)

`handlers.ts` + `crew/utils/config.ts` say, exactly:

- `budget = crewConfig.messageBudgets[crewConfig.coordination] ?? 10` — the level, not a global cap.
  `.pi/messenger/crew/config.json` sets `coordination: "chatty"`, so its `chatty` number is the live budget.
  It was 40 today and I spent all 40 across one session, then wrongly suspected the raise had never applied.
- **The counter is per-session and does not reset at compaction.** The session id survives, so a long day
  burns the budget in one continuous line. Raised to 150 (2026-10-20) after that cliff silently blocked
  mid-dispatch — a coordinator whose messages stop is a coordinator who is quietly not coordinating.
- **Failed sends cost nothing** (`return` happens before `messagesSentThisSession++`), so routing errors do
  not eat budget — but they do eat my attention, and I reported them as budget loss until I read the code.
- **A multi-recipient send costs ONE unit** — the increment is per call, not per recipient.
- **`to: ["A","B"]` does not work despite the schema** (`Type.Any`, description claims arrays are accepted):
  the array arrives marshalled as a single string, so validation fails with the whole bracketed blob as one
  name — `Failed to send: ["UltraBear", "YoungStorm"] (invalid name)`. Use **`broadcast: true`**, which
  enumerates active agents inside the handler and is the supported all-lane path at one unit.

**And `broadcast: true` is unavailable from the tool surface.** `handlers.ts` implements it, but the MCP tool
schema pi exposes has no `broadcast` property, so the call returns `specify 'to' or 'broadcast: true'`. Combined
with the array marshalling above, **there is no multi-recipient send reachable from this harness** — every all-lane
notice is N individual sends. That is why the budget cliff mattered: with 30 lanes' worth of coordination the old
40-unit cap was spent on routing, not content. Both defects are in the *vendored* extension under `.pi/`
(gitignored), so they do not travel with the repo and will reappear on any reinstall or another machine — recorded
here rather than filed as a task, because the fix belongs upstream.

The general rule, since this is the third tooling assumption I have had to correct by reading source today:
when a limit or a failure mode matters, read the implementation instead of the description — the description
in this case explicitly promised something the marshalling does not deliver.


## 21. The pi-messenger send cap is removed (and how to re-remove it)

Decision 2026-10-20: **the per-session send budget is effectively unlimited.** It was throttling *routing*,
not content — the cliff appeared mid-dispatch and left lanes unassigned while I sat silent, which is worse for the
project than any message spam.

Verified, not assumed: after the change, a send rendered `999957 messages remaining`, and the effective value was
computed by replicating `loadCrewConfig`'s merge (`defaults <- user <- project`) rather than by reading one file
and hoping.

Two layers, and **both are outside git**, so a fresh machine or a reinstated `.pi/` silently restores the cap of 10:

- project layer (wins): `.pi/messenger/crew/config.json` — `.pi/` is gitignored
- machine layer: `~/.pi/agent/pi-messenger.json`, under a `"crew"` key

Reapply by putting this in either file (JSON has no `Infinity`, so use a sentinel):

```json
{ "coordination": "chatty",
  "messageBudgets": { "none": 0, "minimal": 2, "moderate": 5, "chatty": 1000000 } }
```

`handleSend` calls `loadCrewConfig` on **every** send, so edits apply immediately — no restart, unlike a `devenv`
script payload (item 18). Keep `none/minimal/moderate` as they are: those are deliberate terseness levels for crew
workers, and removing the cap for the coordinator shouldn't turn a minimal worker chatty.

What the cap *was* protecting is context, not attention — my dispatch messages are long. That is managed by
`/compact` and by the standing rule that lane messages carry payload, not narration. If messages start costing
context again, the answer is shorter messages, and this file is the place to re-tighten it deliberately.


## 22. A uniform store in the original is a claim about *its* layout, not a licence for ours

Implementer-B copied the shipped owner back-reference literally: one blind write at `+0x14` through a header
struct, reasoning that a type-free write in the original can be a type-free write in the port. It corrupted
`JE::ActionMoveTo` — **two of another lane's tests went red** (SpawnMove step-aside and door) — and still did not
link `GameActionAttack`, because our action models are not uniformly shipped-layout:

- `actor` sits at **port offset 0**; the `+0x14` in its comment is the *shipped* offset,
- `mover` is the shipped `+0x11` and is set by the registration path, not by the queue,
- pump-side `Wait` / `Die` / `OperateDoor` have **no owner field at all**.

The general form: **offset-carrying comments describe the original, and a port's offsets drift per type.** Before
trusting a copied store, (1) check the field exists in *each* struct the store touches, and (2) **compile and run
the suites you are not editing** — the two red tests were nobody else's, and only the full gate could see them. A
row that lands green on its own suite while breaking a neighbour's is not green.

Same batch, same habit worth keeping: B wrote the shipped refresh latch (`0x54ef30`) as **cited-and-absent**
rather than inventing a flag to satisfy it, and registered `actions[4]` as the shipped deque's *first block* (slot
= count & 3, block = count >> 2, unbounded growth in the original) as deviation **D-2** instead of quietly
promoting our fixed array to "the list". Refusing a fifth insert with a one-time log line is honest; pretending the
array is a deque is not.


## 23. Auto-compact threshold is a per-model token reserve, not a percentage

Want "compact at 80%"? Pi has no percentage setting. `shouldCompact()` in `dist/core/compaction/compaction.js` is:

    contextTokens > contextWindow - settings.reserveTokens

So a fraction has to be **inverted into a reserve per model**: `reserveTokens = (1 - fraction) * contextWindow`.
Resolution order (`settings-manager.js:609-623`) is `compaction.modelOverrides["<provider>/<modelId>"].reserveTokens`
→ `compaction.reserveTokens` → built-in default 16384, and the model key is exact and case-sensitive
(provider/id, line 614). Two traps I walked into while writing this:

- **The key is `modelOverrides`, not `models`.** My first attempt used `models` and would have been silently
  ignored — the reader only touches named fields, so a wrong key never errors. Verified by reading the resolver
  instead of the docs table (item 18, again).
- **A single top-level value cannot serve a fleet with different windows.** This machine has 116 known models
  spanning 32k to 1M context. The live session model (`opencode-go/qwen3.8-flash`) is a **1,000,000**-token window
  with `maxTokens: 131072`, so the fallback reserve sized for a 262,144 window made it trigger at **94.8%**, not
  80%. Any percentage request must therefore be generated from the model store, not typed once.

Regenerate after adding a model (repo `.pi/settings.json`, which is **gitignored** — per-machine):

```python
import json, pathlib
A = pathlib.Path.home()/".pi"/"agent"
found = {}
def walk(o):
    if isinstance(o, dict):
        if o.get("id") and o.get("provider") and isinstance(o.get("contextWindow"), int):
            found[f"{o['provider']}/{o['id']}"] = o["contextWindow"]
        for v in o.values(): walk(v)
    elif isinstance(o, list):
        for v in o: walk(v)
for f in ("models.json", "models-store.json"):
    p = A/f
    if p.exists(): walk(json.loads(p.read_text()))
FRACTION = 0.80
overrides = {k: {"reserveTokens": round(w*(1-FRACTION)),
                 "keepRecentTokens": min(65536, max(4096, round(w*0.125)))} for k, w in found.items()}
```

Set the top-level `reserveTokens` to `(1-FRACTION) * <most common window>` so unlisted models still land near the
target rather than at the built-in 16384 (93.75% of 262k, 98.4% of 1M). Keep `reserveTokens >= maxTokens` for the
model you run — reserving less than the model's own maximum output can trigger compaction with no room to answer.
`keepRecentTokens` is what survives the summarization; raising it preserves more detail per compaction at the cost
of a smaller next-turn headroom, so it should stay well under the reserve.

**Lanes pick this up automatically:** every session in this repo reads the project `.pi/settings.json`. Existing
sessions apply it on their next compaction check; a new session starts with it.


## 24. `ls` of a payload is not evidence of absence — search inside the archives

RedXenon closed T-016's map-name question **with zero debugger time**, and the reason it had been scoped as a runtime
read is an artifact of my own method: the premise "no INI in the payload" came from **listing the demo tree**. The
shipped `globals.ini` is plaintext **inside** `data/Engine.grp`:

```
$ strings data/Engine.grp | grep -iE 'Mainmenu|map name|Year'
[Mainmenu]
map name = mainmenu.map
Seconds Per Round = 6
Year = 2253
```

Amendment to item 13 ("data before debugger"): **data includes the contents of containers.** Before concluding that a
value must be observed at runtime, grep the archives — `.grp`, `.pak`, `.veg`, `.int`, SQLite files, and any
concatenated resource blob. A directory listing proves only where a file *appears*, not what the shipped binary reads.
The same trap in reverse produced this afternoon's clock story: see T-038, where `Year = 2253` in that same block means
the shipped clock is **date-seeded**, which nobody had checked because we were looking for a `.ini` file.

Corollary for rows I write: when I scope a read, I must state **what statics were searched and where** — if the answer
is "the tree was listed", that is not a static search.


## 25. Two self-inflicted failures, written down because I made them and told others not to

**(a) A readiness probe must identify the owner, and I nearly replaced a false green with a false red.**
`ready.exec = nc -z 127.0.0.1 <port>` passes against *any* listener. When ImHex and the original's gdb stub shared
31337, devenv reported the stub ready while `winedbg` had lost the bind and exited — the entire "cannot attach to the
original" investigation was manufactured by that green. Replacing it with `ss -H -ltnp "sport = :N" | grep -q winedbg`
looked correct and was not: **winedbg hands its gdb socket to wineserver**, so `ss` reports the listener as
`wineserver`, and my matcher would have declared a healthy stub NOT READY — a false red blocking `devenv up` for
everyone. Fixed to `wineserver|winedbg` and falsified in both directions against real processes (healthy stub rc=0;
the actual ImHex squatter rc=1). Residual limit recorded in the file: a killed stub leaves `wineserver` holding the
port and the probe still says ready, so **it rules out squatters, not liveness**.

Two general rules: probe for *identity*, not connectivity; and **never ship a checker whose positive case has only
been reasoned about** — my positive control was only obtained by starting a real `winedbg` against a real prefix.

**(b) Never leave a staged index behind a failed commit.** My commit failed on `argv` size (`Argument list too long`,
message passed with `-m` after two long heredocs had already bloated the environment) — but `git add` had *already
run*, so my three files sat in the index. The next commit in the tree belonged to another lane, and it absorbed them:
the readiness probe, `xdotool`/`wmctrl`, and the T-034 rewrite are now in **`c925bf6`**, whose message is about a
different subject entirely. The content is correct and in the tree, but attribution and rationale landed under the
wrong heading.

Rules: stage and commit in **one** step (`git commit -o -- <paths> -F msgfile` or add+commit inside a single shell
invocation that cannot half-fail); put long messages in a file with `-F`, not `-m`; and if a commit errors,
**check `git status` and `git diff --cached` before walking away** — a dirty index is a shared resource in a
six-lane tree.


## 26. A corrected verdict with a stale index row still spreads the wrong claim

RedXenon's batch-2 self-correction (`c10a3eb`) fixed the decision table's verdict cell, and the **README index line kept
selling the falsified claim** (`00b09b7` found it by grepping its own corpus). The index is how most readers arrive, so
the stale row **outranked** the correction: someone skimming `discoveries/README.md` would have carried the wrong
headline away from a document that says the right thing.

Why this happens structurally: a summary line is written at the moment a result is *surprising*, and the correction
happens at the moment the result is *examined*. Those are different times, in different files, and the earlier one is
usually the one people read. Any doc with a generated or hand-maintained index has this exposure.

Rules:
- **Correcting a claim means grepping every place you made it**, not just the place it is now known to be wrong:
  `grep -rn "<claim phrase>" discoveries/ tasks/ docs/` is the closing step of a retraction, not an optional extra.
- Commit messages for corrections should say what they are: RedXenon's was *"no new claims, only the old ones made
  true."* That framing is more honest than a diffstat suggests, because a correction commit looks like progress.
- Same exposure on my side: `docs/PROGRESS.md` and `tasks/README.md` both restate lane findings in summary form. When a
  finding is re-anchored (route-dependence, withdrawn numbers), the summaries are part of the change set — I propagated
  the "byte-identical clones" phrase through exactly this mechanism (`cfa83a8` -> corrected in `d89f9ad`).


## 27. A writer search scoped to the owning struct can only find readers

I told a lane to decide whether `Entity+0x314` was a count or a flag by reading its writers at
`0x54ab30`/`0x54ab80`. **Both sites are gate predicates that merely test the field**, and they were the only
usages listed — because the field is `mbr_0x10` of a container embedded at `Entity+0x304`, so the database attributes
the stores to the **container type**, not to `Entity`. The real writer (`++mbr_0x10` at `0x54ea71`, the push_back
performed by the order funnel itself) appeared the moment the container was followed.

Method consequences:
- **`usages` on an owner struct is not a writer list.** For embedded substructures it is readers plus the owner's own
  initialisation. Follow the container's methods before concluding a field has no writers — and start from the
  **unpack/store site** and walk outward when a field is persisted.
- **Layout names the field for you.** `0x304/0x308/0x30C/0x310/0x314` — five consecutive size-4 members initialised
  contiguously in the ctor (`0x5515d4…0x5515e6`) — is a container; and reading the loop bound as
  `head != head + count` identifies the last two fields with no guessing. Check structure before trusting a comment.
- **My steer was wrong and is recorded as mine.** The finding came from the lane; the dead end was mine. A dispatched
  hint is a hypothesis, and lanes should follow evidence past it — which is what happened here.


## 28. Deviation numbers are a namespace — check it before you mint into it

I created a collision by writing **D-3** for the clock-seed finding in `docs/DEFERRED-NOTES.md` while another lane had
already minted **D-3** for T-037's ref-key gap. WildYak resolved it (`b0a8a50`) by renumbering **their own** entry out
rather than mine — and their reasoning is the rule: a D-number marks a deviation we *accept* differing, and the clock
finding has no accepted difference at all (both sides seed the date, both leave `totalSeconds` at 0). The one real
port-side difference is the literal fallback, which is a **bug** and already had a row (T-042). So the clock finding is
now an un-numbered finding.

**"A number I mint after someone else has minted the same one is a defect even when the prose under it is correct."**
Identifiers are load-bearing precisely because readers follow them and do not re-read the text. The second-order
damage was a **stale pointer**: `src/F3/CombatMath.hpp:35` cited the RNG seam as *"item D-3"*, written back when the
ledger was two rows long — harmless then, actively misleading once D-3 meant something else. Sweep after any renumber:

    grep -rn "item D-\|deviation D-" src modules

which found exactly that one hit (everything else already cited D-2 correctly). General form: **when a numbered
registry changes, the change is not done until every citation of the old numbers has been re-pointed** — same principle
as item 26, one level down.


## 29. A negative test is only a measurement if you can show the perturbation arrived

SwiftRaven nearly shipped a falsification suite that was inert twice over, and found both only by going looking for
why the checker seemed weak (T-040, `c204c51`):

- a `sed` pattern that **never matched** the fixture, so the "drifted" file was byte-identical to the original and the
  checker correctly printed EXACTLY — a red that could not turn red;
- `rc=$?` captured **after `if ! cmd`**, so it read the *negation's* status and printed the astonishing
  `FAILED (exit 0)`.

Both are this project's signature failure — **a check that reports without measuring** — and both were caught by the same
reflex: don't ask "did the test fail?", ask **"did my perturbation reach the thing under test?"**

Rules, now the standing form for any checker shipped from this repo:
1. **Prove the edit landed** before believing the result: `diff` the mutated fixture against the original and show it
   differs (or show the byte that changed).
2. **Never read `$?` through a negation or a pipe** (see item 17). Capture the command's own status, then branch on it.
3. **Every failure mode prints a reason** — `f3-json-check` emits an explicit `DRIFT REASON:` line, so a red can be read
   rather than re-derived.
4. **A silent fallback is a lie with extra steps.**

**The sharpest instance is a guard that did not enforce its own headline claim** (`1a8d101`, same afternoon, found by
the author after a coordinator asked "what happens when this goes red?"). `f3-json-check` *printed* "the export is
byte-frozen at md5 e8827f12…" while checking **neither** field the sentence rests on: setting `version` to `9.9.9` gave
`keys_exact True`, no drift, **exit 0** — a clean verdict on a tampered artifact. Now the export's `version` must equal
the schema constant and its `filemd5` must equal `md5($OPENVB_ORIG_EXE)`, both folded into the drift verdict as
`DRIFT REASON: PROVENANCE: …`. Coordinator verified: clean → exit 0; version tampered → exit 1 with the provenance
reason; and the embedded `filemd5` `A68C22F3DD…` really is the shipped `F3.exe`'s md5. **Lesson: if a tool states a
claim in its output, the claim is part of its contract — assert it, or stop printing it.** (Their first wrong-binary
test was also invalid: pointing the env var at a nonexistent file made the branch silently no-op and it looked like a
pass — item 29 catching them once more, immediately.)

**Cross-reference, so item 29 is not read as a single-tool story.** The same lesson in the *indexing* half is T-032
(`tools/ea-index.py`, follow-up `2fca7d7`): the lane's own refusal **counters** disagreed with its reported total
(756 vs my 759), and I found it by summing categories the tool already printed. So the rule generalises past mutation
tests: **a checker's arithmetic about itself must be printed by the checker, not reconstructed by the reader.** Any tool
with refusal/skip/fallback counts should emit its own total, because a reader who adds up categories and gets a different
number has learned both that the tool disagrees with itself and — worse — may not notice. My own first measurement of
that disagreement was wrong in the same way (reading `$?` after a pipeline), which is item 17 and item 29 arriving at the
same place from two directions.
 A bad explicit `--results` path used to quietly pick up another copy
   elsewhere on the machine and verify an export against unrelated facts while reporting clean; it exits 2 now.


## 30. A gate number is meaningless without its payload hash — the same command legitimately prints 0 and 2

Measured on 2026-10-21 in the **same repo, same minute, two fresh subshells**:

| invocation | exit | payload resolved |
|---|---|---|
| plain `bash -c tools/gate-integrity.sh` | **2** | pre-T-024 derivation, no stamp |
| `bash -c 'source /tmp/openvb-dev-env.sh; tools/gate-integrity.sh'` | **0** | current, stamped |

Even the launcher paths differed, so this is not one tool being flaky: **a harness subshell keeps the PATH it was
handed at session start**, and every tool call is a new subshell, so the dev-env capture has to be re-sourced per call.

**The correction that matters, because the first theory was mine and wrong.** I predicted two captures taken minutes
apart would resolve to *different* payloads and prove that captures go stale. They did not — both resolved to the same
current derivation. **The variable is sourced-vs-not, not capture age.** So an `exit 2` is not evidence a reload failed
or a capture rotted; it is evidence the call never went through the dev-env at all. `gate-integrity.sh` now *names that
cause* in its own stale branch and prints the re-run recipe, because "a check that reports a state without naming the
cause makes the reader invent one" is the same failure as a guard printing a claim it did not verify (item 29).

**A cause line must be phrased as something to *rule out*, never as a diagnosis.** SwiftRaven's refinement, and correct:
`exit 2` has at least three causes — unsourced PATH, a genuinely stale payload, a gate predating the stamp. Asserting one
in the output re-creates item 29's failure (printing a claim the tool has not verified); naming the **cheapest check that
collapses the possibility space** is what a checker is allowed to say. Hence the shipped wording
*"cause to rule out first"* rather than *"cause"*.

**The third shape of the same bug, also theirs:** they nearly filed a defect against my working fix by grepping for the
literal string *"table clean"*, which the tool never printed — a false finding about a missing feature, caught by reading
the output instead of trusting the grep. Invent a string to search for and you will eventually "discover" that it is
absent. Same reflex that caught the `tail` exit code one message earlier.

Standing rule, extending item 19: **quote a gate number with its payload hash, and when two numbers disagree, check
which environment produced each before deciding which is wrong.** Both may be correct. This is the fourth distinct
"the artifact was fine, the environment wasn't" incident of the day — stale installed gate payload, `devenv-reload`
missing from PATH, ImHex squatting the debugger port, and now PATH per-subshell.

## 31. An operand claim read from a comment is a hypothesis, not a measurement

Same lane, second half of the day. From T-045 I asserted that a frame-0 clock fold was "permanent, and it reaches
`turnDeadline0x29C`" — and the sentence got banked by another lane (`7865068`) and fenced twice (`b0f94ef`, then
corrected at `df8f53c`) before I checked. It does not reach the deadline: `accMs_` has **one** writer
(`CombatManager.cpp:188`), it is fed by the configured `secondsPerRound` per dispatched queue event, and
`GameWorld::CombatTick` discards the clock with `(void)ms`. The clock-fed line I was describing was the **C-23
placeholder, retired by T-011** — and the retirement is spelled out *twelve lines below* the comment I had quoted
(`CombatManager.hpp:114-116` vs the still-advertising `:38`).

Both of my wrong versions came from trusting prose about a data flow instead of the assignment site. The fix is one
command and it costs less than the retraction it prevents:

    grep -rn "accMs_\|<any accumulator name>" src      # writers only — then read the RHS

Standing rule: **a comment that names an operand states what the author believed; only the writer states what the code
does.** Read the writer before generalising, and when a claim has already propagated, correct it in the same message
that learns better — the correction chain (`7865068` → `b0f94ef` → `470ec60` → `df8f53c`) is fine as history and bad as
an accident. Second, cheaper rule from the same trace: when a retired placeholder's name survives in a comment, retire
the comment too, or the next reader quotes it as live code — including me, twice, today.

## 32. A `nohup ... & disown; tail -f /dev/null` process kills the supervisor's ability to restart it

Two facts, both measured during the 2026-10-21 04:36Z MSVC-gate outage:

1. `processes.docker-rootless` starts dockerd with `nohup dockerd-rootless & disown` and then
   `tail -f /dev/null`. When dockerd dies, the **leader bash never exits**, so the devenv session
   guardian (which watches the leader) sees a healthy process and does nothing. `kill -TERM
   <dockerd-pid>` is therefore a one-way action: the daemon is gone and no supervisor brings it back.
2. The correct control path is the native manager that is already running:
   `devenv processes restart docker-rootless` (also `list`, `status`, `logs`, `wait`; the socket is
   `/run/user/1000/devenv-<hash>/processes/native.sock`). Verified end to end: after the restart,
   `docker info` returned 29.8.0, `docker pull ghcr.io/openje/msvc2002:latest` returned digest
   `368eeb63...` with rc 0, and `status` printed `Phase: ready`.

**Why it matters:** the outage looked like "blocked on a human terminal" and a lane correctly
refused to hand-launch a daemon. It was not human-only work; it was a missing command in my own
vocabulary. Standing rule for every lane: before escalating a devenv-managed process as
human-only, run `devenv processes status <name>` and `devenv processes restart <name>`.

**Related, still open (not fixed here — `devenv/**` is not a lane file):** `docker-rootless`
`ready.exec = docker info` passes against ANY daemon, so a generation whose dockerd died reports
PROVISIONED rather than red. Same class as the 31337 squatter false-green (T-034): a readiness
probe that only checks "something answers" measures the port, not the owner.

## 33. An unconditional `docker pull` makes the canonical gate depend on the network it does not need

`devenv/scripts/msvc-build.nix:79` runs `docker pull ${msvcImage}` before any compile. During the
04:36Z outage the daemon's sandbox resolver was wedged, the pull returned "lookup ghcr.io on
[::1]:53: connection refused", and the MSVC gate died before it reached a single source file —
while `ghcr.io/openje/msvc2002:latest` was already present locally.

The gate's contract is "compile with the pinned MSVC 2002 toolchain"; the image's *provenance* is
a separate question, and conflating them means a DNS outage is reported as a build failure. Not
edited (harness file, needs its own row): the fix is pull-if-absent or pull-with-`|| warn` and
state which was done, so the output distinguishes "toolchain refreshed" from "toolchain used as
cached". Recorded so a lane seeing `BUILD-FAIL` with a resolver error checks the daemon before
blaming the code.


## 34. `devenv processes restart` restarts the process, not the LOADED CONFIG

Found the hard way during T-046 verification (UltraMoon 01a15cb7, live-tested): after
editing `processes.<name>.exec`/`ready.exec`, `devenv processes restart <name>` reuses the config the
running session already loaded — verified by comparing the leader's store path before/after:
unchanged, still the pre-edit derivation. A restarted process therefore runs the OLD script and
reports `ready` truthfully, which looks exactly like "my fix didn't work" or, worse, like "there
was nothing to fix". The new process definition lands only at a FULL session re-up (a human action
while `devenv up` owns the session).

Rules that came out of it:
1. **Verify a process-config change by rendering the committed body** (evaluate the nix, run the
   rendered `exec`/`ready` in a live shell, kill the real daemon, watch it exit nonzero) — that is
   a proof of the LOGIC. State it as such; do not claim the running generation adopted it.
2. **A green `ready` after `restart` is not evidence your edit is live.** Check the loaded config
   (leader path / derivation hash) or say the activation boundary in every report.
3. When a tool from the shell is stale-but-present, re-source in the SAME shell as the call —
   sourcing in a different shell (or judging through a pipe) keeps the old PATH and the old baked
   payload, and the tool you invoke is the pre-edit copy. T-046 caught its own first fallback run
   this way (missing bracket proved it ran the pre-digest payload) and re-ran correctly.

Item 30's shape again: the measurement was real, the thing measured was the wrong artifact — this
time not the payload but the *process config generation*.


## 35. A scan that returns EMPTY for a pattern that provably exists is a tool failure until proven otherwise

Pass-4 of T-017 (PureKnight, `discoveries/2025-10-21-t017-pass4-blk-allocation-handlers.md`) re-enacted
the T-041/T-043 lesson a third time, this time on the coordinator-visible record:
- pass-3's disp32-band scan and its window were **both right and both incomplete** — the one store that
  matters (`0x58f1f5`, blk install into `client+0xA0`) sits OUTSIDE the scanned band, so pass-3's
  "no install store exists" was a bounded-search result reported as a census.
- The session's own full-`.text` `89/C7` rescan **returned empty** — against an EA whose bytes pass-3 had
  already verified individually. A whole-code-section scan cannot legitimately return zero for a known
  pattern. The lane noticed, quoted NOTHING from it, and rebuilt every store claim from per-address
  `objdump` verification. That is the correct handling and it is why this result cost one pass instead of
  poisoning a doc.

Standing rules:
1. **Empty + known-present = the scan is broken.** Self-test every scanner against at least one
   pattern whose location is independently known BEFORE any "none exists" conclusion. A refusal that
   fires is evidence; a silence that could be either is not (T-029's fixture principle, applied to scans).
2. **A bounded scan reports its bound.** "No store in band X" is a census only with the band printed;
   otherwise it silently reads as "no store".
3. Same family as items 27/31: `f3_member_index` reader-bias, comment-vs-writer, and now scan-window
   truncation — three ways the *evidence channel* lies while the bytes stay innocent. The fix is one
   each: usage closure, read the writer, self-test with a known-positive.
4. **The failure is often in the CALL SHAPE, not the payload.** The T-041 MCP byte-pattern misses, the
   T-017 empty rescan, and the batch-3 `set_comments` arg-shape rejection were ALL harness-level
   serialization faults, not data problems. Diagnostic ladder before concluding anything about the
   TARGET: (a) same shape through a different tool (`rename` worked where `set_comments` refused —
   coercion exists per-tool); (b) known-positive through the same tool; (c) only then interpret the
   negative. And a refused/empty result never enters a doc as a finding — it enters the ledger as a
   blocked write with the text preserved (T-033 b3: 13 comments verbatim in §APPLY, zero silent loss).


## 36. The last gate on a mechanism can be its UNREACHABILITY — and that is worth a full instrumented hunt

T-010 10e (PureKnight, `discoveries/2025-10-21-t010-10e-manager-producers.md`): both residual combat
manager fields (`+0x4` mode, `+0x14` rearm latch) are **provably constant-zero on every reachable path**
in the shipped image — `meth_0x5a9330` (the only +4 writer) is called exactly once in the whole `.text`
(`0x5a957f`), gated by `[this+14]!=0`, and `+14` has no nonzero producer anywhere (byte-store,
dword-store, `lea [reg+d32]` — three encoding families, 515 sites). The round-walk branch of T-011's
`meth_0x5a94a0` is therefore DEAD CODE in the shipped demo; the whole rearm sweep (join/leave helpers
over the entity list) is equally dead, xref-verified.

Rules banked from the pass:
1. **Negatives require self-tests.** Every scan that concluded "none" had to first recover its known
   positive (call site 0x5a957f; store 0x5a9371) — the item-35 method applied to *all* encodings, not
   just the first. Two of the session's own bounded scans failed empty on known positives and the lane
   DISCLOSED them, drawing conclusions only from forms that passed.
2. **Constant ⇒ no arming row.** When a field is proven constant-zero, the port mirrors the constant
   and the *branch* is filed cited-and-absent — do NOT invent an arming path. T-047 ("the driver could
   arm the walk") is retired by this finding: no shipped door arms it either.
3. **Dead-code discovery is a full-value deliverable.** "The mechanism exists and nothing runs it" is
   the answer a gated row was waiting for; it retires the question instead of forwarding it. (Item 31's
   family: the writer hunt is the instrument; here it produced the strongest possible negative.)

## 37. mingw accepting a construct is not evidence the canonical compiler will — header touches REQUIRE the MSVC leg

T-047 (`attack-pos` verb): the port's ctor used a C++11 braced init in the member-init list
(`overridePos{ 0.0f, 0.0f, 0.0f }`). The mingw cross leg of `sim-driver-build.sh` compiled it silently;
`msvc-build` (MSVC 2002, the canonical toolchain) failed it at `GameActionAttack.cpp(68)` C2447. The fix
is a ctor-body assignment — but the durable fact is the ORDER: any lane that validates a `src/F3` header
edit only against mingw has NOT built the product. One line, per ZenNova's request after commit 8e272e6:
**edit a src/F3 header or a cross-TU table ⇒ msvc-build before quoting any green.** Same family as item
35: a tool that accepts too much is an evidence channel lying quietly — the negative proof (MSVC rc0)
belongs in the report, not the positive one (mingw ok).

## 39. An "unmovable port" is only unmovable if you MEASURED it — my first fix moved the knob, not the socket

(item 38 is reserved for T-034's kill-ownership rule; this is the other half of the same incident.)

T-034's port-collision fix (fd6a578, 2025-10-20) said: ImHex is the non-game consumer, so ImHex moves.
It set `OPENVB_IMHEX_MCP_PORT = "31347"` and pointed both the wait loop and the MCP client at the
variable. Single source of truth, clean diff, **and completely fictional**: 24 h later `devenv processes
list` showed `imhex restart_count 5` in a start/kill loop and `ida-orig-gdb` still exited.

Measured today (ImHex 1.38.1 + the dfdgsdfg/imhexMCP fork built by `devenv/packages/openvb.nix`):

- `ss -ltnp` shows the ImHex pid on **127.0.0.1:31337** while `~/.config/imhex/settings.json` said
  `{"network":{"enabled":true,"port":31347}}` — the file's `port` key is not read. Reproduced twice.
- The only network knob in the binary is a **bool**: `hex.builtin.setting.general.network_interface`
  (strings of `builtin.hexplug`). No IMHEX_\* port env, no CLI flag (only `IMHEX_SKIP_SPLASH_SCREEN`,
  `IMHEX_MEH`, `IMHEX_FORCE_OOBE`). So 31337 is ImHex's by build, not by agreement.
- `imhex-mcp-server` ignores `IMHEX_HOST`/`IMHEX_PORT` **entirely**: `server.py` builds its config from
  argparse (`--host` default `"localhost"`, `--port` default 31337). With ImHex bound IPv4-only,
  `localhost` resolves `::1` first → "Connection refused" → the server exits during initialize →
  pi reports `imhex: failed: MCP connection closed`. Proof both ways: no-arg run refused while 31337 was
  OPEN and answering; `--host 127.0.0.1 --port 31337` returned 29 tools and live `imhex_get_capabilities`.
- The wait loop then converts fiction into a **crash loop**: it polled 31347 for 120 s, `exit 1`, and its
  own `trap cleanup EXIT` killed the healthy ImHex. process-compose restarted it. `ready.exec = true`
  called every stage of that "ready". Net effect: ImHex was killed and reborn every ~2 min and the
  original's stub still never got 31337.

Rules:

1. **Before moving a port to resolve a collision, prove the mover can move.** A config value I control
   is not the same thing as the socket a third-party binary opens. One `ss -ltnp` + one
   `strings | grep -i <setting>` is the whole test, and it costs less than the incident it prevents.
2. Therefore: **the movable side moves.** `winedbg --gdb --port "$OPENVB_ORIG_GDB_PORT"` is fully
   parameterized and every consumer already reads the env var, so the original's stub is now **31437**
   (the port T-034/T-052 already attached with, 5/5 cycles) and `OPENVB_IMHEX_MCP_PORT` went back to the
   honest 31337. The earlier "docs and muscle memory quote 31337" argument bought a fictional contract.
3. **A `ready.exec` of `coreutils true` is a bug report waiting, not a probe.** Every managed process that
   owns a socket gets an *identity* probe (here: `ss -H -ltnp sport = :N | grep -Eq "ImHex|imhex"`),
   matching item 25(a). Grep the OWNER, never just the port; and keep the `wineserver|winedbg` lesson in
   mind — the owner name is whatever `ss` actually prints, verified against a live process, both directions.
4. **A readiness wait that kills its own subject on timeout is a loop generator.** Fail loudly with the
   port owner dumped (`ss -ltnp "sport = :N"`) — which is what this file's loop lacked for a day.
5. **Env-var MCP config is a lie for servers that only read argv.** For a stdio MCP server, verify which
   channel it actually reads (`grep -n "add_argument\|getenv" server.py`) before writing `env:` into
   `.pi/mcp.json`; here the fix is `--host 127.0.0.1 --port <port>` in `args`, and `localhost` is the
   wrong default whenever the peer binds IPv4-only.

## 40. A harness argument-corruption failure is NOT the target refusing you — prove the server, then route around

GoldYak's batch-4 wave (T-033), 2026-10-21. The pi harness's `set_comments` tool delivered `items` as a
STRING where the tool schema demands array/object, rejecting every payload shape (b3 lost 13 comments this
way and correctly banked them in the ledger instead of dropping them). The SAME server, on the same port,
in the same serialized single-writer session, accepted the IDENTICAL payload over direct JSON-RPC
(`POST /mcp tools/call`) — 13/13 issued verbatim, `get_cmt` readback SET at all 13 addresses. `rename`
never showed the bug: the coercion failure is tool-specific, not transport-wide.

Rules:

1. **Localize the failure before honoring it.** When a tool call fails validation, the question is *which
   layer broke*: harness (schema/serialization), transport, or target. One direct-JSON-RPC probe with the
   same payload answers it cheaply and changes the verdict from "path blocked, bank for later" to "banked
   AND re-issued this wave".
2. **Preserve-then-reissue beats re-derive-later.** b3's decision to keep the full comment text in the
   ledger (refused writes never dropped) is exactly what made the reissue mechanical. Any rejected write
   batch goes to the ledger verbatim, not the trash.
3. File the harness bug **pi-side** (here: `set_comments` items coercion — `rename` accepts the shape the
   tool rejects); do not let a harness limitation masquerade as a target constraint in a task row.

## 41. A "no owner" negative is only valid if it answers THREE independent queries — one blind spot does not make a class

EpicRaven's T-054 wave, 2026-10-21. b5 recorded cls_0x445350 as owner-unresolved because "zero
external direct call sites AND zero vft slots pointing into the band". The wave's verdict was WRONG,
and wrong for three SEPARATE reasons — containment ran through callers-of-callers (the band's host is
called by the host's OWN method 0x4465E0, invisible to a direct-site scan), through table->thunk->band
resolution (vft 0x659DB0 idx2 = adjusted thunk 0x4468B0 `sub -0x4(%ecx),%ecx; jmp host` — the table
points at the host's thunk, never into the band), and the vft scan window was .rdata-only while the
install edges live in .data/.data1. Each mechanism defeats a different scan; passing one does not
excuse skipping the others.

Rules for any future "no owner / owner-unresolved" note:

1. State ALL THREE answers explicitly: (a) callers-of-callers, (b) thunk-adjusted table resolution
   (sub/add-ecx + jmp shapes, delta from [view-4]), (c) full .rdata + .data + .data1 coverage.
   A negative that lists one is a note about a scan, not about the class.
2. Prefer the tool: `python3 tools/ecx-flow.py band --cls X` runs all four joins against the
   md5-pinned cache; a negative that skipped an existing tool must say why.
3. The corrected predicate for vft-owner claims (CalmRaven's T-055 finding): ctor = sole OBJECT-SIDE
   storer; a factory slot-install may hold the only other xref — that is EXPECTED, not a rival;
   rival-class ctors = zero. Naive one-xref-equality fails 14/14 on legitimate classes.

## 42. Verify RTTI SIZES before naming a delegate-host band — the embedded VIEW's name is not the owner's

CalmRaven's T-055 wave (naming-deviation ruling), 2026-10-21. cls_0x445350's owner chain was
adjudicated correctly by EpicRaven (delegating host + adjusted thunk + constant -0x198) and proposed
under the interface name; the writer applying it found `.?AVCOptionsHotKeysInterface@@` RTTI is
**size 0** — i.e., the RTTI names the +0x198 VIEW SUBOBJECT, not the object whose methods take
`this = table-base`. The outer owner (live-IDB `JE::HotKeyWindowFormatTable`) is the correct mint.
Both agents ran the same discipline on different data; the routing went by evidence, not by authorship.

Rules:
1. Before minting any band reached through a delegate-host or adjusted thunk, CHECK the candidate
   RTTI record's SIZE: a size-0 / view-shaped RTTI descriptor names a subobject view, not the owner.
2. Delegate-host containment → the band belongs to the OUTER owner (whose ctor installs the view's
   vft at +delta and whose methods call in with the constant this-offset), not to the view's type name.
3. When a writer overrides a verdict-proposer's LABEL (not the verdict), the ledger must carry both
   namings + the deciding datum — here the mint stands, the chain stays cited, and the proposer
   arbitrates bytes, not face.
- Tool citation (item 41): `python3 tools/ecx-flow.py pads` is the standing pad/cleanup-owner
  census; PAD-vs-DELEG is decided BY THE TOOL with a printed `evidence=dataref@0x…` bit (pad VA
  dword-referenced in .rdata/.data). Band censuses quote the mode instead of re-deriving the
  taxonomy; a pad bucket dispute now has machine-readable proof, not prose. (Added T-059, 2026-10-21.)

## 43. Pad/cleanup censuses quote at FUNCTION granularity — a class-level pad sink can be an artifact member, not an owner

EpicRaven's T-059 amendment + CalmRaven's T-058 locator catch, 2026-10-21. The 2,062-pad "owner"
`cls_0x4d8d70` evaporated on function-level re-check: 1,958 of them target ONE function, `0x4B2DD0`,
whose body is the app-wide std::string SSO destructor (cap≥16 -> free, cap->0xF/len->0 rearm) with
2,322 ordinary calls fanning out to EVERY string-owning class. It sat under the StringTree band label
as a T-036 artifact-floor aggregation — OOAnalyzer landing a generic helper inside a real class's
address range. A per-class cleanup never fans like that; a generic dtor always does. Corollary from
the writer side: a census locator (0x507550) that is not a live function HEAD belongs to the
build-era epoch, not to an annotatable object — force-annotating a non-head EA is itself a false
attribution; verify head-ness before commenting, log locator discrepancies instead of adjusting.

Rules:
1. Any pad/cleanup/owner claim names the TARGET FUNCTION EA, never the class key, when counts are
   aggregated; class-level attribution is derived, after the function-level truth.
2. Cross-check fan-out: a candidate "per-class cleanup" with calls from dozens of unrelated classes
   is a generic runtime helper (string/allocator/EH), classify accordingly (T-036 floor).
3. Writers: refuse to annotate non-head EAs; a locator mismatch is a taxonomy note routed to the
   census author, not a byte conflict.

Item 43 TOOL CITATION (reciprocal, per EpicRaven eccdb24): the wording above is the permanent
header of every `python3 tools/ecx-flow.py pads` run and of the module docstring — tool and harness
doc now quote each other. Practical effect: an agent that never read this file still gets the
buckets-vs-headness boundary and the head_hint()/live-read-back authority rule BEFORE its first
verdict. That is how a rule graduates from prose to infrastructure.

## 44. View-adjust = SHARING by default — membership requires a SINGLE-OWNER view (machine-checkable form)

CalmRaven's T-064 [B] falsification + EpicRaven's independent two-query concurrence from the pinned
cache, 2026-10-21. A queue cited "-0xB0/-0xC4 view-adjust callers" as band-MEMBERSHIP evidence for
five functions under CDebugInterface. The opposite reading was true: meth_0x434250/0x4342C0 are reached
from TWO different CEngineInterface methods with DIFFERENT view constants (0x434395 via -0xB0,
0x557C45 via -0xC4), each host storing its OWN interface vft (654304 CFeedback / 692e84 CDebug), and
they call the shared GUI::Interface cleanup 0x4b9d60; 0x434120 is additionally reached through two more
holder fields (GameInterfaces *[this+0xC], Client *[this+0x8C/0x90]). Multi-holder sharing, written all
over; the original evidence was real but NON-EXCLUDING.

Rules:
1. **Machine test before queuing any (a)-candidate mint**: `reached from >=2 distinct view constants
   OR >=2 distinct holder fields => SHARING by default` (computable directly from ecx-flow edges;
   tools/ecx-flow.py data shape). Membership claims need a single-owner view — one host, one adjust.
2. Practical one-check version (CalmRaven): when the only evidence is "a named member of host X calls
   it," ask whether a DIFFERENT host's view also reaches it; two hosts each storing their own interface
   vft settles it without arbitration.
3. This generalizes item 43 (function-granularity for pad sinks) upward on the ownership axis:
   band/caller adjacency is not membership — for fns OR for bases. Author-side adoption (EpicRaven):
   the test runs on EVERY future candidate before queueing — the pre-emptive [B].

## 45. Pairing a live datum against a 'dead/unreachable' pin: ask WHICH HALF of the mechanism the datum exercises

CalmRaven's T-069 gate run, 2026-10-21. T-051's row prose claimed the +0x319 raiser store "is
exercised ONLY through the walk branch - unreachable in shipped-config". The capture (R2/R3 values)
did NOT falsify the pin - and the reason is the rule: the RAISER lives in shared code (`PumpEntity`,
reached by BOTH the world step-5 loop dispatch=0 and manager dispatch=1, ungated on the mode flag) —
shipped-LIVE; only the CONSUMER (`0x5a955d -> 0x5a9572` skip-advance) is walk-only. A store in shared
code stays live even when the branch that READS it is dead. A dead-branch pin has two halves
(producer-site liveness, consumer-branch reachability) and a live datum only refutes the half it
actually exercises.

Rules:
1. Before letting a capture "confirm" or "falsify" a dead-path pin, name the half: which site does the
   datum hit - the store, the load, or the gated consumer?
2. Prose and asserts diverge silently: T-051's OWN suite block was green all along (latch-before/
   no-SetRoundMode/latch-again) while its row prose was wrong - behavior-in-tests is evidence,
   prose-in-rows is a claim. Fix the claim, keep the count; 'assert counts unmoved across the gate'
   is the proof a doc-only change is honest.
3. Related consequence (now in CombatTurnDispatch.md): the walk stays head-stable FOR THE
   RE-ENTRANCY REASON, not for lack of a live store - T-050's gate wording must never drift back.

## 47. A digest is gradeable only after its provenance is proven: mtime post-dates the run, AND the command actually ran

KeenStorm's T-068 near-miss, 2026-10-21. An Oct-20 stale /tmp digest nearly entered the first real
pairing table because the fresh-run command SILENTLY NO-OPPED: `wine` is not on that agent's bare PATH,
rc=127 ('command not found') was read as success by the pipeline, and the old artifact survived. Caught
by inspection; the table quotes only fresh-run lines after stale-file deletion + mtime verification.

Rules:
1. Prove the PRODUCER ran: check the exit code MEANING (127 != failure-of-target, it is absence-of-command;
   grade rc, don't just branch on non-zero-of-nothing) and that PATH/env assumptions were met BEFORE
   trusting the artifact it failed to produce.
2. Prove the ARTIFACT is fresh: mtime must post-date the run's start; delete stale candidates first so a
   silent no-op cannot fall through to an old file. A comparator's REFUSAL (T-031's unrecorded-vs-synthetic)
   is the backstop that made this a near-miss instead of a wrong-table.
3. Same family as item 35 (empty results until proven) and T-047's stale-digest-behind-wineserver flake:
   freshness is part of correctness, not housekeeping.

## 48. Before hunting "the string" at a 300+-xref address, read its FIRST BYTE

SwiftMoon's T-078 adjudication (banked as a mirror of the phantom-pattern family), 2026-10-21.
0x64D7D8 carried 357 aligned referrers and looked like a magnet (or like padding in a raw dump):
it is the image-wide POOLED EMPTY STRING — one NUL + alignment. Every consumer takes the ADDRESS
(pointer→strlen→assign); nobody loads content; the printable bytes eight over are a different
object with one referrer. The float-parsers' "0.0 at this address" reading was a dump-shape
artifact — under-resolving a dump and over-collecting a scan are the same failure from opposite
ends. Rule: high-xref read-only address => check byte[0] (0x00 = pooled "" candidate) and
classify referrers by ADDRESS-vs-CONTENT use before attributing any value semantics.

## 49. Absolute-encoding store scans are structurally blind to register-indirect vft stores — pair them with callee-side imm-vft scans

TrueCastle's T-075, 2026-10-21. Every prior census (b4, T-054) asked 'who stores/loads 0x707E48 by
absolute encoding' — answer: nobody (zero a3/c7-05/mov-imm; only 168 `mov ecx,imm32` THIS-loads). The
singleton was therefore assumed to be a subobject at global+X of an unlocated owner. It is a COMPLETE
object: its ctor stores its vft via `c7 06 cc f2 66 00` (`mov dword ptr [esi], imm32`) — register-
indirect, invisible to any global-address-keyed scan. The identity was on the callee side all along.
Scanner self-tested on the 0x707CE4 known-positive (all three writers reproduced) before trusting the
0x707E48 negative — the positive exists precisely because THAT ctor stored differently.

Rule: for every this==&global edge, run the store census BOTH directions — (a) absolute encodings of the
global, AND (b) `c7 0x <imm-vft>` immediate-table stores inside the callee reached through that edge
(same for `mov [reg], imm32` forms behind `lea ecx,global; call`). A 'no writers' negative about a global
is only about your scanner until both halves print empty.

## 50. RTTI TD/name clusters can link ABOVE a section's stated virtual extent — bound the walker by FILE-adjacent reality, not the PE section header

TrueCastle's T-081, 2026-10-21 (F1 closure). First automated COL->TD walk over the contested vft run
returned ZERO names everywhere — not because the RTTI was absent, but because the TD/name cluster for
this epoch links past .rdata's stated virtual size (0x6FAF2C+, into .data-side). A walker that bounds
TD-candidate validation to .rdata-per-headers silently rejects real RTTI. Fix shape: widen the walk to
observed file layout (here 0x647000..0x708000), then re-gate on a KNOWN POSITIVE (their T-075
GUI::Manager COL->TD geometry) before believing any negative the new bounds produce.
Rule: a zero-hit walker is a scanner verdict first, a target verdict second — the item-35 ladder
applies to BOUNDS exactly as it applies to PATTERNS; section headers are loader claims, link-time
layouts can exceed them.

## 51. A queue ships with its recipe, or it ships half-done

EpicRaven + TrueCastle, wave-2 pre-flight (665e7cc), 2026-10-21. T-084's 323-row mint queue was
banked WITH its doc but the host-derivation script was a heredoc — never committed. The queue was
therefore re-derivable for ~258 rows and *un-provable* for ~65: not wrong, just unrepeatable, and an
unrepeatable queue gets re-litigated at apply time by whoever's brave enough to hold the pen. Fix
shape (all landed): derivation RULE as committed tool mode (`ecx-flow.py hosts --cls X --dump <postdump>`),
provenance failure stated plainly IN the queue doc (AMENDMENT 2, no softening), HOLD list regenerable
verbatim from committed code.

Rules:
1. Any artifact whose rows carry derived claims ships the derivor (code > recipe prose > nothing;
   recipe prose decays the moment the session ends).
2. Apply-time filters that DISAGREE with a queue are evidence about the queue's machinery, not
   automatically about its rows — TrueCastle's intersection-with-proof ('my filter is weaker, the
   doc may be right') is the honest direction; 'the machine disagrees, therefore the doc lies' is not.
3. Terminal-field keying in shared tools makes real axes visible for free: the Client+0x118 vs
   World+0x118 collision is 'two rows by design' in the fixed mode — a derivation that cannot tell
   them apart was always going to look like disagreement somewhere else.

## 52. Intersection-with-proof includes the queue's own sections — a doc that mints in §2 what §3
annotates FOREIGN is self-contradiction, and band preflight is where it dies

TrueCastle, wave-2a (Q-W2), 2026-10-21. The GO said 107 applied-eligible; 99 actually ran. Nine rows
died at PER-BAND PREFLIGHT, not at ruling time: T-084 §2 mints fns whose LIVE cls_ key sits under §3's
foreign-annotation class keys (0x515040 was the pre-declared instance — the doc admitted ONE; the band
gate found its eight siblings: 0x52cc70/0x5150c0/0x515580/0x515d50/0x4a5720/0x4a5a10/0x436410/0x523450).
'Intersection-with-proof' applied to a self-contradicting source yields HOLD, never mint; the 9 joined
the Gate-A 65 into one 74-row arbitration list, each with a per-row reason — none silently minted,
none silently dropped.

Rules:
1. A queue doc's later sections are INPUTS to its earlier queues: run the collision check (key-set
   intersection of mint-list vs annotate-list) BEFORE apply, per band, against the LIVE dump.
2. Pre-declared exceptions tell you the CLASS exists; they do not bound it. One admitted instance =
   assume siblings until a sweep says otherwise.
3. The honest subtraction is the deliverable: report APPLIED 99 / GO 107 with the nine reasons, rather
   than back-fitting the ruling to the outcome or 'fixing' the doc's §3 to match §2.

## 53. Query before you dump: the index answers attribution questions — freshness or it's a shortcut wearing a trap

KeenStorm's H-IDX (81a07c8), 2026-10-21. The tonight-settled shared-stub question (Idle/Message idx5
= 0x5a9070, three independent raw re-dumps to close) had been QUERYABLE since yesterday's session-36
vftable master-index doc: `tools/ea-index.py --query 0x5a9070` returns five docs including the raw
7-slot walk of all three family tables. Raw discipline stays CORRECT (the index is raw output, and prose
never outranks bytes) — but the cheap first move inverts: query to learn what's already walked, dump to
confirm or to answer what the index can't. The meta-law: an index's payoff is INVERSE to its staleness,
so the standing refresh row (PARK H-IDX, 'any idle hand, twenty minutes') is infrastructure, not chores.
--check green = the shortcut is honest; --check red = stop, refresh, then query.

## 54. An adjudication table predates every apply between its cut and its execution — re-verify against the LIVE IDB per row, drop-with-reason over apply-and-undo

HappyStorm, wave-3 (bb5ba3e), 2026-10-21. The class-1 table was cut from t064-postdump; wave-1 had
ALREADY landed T-073's composite rename (JE::Gfx_RenderManager::meth_0x4903e0) after that dump. The
table's row for it (doc=JE::Gfx_Engine, keyed by a trampoline prefix this board had formally retired)
would have UNDONE a correct apply had it been applied as written. The live-vs-plan pre-flight check —
designed for name churn — caught it, dropped the row with the reason in the ledger, and generalized:
any derived queue/adjudication/table artifact is only as young as its dump.
Rules: (1) apply-time re-verification is per-row against the LIVE state, not the source era's snapshot —
'no self-waiver' includes not waiving your own table's age; (2) when a table and a live state disagree,
drop-with-reason beats apply-and-reconcile-later; (3) refresh the table's SOURCE (re-cut from the
post-apply dump) rather than patching rows by hand — the regenerator is the record.

## 57. The port must be as auditable as the research: every port function cites its original-side EA (cited, or absent-by-name)

User-flagged 2026-10-21 after IDB-name archaeology: the corpus learned 'cite or say absent' for
discoveries and ledgers, but 27% of port definitions (97/355) carried no shipped-side address —
meaning a reader CAN'T diff port-vs-binary without re-deriving the mapping. Same failure shape as
the phantom-base 0x691A20 (docs drifting from bytes) but in the product direction.
1. STYLE.md now carries the hard rule; tools/port-annot-check.py enforces it mechanically.
2. Backfill before gate-promotion (R-ANNOT): a checker that fails 97 pre-existing holes gets ignored;
   one that fails zero gets obeyed.
3. Named-absence comments count — 'no shipped analogue located' is compliance; silence is the bug.
   The rule that saved every wave ('as applied, not as proposed') simply extended to code comments.

## 58. Re-source before quoting run-all counts after a gate-wiring commit; a stale PATH is an HONEST-FROM-A-STALE-GATE report, not a lie

SwiftOwl's correction-discovery, 2026-10-22, about its own relay. After the port-annot gate was wired
(`ece5726`), the lane's session-start PATH still resolved run-all-tests to the PRE-WIRING payload: it
correctly ran 10 steps and truthfully reported '10 pass' — the truth came from a stale instrument.
gate-integrity's named-cause machinery (unsourced PATH → wrong payload) fired exactly as designed and
re-resolved via `devenv print-dev-env` (launcher + payload hashes pinned, 3 occurrences of the gate
string = current). Rules: (1) after ANY change to gate-bearing scripts, lanes must re-source (or the
coordinator says so in the wiring commit message — better: BOTH); (2) gate-count quotes carry their
payload hash era; '10 pass' from a 11-step era is a bug report about the board, not about the code;
(3) corrections-when-they're-discovered (SwiftOwl amended its own relay here rather than editing it)
remain the register's highest form. The 11th gate enforcing item 57 now polices everyone including the
gate's own wiring story.

## 59. Captures are WALL truth, traces are DEVICE truth — trust them in that order; black-on-screen can mean a healthy device mid-intro

RedQuartz's cold-bootstrap proof (c815809, replication 5: full rig built from repo+nixstore alone,
non-NULL/CreateDevice/3300+ presents), learned the cheap way: the cold run's T+45s root capture read
BLACK while the device was perfectly healthy — the intro movie occludes the magenta clear early;
magenta appears ~T+8s *between* movie segments. A naive lane would have failed slice-2 acceptance on
a screenshot and declared a working rig broken.
1. Any frame-validation (item-5 sink-proofs, slice-2 acceptance, movie-era fixtures) gates capture on
   a POST-INTRO trace mark or times it at a known inter-segment window — never one lucky screenshot.
2. Device verdicts read from the trace cadence ('presented frame N' advancing) + targeted pixels;
   whole-screen histograms are evidence of what the WALL showed, not of device health.
3. Same-sitting self-corrections, both surfaced by the cold proof: the 'bare prefixes lack
   msvcr70/msvcp70' keeper warning was FALSE (wine-11 ships both natively, byte-identical — the
   template's pair was never a manual install; seed dir stays as harmless insurance, header rewritten
   to evidence). A tool's documentation ages at the speed of its assumptions — the cold bootstrap
   re-tested them all at once.

## 60. CMKR_SKIP_GENERATION=ON means cmake.toml edits are SILENTLY IGNORED — regeneration is now explicit in msvc-build (item 60 fix, from BrightYak's hour)

BrightYak's link-defect hunt (slice-1 relay, 2026-10-22), mechanism found in their own commit message: the
cmkr bootstrap in CMakeLists.txt no-ops once build/CMakeCache.txt exists, and msvc-build.nix passes
-DCMKR_SKIP_GENERATION=ON — so the 12 unresolved externals sat UNSEEN for days: mingw test rows carry
their own per-row source lists (all green), while the MSVC link had been configuring a stale CMakeLists
forever. Two of their runs 'changed nothing' before the mechanism surfaced.
Fix now IN the tool (devenv/scripts/msvc-build.nix, this era): when build/_cmkr_v*/build/cmkr exists,
msvc-build copies it to cmkr_gen.exe, runs `cmkr gen` explicitly (offline) BEFORE configure, and exits 1
if gen fails. Rules: (1) cmake.toml edits take effect at the next msvc-build with no manual step;
(2) `gen` rewrites CMakeLists.txt in the working tree — commit it WITH the cmake.toml change, both trees
or neither; (3) a per-row-source mingw suite can NEVER catch link drift — any test grid that restates
sources is a second source of truth to be reconciled, and 'green tests + red link' is the signature;
(4) RedQuartz's run-identity ledger pairs with this perfectly: builds move under verdicts, so verdicts
now name their bytes. The sleep wasn't laziness — it was a skip-generation flag nobody re-read.
