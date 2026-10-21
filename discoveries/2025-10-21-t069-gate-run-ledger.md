# 2025-10-21 — T-069: T-051's held GATE run + paired against the T-056 live six-row capture

CalmRaven, dispatched by LoudDragon `01a15f82` (`d283ce3`). Build/test lane — **no IDB access at all**.
Evidence: `discoveries/2025-10-21-t056-xorg-dummy-six-rows.md` (run H, private prefix, Xorg-dummy :96).

## 1. VERDICT

**The pins HOLD against the live capture. One PIN WORDING was wrong and is corrected** — not by the
capture, but by the capture forcing me to state the pins' scope precisely: T-051's own row text claimed
T-048's `+0x319` store "is exercised ONLY through the walk branch — reachable in tests, unreachable in
shipped-config". That is false as written, and T-051's OWN suite already disproves it. The capture
consistent-both-ways: R2/R3 are free-run phenomena and neither exercises the dead branch.

## 2. Gate law executed (fresh payload, stamp from THIS run)

| leg | result |
|---|---|
| `msvc-build` (required: a header moved — `src/F3/MapEntitySpawn.hpp` comment) | **exit 0**, `Built target F3`, no `error C`, no `LNK` |
| `run-all-tests` on the NEW exe (17:32:19, 57,344 B) | **10 pass, 0 fail** — 9 suites OK + `app-boot` exit 124 (alive past 6 s) |
| assert floors | cls_0x4d8d70 11≥11 · **PathManager 117≥117** · ResourceSystem 132≥111 · StringSystem 27≥20 · ActionMoveTo 13≥13 · Scripting 6≥6 · GUILoading 197≥116 · SpawnMove 88≥80 · CombatMath 43≥43 |
| `sim-driver-verify` | **PASS** (22 checks), incl. `sim-driver SetRoundMode call sites (want 0) → ok`, `branch trace line present and names free-run → ok (combat branch: free-run)`, `T-051 walk-arming guard → ok (no round-walk trace; free-run only)` |
| stamp | `2026-10-21T21:32:26Z`, HEAD b029da0 / openje 653fa79, table hash 483599247, compiler i686-wingw32-g++ 15.3.0 |
| **TEST-ONLY arming audit** | **0 new arming sites**; every one of the 7 `SetRoundMode(1)` call sites in the suite carries the `TEST-ONLY walk arming (dead in shipped, 10e/045a7ec; T-051 annotation)` comment (PathManager.cpp:437/476/507/509/595/621) |

## 3. Per-datum pairing (capture ⇄ port)

| capture datum | live value (run H) | does it test the pins? | verdict |
|---|---|---|---|
| **R0** World vft slot-6 target | `0x00544A90`, identical to T-045 static, 3 boots | No — world pump, not the manager | CONSISTENT, live-confirms the documented assumption |
| **R1** LCG first return | draw `0x2D81`, state `0x65AD8170`, gate `0x01` | No | CONSISTENT — and per banked rule **comparable only as STATE-at-instant**; never quoted as a cross-boot constant |
| **R2** `[[0x70BD98]+0x3C]` → `Gregorian::totalSeconds` | **frame-0 pump 0s → pump#3 1s**; 21s at the R1 instant | **This is the datum named in the dispatch.** It is the WORLD clock (`world+0x3C`), consumed by the free-run pump. The manager's own round accumulator `accMs_` (`+0x10`) stays 0 in free-run — asserted `fresh.AccMs() == 0u` twice. So the leap happens **without** the walk branch. | **NOT a contradiction.** Pin holds; port mirrors both clocks. The `0s→1s` leap is shipped behavior, and R2 is exactly the datum the CLOCK-CAVEAT (chat C-23) fed from `Gregorian::TotalSeconds()` — **now live-confirmed** as the shipped source shape. |
| **R3** deque `Entity+0x304`, count at **`+0x314`** | `0,0,0,0,0` → head 0/count 1; deepest **1→2→3**; 47 funnel hits at `0x550720` | **Named in the dispatch.** Queue DEPTH growth is the action deque filled by the enqueue funnel — reachable by any script caller, walk or not. It is not the `+0x2E0` animated-event queue the raiser's second shipped branch reads. | **NOT a contradiction.** Pin holds. Per the capture's constraint: **no claim beyond depth 3** (`>4` unavailable, so D-2's count>4 question stays unanswered) and **no dropped-order datum**. |
| **R4** back-link `[action+0x14]` | pre `0x00000000` (12/12), post = owner entity ptr (35/35) | No | CONSISTENT; answers WildYak's garbage-vs-zero for this path — port's slot bookkeeping needs no garbage-init |
| **R4b** slot vfts | `0x6497C4`=GameActionIdle, `0x6A6414`=ActionAnim, 0 UNKNOWN | No | CONSISTENT; **Attack never fired** → **no combat-round claims** (deviation note honored) |
| **Rotation depth / re-entrancy** | — | **Would** test the walk, but the bink-upload SIGSEGV ends the run 2.5–4 min in, before interactive combat | **BLOCKED-ON-RUNG-2, not blocked-on-tooling.** The capture neither confirms nor refutes a live second `meth_0x5a94a0`; T-050's gate stays shut with that honest sentence attached. |

## 4. The correction (pin changes; the capture is cited, not the reverse)

**Wrong as written** (T-051 row + `docs/f3/CombatTurnDispatch.md` × 2 + the `MapEntitySpawn.hpp` field note):
that the `+0x319` raiser is walk-only / that `actedLatch0x319` has NO writer / that "nothing in `src/F3`
writes it". Both doc statements predate T-048 landing the store.

**True, and proven by the port's own test** (`PathManager.cpp:672-691`, the T-051 block, run GREEN this
gate): `PumpEntity`'s raiser (`MapEntitySpawn.cpp:476`) is reached by BOTH entrances — `TickOneEntity`
(world step-5, `combatantDispatch=0`) and `TickEntityServerUpdate` (manager dispatch, `=1`) — and the
store is not gated on that flag. The suite latches a node BEFORE a `Tick` with **no** `SetRoundMode`, and
finds it latched **again** after; maintenance/TurnPrep zeroes it in both branches, so the only way it is 1
again is the raiser firing inside a pump that RAN **on the free-run path**.

**Correct scope:** raiser = shipped-live (free-run and world loop). **Consumer** = the skip-advance
`0x5a955d → 0x5a9572`, which needs a latch surviving its own call's maintenance clear — that is the
walk-only, dead-in-shipped half, and it stays head-stable for the re-entrancy reason already documented,
**not** for lack of a store. `mode +0x4` / `rearm +0x14` still have no nonzero producer (T-010 10e,
`045a7ec`) — untouched, and nothing in the capture bears on it.

**Files changed (comment/doc only, no behavior):** `docs/f3/CombatTurnDispatch.md` (2 stale passages
rewritten, T-048 store + T-056 scope cited), `src/F3/MapEntitySpawn.hpp` (`actedLatch0x319` note now names
writer/clearer/reader), `tasks/T-051-*.md` (pin wording corrected in place + gate evidence pasted). No
`.cpp` statement, no digest field, no test assertion changed — assert counts moved nowhere (117 etc.
unchanged), which is the check that this was prose-only.

## 5. Constraints honored from the table
No combat-round claim beyond the scripted funnel (Attack never fired) · no `>4` queue-depth claim (max 3) ·
R1 used only as STATE-at-instant, never as a cross-boot constant · T-050 stays gated with the named
rung-2 path, not recorded as a null about the seam.
