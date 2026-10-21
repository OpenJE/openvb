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

| T-022 | Assert every "verbatim shipped" string is byte-identical in F3.exe | done | implementer-C (UltraBear) |
| T-023 | Cross-TU compile probe as a command (verify a rename with no linker) | done | implementer-C (UltraBear) |
| T-024 | Gate integrity: revision/payload stamp + per-suite minimum assertion counts | open — **done** || done ||
| T-025 | Shipped-data census generator — **DONE**: 7 gates PASS (coordinator-run), EME2 tiled byte-exactly, trigger names a NAMED GAP with evidence | done | RedXenon (01a157ec) |
| T-026 | Per-frame simulation digest (parity as a diff, no renderer needed) | open — **done** || done ||
| T-027 | Deterministic RNG seam for damage draws (documented deviation) | open — **done** || done ||
| T-028 | Scripted input driver — **DONE both halves**: `tools/sim-driver.cpp` reaches a combat round through the ported seam, verify suite 9/9 | done | RedXenon (01a157ec, was HappyWolf) |
| T-030 | Entity action-queue seam — **done: `0x550720` is the single gameplay-order funnel (40+ xrefs)**; D-2 registered | done | implementer-B (YoungStorm) |
| T-031 | Parity oracle — **synthetic shipped side DELIVERED (`40b215a`)**: `tools/orig-digest.sh` + 47-check verify, PROVENANCE required, MIXED-PROVENANCE refused, invariants violated on purpose. **VALUES still gated on T-034** (R1/R3/R4b) | in-progress | NiceHawk `01a15cb7` (via T-034) |


