# F3 reimplementation — progress and coverage index

Maintained by the coordinator. This is the **durable answer to "how much is left"**, so the number does not have
to be improvised in chat. Two rules: every figure carries the command that produced it, and a number is quoted
**with its name** because the measures below count different things.

Re-measure: `run-all-tests` (stamp first), `tools/gate-integrity.sh` (payload freshness), census recipe in
`discoveries/2025-10-20-session37-t029-function-census.md` §5.

## Coverage, three honest ways

| measure | value | what it counts | produced by |
|---|---|---|---|
| **Shipped function census** | **8,876 game functions** of 10,173 total | all code behind the link-tail boundary `0x5FD000` | T-029 (`list_funcs` 7 pages, integrity-checked; F3.json join) |
| — semantically named | 46.7% | functions whose name says what they do | T-029 |
| — typed-stub | **26.2% after rule R36** (route A 2,340 / route B 2,473 before it) | T-036 closed the question: the "998 helper-theft noise" was **not noise**. Artifact floor = **221 (8.9%)** = 73 DB EAs that are not IDB function starts + 48 link-tail + 100 route conflicts; 777 of the 998 are real methods. The 1,475/998 split does not reproduce under either join — **quote the route with the number**, and do not rank classes on contiguity (rho −0.013/+0.248, and vacuous for 464 of 647 "nameable" classes with ≤2 functions) | T-029 + T-033 batch-1 + T-036 `discoveries/2025-10-20-t036-helper-noise-taxonomy.md`, `tools/t036-noise-taxonomy.py` |
| — unnamed | 26.9% | 2,177 `sub_` in 1,971 owner neighbourhoods | T-029 |
| **Dispatch tables** | 2,156 vftable slots, **97.9% resolved** | virtual-call structure | T-029 |
| **DB vs IDA agreement** | 97.4% | 171 declared methods are tail/mid-function vcall targets, not lost code | T-029 (artifact class recorded) |
| **Our port** | 31 hand-ported headers, 41 `.cpp`, ~7.8k implemented LOC | what we have written | `ls src/F3/*` |
| **DB class coverage** | 31 of 1,101 classes ≈ 3% | class *count* only — NOT the census above | `artifacts/F3.json` |
| **Host gate** | 10 suites pass / 0 fail, floors 11/74/111/20/13/6/116/80/43 | behaviour we can actually assert | `run-all-tests`, quote its `GATE STAMP` |

**Do not conflate the two coverage numbers.** ~3% is class-count against a 1,101-entry IDA database; 46.7% is
named-function share of 8,876 game functions. Both are true and they answer different questions.

## What is solid: the simulation loop

Frame control flow and combat math are ported and asserted, not sketched:

- `CombatMath::CalculateDamage` (`0x413310`) / `CalculateUnarmedDamage` (`0x409f90`), byte-derived from decompile.
- Session-7 damage-apply pipeline in `GameActionAttack`: DEAD gate → HP clamp → hit angle → reaction anim →
  `.veg` → voice roll → `OnDamage` → `GameActionDie` queued.
- `MapEntitySpawn` pump: kind 4 (attack), kind 5 (die), `OnDeath` DYING→DEAD (6→7) server-gated edge.
- **`JE::Entity::meth_0x550720` — the single gameplay-order funnel** (`docs/f3/EntityActionQueueSeam.md`, T-030).
  40+ xrefs: every `GameAction*::OnServerUpdate` re-queue, script natives `F3::Attack`/`JustAttack`/
  `StartDialogue`/`ReloadWeapon`, the server packet path (4 sites), `sub_45C140` reactions, `GameProjectile`,
  effects. Orders arrive here or not at all.
- Shipped RNG mirrored: LCG `sub_48CCC0` (`1664525·state + 1013904223`, `(state>>8)&0x7FFF`, seeded once from
  `time32` behind `byte_707C64`) — 18 draw sites in 10 functions. Deviation **D-1**.
- `SimDigest` (`docs/f3/SimDigest.md`, T-026): per-frame state digest, `PROVES=self-consistency`,
  `DOES NOT PROVE=parity with F3.exe`. Note the app loop is **uncapped** (~924k frame lines / 260 MB per 10 s):
  `frame` is not a cross-run time axis, use `tickMs`/`clock`.
- Command FIFO at `client+0x1F0` carries **no gameplay verbs** (0 of 134 commands) — UI/camera/toggles/engine.

## What is not: presentation and the content systems

**Simulation** = everything that decides game state independent of whether anything is drawn. Operational test:
a headless run must produce identical state to a run with a window.

