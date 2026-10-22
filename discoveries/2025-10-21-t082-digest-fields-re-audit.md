# T-082 — Digest fields vs today's shipped facts: field-by-field honesty audit

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Row:** T-082 (claim `ca8253e`) · **Method:** raw-byte
re-verification (objdump off F3.exe md5 a68c22f3…, post-respawn discipline: nothing cached was
trusted) + enumeration of stored expectations. **Zero digest behavior changed by this row** (type:
research + audit; every drift is QUEUED with a proposed row, not applied).

## HEADLINE — the audit's blade turned on its author's own lane

**Checklist item 1 escalated past its premise.** The row text (written from my T-070 bank) says
"R3 count INCLUDES the **never-finishing** resident." Raw bytes refute the never-finishing claim:

- `0x5a9070` (GameActionIdle vft 0x6497c4 **+0x14** = the update slot, per my own T-070 raw dump)
  is `c6 41 18 01` = **`mov byte [this+0x18], 1`** — writing the EXACT done-byte the predicate
  `0x404b60` reads (`8a 41 18` = `mov al,[ecx+0x18]`). Wait's update `0x5601d0` sets the SAME byte
  only at `ms > [this+0x1C]+[this+0x20]` (due time) — timed finish — while idle's update is the
  UNCONDITIONAL setter. T-080's lane read this raw first ("finishes on its first tick"); my T-070
  claim of "+0x16 ≠ +0x18" was an inference from IDA's `BYTE2(mbr_int_0x14)` member rendering,
  never raw-verified — the exact class of error my own F2 post-mortem named (cite bytes, not
  renderings).
- Pump mechanics make it terminal: `sub_55A1C0` runs start-once (activation), then update (sets
  done, consumes 0 ms), then the walk re-checks the predicate → TRUE → erase + deldtor(1) →
  **the same walk CONTINUES to the next entry in the same frame** (0x54c7f6 false-path only on
  not-done). Idle is a **ONE-TICK transient arm**, not a permanent occupant.

**Consequences (each queued below, none patched here):** (i) steady-state shipped `+0x314` = 0,
not 1 — every banked statement of the form "q includes the resident" is timing-qualified: true
between rebase-insert and the next pump tick, false after; (ii) the T-071 PORT models a
never-clearing resident (SpawnMove stall asserts, d2-stress 3-orders+resident refusal, pair-script
steady `ak=GameActionIdle` frames) — real product drift vs shipped semantics; (iii) run H's
"Attack never fired" — the HOL explanation I banked IS REFUTED as mechanism while the OBSERVATION
stands — the phenomenon reopens, un-explained, with candidates named and NO verdict (guessing is
still not allowed); (iv) the T-077 verdict-reading table (my shape A/B/C, banked f427cc9, adopted
by HappyEagle 3505688) can mis-fire in BOTH directions now — a live redefinition was sent to the
boot lane the same hour (see §5).

## The field-by-field verdicts

