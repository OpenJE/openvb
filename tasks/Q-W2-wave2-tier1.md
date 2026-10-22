---
status: done (wave-2a; HOLD residue routes to arbitration/re-cut per ruling)
assignee: TrueCastle `01a16096-c00e-7404-8afe-e521a9bb7240` (PARK pull Q-W2 per lane contract ed8c1af, 2026-10-21; lane-fit named at PARK; CalmRaven first-in deferred to wave-3 on return)
type: writer wave (sole IDB writer; T-087/wave-1 ruleset VERBATIM)
depends: T-084 queue (doc §2 tier-1), wave-1 precedent (T-087 ledger), T-075 §8 same-this sibling list (for the &global fold rows), harness items 42/44/49/50
---

# Q-W2 — WRITER WAVE-2: T-084 tier-1, 182 MINT-READY fns across 22 hosts

## Scope (PARK line)
T-084's tier-1 queue: 182 fns, host carries >=5 fns, all per-fn share-check-PASS at authoring,
terminal-field keyed. Mints to the suggested host under the host's live class name, same wave-1
mechanics (prefix/stem discipline, no invented roles).

## TWO GATES I ADD to the wave-1 ruleset (sizing findings, 2026-10-21 pre-claim)
GATE A — REGENERATE BEFORE APPLYING. The persisted queue artifacts are NOT complete:
  - byhost.txt truncates both 17-fn hosts with "…" (GameActionEquip + F3 each hide 5 rows);
  - t084-mint.json is a flat 323-EA list WITHOUT host mapping;
  - t084-sweep2.txt is an 85-line partial head (67 parseable rows, 3 tier-1 hosts);
  - spot-check CONTRADICTION: doc §2 Skin row lists 0x490450/0x4904B0; the byhost-era queue lists
    0x4B4570/0x4B5130 — and 0x4B4570 reads NO-EDGE under live share-check. The two "tier-1"
    snapshots disagree on 4 Skin EAs.
  => step 0 = reproduce the FULL per-fn queue from the banked recipe (per-class share-check +
  families over t064-postdump cls_ grouping, same pinned build, md5 a68c22f3), cross it against
  doc §2 + byhost.txt, and adopt ONLY the intersection-with-proof: an EA enters the wave if the
  regenerated share-check says SINGLE-OWNER/PASS for the doc-named host. NO-EDGE/DISAGREE rows are
  LABEL-AND-SKIPPED with reason. This is the live-base lesson from wave-1 applied to the queue
  itself: forecast against the LIVE data, not the archived display.
GATE B — &global SINGLETONE-PIN ROWS FOLLOW T-075, NOT THE CALLER SUGGESTION. Hosts' &global rows
  (e.g. 0x4B2C80 [F3], 0x4B2AC0/0x4B4180/0x4B4CE0/0x4B3690/0x4B5C30 under Graphics/Label/Window)
  are pure this==&0x707E48 members per T-054/T-075 (142/142 band purity, zero heap ctor, zero
  holders). suggested-owner reflects the CALLER's RTTI, not the OWNER. T-075 §8 explicitly parked
  these same-this sibling keys as "writer may fold now or later" — they fold to JE::GUI::Manager::
  with this wave's ledger citing T-075 §1/§8, UNLESS the per-band read-back contradicts purity.

## Ruleset (wave-1 verbatim)
Sole writer from this claim commit. Pre-flight snapshot (head+live-name per EA) BEFORE first
mutation; baseline SYNC save as the first write; item-42 RTTI/size truth per host family (stated
honestly: RTTI-named hosts vs string-named epochs); item-44 share-check at write time per band;
item-49 callee-side where store-mode matters; item-43 head checks for comment EAs. Per-fn ledger
(source row, tier, verdict, applied/label-and-skipped). T2 itemized BY HOST. Batch by host into
~5 band-waves (A: GameActionEquip+OperateContainer+AreaMapWindow; B: F3+JE-rooted+Graphics;
C: Client+World; D: engine/Gfx hosts (Gfx_Engine, ParticleSystem, AnimMesh, ModelInstance,
GfxCameraHolder-adjacent); E: GUI fold band (Skin/Label/Window &global rows + Manager siblings)
+ audio/VFX hosts (CSoundSystem, Gregorian, VFX_Model, VisualEffect, DialogueManager, net::client,
CounterControl, Map)); checkpoint save after EACH band; end save_database + mtime + fresh-dump
disk==live + get_cmt readback; names re-read at EA before each band (churn law); NO product-file
touches.

## Acceptance
Ledger discoveries/2025-10-21-qw2-wave2-ledger.md (+ queue-regeneration dump wavedumps/committed
as recovery source); row flips for T-084 tier-1 board state via relay; census line cls_ total
after (start: LIVE BASE measured at claim, not byhost-2026-10-21-era); [DISCOVERY-BATCH ready]
at close. Regeneration deltas vs the archived doc (rows dropped at Gate A, folded at Gate B)
are BOOKED LOUDLY, not silently.

## Gate A result (executed pre-ruling, 2026-10-21)
STOP-AND-MESSAGE FIRED: 65 HOLD rows > 15 threshold. Full decomposition banked at
discoveries/wavedumps/qw2-gateA-{queue-split,regen-full}.txt (commit 9afc531):
- 107 WAVE-ELIGIBLE rows / 21 hosts (doc-host == regen-host under caller-mode+transitivity,
  shape-safe <=1 view/holder, incl. 6 Gate-B Manager folds cited per T-075 §1/§8).
- 65 HOLD: (a) ~20 real-class divergences for arbitration; (b) ~20 SHAPING embed-rows awaiting
  install-mode derivation; (c) weak-root host rows per T-084's own note.
- ROOT CAUSE of 94-row doc divergence: T-084's host derivation ran through an UNBANKED
  holder-INSTALL driver (sweep2 host= column is not reproducible from banked tools); shared
  field offsets (Client+0x118 vs World+0x118) make caller-mode provably blind on install rows.
  Re-banking that derivation converts HOLD-(b) rows; requested via relay.
- Churn: rows already minted by wave-1 (0x4903E0 family) book APPLIED-PRIOR, never re-minted.
- Zero IDB writes since wave-1 final save (mtime 23:43:17) pending coordinator GO/WAIT ruling.


## Wave-2a result (2026-10-22 00:20 UTC)
99/108 applied in 5 bands (0 read-back mismatches; checkpoints 00:18:43/00:19:07/00:19:38/00:20:07;
final save 00:20:40). 9 rows HELD at preflight on §3-foreign-key collisions (incl the named
0x515040 contradiction) -> joined the 65 Gate-A HOLDs for arbitration/re-cut. 6 Gate-B Manager
folds landed with per-row cites (0x4b2ac0/4b2c80/4b3690/4b4180/4b4ce0 + hidden-10 0x4b4d90).
Census cls_ 1,991 -> 1,892 (exactly -99). Ledger a4db6ca; recovery wavedumps/qw2-wave2a-poststate.txt.