| T-032 | EA→document index — **DONE (6bde789)**: 137 docs / 5,258 EAs / 7,778 mentions, 0 silent misses in a 720k-pair recall audit, refusals counted by category | done | SwiftRaven |
| T-033 | Class-identity batch queue — **DONE, CHAIN CLOSED AT SIX (`30d0362` + coordinator checkpoint)**: final T2 **2,046 = 23.05%**, T1 4,312 = 48.58%; cumulative **26.40%→23.05% (−294 = b2 24 + b3 192 + b4 40 + b5 18 + b6 20, zero-gap reconciliation, every wave dump-verified on disk)**. b6: 0x55dc80 escalation CLOSED by cross-lane G2 evidence (11 manager fns→`JE::Gfx_Texture_Manager`, class 17; LOUD correction: the other 83 band members are _Tree clones in FOREIGN TUs — b3's conservative half vindicated); anchors 707CE0/70838C/708394 → commented honest negatives (708394 = {width,height} value pair, not a class); **NEW PERMANENT TOOL: RTTI method-set sweep** (join `.?AV*@@` DB method sets vs live IDB names — proven sets per T-040 stage-3; rejoined 11 singles incl. `JE::ModelInstance::ctor_0x564ce0`, one double-proof via own vft store; root pattern: map-embedding ctors/dtors swallowed by _Tree clones — RTTI join is the surgical undo, band rules intact). **Closure statement: queue EXHAUSTED at current evidence — t036 30/30 classes adjudicated, sweep yields zero further wins; remaining T2 = vendor bands (46+148), _Tree/allocator families (50dbb0 120/55dc80 83/50db20 79/4542d0 50), n≤2 vacuous tail. Future wins need NEW evidence classes: T-052 lockstep, ecx-flow tooling, unexamined-region string mining. Re-run RTTI sweep as regression check after any IDB rebuild.** IDB durable: saved 14:26, disk==live at hand-back | done (chain-closed) | GoldYak `01a15f83` (b4-b6) + PureKnight `01a15cb7` (b1-b3) |
| T-034 | Lockstep reads — R0 answered from the image (superseded by T-045 statics); **LIVE HALF CLOSED-AS-MISS (`5717bd9`)**: zero of six values, honest miss list per the row's alternative; blocker NAMED = instrumented-boot startup-stop dance (ntdll DbgBreakPoint never consumed; TICK 0x56af50 never fires; uninstrumented boots fine — that delta is the bug); attach/port/xdotool all exonerated; retry re-filed as **T-052** | closed-as-miss (retry = T-052) | NiceHawk `01a15cb7` |


| T-039 | **DONE (633744d)**: `Entity+0x314` is the deque's **LIVE COUNT** (writer `++` at `0x54ea71`); byte deleted → derived `q=` digest field; **D-2 now diff-visible** | done | HappyKnight |
| T-040 | **`artifacts/F3.json` provenance — DONE: it is a frozen 2024-04-04 OOAnalyzer v2.2.0 export; producer identified from the IDB's own `_PyOOAnalyzer__` strings, sibling inputs byte-identical (md5 e8827f12…), `ooprolog` stage-2 inference verifiably absent so the current IDB cannot regenerate it. Stage 3 rebuilt + proven (1,101/1,101 keys, sizes, method sets, member sizes/usages; 4,059 synthesized entries declared UNVERIFIED). `f3-json-check` added. `cls_0x…` = "no RTTI-named vftable in 2024", not "unidentified". ERRATUM (via T-007 G1, committed): the export's `GUI::Fragment` size 3384 contradicts byte-truth 0x158 — export SIZES are not authoritative** | done | SwiftRaven |
| T-041 | **DONE**: `Entity+0x1E4` is the **map-membership flag** `inMap0x1E4` — sole runtime writer `Entity::SetInMap_0x551ba0` (store 0x551c4b), doors = ENGINE_ID_ENTITY_ADDTOMAP/REMOVEFROMMAP packets (client case 14 `0x58c90c`; server `meth_0x555750` `0x555775`); record 3rd int after the `+0x1E0` valid-marker; no `+0x32C`-alias writer; port rename landed | done | PureKnight `01a15cb7` |
| T-042 | **DONE (openje 77c18d8 + this parent commit)**: ctor aligned to shipped `0,1,1` first, then the `?: 2253/10/18` ternaries deleted; absent-key fallback now the ctor's `0,1,1,0,0,0` (`0x548af6..0x548b08`), authored `Year=0` reachable again; StringSystem 20→27 asserts; gate 10/0 + msvc-build exit 0 | done | NiceHawk `01a15bd1` (half-landed) → UltraMoon `01a15cb7` (gated+committed) |
| T-043 | **What is `Entity+0x1DC`?** — **done**: a runtime-allocated reference handle (allocated in `Entity::Create` `0x55190c`, serialized by `PackState`, matched against the raw AMX cell by `meth_0x54be10`); not a form id, not a pointer; `netId` is not a substitute, so D-3 stands | done | WildYak `01a15802` |
| T-044 | **DONE**: a location-targeting attack DOES exist — one door, server packet ID 91 (`0x46df14` in `GameServer::virt_meth_0x46dd70`); record `+0x48` hasPos + `+0x4C/50/54` XYZ via `meth_0x401100`; guard `attacker && (hasPos || target)`; consumers prefer the override; no script-native/AI path (T-037 stands); spawned **T-047** (driver verb) | done | PureKnight `01a15cb7` |
| T-045 | **Slot +24 of WHICH vft?** — **done**: the pump indexes World's `+4` table (`0x6919e4`), whose slot 6 **is** `Update_0x544a90`; the stub is the primary table's slot 6. Also reversed the frame-0-leap verdict: the shipped tick is session-relative elapsed and the stamp is seeded, so the leap is ours — scoped to the absolute `clock` CONTEXT number only | done | WildYak |
| T-047 | **DONE (`8e272e6`)**: attack-pos verb shipped exe-level — packet-91 group verbatim (+0x48 flag, +0x4C/50/54 vec3, Start guard `attacker&&(hasPos||target)` @0x46decb), walk-then-WHIFF with no invented victim (occupant-damage trace = future row, cited not silent), digest by token shape only, verify pairing + grammar-fail cases, flake self-caught (stale digest behind lagging wineserver: rm -f + non-empty assert) | done | PureKnight `01a15cb7` |
| T-048 | **DONE**: `PumpEntity` raises `actedLatch0x319` on processed-and-not-dropped (0x54c833 analog); 0x54c864 filed UNEXPRESSIBLE with cited EAs, no budget invented; drop never clears, TurnPrep keeps the clear; +11 asserts incl. executed-mutant falsification (store killed => 5 named reds) and stable-head-with-store pin; gate 10/0 + msvc-build 0 (stamp 05:31:54Z) | done | UltraMoon `01a15cb7` (dispatched by ZenNova) |
| T-046 | **DONE (live-proof)**: `docker-rootless` readiness pinned to this uid’s socket with distinct no-socket/no-daemon failures, watch/`wait` replaces the bare `tail` (exits nonzero on dockerd death, demonstrated live), one-shot run-path probe; `msvc-build` pull-else-cached prints the source **and cached image sha256**, negative control exits 1 naming the missing image; final gate 10/0 + msvc-build exit 0 fresh payload. Boundary: process config activates at next session re-up (`devenv processes restart` reuses loaded config) | done | UltraMoon `01a15cb7` (build lane, dispatched by coordinator) |
| T-049 | Pull-else-cached for `msvc-build-debug`/`msvc-build-tracing` — **DONE (`5f2e824`, GoldYak)**: bogus-registry proof RUN FOR REAL on both variants (cache line + sha256 + manifest-unknown logged, full build rc=0, F3-dbg/F3-trace linked, alias rmi'd), negative controls exit 1 NAMING the image, grep gate empty + guard 1/1/1, verbatim T-046 shape, parse-OK. ACTIVATION STATED PER RULE: all legs direct-invocation; installed PATH stale for ALL THREE scripts — re-up is the coordinator's call, gated on HappyEagle's live T-052 stub session; IDB untouched (scratch-pinned) | done | GoldYak `01a15f83` |
| T-050 | Skip-advance seam probe: make `0x5a955d -> 0x5a9572` constructible in tests — but ONLY in the shape the lockstep observation dictates (T-048 established the advance is unconstructible without re-entrancy: maintenance-clear-before-test, `endIter_`+14 no nonzero writer, chicken-and-egg on the only window). **GATED: do not start before T-034/R-batch reports re-entrancy either way.** Consolidated canonical row: `tasks/T-050-skip-advance-seam-probe.md` (a 2-min cross-resolve deleted both original files — branch-2 = writer-study reopen if re-entry is falsified; T-048's stable-head test is the interim canonical shape) | open — GATED | owner resolves after T-034/R-batch reports |
| T-051 | Pin the dead round-walk branch (mode/rearm constants + T-048 latch inertness) per T-010 10e `045a7ec`: free-run is the only live shipped path — tests must say `SetRoundMode` explicitly, zero new arming callers, sim-driver branch trace | in-progress — PRE-WORK LANDED & verified (PathManager 117>=74; sim-driver-verify 13/13 incl. 3 new T-051 checks; arming audit printed: sim-driver 0 new callers, all test arming annotated TEST-ONLY; docs banner up); GATE RUN now gated on **T-052** (T-034 closed-as-miss; pairing needs a capture or an explicit no-capture decision) | UltraMoon `01a15cb7` (dispatched by coordinator) |
| T-052 | Fix the instrumented boot startup dance (double-resume, winedbg WITHOUT --no-start), then retry the six reads; if window still unmaps after a clean dance, stop and escalate wined3d env (LIBGL_ALWAYS_SOFTWARE=1 / Xvfb) as a harness decision — one sitting, values table OR named second mechanism; re-land harness v3 under tools/ | in-progress | HappyEagle `01a15f83-1255` |
| T-053 | Shipped-string mining over unclaimed bands — NEW EVIDENCE CLASS per the b6 closure statement: coverage table (self-tested on 3 known positives), prioritized MINT/ANNOTATE/ESCALATE queue handoff-ready for a writer wave, dismissal list; READ-ONLY, discoveries/ only | open | CalmRaven (dispatched) |
| T-054 | ecx-flow owner-attribution tool (tools/ecx-flow.py, OFFLINE analysis): indirect-call/tail-jump/this-propagation/allocator-caller joins; self-test on Gfx_D3D/AnimMeshInstance/Texture_Manager knowns; adjudicate 445350 + 4b28a0 + 507810; no IDB writes — verdicts feed the next writer wave | open | EpicRaven (dispatched) |

| T-035 | ODR collisions — **done: `GameActionWait` (RedXenon) + sibling `GameActionDie` (coordinator); 5 others checked, false alarms** | done | RedXenon + Coordinator |
| T-038 | **DONE (01554a2/a023a62)**: Year has a consumer but `totalSeconds=0` makes it inert — `clock` stays compared, no `year` field; spawned T-042 | done | WildYak |
| T-036 | Helper-theft taxonomy — **DONE (38118cb)**: the 998 was two-route arithmetic; artifact floor 221 (8.9%); T-029 denominator understates | done | SwiftRaven |
| T-037 | Script-native doors — **statics DONE (0160a9c)**: ctor IS the record; sizes 0xC8/0x24/0x3C; spawned **D-3/D-4** + ledger 7 | done | WildYak |
| T-029 | IDA function census — **done: 8,876 game fns of 10,173; 46.7% named / 26.4% typed-stub / 26.9% unnamed** | done | IronStorm (research) |
| T-004 | Test coverage: EMAP v5 fog/ambient byte stores | done | implementer-B |

| T-005 | GUI: `.int` reader + widget tree with name lookup | done | implementer-B |

| T-006 | GUI/SS: `Format_0x56be30` `<tag>` markup + Label text | done | implementer-B |

| T-007 | GUI skin sub-records + texture binding — **DONE**: steps 1-2 (`06ac911`, G1 = GUI::Fragment 0x158 / vft 0x66ea1c slot 7 = 0x4b20a0; G2 = Gfx_Engine@0x707CE4 + slots 49/54/55 semantics, byte-verified) + steps 3-4 PORTED (openje `a166dfa` / parent `2b17631`): ReadFragmentRecord byte-exact seam, TextureSlots/BindFragmentTexture/ResolveSkinTexture as BEHAVIOUR (probe-skip / demand / clamp-still-releases / gate-clears asserted as separate shapes, chains NOT unified), +0x2C parse-and-store, broadcast as literal COPY, key-form left open for T-013; gates: 10/0 run-all-tests (GUILoading 187>=116) + cppcheck 0 + msvc rc0 + gate-integrity rc0, independently confirmed by GoldYak's 14:34 fresh-payload run | done | KeenStorm `01a15f83-3b1a` |

| T-008 | Onboarding: mesh + build + full gate green | done | implementer-C (UltraBear) |

| T-009 | `VFX_Parameter` section/entry iterator (unblocks T-007) | done | implementer-C |

| T-010 | Research queue — **DONE (checkpointed by coordinator `01a15f82`)**: 10a answered, 10b resolved-as-defect, 10c CLOSED via T-018, 10d closed, 10e CLOSED (`+0x14` has no non-zero producer ⇒ `meth_0x5a9330` dead, mirror-and-cite constants) | done | research (+PureKnight `01a15cb7`) |

| T-011 | Combat: per-turn dispatch + join-on-attack — **steps 2-6 LANDED (`20827b2`)**; Q1 **CLOSED** (raiser = entity's own update, ported at T-048 `af1da87`; +8 owner = MANAGER DB-closed) — re-entry DEPTH routed to lockstep with probe seam deferred as **T-050**; Q2 **CLOSED statically via T-010 10e** (no non-zero producers for +4/+0x14; dead-walk = mirror-and-cite, feeds T-051); Q3 tails cited-absent, unchanged | in-progress | coordinator `01a15cb6` |

| T-012 | **P1** Combat: replace the falsified from-behind rule (3π/4 is proven wrong) | done | implementer-C (UltraBear) |

| M-001 | D3D8 slice 1 — **PROBE ANSWERED: headers compile, `d3d8.lib` ABSENT** → blocked on a human decision | blocked | implementer-B |

| T-013 | Engine texture registry (`cls_0x55dc80` @byte_70B200) = T-007 tail — **NARROWED by T-007 G2 §d (committed)**: id=node+0x0C, name string node+0x10, usecount obj+44, intrusive list head mbr_0x28, id→object map manager+0x24/0x28; REMAINING: runtime key string form (stem vs name.tga — lockstep), Fragment+0x2C dword meaning, +0xDC/+0xF8/+0x114 broadcast targets | open (partially answered) | implementer-B (after M-001 slice 1) |

| T-014 | Who sets the sub-attack record's +0x19 "marked" byte (volley-batch loose end) | done | FastArrow (research) |

| T-015 | GWAM record map — **DONE (full remainder closed)**: slot table definitive per doc §1 (**22 wire reads = B11 §1 canon, re-affirmed against B9's "21 fields"** — string does NOT clobber #18/#19), creature twin grammar-identical with 2 real deltas (mode slot NEVER stored ⇒ stack residue on creature paths; list base +0x6F8 vs +0x40C), **attack-mode records are LISTS** (0x40c979 intrusive append), style map 1..5→1/6..8→0/9..10→3 + #17→2 override with uninitialized-mode hazard flagged; T-019 FINAL on corrected pair | done | PureKnight `01a15cb7` |
| T-016 | Runtime batch — **read 3 CLOSED statically** (INI is plaintext inside `Engine.grp`; existence-gate = no `none.map` safety net) | done | RedXenon (01a157ec) |
| T-017 | F3 UI surface: Command* -> GameInterfaces slot / root-Open mapping — **DONE (pass 4 closed the queue)**: blk block (vft `0x656650`, 0x64 B) allocated by the lazy UI-cluster bootstrap `JE::Client::virt_meth_0x58f010` via client-vft+0xB0 factory (`0x45e8e0`), install store `0x58f1f5` (outside pass-3's band — its census was bounded, not empty), ctor builds the 16 named windows; 11 handlers all classified (5 Options commands = selector thunks into SHARED `0x45ed10` panel arg — one Options window, not five; UseSkill = dialog iface client+0xA4, not a blk window; Actionbar surface serves BOTH Feedback+Actionbar commands on +0x10); Feedback-vs-Actionbar name interleave flagged MED, not guessed | done | PureKnight `01a15cb7` (was HappyWolf) |
| T-018 | **DONE (92461cd)**: helmet `GameItem+0xA8` required-slot vector = **(1)** — STORED by the GITM parser `sub_410b80` (count `0x410beb`, values `0x410c19`), consumer `Equip_46be40` -> B8 pair table; head-family naming stands, T-010c closes; side finding: gitm.csv `equip`/`eqslot` = req-count/reqvec[0] mislabels (verdicts unaffected) | done | PureKnight `01a15cb7` |
| T-021 | Retire the anatomy wording 10c disproved (rename + comments, no link needed) | done | implementer-C (UltraBear) |
| T-020 | **DONE — three-source CONFIRMS {33,34,37,38,40,41}, 39 unreachable** (Implementer-C derivation + PureKnight cold byte re-read + coordinator dd/objdump re-check); §5b pair wording and the 40/41-death-boundary premise both falsified and corrected in the T-010a doc; port tables were already right | done | PureKnight `01a15cb7` |
| T-019 | Consumed+thrown FatalError on shipped data — **FINAL (T-015 pass)**: guard = independent u8s at payload +54/+55 (B11 §2 mapping refuted); **0/57 trips unconditionally**; custom-record rule byte-exact: trip iff p54!=0 && p55!=0 | done | PureKnight `01a15cb7` (was FastArrow) |