- **Presentation is unverified, and a visible frame is measurably impossible on this host.**
  `tools/render-device-probe.sh` (three cases + boundary in its own output): no `DISPLAY` → exit 1
  (`nodrv_CreateWindow`); `Xvfb` + device off → 124 alive; `Xvfb` + device **on** → exit 1 with
  `Failed to find a suitable pixel format` / `Failed to get a GL context`. Software GL and a virtual X server were
  both tried and neither helps, so **slice 1's visual acceptance is a harness limit, not our code**. The verifiable
  part here is that the device-creation path fails as a logged no-device state instead of crashing. `d3d8.lib` was never missing — `vcvars32.bat` omits the two `prerelease` dirs, appended by
  `msvc-sdk-prerelease.bat`.
- **No parity oracle yet** (T-031): every check we own proves self-consistency. This is the highest-leverage row on
  the board and it also settles the time-base question (`ms` passed into `World::Update_0x544a90`, which has no
  direct caller — vftable xrefs `0x668584`/`0x6919fc`; the jump is structurally present in the original too).
- **Two input doors we still lack**: the script natives and the server packet path, both of which land on `0x550720`.
- **Biggest untouched systems:** items/inventory, effects/VFX, AMX, dialogue, party management, save/load.
  Named unported/deferred markers on the board: 57.
- **T-025 census generator** (in flight) and **T-032 EA→document index** exist to make the above numbers
  automatically re-measurable instead of re-derived by hand each time someone asks.

## What `artifacts/F3.json` actually is (T-040, `c204c51`)

A **frozen OOAnalyzer v2.2.0 export dated 2024-04-04**, not a live index. Producer identified by fingerprint rather than
by memory: the IDB's own `.id0` carries 11 occurrences of `_PyOOAnalyzer__json_filename` = `Y:\F3.json` and the full JSON
text under `_PyOOAnalyzer__json_data`, and the sibling archive's `F3.json` md5 `e8827f12560f…` **equals our checked-in
file** (coordinator re-verified) — so our 2026-05-15 mtime is a *relocation* date, not a generation date. Stage 2
(`ooprolog`, Java) is gone from this machine and the installed OOAnalyzer can't export facts, so **an IDB rename cannot be
flowed into this file here.**

**The definitional consequence, which re-reads every number above:** keys follow *"a class owning an RTTI-named vftable is
keyed by its mangled name; everything else is `cls_0x<cea>`."* So **`cls_0x…` does not mean 'we failed to identify it' —
it means OOAnalyzer found no RTTI-named vftable in 2024.** Batch 1's "name the anonymous classes" framing should be read
against that: some of the 698 may simply have no RTTI to name them from, and the 403/698 split is fully explained without
anyone's effort being missing.

Regression is now caught mechanically: `f3-json-check` (devenv script) re-derives class keys, class sizes, method sets,
member sizes and usages — **1,101/1,101 keys, 7,736/7,737 member sizes, 0 usage mismatches** — refuses to write the
file, and declares the 4,059 synthesized base/struc/vftptr entries **UNVERIFIED** rather than passing them. Two rules it
pins, both of which would have wrecked a guessed exporter: method EAs come from `finalClass`'s 6th argument (not
`finalMethodProperty`, which covers only 3,384 of 6,624), and member size is `max()` over candidates (first-element gave
143 wrong). **But do not overshoot that finding.** RTTI absence explains *why a key looks anonymous*; it does **not** mean
"no class here is nameable." 43 of them (`cls_0x49c610`) were named in the IDB all along and are nameable right now on
route A. The honest reading is: **some** of the 698 may have no RTTI to name them from, and identity must come from
evidence — embedded member types, vftable shape, shipped data — never from a key format. If the coverage table starts
reading as "lower the bar," it is wrong.

**Measure against the live IDB; quote the export as the 2024 snapshot it is.**

## Corrections to my own propagated claims (2026-10-21)

