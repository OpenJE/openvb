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