| # | digest/contract field | today's shipped facts it's measured against | verdict | evidence + row cites |
|---|---|---|---|---|
| 1 | `frame` / `clockDelta` | R2 leap (T-056/T-067 cross-runtime reproduced) | **BENIGN** | CONTEXT-excluded axis unchanged; leap is harness-context (T-068 logged); pairing unaffected |
| 2 | `tickMs=`,`clock=` (after `ctx:`) | T-067 pairing law | **BENIGN** | never hashed; law already pins values cross-env / addresses same-env |
| 3 | `ents`, `eN.name/pos/yaw` | — | **AGREE** | no shipped fact today touches them |
| 4 | `hp=cur/max` | **T-074: shipped damage = QueuedHit → victim +0x2E0 inbox → apply fires [vft+0x2C] @0x55a501 when the root's done-predicate flips; SYNCHRONOUS inline only when the ready gate [vft+0x28] is already satisfied (0x5506ef-0x5506f7)** | **DRIFT (prospective)** | our sim lands inline at hit-frame (GameActionAttack.cpp:286, "ordered exactly as Apply" — the COMPUTATION mirrors 0x45C243.. but the LANDING does not). Ready-gate-satisfied cases AGREE by accident of timing; pending cases differ by ≥1 pump. NO stored shipped-vs-port hp pairing exists that straddles the gap (run H never read hp; T-068 table has no shipped hp row) — so nothing banked is wrong yet. **Queue:** effect-inbox port row (T-074 §5 already NAMED it; this audit re-times it from cosmetic to pairing-material) |
| 5 | `st=entityState,moving,actedLatch0x319` | **T-085 (open, WildZenith): the raiser chain has never been anatomized** | **AGREE, rationale AMENDED** | the latch mechanics in the port stand; but my T-071 raiser-exclusion comment justifies itself with "the resident never raises because a resident head must not freeze the manager" — under one-tick transients the resident never SURVIVES to raise anyway. Exclusion stays behaviorally correct (a finished+erased slot does not take the raise branch), rationale rewritten to cite 0x5a9070's raw setter. Cross-notified to T-085: their raiser census should expect idle at NEITHER 0x54c833 site for more than zero frames |
| 6 | `q=` (the +0x314 element count) | **T-070 §4 re-expression + THIS row's byte finding** | **DRIFT (semantics), AGREE (field)** | Field still models +0x314 exactly (T-039 law unchanged). Three banked quotes need timing-qualification: T-071 landing note "counts the resident - shipped R3 semantics" (true at insert-sampling, false post-pump); T-068 §4/R3 state-agreement (still holds — both sides show q=1 + GameActionIdle AT THE INSERT INSTANT, which is where T-056 sampled; the pair-script frames 1+ are where our port now over-shows). **Queue with row 7's product fix** |
| 7 | `ak=` + `ActionKindName` numbering | **T-080 engine jump table 0x5450cc raw: 0=Die(0x1C) 1=Idle(0x1C) 2=MoveTo(0x48) 3=PickUp(0x30) 4=Wander(0x44) 5=Wait(0x24) 6=Anim(0x28) 7=Message(0x34) 8=FATAL 9=Equip(0x1C) 10=Unequip(0x1C) 11=setActiveWeapon(0x1C); -0x230 this-adjust passes the kind UNMODIFIED ⇒ F3-side and engine-side numbering are IDENTICAL** | **AGREE** | T-071's mapping comment ("6/7 = JE::World::CreateAction passthrough") verified against both tables; the F3-side alloc sizes in MapEntitySpawn.hpp match my T-070 raw re-read; engine 8=FATAL vs F3 8=FloatingSpeechText is a real two-factory distinction and our digest axis (class NAME) is unaffected. Message kind 7 is absent from our enum BY DESIGN (no engine passthroughs ported) — BENIGN; if one is ever enqueued in a pair it prints UNKNOWN_KIND, never a wrong name |
| 8 | `pathJobs=` | T-074 three-queue map: path jobs live in none of the entity queues | **BENIGN** | NA-semantics unchanged ("no path manager in this context" is still exactly what it says) |
| 9 | `turns=` / `turnOrder=` | T-050 CLOSED dual-unreachability (concurrence mine, bp10 zero cross-runtime); T-074 busy-math counts BOTH deque fields (0x54ab00 reads +0x2F0 and +0x314) | **AGREE + one watch** | turn protocol pairs against state-5 list math, untouched by idle's transience. Watch: if the reopen of run-H's stall (§11) lands on a dispatch-gating mechanism, turn cadence expectations get re-audited — named now so it can't surprise later |
| 10 | `draws=` + the CombatMath real-pair pins | T-067 pairing law: pin = STATE 0x65AD8170 / DRAW 0x2D81 / preimage 0x6AD92B15 — all CONSTANTS, zero addresses | **AGREE** | the suite pins are cross-env-stable by construction (formula + measured constants); DXVK shifts cannot invalidate them; R1 stays the model for what a stored expectation should bake |
| 11 | D-2 capacity semantics | one-tick idle | **DRIFT (restated)** | old (T-039): "shipped q>4 beside our q=4 = dropped order". Middle (T-070): "3 orders + resident fills first block". TRUE (this row): the transient idle occupies a block SLOT between insert and next pump only — sustained-shipped first-block holds 4 real orders; a shipped count of 5 at ANY instant still means block-chain growth (D-2 intact), but the d2-stress refusal-at-4th-push models the insert-window, not steady state. Fixture header already says which instant it samples — annotate once more at the product fix |
| 12 | `EXCLUDED=` line (reaction-anim-id, gfx-texture-slots, gui-fragments, voice-roll, effect-rolls) | T-074: the +0x2E0 inbox IS the effect/voice/animation-event family's transport, apply sites pinned (0x55a501 fire, 0x55a570 finish) | **BENIGN + strengthened** | exclusions stay justified; T-074 gives the EXCLUDED items a named shipped mechanism, so "presentation-side" is now an evidence class, not a vibe |
| 13 | +0x184 stamp + state 9/1 observables | same bytes | **AGREE** | activation order survives the correction untouched: sub_55A1C0 runs start (0x405620: stamp 0x548f90, states 0x4673d0(9,0)/(1,+0x1C)) BEFORE the update that finishes it — the one tick a transient idle exists, it does everything T-070's §1 claimed; only its LIFETIME was misread. Port observables need no change; lifetime does (queued) |