- **"Order classes by contiguity ratio"** — withdrawn as a filter. The ratio measures **link placement and compiler
  clone spread**, not attribution quality (`cls_0x49c610`'s 0.02 is five near-duplicate copies of one ~0x355-byte
  method, at 93.3-93.7% identity differing only in SEH immediates — **not** byte-identical, a detail
  I propagated from the lane's earlier draft before their own normalizer falsified it). Body evidence is nearly identical across low- and high-ratio classes (81% vs 88%), and the
  "1,475 nameable + 998 helper-theft" split **reproduces under no single join route** (IDA-name = 2,340; DB-method =
  2,473, 2,425 in the game region). Quote a route with any attributed count.
- **"13 self-naming classes are the cheap win"** — corrected by batch 2: five of the thirteen are MSVC **derived-class
  base subobjects** (49-byte `CEngineInterface` embeds at non-zero offsets). Those take an annotation
  (`derived@CEngineInterface`), not a name; minting the base would have poisoned five decompile trees. Evidence-ordered
  ceiling on tier movement is **<=1.3pt**, which is why the pre-registered prediction was voided and re-registered
  *before* measuring. I had put that claim in front of three lanes.
- **`docs/PROGRESS.md` did not exist until today**, though three rows cited it, and a readiness probe plus two shell
  packages landed inside another lane's commit (`c925bf6`) because my own commit half-failed. See harness items 18,
  25, and the sweep rule in 25(b).

- **The 998 "helper-theft noise" figure is withdrawn outright (T-036, 38118cb).** It was arithmetic across two join
  routes: batch 1 subtracted a route-A split from a route-B total, so 998 appears in **no** single-route variant. The
  real artifact floor is **221 (8.9%)** = 73 DB EAs that are not IDB function starts + 48 link-tail + 100 route
  conflicts; **777 of the 998 are genuine methods**. "26.4% -> 16.6% + 11.2% noise" no longer exists.
- **T-029's denominator understates.** Only 49 of its 171 "vanished" methods are mid-function tails; the other **122
  are secondary chunks of multi-chunk functions** (live IDB: `0x4154ef` marked "Not a function", `0x4154f0` ->
  `F3::GameWeapon::virt_meth_0x414680`). `list_funcs` is not a complete entry-point enumeration, so counts derived from
  it are bounds, not totals.- **A named class can be missing from the export entirely.** `cls_0x49c610` (ratio 0.02, batch 1's worst case) is
  `JE::ResourceSystem` — all 43 methods named so in the IDB, 36/43 with in-body member usage — and `F3.json` contains
  **zero** occurrences of `ResourceSystem` (coordinator-verified by grep), while our port ships `JE/ResourceSystem.hpp`.
  Stale export, not misattribution.

## Deviations and defects (the ledger is the list, not a summary)

- `docs/DEFERRED-NOTES.md` — **shipped-defect ledger** (6 entries: null-page; DR-invisible req-slots; 4 console
  commands; `FatalError` consumed+thrown; `var_B1` uninitialised residue; entry 4's twin) plus **port deviations**
  **D-1** (RNG mirrored, and it *removed* a deviation) and **D-2** (`actions[4]` is the shipped deque's *first
  block*, so a fifth insert is refused rather than appended). Standing rule: **mirror and cite, never silently
  improve.**
- Tooling/harness traps that changed conclusions: items 17 (a status read through a subshell is not a status),
  18 (verify a changefile edit by evaluating its consumer), 20 (messenger counter semantics), 21 (send cap),
  22 (a uniform store in the original is a claim about *its* layout, not a licence for ours).

## How to read a claim in this project

1. Which **artifact** produced it — the payload path and table hash from `GATE STAMP`, after
   `tools/gate-integrity.sh` exits 0. A gate that prints no stamp is not the tree's gate.
2. Which **measure** it is (class count ≠ function census ≠ suite count).
3. Whether the evidence is **data, static, or runtime** — `docs/notes/harness-items.md` item 13: data first.
4. Whether it is **self-consistency or parity**. Only T-031 changes that answer from the first to the second.

## Refresh 2026-10-23 (splitseat; every figure with its command)

- **PARK board**: **25 closed rows** — `grep -cE "^\| ~~" tasks/PARK.md`. All remaining open rows are
  seat-held (SwiftOwl/TrueRaven/keeper/EpicRaven lanes), trigger-gated, or need the user (M-001-S2 slice 2).
  Closures tonight: G-FLAKY (0719361+025693a), R-ST (verify+restore), X-PUSH tier-2 wave + successor band
  (a1ae175/2f14403), ST-RENAME (03be695, verify+strike of wave-3), Q-W2b ×2 dupes (cb9ce7a, verify+strike
  of wave-3/wave-4).
- **Live IDB census** (evalfile over idautils.Functions on ida_original_f3): **10,173 functions /
  7,648 semantically named (75.2%) / 1,718 `cls_`-prefix / 2,483 `sub_`-prefix** — the named share is the
  live-DB reading; the T-029 route tables above remain the audited baseline (different join routes, do not
  mix). The 75.2% live number includes this week's tier-2 mint waves (T-084: 129 mints; wave-3: 50 renames;
  std-tier: 5 fns), which is tier movement, not new coverage of behavior.
- **Wrapper transport defect CLOSED at root** (5d2597e, `action=evalfile`): the harness arg-envelope
  corrupts long/quote-dense tool args (field-observed twice); renames/annotations now travel as /tmp
  files, read by the wrapper, applied in one py_eval hop with in-payload read-back. Standing convention:
  complex args never cross the envelope inline.
- `docs/PROGRESS.md` itself was last stamped 2026-10-20; the three days of board motion are the rows
  above. Suite figures were NOT re-stamped this refresh (heavy wine boot) — the run-all stamp from the
  coverage table remains the last audited suite number.

## Refresh 2026-10-24 (executor seat; every figure with its command)

- **PARK board: 38 closed rows** — `grep -cE "^\| ~~" tasks/PARK.md` (was 25). The 13 strikes since
  the last refresh: R-PORT-Q (qi= inbox-era digest grammar, three legs), S3-INJ (all five deep-input
  readings exhausted: T-050 falsifier confirmed live, kind-stream census, third SetCursor caller
  identified, two honest negatives), H-IDX refresh receipt (ea-index current 211 docs / 8,106 EAs,
  recall 0 missed), R-USERRE-A (queue call verified done: Law-#6 SEH triage + WAVE-4 landed), 
  R-KEEPER queue receipt (item-5 sink-proof + slice-2 acceptance both receipted; seat stands), and
  COMMENT-CHAN (the set_comments coercion defect root-caused as lazy-MCP-route-only — the direct
  `ida action=call srvtool=set_comments` route is the recorded convention; W4-R4's three annotation
  rows landed + saved).
- **Live IDB census** (ida action=save census): **10,173 functions / 7,690 named (75.5%)** — the
  +42 named since 2026-10-23 is tier movement (wave-4 renames + the landed annotation-era work),
  same caveat as the refresh above: not new behavior coverage.
- **Standing rows** (both live, not owed): R-KEEPER's seat (rig entry point, re-arms on lane
  window requests) and H-IDX (re-arms on next --check drift). **No actionable open rows remain**;
  the named successors queued for minting: values-era owner-chase 21-row batch (boot data =
  values-s2.log now on disk), P2-surface instantiation (~35 undefined vft virtuals, LNK2001 both
  toolchains, sequence EA-named), FastNova hp=0 fixture (damage routes through the inbox since
  P-CMBT Step 2 — constructible), the cls_0x616328 dual-role band-evidence re-cut, item5b
  probe-line, keyboard-channel injection, ~793 unsighted 801-pool members.

## Refresh 2026-10-24 #2 (executor seat; every figure with its command)

- **PARK board: 47 closed rows** — `grep -cE "^\| ~~" tasks/PARK.md` (was 38 at refresh #1). The 9
  strikes since: PROGRESS-TALLY (refresh #1 itself), HP0-FIXTURE (the hp=0 death-threshold digest
  fixture: seed-pinned 2-dmg push, full death arc in the digest, dieQueued=1 in the
  ApplyDamageEffect path), FLOOR-RELAX (P-CMBT Step 3 landed: the HP write floor RETIRED —
  ApplyDamageEffect writes min(new,max) only, shipped-faithful to 0x4629C0's bytes; the over-kill
  arc holds hp=-3/2 with the `<= 0` death conversion still firing; SpawnMove floor 157→166→175),
  BUSYMARK-171 (P-CMBT design Step 3: the +0x171 effect busy mark — set at fire sites + drain
  tail, cleared once per world step after the pump, Busy0x54ab00() composite predicate; SpawnMove
  floor now 175), and OWNER-CHASE-C1..C5 (the values-era owner chase 21-row batch **COMPLETE
  21/21**: C1 the five-interface switch module confirmed no-owner; C2 the dialogue-list owner
  PINNED to GameServer.mbr_0x5064; C3 the display-settings wrapper band ×7 rows with the
  adapter-array stride 0x44C surface; C4 the map sound/effect group + Bink movie player family
  ×5; C5 the final rows including 3 STALE holds re-censused — the re-census-before-re-hold
  lesson).
- **Live IDB census** (ida action=save census): **10,173 functions / 7,690 named (75.5%)** —
  unchanged count; this refresh's chase cells landed ~20 def-site comments instead (C1..C5, all
  read-back verified + saved, ledgers 02-00-35Z..02-12-51Z).
- **Standing rows** (both live, not owed): R-KEEPER's seat and H-IDX. **No actionable open rows
  remain**; the named successors NOW QUEUED: P2-surface instantiation (~35 undefined vft
  virtuals, LNK2001 both toolchains, sequence EA-named), the cls_0x616328 dual-role band-evidence
  re-cut, item5b probe-line, keyboard-channel injection, ~793 unsighted 801-pool members — plus
  the chase's newly exposed port-era surfaces: the shipped adapter array (byte_708380, stride
  0x44C) and the Bink movie player (cls_0x5b4f50, byte_707CE4 device, movie flag
  global_area_map_icons->[+2C]+0x108).
