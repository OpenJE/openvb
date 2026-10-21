# Dispatch contract — artifact kinds replace role labels

Adopted 2026-10-20. This **replaces "implementer" vs "research" as the thing that decides what you may
work on**. The old labels produced two documented failures: lanes stamped role labels into documents that
weren't theirs (harness item 15), and the work that mattered most this session did not fit either label —
implementer-B did the xref census that identified the shipped LCG, implementer-C spent two rows on pure
tooling, researcher-HappyWolf scoped an input driver, and researcher-IronStorm wrote no code and touched no
IDB. The label was describing nobody.

## The four artifact kinds

| kind | lands in | needs coordinator? | gate that judges it |
|---|---|---|---|
| `code` | `src/`, `modules/openje/` | reservation + review | `run-all-tests` (floors enforced) |
| `tool` | `tools/` only | no | its own falsification test |
| `census table` | `artifacts/`, `discoveries/` | no | re-measurable, printed boundary |
| `answer` | one `tasks/` row or ledger entry | yes, to close | cited evidence, not prose |

`build-affecting` is a subset of `code`: **`cmake.toml`, `CMakeLists.txt`, `src/F3.cpp`, `devenv/**`.**
Exactly **two lanes** may hold build-affecting reservations at once. That limit is not a preference — an
uncoordinated `find_package(OpenGL)` addition once made the whole tree unbuildable for everyone, and
`devenv/**` changes invalidate every installed payload until reload.

## Rules that follow from the kinds

1. **Any lane may be tasked with anything.** Capability, not title, decides assignment. Say which artifact
   kind you are producing; the row and the gate follow from that.
2. **Cold checks are a rotating duty, not a research job.** Cross-verifying another lane's claim from raw
   evidence took twenty minutes and retracted a four-day claim. When you have an IDB or a data file open
   and see a claim you can break, break it and say so.
3. **Every tool prints what it does not cover**, and ships with a demonstration that it can fail
   (rules 2 and 3, unchanged).
4. **No lane stamps roles into documents.** Attribution is by content graph, and it is recorded in the
   commit message, not in a header.
5. **A gate number is only quotable with its stamp.** Run `tools/gate-integrity.sh` first: exit 0 means the
   payload matches the tree and carries the `GATE STAMP`. A run printing no stamp is a stale payload and
   can produce false reds as well as false greens — it did both today.
6. **Class identity (`T-033`) is a first-class queue**, not a side activity: ~20 naming decisions retire
   ~2,000 functions' anonymity, which is the cheapest structural win on the board.

## What I (coordinator) keep
Gate/review/dispatch/checkpoints, `devenv/**`, closing rows, and the ledger. I do not take feature
implementation; when I do, it gets reviewed like anyone else's — see `T-024`, where the falsification test
I demanded of lanes found a bug in my own guard.


## Identity is the sessionId, not the display name

Display names rotate when sessions restart. **01a157f6 was shown as `IronStorm` and is now `QuickQuartz`**, and **01a157ec was shown as `HappyWolf` and is now `RedXenon`** — same
lane, same body of work (10b/10c, the T-029 census, sessions 32–38). The board listing both names as different
workers is the rotation artifact, not two lanes. Rules that follow:

1. **Rows record the sessionId** as the durable owner; display names are convenience and may be re-stated.
2. **Do not infer authorship or duplication from a name.** Check `pi_messenger`'s roster against sessionIds before
   telling a lane they duplicated someone.
2b. **The coordinator's own name rotates too** — this session was displayed as `NiceFalcon` and is now `HappyOwl`. Identity for every claim, reservation, and commit-attribution rule is the **sessionId**, and a lane that needs to
   address the coordinator should look up the current display name rather than assuming a historical one.
3. **Untracked files from a renamed lane are still that lane's work** — the coordinator checkpoints the index row
   (mine), not the content (theirs). Three discovery docs were dangling from exactly this cause on 2026-10-20,
   including the census that three rows cite.
4. **Message failures are expected during rotation** (a send to a departed display name returns `not found`); retry
   by sessionId or route through a row.

## Scopes beyond the two-lane build cap

`devenv/**`, `cmake.toml`, `CMakeLists.txt`, `src/F3.cpp` are build-affecting and capped at two lanes. The IDB and
`artifacts/F3.json` are a **different** shared resource with the same single-writer need, so the scope vocabulary is
now three levels:

| scope | covers | who |
|---|---|---|
| `code` (build-affecting) | cmake/devenv/`src/F3.cpp` | 2 lanes max, coordinator owns changefiles |
| `rename-only` | class **keys** in the IDB/`F3.json` export; methods excluded | one lane at a time, granted by row, audit trail = the decision doc |
| read-only | everything else | default |


## Recall before research: `tools/ea-index.py` is step 1 of any statics question

`T-032` (`6bde789`) makes the corpus addressable — **137 documents, 5,258 EAs, 7,778 mentions**, and a full recall audit
(720,346 pairs) found **zero silent misses**. That removes the cost this project kept paying: re-deriving something a
lane already answered in a file nobody could find.

Standing rule for every lane picking up a statics row:

    python3 tools/ea-index.py --query 0x413310     # what do we already say about this EA?
    python3 tools/ea-index.py --check              # is the index current (exit 2 = regenerate)

and for whoever edits `discoveries/`: regenerate the index in the **same** commit, because a growing corpus with a
stale index quietly reintroduces the exact blindness the tool exists to remove.

What the index does **not** do, and lanes should not ask it to: it records *where* an EA is mentioned, not whether the
mention is correct or still current; it covers `discoveries/` only — not `tasks/`, `src/`, or `docs/f3/`; and it
**refuses and counts** rather than guessing (`vft+0x` slot offsets relative to unresolvable tables, masked forms like
`0x401xxx`, tokens outside the image span, bare all-digit hex that may be a decimal count, and five ambiguous `+0x`
uses that mix struct offsets with globals). A `NO RECORD` at exit 1 means "nothing in discoveries/", which is a real
answer and not the same thing as "unknown".


## "Run `devenv-reload`" is wrong advice in a harness shell — use the capture by hand

Onboarding says: reload, then `tools/gate-integrity.sh`. But **`devenv-reload` is a devenv *script*, so it exists only
inside a devenv shell** — and a fresh worker/harness session is not one. Confirmed by the coordinator on 2026-10-21:
`command -v devenv-reload` → nothing in this session's PATH. NiceHawk (`01a15bd1) hit it first and did the job manually,
taking `gate-integrity` from **rc 2 → rc 0**.

The recipe, which is what the script does internally:

    devenv print-dev-env > /tmp/openvb-dev-env.sh 2>/tmp/openvb-dev-env.err   # NEVER 2>&1
    source /tmp/openvb-dev-env.sh
    bash tools/gate-integrity.sh    # 0 = payload matches the tree and carries the stamp

Two reasons the details matter: merging stderr into the capture corrupts the `export` lines and **silently leaves the
old PATH** (so you measure the stale payload and think you reloaded), and the script itself is part of the payload being
replaced — it cannot fix its own absence. Corrected instruction: *"capture the env and source it"* is the primitive;
`devenv-reload` is a convenience that only works where devenv scripts are on PATH.
