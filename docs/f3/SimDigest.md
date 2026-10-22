# T-026 — the per-frame simulation digest

Code: `src/F3/SimDigest.hpp` (header-only), `F3::CombatManager::TurnOrderSignature`,
`JE::Host::{EnableSimDigest,DumpSimState,CloseSimDigest}` in `src/F3.cpp`,
comparator `tools/diff-digest.sh`. Tests: appended section in
`modules/openje/tests/je/PathManager/PathManager.cpp` (it uses that row because that row already
compiles `CombatManager.cpp` + `MapEntitySpawn.cpp` + `CombatMath.cpp` together, and no lane may
edit `run-all-tests.nix`).

## What it is

One line per sampled frame of *game state decided independently of whether anything is drawn*:

```
frame=4 tickMs=64 clock=4 ents=2 e0=DIGEST_A,pos=0.00/0.00/0.00,yaw=0.00,hp=42/45,st=4,0,0,q=1,ak=GameActionMoveTo/-/-/- \
  e1=DIGEST_B,... pathJobs=0 turns=2 turnOrder=DIGEST_A,DIGEST_B draws=2 drawsSrc=injected-counter \
  fnv=7E1C8ABB
```

Named fields, not just a hash — a hash tells you two runs differ, a field name tells you **what to go
look at**. `fnv` is FNV-1a over the field text for cheap whole-line equality.

Run it:

```sh
OPENVB_SIM_DIGEST='Z:\tmp\run.digest' OPENVB_SIM_DIGEST_EVERY=500 wine build/F3.exe 'C:\f3data'
tools/diff-digest.sh run-a.digest run-b.digest      # 0 identical, 1 diverged, 2 usage/IO
```

Default **off**: nothing opens a file unless the variable names one. A game loop that silently writes
files is a fidelity change, and the shipped loop must stay byte-behaving.

## Why the sampling interval exists

Measured, not guessed: the game loop is uncapped, so an unthrottled 10-second boot wrote **923,975
frame lines (260 MB)**. With `EVERY=500` the same run wrote 1,578 lines (626 KB). Frame *number* is
therefore not a cross-run time axis; `tickMs` and `clock` are. A frame-synchronous comparison (T-028)
must either pin `EVERY=1` and cap the run length, or align on `tickMs`.

`# END frames=N every=M` closes the file, but only on a clean `Shutdown()` — a run killed by `timeout`
has no END line, so a missing END means "killed", not "corrupted".

## The allowlist, and why these fields

| field | source | note |
|---|---|---|
| `frame` `tickMs` `clock` | caller / `Host::TickMilliseconds` / `GameWorld::ClockSeconds` | see the time-base note below |
| `ents`, `eN=name,pos,yaw,hp,st,q,ak` | `F3::g_spawnedEntities[128]` | `hp=statCur/statMax`, `st=entityState,moving,acted`, **`q` = queued-action COUNT** (see the T-039 section), `ak` = the 4 action-queue slots — this *is* the per-entity action queue in our architecture |
| `pathJobs` | `JE::PathManager::PendingJobCount()` | world+0xF0 == the steerer |
| `turns`, `turnOrder` | `F3::CombatManager` | **order-sensitive on purpose**: who-acts-when is the gameplay fact, so the same actors in a different order must differ |
| `draws`, `drawsSrc` | T-027's `RandFn` seam | see exclusions |

`kMaxEntitiesInLine = 12` caps the line, and when the cap bites the line says `entsMore=1`. A stable
hash over silently truncated state is exactly the thing this row exists to prevent.

## Exclusions — the judgment call, made in the output

Excluded (presentation-side): **reaction-anim id**, Gfx texture slots, GUI fragments, voice roll,
effect rolls.

The hard one is the reaction id, and it is *not* a clean exclusion. One global sequential LCG feeds
gameplay **and** reaction (`docs/f3/CombatRngSeam.md`: 18 draw sites, **8 of them inside `sub_45C140`**,
the very function whose stack residue is shipped-defect ledger entry 5). So the reaction outcome depends
on how many draws damage consumed earlier in the frame. Excluding the reaction id while *not* counting
draws would leave that coupling in the digest unnamed — which is worse than either including it or
saying plainly that it is out. Therefore:

- `draws` is **IN**, and `drawsSrc` says where the number came from.
- the reaction id stays **OUT**, and the header line states the coupling rather than implying separation.
- the shipped path (`F3.exe`) prints `draws=NA:no-counter-on-global-stream`, because the generator has no
  counter and adding one means editing `CombatMath.cpp` — a file another lane was editing at the time.
  **It never prints `draws=0`**, because 0 is a false statement about state, not a missing seam.

`NA` in this digest means *the seam is not ported*. It never means *unchanged*, and every header prints
`# SOURCES combat=… paths=…` so a half-wired run cannot masquerade as a healthy one.

## What it proves, and what it does not

Proves **self-consistency**: same input → byte-identical digests; a simulated input change → divergence
at the right frame, naming the field (`e0.hp: [42/45] -> [39/45]`).

Does **not** prove parity with the original. There is no shipped-side digest until a bootable `F3.exe`
can be driven frame-by-frame — that is T-028, and the digest says this in its own first lines so a
reader with only the log file cannot over-read it.

## COMPARED vs CONTEXT (added while building the T-031 grammar)

The first digest format hashed `tickMs`, which meant **two runs of the same session were never
byte-equal** — fatal for an oracle whose whole job is comparing a replay against a capture. The format now
splits the line:

```
frame=20000 clockDelta=0 ents=3 e0=...ak=-/-/-/- pathJobs=0 turns=0 turnOrder=-   draws=NA:no-counter-on-global-stream fnv=D7239AA8 ctx: tickMs=34540789 clock=34540
```

- **COMPARED** = everything before ` fnv=`: frame, `clockDelta`, entities (name/pos/yaw/hp/st/ak),
  pathJobs, turns, turnOrder, draws.
- **CONTEXT** = everything after ` ctx:` (`tickMs`, absolute `clock`): printed, captured, deliberately not
  hashed, and promotable later by editing the comparator — never by re-running a session.