## §4 — pairing-law enumeration (every stored expectation, heap-address test)

Swept: `tools/*.sh *.py *.txt`, `modules/openje/tests/**`, `src/F3/SimDigest.hpp`,
`devenv/scripts/run-all-tests.nix`, the comparator suite. Result: **zero runtime heap addresses in
any comparator or test input.**
- GUILoading "0x0XXXXXXX" hits = COLOR literals (0x00112233 ARGB), not pointers — BENIGN.
- `orig-capture-t056-runH.txt` `address=` columns (0x79B3600+0x314, [0x70BD98]…) = PROVENANCE
  records, not parsed by `orig-digest.py`'s render path except as `record=read` values the
  cross-check consumes as NUMBERS appearing in rendered frames — re-keyed? No: the cross-check
  compares VALUES (q, vt, lcgstate), and the values (1, 3, 0x6497C4, 0x65AD8170) are all
  same-env-independent (deque counts, class vfts, LCG state) — **AGREE with the law as-banked**;
  the ADDRESS columns alone would go stale under a base-shifted runtime and the file already
  declares session=t056-runH for exactly that reason.
- T-067 harvest rig (drive scripts) bakes same-env pointers by the law's own clause (addresses
  same-env) — exempt BY DESIGN; any future replay under DXVK uses the law's re-derive note.
- CombatMath pins, d2/host scripts, sim-driver-verify fixtures: constants only — AGREE.

## §5 — the T-077 redefinition (sent live; recorded here as the audit's operational output)

My own shape-A/B/C table predates the byte finding and had to be corrected BEFORE HappyEagle's
sitting burns the last frontier shot: idle-at-slot0 at an INSERT bp = normal and says nothing
about blocking (it always self-clears next tick); advancing-past-idle after a pump = normal too.
The LIVE QUESTION that actually separates models is now narrower and cheap: **dump the deque on a
PUMP-frame bp (0x54c7f6 area), not an insert bp** — if idle is ever seen there while unfinished,
or orders wait behind it across pump ticks, my ORIGINAL never-finish reading is vindicated on
live data despite the bytes; if the walk shows the transient erase+continue, the port fix (§1
queue) is mandatory. Either result is decisive in one dump — ping agreed same-session, and the
watch-list mutual (bp10 falsifier) is untouched by this correction.

## Queues (writer/product slots — nothing applied in this row)

1. **P-REV (product, next port row):** idle becomes a one-tick arm — update sets done0x18, the
   walk erases+continues; SpawnMove stall-asserts flip to transient-asserts; d2 arithmetic
   re-timed to insert-window vs steady; pair-script expected lines annotated; T-068/T-071 banked
   text gets the timing qualifier at the T-079 batch-apply.
2. **RE-REOPEN (research row):** run-H "Attack never fired" — phenomenon stands, mechanism
   unnamed; candidates (do NOT pick without bytes): range/first-strike due-window gating,
   state-5 dispatch ownership, inbox-busy interactions (T-074 busy-math), script-side Start(nowMs)
   defaults. This is now the board's most interesting unexplained shipped observation.
3. **T-085 cross-note:** raiser census should expect idle at NEITHER 0x54c833 site across frames.
4. **T-077 boot-note:** pump-frame dump replaces insert-frame dump as the deciding read (sent).
5. **hp pairing watch:** when ANY shipped hp-adjacent row lands, re-audit field 4's straddle.

