# P-CMBT — combat/anim-queue port design (ordered, per-step acceptance, evidence-cited) — genre: `docs/f3/T013-registry-port-design.md`

**Author:** SwiftMoon `01a16092` · **Date:** 2026-10-21 · **Status:** design-only (no code, no `src/`
commits; this file banks to `docs/f3/` at the coordinator's checkpoint)
**Purpose:** when the combat-era port work opens, ZERO thinking cost: every decision below is
evidence-backed, each step is small, independently gated, lands with its own asserts, and the
device-dependent bits are LAST by construction.

**Evidence base (all accepted/committed):** `discoveries/2025-10-21-t074-entity-2e0.md` (effect
inbox, three-queue table), `2025-10-21-t080-37c-onmessage-inserters.md` (mailbox, kind-7 bridge),
`2025-10-21-t082-digest-fields-re-audit.md` + `tasks/P-REV-transient-revision.md` (TRANSIENT idle —
the landed arm this design ALIGNS to, never re-derives), `2025-10-21-t085-animated-event-deque.md`
(anim deque + +0x319 truth-table + BP box), `2025-10-21-r-amx-native-queue-map.md` (R-AMX annex,
§Annex below — annexed into this row by the coordinator at `d7ae655`), `docs/f3/EntityActionQueueSeam.md`
(funnel/D-2), `docs/f3/CombatTurnDispatch.md` (manager, latch consumer), T-048/T-069 (raiser port),
T-056/T-068 (pairing instants), `docs/f3/GameEffect.md`+`GameEffectDamage.md` stubs (claims live in
the discoveries, not the stubs).

**Provenance note (honesty first):** this is a RECONSTRUCTED copy. The original dirty-untracked
deliverable was destroyed mid-session by a co-lane `git clean -fd` (the exact hazard this lane
warned about in the T-072 notes; zero blame, real loss). Full content survived in the authoring
session's context and was re-emitted with the commission-line patches and the R-AMX annex folded
in. During recovery my protection commit (`2269ea6`) ACCIDENTALLY included this file — a lane-law
violation, corrected by the restoration commit (`cb2e0bf`: removed from the index, retained on
disk). Current state: **STAGED as an add, NOT committed** — index-staged files are invisible to
`git clean -fd`, so it survives co-lane hygiene until the checkpoint, which reads the index
exactly as it would read an untracked file (bank it, or `git reset HEAD -- <path>` to restore
untracked-dirty, per coordinator preference). The incident's full record is in
tasks/P-CMBT-combat-anim-port-design.md §Addendum.

## Scope freeze

IN: the **effect inbox (+0x2E0) and its damage path**, pump-walk ORDER, the +0x171 busy mark,
state-7 `+0x1BC` semantics, the **script mailbox (+0x37C)** seam + kind-7 Message bridge, raiser
PAIR completion as cited-absent with corrected cites, digest/driver hooks, the four-queue
clear/survival matrix (annex), and a three-line annotation-errata list the port era must not
re-inherit.
OUT (named, later, or never): the **anim-event deque +0x1A8 timeline** (device-side sink; gated
step 6), AMX VM (native-registry stand-in stays), re-entrancy (T-050's lockstep question — a
SITTING, not a design), `+0x174` budget grant formula (T-011 territory, cited not re-derived),
effect-tree CHILDREN (+0x1C list — step 7 candidate, out of v1), the `0x5506f1` GUI-busy kick
`meth_0x54ef30` family (cited-and-absent law unchanged), the T-080 §3 destroy-leak question
(named; NOT fixed here), `RE-REOPEN` run-H stall mechanism.

## Step 1 — the effect inbox, standalone (no combat touch, no GUI)

**Why first:** shipped damage (the ONLY inbox content today's paired observables could ever see)
lands THROUGH this container; and the container is 20 bytes of list + four EAs of semantics, so
it is testable with zero new classes — the exact T-013 Step-1 argument.

**Shipped shape (all T-074, re-verified against raw where this doc's steps lean on it):**
`Entity+0x2E0` deque, entries = Effect-tree ROOT POINTERS; **walk #1 = FIRST thing the entity
pump does** (0x54c597, before the mailbox pop 0x54c643, before walk #2 0x54c77c, before the anim
deque 0x54c89f): per entry → tick → done-predicate `[vft+0]` (= byte +0xB @0x45c090) → done ⇒
erase @0x54c61d + **finish/fire `[vft+0x2C]` @0x55a501 then dispose** (`0x55a570`); not-done ⇒
`inc cursor`, KEEP SCANNING — **skip-past, no head-of-line, whole deque visited per frame**; push
API `meth_0x5506c0` INLINE-FIRES an already-ready effect instead of enqueuing (0x5506ed-0x5506f9);
full drain `meth_0x54c390` ONLY from entity reset (0x5501a2).

**HOL OWNERSHIP, stated once (the four-queue law):** the ONLY head-of-line in the entity is
walk #2 on **+0x304** — the unfinished ACTION at the head breaks the walk (0x54c7f6→break,
T-070/T-085; "unfinished WORK," never unfinished idle: the transient arm completes its one tick,
P-REV/T-082). Inbox walk #1 = skip-past; mailbox = one-per-frame pop; anim deque = stamp-gated
pop. The port's FIFO must NOT generalize HOL into any other queue — step 1's assert (b) is the
pin that keeps it that way.

**Change (behavior, not bytes):** in `MapEntitySpawn.{hpp,cpp}` — `SpawnedEntity` gains
`EffectInbox { struct PendingEffect { int kind; SpawnedEntity* source; int dmg; int damageType;
bool pending0xA; } fx[4]; int fxCount; }` (fixed 4-slot block — SAME D-2 law applies, refuse at
capacity, cited to 0x54ea54's unbounded growth). **D-2 phrasing carried at its CURRENT restated
form (T-082 row 51), not the '3+resident' middle:** the transient idle occupies a block SLOT only
between insert and the next pump; a sustained-shipped first block holds **4 real orders**; a
shipped count of 5 at ANY instant still means block-chain growth and the D-2 refusal-at-4 stands —
the d2-stress fixture models the INSERT WINDOW, say so in its header once more. One primitive
`EnqueueEffect(target, pe)`: if `pe` is already ready (the 0x5506f1 analog: our v1 effects are
always ready-on-arrive ⇒ INLINE branch first, matching shipped order) fire+free, else append. Pump
runs `RunInboxWalk(e)` FIRST inside `PumpEntity`, per entry fire-on-done/erase-not-done as above.
Names carry no `0x` (item 42 rule), EAs in comments only.

**Acceptance:** new `SpawnMove`-suite block (or a new `EffectInbox` suite under the Action family):
(a) inline-fire-when-ready assert (enqueue of a ready effect fires during the ENQUEUE call, queue
stays empty); (b) skip-past: enqueue A(pending) then B(done) → B fires same walk, A survives;
(c) erase+fire+free ordering assert (fire before erase-visible state); (d) refusal at 5th with
D-2-style assert; (e) drain-at-reset assert (the port's entity-reset analog flushes and FIRES —
0x54c390's apply-not-cancel difference vs the action drain's cancel-first is asserted, NOT unified);
(f) annex survival-matrix assert — the port's ClearActions analog drains ONLY the action deque;
inbox/mailbox/anim entries must be provably untouched (assert counts unchanged after the call).
**Risk named:** v1 fires-never-cancels (no `[vft+8]` analog exists for effects; T-074 §4b says so
explicitly) — do not invent a cancel pass.

## Step 2 — route damage through the inbox (the deferral that makes it a combat-era doc)

**Shipped (T-074 §3 feeders, §2 census):** the attack finisher `GameActionAttack::meth_0x403770`
creates one `GameEffectDamage` per hit and pushes it to the TARGET via `meth_0x5506c0` @**0x403906**;
Apply `[vft+0x2C]` = 0x45C140 (HP write @0x45c1a3, clamp `cur:=min` 0x4629C0, reaction block with
`+0xAC/+0xAD` batch marks and the var_B1 residue) runs at FIRE time on the TARGET's walk #1 —
earliest: the target's next pump (same frame iff the world pumps the target after the attacker).

**Change:** the port's current direct-HP drain (GameActionAttack.cpp finisher, "Deviation: direct
HP write" comment) becomes: finisher builds `PendingEffect{kDamage, actor, dmg, damageType}` per
hit and `EnqueueEffect(target, …)`; the HP write, the ordered Apply chain (angle chain 0x45C243..,
unarmed-marks pre-pass, residue var_B1 mirror — ALL already ported inside the finisher loop), and
the `+0x171`-style mark move to the effect's fire slot. The per-hit math does not move; WHO-WRITES
and WHEN move. Reaction `+0xAD` suppression + pre-loop census keep their exact order (T-074 §3
already re-pinned it).

**Acceptance:** (1) deferral assert: attacker pumped before target in frame N → target hp shows
damage during frame N's target pump; attacker-before-target order swapped in a fixture → damage
lands frame N+1 (both orders are shipped-correct; the port pins WHICH order it uses in a comment
citing the busy-list iteration order question, not in code); (2) same-volley in-order drain still
ordered exactly per Apply; (3) digest `hp=` at the T-068 paired instants RE-RUNS green (the
transient-arm P-REV pins + D-2 refusal fixture must not move: inbox is NOT +0x304 — assert
`q=` unchanged by inbox traffic); (4) register the retired "direct HP write" deviation line in
DEFERRED-NOTES' deviations section via the T-079 batch lane (LIST, per lane law).
**Risk named:** multi-attacker same-frame interleave = block-order question (two inbox walks
compete per target pump order) — named in §7, no invention.

## Step 3 — the `+0x171` busy mark (cheap, pins the world-step coupling)

Shipped: effect fire sites store `movb $1,[ent+0x171]` (0x54c631/0x55b082/0x54c429); the WORLD
step scans busy-list and re-gates pending effects via `meth_0x54aa20` (walks +0x2E0 firing
gate-satisfied entries @0x54aa78, clears +0x171 @0x54aa82); composite busy predicate
`virt_meth_0x54ab00` counts `+0x2F0 != 0` (inbox size) as busy (T-074 §4e).
**Change:** one bool `effectMark0x171` on `SpawnedEntity`, set by fire/erase sites, consumed+cleared
by the world-step loop AFTER each entity's pump; `QueuedActionCount()`-sibling
`InboxCount()` feeds the busy predicate so an entity with a pending inbox and empty action deque
stays pump-registered. **Acceptance:** set/clear-once asserts; busy-with-empty-q fixture (the
idle-arm transient makes this easy now: post-idle frame, q=0, inbox=1 ⇒ mark path runs).
**Risk:** the re-gate walk `0x54aa78` iterates via mbr_0x2ec..0x2f0 — SAME container ops as step 1's
walk; reuse, do not duplicate (the T-074 §1b "instance chosen by this" law).

## Step 4 — state-7 `+0x1BC` semantics, aligned not renamed-blind + TurnPrep identity (already ported — VERIFY, don't re-derive)

Shipped writers: set on state 4/7 transitions @0x451c34-0x451c3a; **cleared by the anim-deque pop
flag rec[+0x14]** (0x54c93d/0x54c9f0 — step 6's territory); ServerUpdate lets state 7 pump ONLY
while `+0x1BC != 0` @0x5491c0. The port's `worldLinked` behavior (DEAD pumps while set) MATCHES;
its NAME implies a world-membership link the bytes never claim.
**Change:** comment-only (product lane): re-title to `pumpWhileDead0x1BC`, cite both writers and
the state-7 gate EA; the anim-pop clearer is CITED-AND-ABSENT until step 6 — and step 6's design
already schedules that writer. **Acceptance:** `ChangeEntityState` log-only; no behavior moves;
asserts pin current set sites + the 7-with-clear-then-no-pump behavior (already SpawnMove-pinned).

**TurnPrep clear identity (PARK-row element, stated as the VERIFY it is):** the shipped
clear+stamp virtual `virt_meth_0x549090` (`[ent+0x300] = arg_ms` @0x549094 + `movb $0,0x319` —
C1 in the T-085 §4 truth-table, per-entity per-state-5-node in manager maintenance 0x5a94cc-0x5a94fd
AND from GameCreature turn accounting `sub_409BE0` @0x409bfa) is **already ported**
(`CombatManager.cpp:113`, T-048 era; CombatTurnDispatch row 0x5a94a3-0x5a9511). What THIS era
adds: with damage deferral, the turn-cadence observables move — `[ent+0x300]` (last-turn-ms) and
the latch-clear must stay in lock-step with the manager's per-round walk while the volley's HP
write now lands one inbox-step later. No new port code in this step; **acceptance** = extend the
PathManager T-048 block with one assert-pair: round N's TurnPrep fires (latch 0, stamp set) while
the volley's damage from round N−1 is still pending in the target's inbox at stamp time — pinning
that the CLEAR clock and the DAMAGE clock are provably different clocks (this is the row's
'TurnPrep clear identity' element, consumed as a test, not re-written as prose).

## Step 5 — the script mailbox (+0x37C) seam + the kind-7 Message bridge, DUAL-PATH by annex

Shipped (T-080 + R-AMX): 3-alloc record via `sub_54EE60` (5 sites: 3 natives + ActionMessage start
fan-out 0x5a9107/0x5a91a7/0x5a9214); **pump pops ≤1/frame** between walk#1 and the script
dispatches (0x54c643, RunScript 'OnMessage' 0x54c676), in-flight guard +0x384 makes re-entrant
drains safe. **The dual-path law (R-AMX, the axis both committed rows kept true):** bare
`SendMessage`/`SendMessageToTeam`/`ToSquad` natives = **SYNCHRONOUS** — direct push to target
mailbox, no deque hop; `A_`-prefixed trio = **ASYNC** — native creates ONE kind-7 ActionMessage
funneled to the **SENDER's** +0x304, fan-out DEFERRED to its start (reading action+0x20/+0x24
team/squad cells) — so a queued-but-undispatched A_SendMessage **never delivers if the sender's
deque drains first** (A_Kill @0x5c8ec0, StartDialogue @0x45a0c2, TeleportParty, ClearActions —
the §2c drain set erases it; cancellability is the observable difference). ActionMessage update =
0x5a9070 — the SAME one-tick transient stub idle shares (P-REV stub law: name the sharing where
the kind table lives).
**Change:** `SpawnedEntity` gains `ScriptMail { int from; int msgId; int n; unsigned* payload; }`
ring (4-slot, D-2 law — shipped is an unbounded list, say so at the cite); `PushScriptMail`
primitive; pump pops one AFTER `RunInboxWalk`, BEFORE walk #2, hands it to the script-layer seam
(`EntityRunScript("OnMessage", from, id, payload...)` — native-registry stand-in already exposes
the surface). Bare-SendMessage stand-in native = direct `PushScriptMail` (sync); an `A_` variant,
if/when a script ever pairs, = kind-7 `kActMessage` transient arm whose start fans out — reusing
the P-REV transient shape, cited to 0x5a9070 once, and DELIVERED-vs-CANCELLED both tested. Driver
gains `push-mail <who> <from> <id> [payload…]`.
**Acceptance:** cadence assert (2 messages queued → one per frame, FIFO); re-entrancy guard
fixture (a native that pops during dispatch does not double-free); from-composition assert
([[action+0x14]+0x1DC] mirror: actor handle in the FROM slot); sync-vs-async pair: bare-send
survives a subsequent ClearActions on the sender; A_-send queued then drained before dispatch does
NOT deliver (the R-AMX cancellability observable, asserted not described); kind-7
done-before-second-frame assert pairing the idle transient (same stub, same lifetime).
**Risk:** the port has no AMX `myself`-style from-handle (D-3 stands) — mailbox carries whatever
handle the port mints at spawn; the FROM-field pairing story opens with D-3's fix row, NOT here.

## Step 6 — the anim-event deque +0x1A8 timeline: LAST, VALUE-GATED, already fully specced

Shipped (T-085 §1/§5, mirror spec verbatim): records = `new(0x18)` by-value; **explicit-time math
pinned**: `rec[+0x10] = Duration × param_5 × 10.0` at the feeder (0x5506a1), duration source
`Gfx_AnimationObject::GetDuration_4a26d0 × −1000.0` at the sink (0x548e79 — the neg-ms timeline,
T-070 §3 source-pinned), applied as `[ent+0x180] = worldms − rec[+0x10]` (0x548e3a) and
`[ent+0x184] = start + mode×dur` (0x548e8f/ea5); Path A (`[ent+0x188]≠0`, ungated inline pop) vs
Path B (ready-gate `worldms ≥ [+0x184]` + catch-up fast-forward `0x5643A0`); default-anim fallback
@0x54c9fe-0x54ca2b gated on the never-written `[ent+0x295]`. Nothing ported today needs it
(T-085 §8: no paired observable touches it; R4b's ActionAnim is a DIRECT-SINK bypass — the
existing pairs match without it). Land ONLY when either (a) T-067 BONUS-3 live values (bp12/bp13)
show mode≠0 / non-trivial rec[+0x10] records in real boots, or (b) an animation-timing field
enters the compared set. When it lands it also OWNS the deferred `+0x1BC` clearer (step 4).
Until gated: design frozen, `queuedAnimEvent` placeholder stands, no budget invented. The
VFX-adjacent `.veg` decode (if a step-6 fixture ever loads real files) consumes **R-VEG-E §1+§3
verbatim** — the checkpoint called it a LIFT-READY wire spec; do not re-derive field widths.

## Annex — the four-queue CLEAR/SURVIVAL matrix (R-AMX, annexed into this row at `d7ae655`)

`ClearActions` @0x5c8a00 drains ONLY +0x304 (via 0x5c8a50 → meth_0x54c440, the cancel-first
action-drain). **The +0x37C mailbox, +0x1A8 anim deque, and +0x2E0 inbox SURVIVE every native
drain** — containment-proven over the five drain primitives; no native invokes the inbox drain
(0x54c390's sole caller stays the entity reset @0x5501a2). Port law: the clear-primitive analog
drains actions (+ runs `[vft+8]` cancel) and touches nothing else; step 1 assert (f) + step 5's
cancellability pair ARE this matrix. Also preserved by annex: the "fifth queue"
(QueueFunction/ClearFunctionQueue) is AMX-side, NOT an entity surface — the port's queue inventory
stays FOUR, do not mint a fifth entity deque. bpN2 crosswalk (which drain fires live) belongs to
T-067's boot sitting — referenced, not duplicated.

## Cross-cutting rules (every step)

- **P-REV law:** the transient idle arm and walk-continue are LANDED — steps verify against them
  (step 2's `q=`-unchanged assert IS the alignment check), never re-derive or revisit them.
- Gate every step: `gate-integrity.sh` rc0 + `run-all-tests` (floors may rise, table edits are the
  two-lane cap) + `cppcheck-openje` 0 + `msvc-build` rc0 (headers move ⇒ MSVC leg is not optional,
  item 37) + `sim-driver-verify` PASS; submodule-first commits; both repos clean at close.
- Cited-and-absent stays the default for GUI/net kicks (0x54ef30 family), the anim pop-clear of
  +0x1BC, and the E2/E3 pair until step 6.
- No `0x` in new identifiers (item 42); EA in comments; D-numbers minted only for ACCEPTED
  differences (the 4-slot ring is a D-candidate once it refuses a 5th — register then, not now).
- M-001 (device frame era, landing as you read) sits BETWEEN steps 5 and 6 chronologically but
  this design's order is dependency order, not calendar: device decode stays LAST by construction.

## Annotation-errata list this era must not re-inherit (LIST; T-079 batch lane applies; raw re-read
this sweep at 0x54c828-0x54c86d: `mov eax,[ebp+0x174]` @0x54c828, `c6 85 18 03 00 00` arm stores
@0x54c85d-region/0x54c86d-ARM@0x54c86d, latch `c6 85 19 03` @0x54c833)

1. `docs/f3/CombatTurnDispatch.md` "§+0x319 raiser, deverified" + `docs/f3/SimDigest.md` T-048
   line: **"the +0x2E0 animated-event queue per T-039's structural note"** → +0x2E0 = **EFFECT
   INBOX** (T-074), anim deque = **+0x1A8** (T-085); the raiser sits on the **walk-#2 tail of the
   +0x304 ACTION deque** (T-085 §4/§7 order sentence is the canonical wording).
2. Same passages: **"dt budget at Entity+0x179"** → budget field is **[ent+0x174]** (raw @0x54c828;
   T-085 §4); "the +0x2E0+0x34 sibling byte" → arm byte is **[ent+0x318]** (raw c6851803 stores).
3. `src/F3/MapEntitySpawn.cpp` ~:556 comment "The sibling byte stores at 0x54c85d/0x54c86d
   (`+0x2E0+0x34`) belong to the unmodeled animated-event deque" → belong to the raiser's OWN
   exhausted-with-budget PAIR on the action walk (T-085 §4 E1-E3); the cited-and-absent decision
   STANDS, the attribution sentence is amended. Product lane owns this one, T-079 owns 1-2.
4. The PARK commission line carried 'D-2 3+resident semantics' — one generation stale (T-070
   middle wording); CURRENT form is T-082 row 51 (transient window; sustained first block = 4 real
   orders; 5-at-any-instant = growth; the D-2 refusal stands) — restated inside step 1 rather than
   quoted from the commission; relayed so the board text ages honestly.
5. R-VEG-E re-timed the ParameterHolder's 0x28 rows as an **RB-tree (name-keyed, `_stricmp`)** with
   the `&node.parameter` return explaining the old 'stride' reads — any port-adjacent prose that
   copied "0x28 stride rows" wording from R-VFM-2-era notes should cite R-VEG-E (no P-CMBT step
   depends on this; listed so the atlas stays one truth deep).

## What this design deliberately does NOT decide (honest gaps, named)

1. Two-attacker same-frame interleave INTO one target's inbox = busy-list pump order (shipped
   iterates `byte_70BE24`-family; port iterates `g_spawnedEntities` order) — settled by a
   driver-order fixture, not by inventing an order law.
2. The `+0x174` grant/reset formula (F3 round band 0x446fb6/0x455e11 cited; formula = T-011/turn
   territory).
3. Mailbox payload lifetime at destroy (T-080 §3 leak question) — if live-instrumentation ever
   covers a destroy-behind-queued-mail scenario, its answer edits step 5's ring capacity note.
4. Whether effect TREES (children at +0x1C) ever need porting at all: v1 is flat single-node
   effects (damage/equip notices); the tree tick/recycle (0x55afb0) is specced in T-074 §3 if a
   delayed-child effect ever pairs against something.
5. Anim timeline granularity (step 6 gate (a) data decides; the BONUS-3 fields are ALREADY on
   T-067's boot list — no new sitting is invented here).
