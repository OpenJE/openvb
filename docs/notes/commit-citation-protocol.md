# Citing commits in a two-repository tree (2026-10-22)

Caught while closing a relay loop, and worth a file because both halves of the mistake were made in good faith
by different lanes, and the cause is structural rather than personal.

## The structure

OpenVB's product tree records `modules/openje` as a **gitlink** (`160000 commit <sha>`), so there are two
object databases:

| what | where it resolves | example |
|---|---|---|
| parent commits (`src/`, `docs/`, `devenv/`, `tasks/`) | the parent repo only | `5a4b0a3` |
| suite commits (`modules/openje/tests/...`) | the **submodule** only | `2521b31` |
| either hash, looked up in the wrong database | **MISSING**, which reads exactly like a phantom | `git cat-file -t 2521b31` in the parent |

Every batch commit in the parent also bumps the gitlink (`modules/openje | 2 +-`), so a suite change is real in
both histories - one as a commit, one as a pointer.

## What went wrong, in order

1. A suite hash was typed from memory (`f2b3a1c`) and never existed in either database: a genuine phantom, self-
   caught.
2. The replacement citations (`23418f1`, `2521b31`, `f7f2d4c`) were **correct but unqualified**, so they failed
   `git cat-file` in the parent database.
3. A verifier who ran the query in one database only concluded those were phantoms too, and generalized to
   "batches are single commits with the suite inline" - which the tree contradicts: `git show --stat` on the
   parent shows `modules/openje | 2 +-`, and `git -C modules/openje log` shows the suite commits by name.

So the same three hashes were simultaneously "not a phantom" and "not resolvable where they were checked." That
is not a contradiction about the commits; it is a missing field in the citation.

## The rule

- **Never emit a hash from memory.** Re-emit it from `git log`/`git show` at message-writing time. This is the
  same law already recorded for offsets, constants, and slot numbers: re-derive from the artifact, do not
  regenerate from recall.
- **Qualify the repository whenever both are in scope.** Write `openje:2521b31` or `parent:5a4b0a3`, not a bare
  seven-hex-digit string. In a two-database tree an unqualified hash asserts resolvability everywhere, which is
  the one thing it cannot guarantee.
- **Verify with the command that matches the claim.** `git -C modules/openje cat-file -t <sha>` for suite commits,
  `git cat-file -t <sha>` for parent commits. A MISSING result proves only "not in this database."
- **Do not collapse the citation to the parent.** Citing only the batch parent loses the suite commit's identity;
  the pair (`5a4b0a3` bumping `openje:2521b31`) is the accurate unit and costs one qualifier.
- **When a peer reports a phantom, check the second database before correcting your own line.** The temptation
  in that moment is to accept the verdict and amend, which is how a correct record becomes a wrong one. Ask which
  object store the query ran against.

## Verified lines as of this writing

parent: `b2693a8 cc8e54b b44b459 e8e1a6a b8d91b9 a63fffc 9e11a57 ac88048 968343e 5a4b0a3`
openje: `1489def f7f2d4c 23418f1 2521b31`

## Sweep of every 7-hex token in this tree (run 2026-10-22)

247 unique tokens across `docs/`, `tasks/`, and `modules/openje/tests/`, resolved against BOTH object
databases. Unresolved came to exactly seven, and none is a commit citation:

| token | why it does not resolve |
|---|---|
| `1000000`, `1664525`, `3194880`, `3456789` | decimal literals (buffer sizes, LCG constants, the exe's byte count) |
| `545044f`, `991112f` | test-data magic values in the suite, not hashes |
| `f2b3a1c` | the phantom from this file, cited **as** a phantom in the narrative and the ledger |

Everything else resolves in the parent or the submodule. Two notes on method, because a sweep that returns
"clean" is only as good as its filter: a seven-hex-digit token is ambiguous by nature - `3194880` is both a valid
hex string and the shipped exe's size - so the check is "does it resolve as an object in one of the two stores,
and is it being used as a commit", not "does it fail". And a documented phantom must STAY unresolved: rewriting
`f2b3a1c` out of the prose would delete the evidence for the rule, which is the difference between a correction
and a cover-up. If this file is ever re-swept, expect that token and don't "fix" it.