## What this row did NOT change
No digest output, no contract line, no test, no port behavior. One banked-mechanism claim from my
own lane was byte-refuted and is queued, not patched; one cross-lane claim (T-080's raw read) was
vindicated against me and adopted; the audit's checklist items 2-5 each received their verdict
with EAs; the pairing-law sweep came back clean by design. The tools stayed honest because the
row pointed them inward first.


## §6 ADJUDICATION of HappyEagle's DB gap (same-day, raw re-dump; his 7a5399b question)

HappyEagle flagged correctly that artifacts/F3.json registers 0x5a9070 only as **ActionMessage
slot-5 (vft 0x6a5f34)** and maps GameActionIdle's 0x6497c4 to slots 0/4 (+0x14 UNMAPPED) —
question: shared stub (a) or Idle's slot-5 is someone else (b)? Raw bytes answer it directly
(F3.exe md5 a68c22f3, this session):

```
GameActionIdle 0x6497c4   idx0 0x405690  idx1 0x404b60  idx2 0x404b70  idx3 0x4010d0
                          idx4 0x405620  idx5 0x5A9070  idx6 0x404bb0            (7 dwords)
ActionMessage  0x6a5f34   idx0 0x405350  idx1 0x404b60  idx2 0x404b70  idx3 0x4010d0
                          idx4 0x5a9080  idx5 0x5A9070  idx6 0x404bb0            (7 dwords; idx7
                          reads ASCII 'Acti' - table boundary, both length 7)
```

**(a) VERDICT: shared-stub + DB under-registration.** Idle's slot-5 (+0x14) IS 0x5a9070 in the
binary; the frozen DB simply names fewer entries than the tables have (same class of loss as F2's
flattened ten-table run). The whole protocol skeleton is a SHARED POOL across the family -
predicate 0x404b60, cancel 0x404b70, queued-getter 0x4010d0, on-finished 0x404bb0, and the
done-setting update 0x5a9070 all identical between the two classes; only dtor and start differ
(0x405620 idle-activation vs 0x5a9070's owner-named 0x5a9080). IDA registered the stub under the
class it first analyzed - the DB's slot registration is a lossy MIRROR; the generalization of F2's
lesson, stated for the bank: **cite raw table dwords + byte offsets; treat F3.json slot maps as
under-population-prone.**

Consequences for the sitting (his triple at 0x54c7f6): the transient model predicts [head+0x18]==1
AT THAT SITE on the FIRST walk to touch a fresh idle (sub_55A1C0 runs update BEFORE the walk's
predicate retest), erase-and-continue same iteration; a [head+0x18]==0 catch with idle head is
physically impossible under the transient model (would mean the update was skipped) and would
vindicate never-finish. One hit, byte decisive; his frame-series demotion to nice-to-have is
correct, and the dword probe (vft+0x14 vs 0x5a9070) is pre-answered: expect equality.


## §7 CHECKLIST ITEM 5 CLOSED — jump table raw-verified; one size erratum found IN MY OWN TEXT

Item 5 asked: "verify T-071's mapping table matches the raw jump table exactly." Verified raw
(F3.exe a68c22f3): the switch at 0x46f2a2 (`cmp eax,0xb` / `ja 0x46f4dc`) jumps through table
**0x46F4FC** (12 entries read): kind 0->0x46f2b9, 1->0x46f2ef, 2->0x46f325, 3->0x46f35b,
4->0x46f391, 5->0x46f3c7, **6 AND 7 ->0x46f4dc (the DEFAULT = JE::World::CreateAction passthrough
with the kind UNMODIFIED - T-080's identity claim confirmed from the F3 side too)**, 8->0x46f3fd,
9->0x46f433, 10->0x46f465, 11->0x46f497. Every push-size at each target read raw:

| kind | class (ctor stored vft) | raw alloc | my text said | verdict |
|---|---|---|---|---|
| 0 | GameActionDie (0x404e50 -> vft 0x648694) | **0x2C** @0x46f2b9 | **0x20** | **ERRATUM (mine)** |
| 1 | GameActionIdle (0x4055f0 -> 0x6497c4) | 0x24 | 0x24 | AGREE |
| 2 | GameActionMoveTo (0x4056f0) | 0x48 | 0x48 | AGREE |
| 3 | GameActionPickUp (0x406e70) | 0x30 | 0x30 | AGREE |
| 4 | GameActionWander (0x46f140) | 0x44 | 0x44 | AGREE |
| 5 | GameActionWait (0x407ed0) | 0x24 | 0x24 | AGREE |
| 6/7 | default passthrough | n/a | n/a | AGREE (both targets = 0x46f4dc) |
| 8 | FloatingSpeechText (0x405240) | 0x34 | 0x34 | AGREE |
| 9 | Equip (0x405060) | 0x30 | 0x30 | AGREE |
| 10 | Unequip (0x407af0) | 0x28 | 0x28 | AGREE |
| 11 | SetActiveWeapon (0x4075d0) | 0x44 | 0x44 | AGREE |

The kind->CLASS->ctor->vft chain agrees 11/11; exactly one NUMBER is wrong and it is mine, twice:
`0=GameActionDie 0x20` in the banked T-070 doc (line 56) and in the LANDED MapEntitySpawn.hpp
T-071 comment (line ~146). Origin of the error reconstructable honestly: F3-side Die 0x2C vs
engine-side Die 0x1C (T-080) - I pattern-completed "0x1C-ish" into 0x20 without reading the push
at the jump target (I had read 0x46f2c0 mid-block earlier and the push lives one block BEFORE
that window at 0x46f2b9; never in my earlier dumps). Discipline echo of the day's own lesson -
the size column is the one thing I cited WITHOUT a raw push read, and it broke.

HANDLING per row doctrine (queue, don't patch):
- **T-079 batch-apply input (factual erratum):** T-070 doc line "0=GameActionDie 0x20" -> "0x2C
  (@0x46f2b9 push raw)"; MapEntitySpawn.hpp comment identically. Both are comments/docs - zero
  behavior change; fold into P-REV if the coordinator prefers one row touching that comment block
  (P-REV already rewrites it for transient semantics).
- The engine-vs-F3 size distinction (Die 0x1C engine / 0x2C F3) is now a named fact for the bank.

## §8 STATUS: T-082 complete pending ONE live adjudication

Checklist 1-5 all have verdicts (1: DRIFT-transient + timing qualifiers; 2: DRIFT-prospective hp;
3: AGREE via S6 shared pool; 4: pairing-law sweep CLEAN; 5: AGREE 11/11 + this erratum queued).
The row closes when HappyEagle's 0x54c7f6 done-byte catch lands (either outcome folds into S1/6
and the P-REV queue item finalizes). Queues unchanged; zero code touched by this row.


## §9 P-REV LANDED - the queues this doc filed are now closed in code (same-day loop)

P-REV (claim 2d54e2c) executed the S1 queue item verbatim: kind-6 idle is now a one-tick ARM
(update writes done0x18 per 0x5a9070 raw; branch erases; walk COMPACTS and CONTINUES same frame,
0x54c80a-0x54c819 shape), the +0x319 kind-6 exclusion retired as vacuous (S-note), the Die-alloc
erratum fixed at the hpp with the origin story kept, the three-drain name-vs-site crossing
(gen-5) annotated at ClearActionQueue with all six EAs, ActionMessage kind-7 recorded as the
shared-pool twin, and the digest tool gained `peek` (tick-less frame) so the insert window - the
ONLY place idle is observable, and exactly where T-056 sampled - is pairable on our side too:
t071-r3-pair.txt now shows `q=1 ak=GameActionIdle` at peek, `q=0` steady, `ak=GameActionAttack`
in its own insert window. The (c) HOL assertion became a continuation PROOF (unfinished Wait
blocks; its due-tick erases AND runs the Attack in the same walk) - the correction landed in
the TESTS, not in prose. Gates on this payload: run-all 9/9 (SpawnMove 114>=80), cppcheck 0,
msvc rc0 (headers moved), gate-integrity rc0, sim-driver-verify PASS, d2-stress refusal intact
("3 orders + unpumped arm"). RE-REOPEN stays OPEN as designed (the doc's candidates stand;
P-REV did not touch it). T-082's queue-surface claims were untouched by the row, as agreed -
S8's remaining input (the 0x54c7f6 boot catch) is now about LIVE confirmation of a model the
port already implements, not about a pending fix.
