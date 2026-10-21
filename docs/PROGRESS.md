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
| — typed-stub | 26.4% (restated) | 2,473 attributed = **1,475 nameable + 998 helper-theft noise** — raw counts overstate class mass; order by contiguity-ratio × fns | T-029 census + T-033 batch-1 adjudication (01a157f6) |
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
