# T-068 — FIRST REAL shipped-vs-port pairing: field-by-field diff of the T-056 six values against the port

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Row:** T-068 (claim `b029da0`)
**Inputs:** shipped side = `tools/scripts/orig-capture-t056-runH.txt` (T-056 run H reads, md5
`a68c22f3…` = artifacts/F3.json filemd5); ours side = fresh `sim-driver.exe` runs (wine,
digests verified mtime-Oct-21 after a stale-file catch — an Oct-20 `/tmp/sd-demo.digest` nearly
entered the table; deleted and re-run). Comparator = `tools/diff-digest.sh`, emitter =
`tools/orig-digest.sh`. **Discipline: the diff IS the product; nothing is fixed ad hoc.**

## Headline

The six shipped values pair CLEANLY against the port in four fields (R0, R1, R2-semantics, R3),
pin a real same-state draw pair into the CombatMath suite, and produce TWO REAL DIVERGENCES
logged as findings (ak-classes at first insert; the harness clock leap being context-only) plus
ONE honest refusal (the emitter cannot render a lockstep digest from the six-row read set — the
grammar demands fields T-056 never read, and faking them is exactly what the provenance system
exists to stop).

## The field-by-field table

| field | shipped value (run H) | ours (fresh run) | verdict |
|---|---|---|---|
| **R0** World vft slot-6 target | `[0x668584]` = **0x544A90** (bp 0x552bb5, 5× stable, 3 boots) | statics T-045 `virt_meth_0x544a90` = World::Update; port routes the same call through `TickWorldEntities` (sim-driver line: "same routing as src/F3.cpp:790") | **MATCH** (live-confirmation of the documented assumption; no port change) |
| **R1** LCG first-return pair | state **[0x707C60]=0x65AD8170**, draw **0x2D81** (bp 0x48ccf6) | `(0x65AD8170>>8)&0x7FFF == 0x2D81` verified; `SeedShippedRand(0x6AD92B15)` (the unique modular preimage under the T-045 constants) → our `ShippedRand()` returns **0x2D81**, next draw continues from the captured state | **MATCH + PINNED** — first real shipped pair inside the suite: `modules/openje/tests/je/CombatMath/CombatMath.cpp` 3 asserts (46≥43). This is the §1(B) same-state parity claim, made on measured values |
| **R2** clock semantics | frame-0 pump 0s → pump#3 **1s** (leap IS shipped behavior, all boots) | port models the leap **in the app path** (SimDigest contract: `mbr_0x48` seeds to 0 vs an ABSOLUTE tick → first pump adds the whole uptime). The **harness** context (sim-driver owns `g_clockMs` starting 0) makes the leap vanish into CONTEXT `clock=`; digest `clockDelta=0` at frames 0-1 | **SEMANTICS MATCH, OBSERVABLE CONTEXT-DIVERGENCE — logged:** the driver cannot exercise the absolute-uptime seeding at all; a bootable-exe comparison (T-056 rung-2 / T-067) is the only fixture where our leap is comparable. NOT fixed (would be simulating an uptime, i.e. inventing context) |
| **R3** deque count at first insert | `Entity+0x314`: 0→**1** first insert; deepest 1→2→**3**; never >4 pre-crash (no D-2 datum either way) | ours first insert `q=1` (fresh host-demo, frame 0); cap 4 then D-2 refusal (fresh d2-stress: "SIM-DRIVER FAIL line 19: entity brute queue block full (D-2 refusal…)") | **MATCH at every paired instant; >4 question stays OPEN exactly as T-056 phrased it** — no dropped-order evidence exists in either direction; the digest must not imply a combat round beyond scripted-path evidence (coordinator rule honored) |
| **R4** owner back-store | `[action+0x14]` pre **0** (12/12), post **== entity ptr** (35/35, bitwise C==H) | our port reaches the owner store only from re-queue sites (T-061/design §; `docs/f3/EntityActionQueueSeam.md`); digest prints no owner field | **CONSISTENT, NOT COMPARABLE — logged:** the shipped pre-state being ZERO (not garbage) is a NEW fact for our port's model (WildYak's question answered); no digest field exposes it, so pairing is statics-side until an owner-visible field exists. Related live fact from T-069's close: the +0x319 RAISER is shipped-LIVE via shared PumpEntity — any latch comparison must pin the CONSUMER half |
| **R4b** action classes at first insert | first-insert class = **GameActionIdle (0x6497C4)** on ALL 12 sampled; later inserts ActionAnim (0x6A6414); zero UNKNOWN_VT | fresh ours frame 0: victim queue = **`ak=GameActionWait/-/-/-`** (script's own wait); our kind enum has **no Idle class at all** (`ActionKindName`: MoveTo/Wait/OperateDoor/Attack/Die) | **REAL DIVERGENCE — logged, unfixed:** the shipped engine enqueues a live Idle action as its baseline queue resident; our port has no Idle action kind, and our scripted first-inserts are Wait/Attack. Two readings (both open): our model treats idle as *absence of action* where shipped stores *an action of idle class*, or the engine's Idle insert is the seam where shipped advances state our way doesn't. This is precisely the kind of finding the pairing exists to surface — **it is the first shipped-vs-port STATE divergence produced by the oracle pipeline on real data** |

## The emitter's honest refusal (first finding, mechanical)

Rendering run H's capture as a shipped-side digest **must and does fail**:

```
PROVENANCE-UNVERIFIED: claims lockstep but its read records and its frames disagree:
  r1 value 0x65ad8170 appears in no frame's lcgstate=;
  r3 value 0x1 appears in no q= field;
  r4b value 0x6497c4 appears in no vt= field
```
(exit 3, nothing written). The six-row schema recorded reads but no `record=frame` (tickms was never
read) and no complete entity rows (name/pos/hp/state never read for the sampled entities) — the
grammar's earned-label check caught the coverage gap rather than letting half-real rows through.
Follow-up read sets that close it (cheap, same :96 rig, listed for the boot lane):
**R5** = full entity state tuple (`name/pos/yaw/hpcur/hpmax/state/moving/acted`) for ONE named
entity at the R3 instant (pointer known: 0x79B3600); **R6** = two `[tickms, clocksec]` pairs
3+ pumps apart on a fresh boot. With R5+R6 the capture renders and `diff-digest.sh lockstep↔ours`
becomes possible unchanged.

Comparator self-checks run today: ours-vs-ours → `IDENTICAL` (rc 0); synthetic↔unrecorded →
`MIXED-PROVENANCE` refusal (rc 3) — the guard rails hold on the first day real data exists.

## What is NOT claimed

- No "parity achieved" sentence: one field pair (R1) is pinned; the rest are statics agreements
  or logged divergences. PARITY= stays exactly as the contract header says (self-consistency).
- The ak divergence gets no root-cause verdict this row (naming it is the deliverable; root-
  causing Idle-insert semantics is a follow-up row candidate).
- R3's ">4" and R2's leap-comparability: open, fixtures named above.

## Artefacts touched

- `tools/scripts/orig-capture-t056-runH.txt` (NEW: first real capture, reads + COVERAGE NOTES;
  provenance passed via CLI per the grammar's single-token value rule).
- `modules/openje/tests/je/CombatMath/CombatMath.cpp` (+3 asserts: real-pair pin; submodule-first
  commit).
- Row status + gates: run-all-tests full 9/9 (CombatMath 46≥43), cppcheck-openje 0; msvc NOT
  required (no src/F3 or public-header changes this commit — item 37 trigger absent, stated).
- Deliberately untouched: src/F3, the sim clock model, the action-kind enum (logging ≠ fixing).
