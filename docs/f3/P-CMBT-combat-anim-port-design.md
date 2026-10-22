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
(anim deque + +0x319 truth-table + BP box), `docs/f3/EntityActionQueueSeam.md` (funnel/D-2),
`docs/f3/CombatTurnDispatch.md` (manager, latch consumer), T-048/T-069 (raiser port), T-056/T-068
(pairing instants), `docs/f3/GameEffect.md`+`GameEffectDamage.md` stubs (claims live in the
discoveries, not the stubs).

## Scope freeze

IN: the **effect inbox (+0x2E0) and its damage path**, pump-walk ORDER, the +0x171 busy mark,
state-7 `+0x1BC` semantics, the **script mailbox (+0x37C)** seam + kind-7 Message bridge, raiser
PAIR completion as cited-absent with corrected cites, digest/driver hooks, and a three-line
annotation-errata list the port era must not re-inherit.
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

**Change (behavior, not bytes):** in `MapEntitySpawn.{hpp,cpp}` — `SpawnedEntity` gains
`EffectInbox { struct PendingEffect { int kind; SpawnedEntity* source; int dmg; int damageType;
bool pending0xA; } fx[4]; int fxCount; }` (fixed 4-slot block — SAME D-2 law applies, refuse at
capacity, cited to 0x54ea54's unbounded growth). One primitive `EnqueueEffect(target, pe)`: if
`pe` is already ready (the 0x5506f1 analog: our v1 effects are always ready-on-arrive ⇒ INLINE
branch first, matching shipped order) fire+free, else append. Pump runs `RunInboxWalk(e)` FIRST
inside `PumpEntity`, per entry fire-on-done/erase-not-done as above. Names carry no `0x` (item 42
rule), EAs in comments only.

**Acceptance:** new `SpawnMove`-suite block (or a new `EffectInbox` suite under the Action family):
(a) inline-fire-when-ready assert (enqueue of a ready effect fires during the ENQUEUE call, queue
stays empty); (b) skip-past: enqueue A(pending) then B(done) → B fires same walk, A survives;
(c) erase+fire+free ordering assert (fire before erase-visible state); (d) refusal at 5th with
D-2-style assert; (e) drain-at-reset assert (the port's entity-reset analog flushes and FIRES —
0x54c390's apply-not-cancel difference vs the action drain's cancel-first is asserted, NOT unified).
**Risk named:** v1 fires-never-cancels (no **slot 8** / `[vft+0x20]` analog exists for effects —
the bare `[vft+8]` this line once carried was the notation trap: 8 is a SLOT number, and the
slot's byte displacement is +0x20, whose body is the probe at `0x45c100`); T-074 §4b says so
explicitly) — do not invent a cancel pass.

## Step 2 — route damage through the inbox (the deferral that makes it a combat-era doc)

**Shipped (T-074 §3 feeders, §2 census):** the attack finisher `GameActionAttack::meth_0x403770`
creates one `GameEffectDamage` per hit and pushes it to the TARGET via `meth_0x5506c0` @**0x403906**;
Apply `[vft+0x2C]` = 0x45C140 (HP write @0x45c1a3, clamp `cur:=min` 0x4629C0, reaction block with
`+0xAC/+0xAD` batch marks and the var_B1 residue) runs at FIRE time on the TARGET's walk #1 —
earliest: the target's next pump (same frame iff the world pumps the target after the attacker).

> **SLOT CORRECTION + SCHEDULE CORRECTION (NiceTiger `c1fdb7a`, independently re-derived from the PE at
> TrueRaven's request before this line was touched — item 75's adoption rule).** Two of the cites above are
> wrong, and they are wrong in *different* directions, so read both:
>
> 1. **The apply path is `[vft+0x28]` (slot 10), not `[vft+0x2C]`.** Slot 11 (+0x2C) of the effect vft is
>    `0x404b70`, whose first byte is `c3` — a bare `retn`. The wrapper at **`0x45c6b0`** is the vft entry, and
>    **`0x45c140` appears in no vft cell anywhere in the image**: it is reached only by the wrapper's
>    tail-jmp `e9 76 fa ff ff`. (So "apply = 0x45C140" was right about the function and wrong about how the
>    table reaches it.) The raw wrapper, byte by byte:
>
>        45c6b0 56 8b f1          push esi / mov esi,ecx
>        45c6b3 c6 46 0b 01       mov byte [esi+0Bh],1   <-- ready set BEFORE anything else
>        45c6b7 e8 64 5f 00 00    call 0x462620          <-- gate; can throw SEH
>        45c6bc 84 c0 75 02 5e c3 test al,al / jnz / pop esi / retn
>        45c6c3 8b ce 5e          mov ecx,esi / pop esi
>        45c6c6 e9 76 fa ff ff    jmp 0x45c140           <-- tail-jmp into the apply body
>
> 2. **"Damage never queues" — so the sentence above claiming it "runs at FIRE time on the TARGET's walk #1"
>    is wrong about the schedule for damage effects.** Because `[+0xB]` is set to 1 *unconditionally at
>    0x45c6b3, before the gate even runs*, the ready test downstream always passes and the effect is
>    inline-fired during the enqueue call; the `+0x2E0` enqueue path @0x550701 is **unreachable for
>    GameEffectDamage**. The only other writers of byte +0xB are `0x55a460`'s false-predicate leg and
>    `0x45c35f`, both of which also store 1 — no path leaves damage at 0.
> 3. **REFUTED, AND MY ORIGINAL SENTENCE WAS THE ERROR (FastNova, byte-clean; re-verified here before
>    editing, then again by NiceTiger).** A lethal hit does enqueue `GameActionDie` via `call 0x550720`
>    @`0x45c626` - true. But that funnel's own stream is `8d 8e 04 03 00 00` = `lea ecx,[esi+0x304]`
>    @0x550726: the **ACTION deque**, not the effect deque at `+0x2E0`. So this sentence was never evidence
>    about the container Step 1 builds. **Surviving claim, narrow: damage never queues** - and its counterpart
>    is now ANSWERED, by the fourth site neither seat had gated: **spawn / delta-record replay @`0x552692`**,
>    inside Entity vft `0x692084` **slot 10** (function begins `0x552420`). Verified a third way here straight
>    from the PE: `8d 8e e0 02` = `lea ecx,[esi+0x2E0]` @**0x55268c**, and **no write of byte `+0xB` anywhere
>    in the surrounding 0xa0 bytes** - so the replay path pushes re-created effects (factory by recorded type
>    word; slot-2 assign copies the recorded `+0x9C` damage cell) with **no readiness test and no fire-walk**.
>
>    **The distinction is the finding, not the fact of queueing: replayed damage queues; live-feeder damage
>    still never does**, because `0x45c6b3` forces `+0xB=1` before the gate on the live path. So Step 1's
>    `skip-past`/multi-entry law IS reachable by shipped traffic - via reconnection/delta replay, not via
>    combat. Better grounds than the withdrawn death claim, and weaker than "combat exercises it": replay is a
>    path a given demo session may never take. Both halves are bounded absences with windows stated - the 0xa0
>    bytes around 0x55268c, and the still-unchecked equip-notice / `DropPayload` / `EffectTreeTrigger` feeders.
>    Full receipt is FastNova's `79b6c81` §2b; the question this section left open was closed by NiceTiger.
>
>    **The error class, in two layers, because both are needed.** (i) *A reused verb is not a shared data
>    structure* - "enqueues" is said of two containers and the inference crossed without a byte doing it.
>    (ii) escalated by NiceTiger's self-audit: **a reused TYPE is not a shared instance either.** `+0x304` and
>    `+0x2E0` are the SAME class (`cls_0x54e070`) and different instances, which is why the claim felt safe -
>    there WAS a real shared type beneath it. Type-checking is the reassurance to distrust, exactly like the
>    slot number and the byte offset before it. **Rule: name the FIELD OFFSET HOLDING THE INSTANCE** - not the
>    class, not the verb.
>
>    Step 2 consequence unchanged in direction: model **synchronous push-side** application, not a drain.
>    Step 1's `skip-past`/multi-entry law is asserted from the WALK's bytes (@0x54c63d `inc` / @0x54c63e
>    `jmp`), i.e. container law rather than a shipped-traffic claim - which is the honest basis now that the
>    traffic claim is withdrawn.
> 4. **Step 2's blast radius is larger than "HP write."** Apply's target-side touch list (all cited in
>    `docs/f3/P-CMBT-step2-feeder-evidence.md` §4): clamp-store `[tgt+0x24]` with **no lower bound** (negative
>    HP is proven for this writer — the clamp hole's byte-level receipt), first-hit-and-survive-gated
>    `QueueAnimEvent` (ids `0x21/0x22/0x25/0x26/0x28/0x29`, keyed on tgt-adj `+0x3A6` + var_B1 backhit),
>    VEG by type/material with a **damage==0 GRAZE** path → `FX_HitRicochetMetal`, sound via `sub_48CD00`
>    rolling `[eff+0xA8]`, a **SYNCHRONOUS `OnDamage` script invoke** (`sub_55A220` → `RunScript_54afe0`,
>    4-cell block {clock base, src handle, damage, server} — a second script-by-name witness for gap 3, this
>    one combat-side), a `GameServer::meth_0x46d020` broadcast (+0x1E0 field), and `GameActionDie`
>    construction+seam-enqueue on lethality. Damage is **copied** per hit, not re-rolled: fresh factory call
>    inside the per-hit loop (`[server vft+40](type 1)` @0x403851), stamped +0x9C/0xA0/0xA4/0xA8 by DIRECT
>    movs from the hit entry @0x4038b3-0x4038d1; batch marks +0xAC (multi) / +0xAD (non-first) @0x4038e8-f7.
>    Multi-hit attack = N objects, N pushes to the same resolved target.
> 5. **Lock-step caution, and a new member of the B8 family.** Apply is SEH + `__security_cookie` + two
>    jump tables + x87 — step counts will not match a naive port. And the gate at `0x462620` at the top of
>    slot 10 **throws the shipped SEH exception "Invalid attempt to retrieve world server instance."** on a bad
>    world-server flag, with a null-singleton leg at `0x46265d-64` that is the guarded-selection/unguarded-use
>    idiom verbatim — the fourth known member of that family, now inside the damage path itself.
>
> **Left in place, not rewritten:** line 41's `finish/fire [vft+0x2C] @0x55a501` was NOT re-verified here.
> That call site's object may be a different class from the effect vft, and a bare `retn` at slot 11 of THIS
> vtable is not evidence about the other. Per item 73, that is an unverified neighbor, not a refutation.

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

## Step 4 — state-7 `+0x1BC` semantics, aligned not renamed-blind

Shipped writers: set on state 4/7 transitions @0x451c34-0x451c3a; **cleared by the anim-deque pop
flag rec[+0x14]** (0x54c93d/0x54c9f0 — step 6's territory); ServerUpdate lets state 7 pump ONLY
while `+0x1BC != 0` @0x5491c0. The port's `worldLinked` behavior (DEAD pumps while set) MATCHES;
its NAME implies a world-membership link the bytes never claim.
**Change:** comment-only (product lane): re-title to `pumpWhileDead0x1BC`, cite both writers and
the state-7 gate EA; the anim-pop clearer is CITED-AND-ABSENT until step 6 — and step 6's design
already schedules that writer. **Acceptance:** `ChangeEntityState` log-only; no behavior moves;
asserts pin current set sites + the 7-with-clear-then-no-pump behavior (already SpawnMove-pinned).

## Step 5 — the script mailbox (+0x37C) seam + the kind-7 Message bridge

Shipped (T-080): 3-alloc record via `sub_54EE60` (5 sites: 3 natives + ActionMessage start fan-out
0x5a9107/0x5a91a7/0x5a9214); **pump pops ≤1/frame** between walk#1 and the script dispatches
(0x54c643, RunScript 'OnMessage' 0x54c676), in-flight guard +0x384 makes re-entrant drains safe
(the handler may pop the node the pump still holds); ActionMessage = engine kind 7 (table raw),
start = bridge (+0x304 → target mailboxes), update = 0x5a9070 — the SAME one-tick transient stub
idle shares (P-REV's stub note says the DB under-registers shared stubs: name the sharing in the
port comment where the kind table lives).
**Change:** `SpawnedEntity` gains `ScriptMail { int from; int msgId; int n; unsigned* payload; }`
ring (4-slot, D-2 law — shipped is an unbounded list, say so at the cite); `PushScriptMail`
primitive; pump pops one AFTER `RunInboxWalk`, BEFORE walk #2, hands it to the script-layer seam
(`EntityRunScript("OnMessage", from, id, payload...)` — the native-registry stand-in already
exposes the surface); a `push-mail <who> <from> <id> [payload…]` driver verb. Kind-7
`kActMessage` = port-side transient arm: start fans out to PushScriptMail on resolved targets,
first processed tick sets done (mirror the idle P-REV shape verbatim, cite 0x5a9070 once, note
the stub is SHARED with idle).
**Acceptance:** cadence assert (2 messages queued → one per frame, FIFO); re-entrancy guard
fixture (a native that pops during dispatch does not double-free); from-composition assert
([[action+0x14]+0x1DC] mirror: actor handle in the FROM slot); kind-7 done-before-second-frame
assert pairing the idle transient (same stub, same lifetime — the test shows it).
**Risk:** the port has no AMX `myself`-style from-handle (D-3 stands) — the mailbox carries
whatever handle the port mints at spawn; the pairing story for FROM fields opens with D-3's fix
row, NOT here.

## Step 6 — the anim-event deque +0x1A8 timeline: LAST, VALUE-GATED, already fully specced

Nothing ported today needs it (T-085 §8: no paired observable touches it; R4b's ActionAnim is a
DIRECT-SINK bypass — the existing pairs match without it). Land ONLY when either (a) T-067
BONUS-3 live values (bp12/bp13) show mode≠0 / non-trivial rec[+0x10] records in real boots — then
mirror spec = T-085 §1 record table + §5 stamp equations (`[+0x180]=worldms−rec[+0x10]`,
`[+0x184]=start+N×dur`, Path A/B, catch-up fast-forward `0x5643A0`), sink behind the device seam
(M-001 family); or (b) a digest field for animation timing enters the compared set.
When it lands it also OWNS two deferred writes: the raiser E2/E3 arm-pair is NOT anim-side (step-6
adjacent correction — see the errata list) but the `+0x1BC` clearer IS. Until gated: design frozen,
`queuedAnimEvent` placeholder stands, no budget invented.

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

## Annotation-errata list this era must not re-inherit (LIST; T-079 batch lane applies; raw re-read
this sweep at 0x54c828-0x54c86d: `mov eax,[ebp+0x174]` @0x54c828, `c6 85 18 03 00 00` arm stores
@0x54c85d-region/0x54c864-ARM@0x54c86d, latch `c6 85 19 03` @0x54c833)

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

## What this design deliberately does NOT decide (honest gaps, named)

1. Two-attacker same-frame interleave INTO one target's inbox = busy-list pump order (shipped
   iterates `byte_70BE24`-family; port iterates `g_spawnedEntities` order) — settled by a
   driver-order fixture, not by inventing an order law.
2. The `+0x174` grant/reset formula (F3 round band 0x446fb6/0x455e11 cited; formula = T-011/turn
   territory).
3. Mailbox payload lifetime at destroy (T-080 §3 leak question) — if live-instrumented ever
   covers a destroy-behind-queued-mail scenario, its answer edits step 5's ring capacity note.
4. Whether effect TREES (children at +0x1C) ever need porting at all: v1 is flat single-node
   effects (damage/equip notices); the tree tick/recycle (0x55afb0) is specced in T-074 §3 if a
   delayed-child effect ever pairs against something.
5. Anim timeline granularity (step 6 gate (a) data decides; the BONUS-3 fields are ALREADY on
   T-067's boot list — no new sitting is invented here).
