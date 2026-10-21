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