- `clockDelta` (seconds elapsed since this run's first tick) replaced the absolute `clock`, because the
  shipped frame-0 leap makes absolute clock uptime-shaped and therefore incomparable across processes.
  It is captured on the **first tick, not the first written frame** — with `EVERY>1` those differ, and the
  off-by-one was measured (two boots reported `1` then `0` for the same sampled position).
- `ak` prints **class names**, not our kind digits: ours come from `ActionKindName()` (a mapping we own),
  the original side will map `[action+0x00]` vftptr → class through `artifacts/F3.json` and print
  `UNKNOWN_VT=<ea>` plus a count when unresolved. Comparing pointers would compare allocator history;
  dropping the field would leave the shipped action queue permanently unverified.

Verification, on real data rather than in a unit test: two separate boots of the app, sampled at
`EVERY=20000`, now report **`note: line N differs only in CONTEXT fields (not a divergence)`** for every
aligned frame — the state comparison is clean across processes. The one remaining difference is the trailing
partial line (`clockDelta` 2 vs 1), which is inherent to sampling an **uncapped** loop by frame index and is
stated in the header: cross-run elapsed time is only sound when the clock is *supplied*, which is T-028's
pinned-clock driver. The suite also carries the CONTEXT falsification directly: same state at two different
uptimes must hash **equal**, while the printed lines must still **differ** — so a future edit that folds the
CONTEXT span back into the hash fails a test instead of quietly making every replay look divergent.

## PARITY= — where the claim stands now, and what it does not mean

`PARITY=not established. The shipped side has a PRODUCER (`tools/orig-digest.sh`, grammar in
`docs/f3/OrigCapture.md`) and no VALUES: every artifact it writes today is labelled
`# SYNTHETIC`, and the comparator refuses to compare a synthetic side against evidence
(`MIXED-PROVENANCE`, exit 3). So the digest row is no longer the only place that says "not parity" —
the shipped side says it too, in its own header, and the boundary is the same on both files:
same-state parity under a pinned LCG state, never same-seed-from-clock parity.

## T-039: `q=` replaced a boolean that was never a boolean (correction, not a feature)

The `st=` triple used to carry a fourth value: `busy0x314`, a `unsigned char` in
`src/F3/MapEntitySpawn.hpp` named after a *busy flag*. It is not one. Shipped `Entity+0x314` is the
**live element count** of the block deque at `Entity+0x304` (DB `mbr_0x314`, size 4; the deque's own
`mbr_0x10`, and `0x304 + 0x10 = 0x314`), whose writers are `++` at `0x54ea71` (the push_back funnel
`0x55072e` calls) and `--` at `0x54b637`/`0x54b672` (the erase that walks it), zero-initialised at
`0x5515e6`. Readers agree: `Entity::virt_meth_0x4084c0` is literally `return Entity+0x314 == 0`, and the
pump loops at `0x54c447`/`0x54c788` bound themselves by `head != head + count`. Full write-up:
`docs/f3/EntityActionQueueSeam.md`.

Why this row owns the fix rather than the gate row: **a hashed comparison field must mean what its name
says.** Printing a depth inside a `st=` state triple made the digest claim "this entity is busy (1)"
where the shipped state is "this entity has N orders queued". And a byte cannot hold the number that
matters — 256 aliases to 0, i.e. *idle*, which is the worst possible value for a parity oracle to read
as a fact. So:

- the field is now **`q=`**, printed as a count, inside the same comma-chained `eN` token (so
  `tools/diff-digest.sh` names a divergence as `e0.q`, not `e0.st`);
- `st=` is three values (`entityState,moving,acted`) — a fourth means the old byte crept back, and the
  PathManager suite asserts the exact substring `,st=5,0,0,q=4,` to keep that true. **Since T-048 the
  third value is no longer decoration: the entity pump now raises the +0x319 latch exactly where the
  shipped `Entity::virt_meth_0x54c570` does (`0x54c833` — an action processed this tick that did not
  finish; the `0x54c864` budget-exhaustion store is unexpressible in the port and cited-and-absent in
  `MapEntitySpawn.cpp`). So `st=…,1` now APPEARS in runs where something defers, and stays `…,0` where
  nothing does — which is the shipped shape; a digest diff against the original must compare that byte
  as live data, not noise. The PathManager T-048 block pins the raise, the no-clear-on-drop, and the
  `,st=4,0,1` line.**
- the port **derives** it from `actions[]` (`SpawnedEntity::QueuedActionCount()`) instead of storing a
  second number that must be updated at every push and at each of the five auto-clear sites in
  `MapEntitySpawn.cpp` — a derived count cannot disagree with the queue it describes;
- **D-2 became visible, which is the payoff.** A derived count cannot exceed 4 because our queue
  cannot; the shipped `mbr_0x10` grows past one block. So a parity diff showing shipped `q>4` against
  our `q=4` is deviation D-2 *firing* — a dropped order — rather than a digest artifact. That is the
  same quantity T-034's R3 reads live (`Entity+0x314`), so the digest row and the debugger row now
  measure one thing with one name.

The companion field rename in the same commit: `noTurn0x1E4` → `flag0x1E4`, `unsigned char` →
`unsigned int`. `mbr_0x1e4` is size 4 and its *meaning* is unresolved (its readers gate per-map entity
counts at `0x54bc3a`), so the port keeps a neutral name and the question stays on the board instead of
being answered by a field name.

## What the digest found on its first run (routed, not fixed here)

`clock` behaved like this in an app run:

```
frame=0 tickMs=29242857 clock=29242      <-- the world clock is already ~8 hours old at frame 0
frame=1 tickMs=29242857 clock=29242
```

Mechanism, read from our own code: `World::Update_0x544a90(ms)` computes
`delta = ms - mbr_0x48` (`modules/openje/src/JE/World.cpp:119`), `mbr_0x48` initialises to **0**, and
`ms` is `Host::TickMilliseconds()` = `timeGetTime()` = **machine uptime**. So the first pump folds the
entire uptime into `mbr_0x44`, and the step-6 carry (`World.cpp:99-106`) adds ~29,000 world seconds in
one frame. The value then sticks (only deltas accumulate), so the sim runs with an 8-hour-old clock.

**SETTLED without a debugger** — see `docs/f3/WorldTickPump.md`. The pump is
`JE::Server::virt_meth_0x552b60`, which calls World **vftable slot +24** with
`JE_TickMilliseconds_0x56af50()` straight into the argument (0x552bad → 0x552bb5): `ms` is absolute
machine uptime, just like our `Host::TickMilliseconds()`. And `JE::World::virt_meth_0x546690` (the
world-entry path) zeroes `+0x48` at 0x5466b5 while seeding `+0x4C` (0x5466bd) and `+0x54` (0x5466c8) with
the tick. So the original computes the same `uptime - 0` on its first `Update` and leaps the same distance.
**Our behaviour is faithful — mirror it, do not "fix" the stamp.** For the parity oracle this is good news:
`clock` is a valid comparison field, because both sides start from the identical leap, so a `clock`
divergence reads as elapsed sim time rather than an epoch mismatch.

---

## T-079 batch-1 correction (2026-10-22) — this page's errata from the T-072 sweep (S1–S4)

Per `discoveries/2025-10-21-t072-doc-errata-sweep.md` §1/§4 (SwiftMoon). Stale sentences above stay
visible per the board state rule; this block referees them. Byte claims re-verified against
`$OPENVB_ORIG_EXE` at APPLY time (T-079 lane; receipts in `tasks/T-079-batch-doc-correction-apply.md`).

- **S1/S2 — WRONG, reversed by T-045** (the "SETTLED without a debugger" paragraph above, incl. the
  two sentences "ms is absolute machine uptime" / "Our behaviour is faithful — mirror it").
  `JE_TickMilliseconds_0x56af50` is NOT `timeGetTime()`: image bytes `a1 b8 c2 70 00 c3` =
  `mov eax,[0x70c2b8]; ret` — a WinMain-seeded **session-elapsed accumulator** (sole writer
  `ProcessMessagesAndUpdateTime` 0x56afb8..0x56afd7). `Update` is entered `this = X+4`
  (0x552ba4 `lea esi,[eax+4]`) and subtracts primary **`+0x4C`, which the world-entry path SEEDS**
  (`0x5466b8 call 0x56af50` → 0x5466bd `mov [esi+4Ch],eax`). The ~8-hour fold at frame 0
  (`clock=29242`) is a **port artifact**; fidelity is *seeding the stamp*, the opposite of
  "mirror the leap, do not fix it". Authority: `docs/f3/WorldTickPump.md` §Conclusion.
- **The magnitude qualifier** (corpus reconciliation, §1): "the frame-0 leap" names TWO different
  quantities in the corpus. The **FOLD** (~8 h uptime) is ours (T-045); the small **0s→1s
  seconds-carry** in the first pumps IS shipped (T-056 R2, live-confirmed; 19–21s boot-offset
  character at the R1 instant); T-068: the carry is ported+correct in the app path but
  UNOBSERVABLE in the harness (driver clock starts 0). Every older "leap is shipped / not
  shipped" sentence on this page predates the split and must be read against it.
- **S3 — SUPERSEDED by T-056/T-068** (the `PARITY=` section: "every artifact it writes today is
  labelled `# SYNTHETIC`"): T-056 delivered the six values and T-068 rendered the first real
  `lockstep ↔ unrecorded` artifact (`tools/scripts/orig-capture-t056-runH.txt` @27d8e86; real-pair
  pin openje 8e0568b). Remainder still true: the full per-frame shipped digest awaits R5/R6
  (T-067).
- **S4 — re-expressed by T-070** (T-039 section: shipped `q>4` vs our `q=4` "is deviation D-2
  firing"): shipped `q` includes the idle RESIDENT, so the capacity comparison is **3 orders +
  resident**, shipped `q` at rest reads **1**, never 0, and T-056 never observed >4 pre-crash.
  D-2's ">4" question re-expresses under T-071 step 4 (digest upgrade owns it).

