# T-071 landing note — T-068 diff table re-run: R3 upgrades to STATE agreement in the tool

**Supersedes the R3 row of** discoveries/2025-10-21-t068-first-real-pairing-diff.md (banked `32945f9`;
original kept unmodified for audit — both measurement layers stay readable, per the annotation rule).

| field | shipped (run H, unchanged) | ours AFTER T-071 (fresh, mtime-verified per harness item 47) | verdict |
|---|---|---|---|
| **R3** first-insert | `q=1`, slot0 class **GameActionIdle** (0x6497C4) on 12/12 | `frame=0 … q=1,ak=GameActionIdle/-/-/-` — tools/scripts/t071-r3-pair.txt, sim-driver.exe built 2026-10-21 18:2x | **STATE AGREEMENT, IN THE DIGEST.** The correction T-068 §4 self-reported ("counts agree, states do not") is closed by the tool, not prose |
| R3 deepest observed | `1→2→3` with the resident included (T-070 §4) | `frame=14 q=2, ak=GameActionIdle/GameActionAttack/-/-`, victim **hp=30/30 unmoved** | the push-behind-resident shape now exists on our side too: count semantics identical (resident counts), HOL visible, native order stalled exactly like run H |
| R4b | first-insert vt 0x6497C4 | ak-name axis maps kind 6 → "GameActionIdle" (SpawnMove assert; mapping table EA-anchored) | resolves to the SAME shipped class NAME the vt side prints via F3.json |
| R4 | owner +0x14 == entity, 35/35 | resident is the one kind carrying a REAL port-side +0x14 (IdleResident.owner0x14), written through EnqueueEntityAction | uniform-store analog now fires for the resident class shipped always showed |

**Residuals logged (not fixed):**
1. R2 (clock leap) unchanged by this row — still harness-context, still open for T-067/R6.
2. >4 depth: shipped max-observed 3 (resident+2 orders) stays the honest cap; our D-2 refusal now
   fires at 3 ORDERS + resident (d2-stress switched to `push` lines — same D-2 fact, re-expressed;
   fixture intent preserved, header rewritten with the T-071 rationale).
3. The `st=5` on the push sentinel is GameActionAttack::Start's existing combat-engage machinery
   (identical under the bare `attack` verb pre-T-071) — NOT resident-caused; noted so the digest
   reader doesn't mis-bank it as new behavior.

**Command-shape split shipped by this port** (the one judgment call, EA-backed): the harness models
the SERVER PACKET PUMP for `attack/attack-pos/wait` (drain-then-issue, virt_meth_0x556ea0 cases
0x1A/0x3C: drain @0x5576c8 → create @0x5576d8 → funnel @0x5576f7 — the move-packet shape; this is
why in-game orders ever execute at all with a resident head), while `push` models the script
NATIVE (F3::Attack 0x459f5d has NO drain on its path — the run-H never-fired truth). Both shapes
are shipped; the port now carries both instead of silently collapsing them.

Gates on 05ab4e1/7abc975: 9/9, SpawnMove 109≥80, cppcheck 0, msvc rc0 (headers moved), integrity rc0,
sim-driver-verify PASS, digest freshness mtime-proved.

## T-050 concurrence (checker, 2026-10-21) + T-077 falsifier (co-issued with HappyEagle)

Concurred to coordinator: closed-by-dual-unreachability STANDS and my T-071 work strengthens it —
the resident is EXCLUDED from the +0x319 raiser (raiser = anim-deque), so the latch's live
producer set is exactly the 10e-enumerated non-resident processed-not-finished work, and
HappyEagle's bp10 zero-fire (sustained, cross-runtime, bp9-25 same object) is now consistent
with the statics from BOTH directions. Recorded falsifier, named before closure was final:
if T-077 input injection ever produces a bp10 hit, the ruling reopens as live-refuted.

T-077's highest-value single read (sent to HappyEagle, 2026-10-21): after any injected funnel
hit, dump slot0 class + count across 2+ frames. Idle-head + stalled order = dual-shape model
live-confirmed (run-H HOL demonstrated, not inferred); order advancing WITHOUT a drain-visible
transition = my model wrong, T-071 verb split gets revised. D-2's count>4 rides the same boot.
Resident live-reads list: T-070 doc §5. Note: tasks/T-077-input-injection.md did not yet exist
at send time (board row only) — file creation is the coordinator's per the new README/board law;
this lane committed zero tasks/README.md edits after this note (05ab4e1's stale-buffer clobber
of three lanes' rows was the LAST one; post-mortem sent, law accepted).

## Verdict-reading table for the T-077 injection (co-issued with HappyEagle, 2026-10-21)

HappyEagle's data half-answers a census question my model must state precisely: a shipped queue
CAN legitimately show a head that is NOT idle, without any pump-drain at that instant. Three
producible shapes, only ONE of which falsifies the resident/HOL model:

- **Shape A (CONFIRMS):** slot0 vft == 0x6497C4 (Idle) AND the injected order at slot1+, count
  stable, order NOT advancing, frame-over-frame (2-3 dumps). Head-of-line blocking live.
- **Shape B (FALSIFIER — the only true one):** slot0 STILL 0x6497C4 while the order at slot1+
  ADVANCES (count decreases / order Update hit) or the idle is skipped/predicated-away without a
  meth_0x54c440 drain hit. The pump would be eating past a never-finishing head — my model dies,
  T-071 verb split revised within the session, not at leisure.
- **Shape C (CONSISTENT, do NOT raise the alarm):** head is the order itself, or the queue is
  EMPTY (count 0). Reachable without any drain because NOTHING re-baselines onto idle after
  natural completion: the pump's empty-queue branch (0x54c849-0x54c88c) is the ANIM-level default
  (meth_0x548d80 via vft+20/+22), NOT an action re-enqueue. Idle is present only AFTER rebase
  sites - SetInMap_0x551ba0 (membership ADDTOMAP) or the packet-pump cases 0x1A/0x3B. Two further
  no-drain-at-instant presentations: (1) mid-round, the executing order IS the head (after an
  EARLIER drain - sub_45C140 0x45c5c6, or a MoveTo waypoint self-requeue 0x4064bd, or an attack
  self-requeue 0x40470e, which present MoveTo/Attack at head continuously); (2) an order enqueued
  into an already-empty queue fires immediately and is NOT shape-B - the stall only holds while
  the resident lives. Run H's never-fired Attack = resident alive since spawn-rebase, no drain,
  no natural completion to empty.

Reading rule at any order-advancing observation: dump slot0-vft + count AS A PAIR. Idle-present +
advance = B. Idle-absent + advance = C (or A's earlier drain upstream). The distinction is
slot0-present-vs-absent, not advance-per-se. bp10 post-injection fire goes on HappyEagle's
watch-list with my T-050 falsifier; either ping folds the verdict here within the session.

HappyEagle's DXVK-12min datapoint (entity 0x0A6C6060: count 1->2->3, slot0 = 0x6497C4 at first
insert, 0x6A6414 Anim after) is a fresh INDEPENDENT re-confirmation of T-056 R4b (12/12
Idle-first + Anim-second) on a second boot, second runtime, and a third entity pointer - folded
as confirm, not contradiction.
