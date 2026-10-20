# tasks/ — shared task board (parallel implementers)



Two implementers plus a growing research side work this repo in parallel. This board is the

source of truth for **who is doing what**; the **pi-messenger mesh** is the conversation

(protocol v3, `chat/README.md`) — `chat/from-{A,B}.md` are the frozen `C-<n>` archive.



**`NiceFalcon` (implementer-A) is now the coordinator, not an implementer.** It does not own

features; it hands out rows, reviews and commits research checkpoints, owns the gate

(`run-all-tests` + `msvc-build` + app boot), arbitrates lane collisions, and routes uncertain

facts to research instead of letting them be guessed in code. Route anything blocked, contested,

or half-decided to it. Research agents write `discoveries/` files and never commit them; the

coordinator checkpoints on their "[DISCOVERY-BATCH-N ready]" line.



Mesh **names are re-randomized on every reload/restart** — identity is the `sessionId` in

`~/.pi/agent/messenger/registry/<name>.json`, not the name you see today.



## How to take a task



1. Check the mesh first (`pi_messenger` `status` + `list` + `feed`), every session — see

   `chat/README.md` (protocol v3). `chat/from-A.md` / `from-B.md` are the frozen archive of the

   old `C-<n>` threads; read them only when your task cites one.

   ALSO poll `discoveries/` (new/changed files: `ls -t discoveries | head`) — the discovery

   agent writes live RE notes there between commits (files, not mesh messages) and tasks often

   get unblocked or invalidated by them (field naming, formats, "don't trip on this" notes).

   Reference by filename; treat everything there as evidence-backed but verify claims that gate

   your edits.

2. Pick a task file with `status: open`, set `assignee`, `status: in-progress`; `pi_messenger`

   `reserve` the paths you are about to write (this replaces the one-writer-per-file rule —

   reservations actually block a peer's edit); then commit the claim **before** editing code.

   Release (back to `open` + a mesh message) if blocked or idle >1 session.

3. One task at a time; finish or hand off before claiming another (WIP limit = 1).



## Conventions



- File name: `T-<nnn>-<slug>.md`.

- Sections: Objective / Steps / Acceptance / References / (optional) Notes.

- Acceptance must be mechanically checkable (a command, a test name, a diff).

- Evidence style: binary addresses `0x…`, member offsets, and file paths — same as

  `docs/DEFERRED-NOTES.md`.

- When a task is done: `status: done`, paste the acceptance evidence into the file, commit,

  and announce in chat.



## Lane ownership (do not edit outside your lane without a chat handshake)



| lane | owner | areas |

|------|-------|-------|

| Engine core | implementer-C (junior, coordinator reviews) | `modules/openje/src/JE/*` additive accessors and small ports; anything structural needs coordinator sign-off first |

| Coordination / gate / review | coordinator (`NiceFalcon`, formerly implementer-A) | board, reservations arbitration, `run-all-tests` table, `msvc-build`, app boot, `discoveries/` checkpoints |

| Game layer | implementer-A | `src/F3/*` (world, entities, actions, spawn) |

| Tooling & tests | implementer-B (new) | `.devenv/scripts`, `devenv/scripts`, `run-all-tests`, test harness glue, `modules/openje/tests/**` (existing suites), cppcheck/lint scripts, `docs/notes/implementer-B.md` |

| GUI/skin lane | implementer-B → A review | RE + `JE::GUI`/`F3` interface loading (`.int`, skins, fonts) — new files only until handshake |

| Docs | both (append-only sections) | `docs/`, `discoveries/RESPONSES-from-reimpl-agent.md` (B may append under its own heading) |



Submodule note: `modules/openje` is a git submodule — commit inside it first, then bump the

parent pointer (both repos must end each task clean).



## Active board



Rows in this table are maintained by the task's assignee (claimant may flip their own

row; A keeps editorial merge rights).



| id | title | status | assignee |

|----|-------|--------|----------|

| T-001 | Onboarding: build + run every suite green | done | implementer-B |

| T-002 | Fix `cppcheck-openje`; add `run-all-tests` script | done | implementer-B |

| T-003 | GUI RE recon: `.int` load chain field notes (read-only IDA) | done | implementer-B |

| T-022 | Assert every "verbatim shipped" string is byte-identical in F3.exe | open | implementer-C (UltraBear) |
| T-023 | Cross-TU compile probe as a command (verify a rename with no linker) | open | implementer-C (UltraBear) |
| T-024 | Gate integrity: revision/payload stamp + per-suite minimum assertion counts | open | coordinator (NiceFalcon) |
| T-025 | Shipped-data census generator -> committed greppable tables | open | FastArrow (research) |
| T-026 | Per-frame simulation digest (parity as a diff, no renderer needed) | open | implementer-B (YoungStorm) |
| T-027 | Deterministic RNG seam for damage draws (documented deviation) | open | implementer-B (YoungStorm) |
| T-028 | Scripted input driver + how input really enters the client | open | HappyWolf (research) |
| T-029 | IDA function census: the missing coverage denominator | open | IronStorm (research) |
| T-004 | Test coverage: EMAP v5 fog/ambient byte stores | done | implementer-B |

| T-005 | GUI: `.int` reader + widget tree with name lookup | done | implementer-B |

| T-006 | GUI/SS: `Format_0x56be30` `<tag>` markup + Label text | done | implementer-B |

| T-007 | GUI: skin sub-records (Window+0x90 x9) + texture binding | in-progress | implementer-B |

| T-008 | Onboarding: mesh + build + full gate green | done | implementer-C (UltraBear) |

| T-009 | `VFX_Parameter` section/entry iterator (unblocks T-007) | done | implementer-C |

| T-010 | Research queue (10a answered, 10b resolved-as-defect, 10c one data read, 10d closed, 10e open) | open | research |

| T-011 | Combat: turn-queue per-turn dispatch + join-on-attack | open | unassigned |

| T-012 | **P1** Combat: replace the falsified from-behind rule (3π/4 is proven wrong) | done | implementer-C (UltraBear) |

| M-001 | D3D8 slice 1 — **PROBE ANSWERED: headers compile, `d3d8.lib` ABSENT** → blocked on a human decision | blocked | implementer-B |

| T-013 | Engine texture registry (`cls_0x55dc80` @byte_70B200) = T-007 tail | open | implementer-B (after M-001 slice 1) |

| T-014 | Who sets the sub-attack record's +0x19 "marked" byte (volley-batch loose end) | done | FastArrow (research) |

| T-015 | GWAM record map — B9 pre-map + B11 slots part 1 (22-read order confirmed, frame captured); remainder = slot table part 2 + creature-twin parity | in-progress | FastArrow (research) |
| T-016 | One debug session, three pinned runtime reads (naming / World+0x160 vft / Map Name fallback) | open | unclaimed — Read 1 optional now |
| T-017 | F3 UI surface: Command* -> GameInterfaces slot / root-Open mapping | in-progress | HappyWolf (research) |
| T-018 | Data read: shipped helmet record's `GameItem+0xA8` required-slot vector | in-progress | IronStorm (research) |
| T-021 | Retire the anatomy wording 10c disproved (rename + comments, no link needed) | done | implementer-C (UltraBear) |
| T-020 | Verify the reachable reaction-id set {33,34,37,38,40,41}; 39 unreachable | open | NiceWolf (research) |
| T-019 | Consumed+thrown FatalError on shipped data — **answered: 0/57, never fires** (B11) | done | FastArrow (research) |
