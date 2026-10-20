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
frame=4 tickMs=64 clock=4 ents=2 e0=DIGEST_A,pos=0.00/0.00/0.00,yaw=0.00,hp=42/45,st=4,0,0,0,ak=0/0/0/0 \
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
| `ents`, `eN=name,pos,yaw,hp,st,ak` | `F3::g_spawnedEntities[128]` | `hp=statCur/statMax`, `st=entityState,moving,busy,acted`, `ak` = the 4 action-queue slots — this *is* the per-entity action queue in our architecture |
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

Whether that is *our* deviation or shipped behaviour is a parity question, not a fix I should make
inside a tooling row: the original may initialise `+0x48` to the first observed tick, or its `ms`
source may be a per-session timer rather than uptime. Settled by either (a) the ctor of the world
object at the `mbr_0x48` offset, or (b) one lockstep read of `+0x48` before and after the first
`Update` call. Logged in `docs/DEFERRED-NOTES.md` and routed to the coordinator; every digest comparison
until it is settled carries this as a shared constant, not a bug in the comparison.
