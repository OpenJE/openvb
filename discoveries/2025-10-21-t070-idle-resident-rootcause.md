# T-070 — GameActionIdle baseline resident: root cause (verdict: the port NEEDS an idle kind)

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Row:** T-070 (claim `b8a2b37`) · **Method:** read-only
statics on F3.exe (objdump off the nix-store exe + ida_original_f3 READ-ONLY decompiles; every claim
EA-anchored per rename-churn rule). IDA's `net::critical::section::*` names in the entity band are
namespace CORRUPTION (this is JE entity code) — EAs only.

## VERDICT (front, with the three disproofs of absence-equivalence)

**Shipped models idle as an ACTION, and the port's idle-as-absence is NOT behaviorally
equivalent.** Three shipped observables fail under absence:

1. **The resident is real queue content.** First-insert class = GameActionIdle 12/12 (T-056 R4b)
   because the feeder DRAINS the whole deque then enqueues a fresh Idle (0x551bbe→0x551bde→0x551be4).
   Every shipped `q` reading includes the resident → all T-056 counts (1→2→3) are "idle + N orders".
2. **Head-of-line blocking by a never-finishing action.** Idle's completion predicate is byte
   `[action+0x18]` (shared getter 0x404b60, vft+4); the wait-family base ctor zeroes it (0x56018d:
   `mov byte[+0x18],0`) and NO idle-owned code path ever writes +0x18 (its per-frame handler
   0x5a9070 writes +0x16 and returns 0 = consumes no ms). The pump (0x54c570, deque cursor
   Entity+0x304) BREAKS on a not-finished head → **orders enqueued behind an idle resident do not
   execute until a DRAIN removes the resident.** This EXPLAINS T-056's most confounding fact:
   "Attack never fired" in run H was not the bink crash and not a missed breakpoint — the scripted
   `F3::Attack` native (0x459f5d) enqueues WITHOUT draining (decompile shows: new 0xC8 → ctor →
   0x403fa0 set-target → funnel 0x550720, no drain call), so the attack stacked behind the resident
   and head-blocked forever. Real combat rounds fire because `sub_45C140` (the round handler, 8-of-18
   LCG draws doc) DRAINS via meth_0x54c440 at 0x45c5c6 BEFORE issuing.
3. **Idle's activation has entity-visible side effects.** [vft+0x10] start = 0x405620: resolves owner
   (R4's +0x14 store) through lookup 0x6165e0; on flag +0x20=1 (ctor 0x405605) stamps the entity's
   "action start" clock `0x548f90 → [entity+0x184] = world-sub vft[0]` (current world ms); then
   applies entity-state calls `0x4673d0(ent, 9, 0)` and `0x4673d0(ent, 1, [action+0x1C])` (+0x1C=0
   from ctor 0x4055fe) through the 141-case state machine. Absence reproduces none of this.

## 1. Class anatomy (vft 0x6497C4, raw 7 dwords — the export's "slot 0/slot 4" are indices; byte
##    offsets below, byte-offsets are what the pump uses)

| vft off | target | protocol role (pinned by sub_55A1C0 + pump 0x54c570) |
|---|---|---|
| +0x00 | 0x405690 | deleting dtor (dtor 0x405610 stores vft, tail ActionIdle dtor 0x4066f0; free 0x6162e0 when flag&1) — pump calls it with (1) on erase |
| +0x04 | 0x404b60 | **isFinished** = `return byte[this+0x18]` — idle NEVER sets it → resident |
| +0x08 | 0x404b70 | cancel/interrupt hook = `ret 0` (idle ignores cancel notification) |
| +0x0c | 0x4010d0 | getter byte[this+4] ("queued/valid" flag, ctor sets 1) |
| +0x10 | 0x405620 | **start(ms)** — owner resolve, [ent+0x184] stamp, state 9 then 1 calls (disassembled above) |
| +0x14 | 0x5a9070 | **update(ms,timer) → consumed** — sets [this+0x16]=1, returns 0; timer budget untouched |
| +0x18 | 0x404bb0 | on-finished notification = `ret 0` (unreachable for idle, predicate never true) |

Ctor 0x4055f0: base ctor 0x560180 (sets +0x04=1, +0x10 start-pending=1, **+0x18 done=0**, base vft
0x695afc) then vft→0x6497c4, **+0x1C=0** (state-1 call arg), **+0x20=1** (stamp flag). Size 0x24 alloc
(0x21 class). Shared slot pool: +4/+8/+0xc/+0x18 identical to GameActionWait (vft 0x64bc6c) —
Wait differs only at +0x14 (0x5601d0 timer-expiry update, which DOES set +0x18 done). Attack (vft
0x647bbc) overrides ALL protocol slots — confirming +4/+0x10/+0x14/+0x18 as the action protocol.

## 2. Producer census (who enqueues idle)

`F3::GameWorld::CreateGameAction_0x46f290` (arg = GameActionType; this-adjusting wrapper at
GameWorld-sub vft **0x668494 +0x14** → 0x46fdb0 → 0x46f290; sub-object installed by ctors 0x46f1c7/
0x47039a). THE FACTORY ENUM (case → class → alloc size): 0=GameActionDie 0x20 (+0x1C=7), **1=GameActionIdle
0x24 (ctor 0x4055f0 — the class's ONLY construction site)**, 2=GameActionMoveTo 0x48, 3=GameActionPickUp
0x30, 4=GameActionWander 0x44, 5=GameActionWait 0x24, 6/7=fall-through JE::World::CreateAction
(engine kinds), 8=FloatingSpeechText 0x34, 9=Equip 0x30, 10=Unequip 0x28, 11=SetActiveWeapon 0x44.
(GameActionAttack/OperateDoor/UseSkill/Give/InitiateDialogue are NOT factory kinds — direct-constructed
at their command sites.)

Idle feed sites (create kind 1 → drain or funnel):
- **`meth_0x551ba0` (entity-band; `this`=entity — THE rebasing feeder):** guard → **drain-all
  meth_0x54c440 @0x551bbe** → `push 1; call [worldsub vft+0x14]` @0x551bde (CreateGameAction(Idle)) →
  `ecx=ent; push obj; call funnel 0x550720` @0x551be4 (owner store +0x14 fires = R4) → anim bookkeeping
  ([ent+0x1EC]→worldsub vft+0x54→[ret+0x118]++). Callers: `JE::Server::meth_0x555750` and
  `JE::Client::virt_meth_0x58c660` — **BOTH net contexts**, i.e. the demo's client and local server
  each rebase entities onto idle.
- **`JE::Server::virt_meth_0x556ea0` (server command pump):** drain @0x55763b + create-idle
  @0x557657 (`call sub_54a8a0; call sub_401130; push 1; call [vft+0x14]`) + funnel @0x55765d/0x5576f7 —
  per-command rebasing on the server path (3 drains/2 funnels inside the 0xb0b-byte handler).

Funnel census (`0x550720` = push_back + `*(action+0x14)=entity` + GUI-busy kick 0x54ef30; xrefs, 60-cap
truncated): script natives F3::Attack/JustAttack/StartDialogue/ReloadWeapon, GameActionAttack
self-re-queue ×6 (OnServerUpdate 0x404320 — combat rounds), MoveTo waypoints ×6, OperateDoor ×2,
OperateContainer ×2, PickUp ×2, UseSkill ×2, Give ×2, InitiateDialogue ×2, Wander ×3, JE-side
PickUp/Wander, TransitionTrigger, FloatingSpeechText, SendMessage ×3 families, projectile,
GameServer::virt_meth_0x46dd70 ×8 (net ingest), and the feeders above. ALL inserts share the funnel →
T-056's "35/35 owner stores bitwise" is this universal store.

## 3. Consumer map (what advances on idle) — CONSUMER HALF PINNED (item 45/harness 46 honored:
the +0x319 raiser is shipped-LIVE shared PumpEntity; nothing here relies on the walk-only half)

- **Per-frame pump `JE::Entity::virt_meth_0x54c570`** — TWO deques: first walk (Entity+0x2E0 region;
  dispatches [vft+0] as predicate?? NO — that walk ticks message-objects via meth_0x55afb0 and
  erases on [vft+0] nonzero — separate structure, NOT the +0x304 action queue; do not conflate) and
  **the +0x304 action walk** documented here: cursor=+0x310 (T-056 R3 `mbr_0x10`), blocks via
  `[blocks[i/4] + 4*(i%4)]` (first-block cap 4 — matches our port's actions[4] shape question),
  per-entry `timer(+0x174-ish budget) -= sub_55A1C0(action, ms, timer)`; predicate [vft+4] TRUE →
  erase via cls_0x54b5c0::meth_0x54b5c0 + deldtor(action,1), CONTINUE walk; FALSE → set wait-byte,
  **BREAK** (head-of-line block).
- **`sub_55A1C0` = the protocol runner:** if start-pending byte (+0x10 object byte, set by ctor) →
  clear it and call **[vft+0x10] start(ms)**; then **[vft+0x14] update(ms, timer) → ms consumed**;
  if **[vft+4] isFinished** → **[vft+0x18]** notify. Idle: start runs once (activation side effects),
  update consumes 0 ms forever, predicate 0 forever → break-every-frame resident.
- **Reapers (callers of drain-all meth_0x54c440, full census):** JE::ClearActions_5c8a00,
  JE::Kill_5c8e70, JE::TeleportParty, F3::StartDialogue @0x45a0c2, **sub_45C140 (combat rounds)
  @0x45c5c6**, GameServer::virt_meth_0x46dd70 ×2, Entity update glue meth_0x54fff0 @0x5501a9,
  **the idle feeder itself @0x551bbe**, Server command pump ×4 @0x5572xx/0x5576xx,
  TransitionTrigger @0x5b1b82. Drain runs [head vft+8] (idle: ret 0) BEFORE erasing each — idle
  survives cancel-notification-free but never survives a drain.
- **Empty/presence side effects:** enqueue kick 0x54ef30 registers the entity in the GUI busy-list
  (global byte_70BE24 doubly-linked list, byte_70BDE8=1); when a walk finds nothing active,
  meth_0x54bf00 de-registers. Plus the pump's own else-branch: empty + flags → **animation-level**
  default via vft+20/+22 → meth_0x548d80 (anim START path — AnimMeshInstance + duration×-1000 into
  the +0x1AC/+0x307C timeline fields) — NOT an action re-enqueue; do not conflate the idle ACTION
  resident with the idle ANIMATION default (two distinct mechanisms, both "idle").

## 4. Why T-068's R3 pairing looked like a MATCH (and wasn't)

First-insert shipped count=1 and ours q=1 both read "one element". Shipped's element is the idle
RESIDENT (activation side effects + block semantics); ours was the first REAL order. The counts
agree; the states do not. R4b was the tell. (Our port's "q derives from actions[] so cap 4" comment
in SimDigest.hpp stands, but shipped's usable capacity is effectively 3 + resident under the drain
protocol, and a shipped q=4 beside our q=4 is NOT the D-2 comparison any more — D-2's ">4" question
must now subtract the resident when R5/R6 data arrives.)

## 5. Named follow-ups (verdict's second half — no guess was allowed, none made)

**Port row (new, feeds from this doc):** add the idle resident with its THREE proven behaviors:
(kIdle=1 kind in the port enum per the FACTORY table above — note our current ActionKindName numbering
is NOT the shipped GameActionType numbering: shipped 2=MoveTo ours 1=MoveTo; rename-wave-proof it by
the factory table), the drain-then-enqueue rebase on the "state changed / command boundary" sites
mapped above, never-self-finishing predicate, +0x16/+0x18 field separation, activation side effects
(stamp + start-pending byte), and head-of-line break (port pump already blocks-on-head — the missing
piece is ONLY the resident that gets broken-ON). D-2's cap question must be re-expressed with the
resident counted.

**T-067 live-confirm candidates (R5-adjacent, cheap on the :96 rig):** (a) at a scripted-Attack
instant, dump deque slot0 class (expect GameActionIdle still head, count 2, attack NOT advanced —
proves HOL live); (b) byte reads [idle+0x16] and [idle+0x18] across frames (expect 1 and 0);
(c) [entity+0x184] before/after a rebase (expect world-ms stamp at activation); (d) whether drain
@0x551bbe and create @0x551bde fire in one frame per command (cadence of rebasing).

**Deferred (named, not guessed):** the first deque (+0x2E0 walk)'s protocol ([vft+0]-as-predicate
reading there is suspicious — likely a message/event queue with its own class family; belongs to
T-052 band, NOT this row); `0x6165e0` lookup table args (0x6f1e78/0x6f1f24 are .data pointers,
not strings — entity-table handles, unresolved this pass).

## 6. Gate/evidence posture

Research row — no build-affecting edits. Every claim carries an EA; EAs re-verified raw off
F3.exe (md5 a68c22f3…) this session, post-respawn, nothing pre-respawn trusted. Doc left DIRTY
per protocol; row status flip committed separately.
