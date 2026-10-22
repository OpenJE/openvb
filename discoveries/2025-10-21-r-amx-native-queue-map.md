# R-AMX — The AMX-natives → queue-surface consumer map (the atlas front door, closed)

**Agent:** WildZenith (sessionId `01a16092-2ef1-703e-b403-9e417653ef5f`) · **Date:** 2026-10-21 · **Row:** R-AMX (PARK pull, claim `7de33f4`) · **Method:** read-only; native f3_*/xref + raw objdump on md5-pinned exe; ecx-flow selftest 6/6 gate before verdicts; zero IDB writes.

## 0. THE COMPLETENESS ARGUMENT (this row's backbone — read first)

Every push/pop/drain of every queue surface funnels through FIVE primitives, each established with a COMPLETE (`more:false`) xref census across T-074/T-080/T-085 and this row:
- `0x550720` (+0x304 action funnel) — **63 sites, complete** (re-pulled at limit 300 this row; T-070's 60-capped census superseded — full list below)
- `0x54c440` (+0x304 drain-all) — **15 sites, complete**
- `0x54EE60` (+0x37C mailbox push) — **5 sites** (T-080)
- `0x5505F0` (+0x1A8 anim push) — **267 sites** (T-085; host = `sub_54EB40` push primitive has exactly 2 xrefs)
- `0x5506c0` (+0x2E0 effect-inbox push) — **6+1 sites** (T-074)

Therefore a native touches a queue **iff its handler address-range contains one of those sites**, and the native handler ranges are exactly the `{name, fn}` entries of the registration tables (§1). No per-handler walking needed — containment against complete censuses is the whole classification. This is the atlas earning its keep: the surfaces were closed callee-side; the door is classified without opening 209 doors.

## 1. Registration surface (the census)

**One registrant, three tables, 209 natives, zero truncation.** `JE::Script_Small::Load_5a8880` (per-script load) initializes the AMX (`sub_5CC810`) then makes exactly three register calls to `sub_5CCDE0` (= amx_Register; EA @0x5a8ada/0x5a8ae7/0x5a8af4):

| table | EA | entries | character |
|---|---|---|---|
| float library | `0x7004C0` | **9** | float/floatstr/floatmul/div/add/sub/fract/round/cmp @0x5ce5f0-0x5ce7c0 |
| JE engine natives | `small_script_functions` @0x6ffaf8 | **155** | the entity/action/inventory/team/dialogue/clock API |
| F3 game-layer natives | `off_6F3750` | **45** | A_Attack/JustAttack/StartDialogue/Equip/Unequip/Lock/Reload/stats getters/quests |

Method + strength (item 41): (a) the `amx_Register`-equivalent has 4 xref sites total — 3 real (above) + one `sub_5CCDE0(amx, 0, 0)` inside `sub_5CCE50` = **the AMX VM interpreter core itself** (the giant bytecode switch — not a registrant); (b) the 155-table was walked to its terminating `{0,0}` sentinel at 0x6fffd0 (Count = 155 exact, not truncated); same for 9 (0x700508) and 45 (0x6f38b8); (c) name-string cross-check: the .rdata name cluster @0x6ad534+ contains every table name contiguously. Total = **209**. The F3-45 table's `OpenQuest/CloseQuest/AddQuestEntry/AddJournalEntry` all share one handler `0x420990` (4-to-1 — noted, not a queue toucher).

Handler arg-plumbing (for the boot lane reading scripts): prologues are `sub_59AF40/59AF90(amx, params, n, "Name")` arity checks; entity handles resolve via `sub_59AEF0(amx, cell, &out, 1)` ("…was unable to find the requested entity." strings); point/float triples via `sub_59AC40`; strings via `sub_59AB20`; AMX-param readout `sub_5CCCC0(amx, FOURCC, &out)`; errors through `sub_59AD20(amx, Format)`.

## 2. THE MATRIX — 19 queue-touching natives (all cells EA-anchored; selection per §0 helpers)

### 2a. ACTION DEQUE +0x304 — funnel `0x550720` (16 natives; kind per `push $K; call *[vft+0x14]` at the create site, or direct ctor noted)

| native | table | handler | funnel site | action created | selection → target |
|---|---|---|---|---|---|
| A_MoveToEntity | JE | 0x5c8b70 | 0x5c8c3a | **direct ctor `0x55b350` (ActionMoveTo, no factory)**; fields +0x30/+0x38 (entity id)/+0x3C=1 | handle cell → entity |
| A_MoveToPosition | JE | 0x5c8c50 | 0x5c8d28 | factory **kind 2** @0x5c8d00; floats +0x24/+0x28/+0x2C, +0x30 | handle cell |
| A_Wander | JE | 0x5c8d40 | 0x5c8e58 | factory **kind 4** @0x5c8e3f + bounds ctor `0x5a2820(UL,LR)`, +0x34=param4 | handle cell; bounds = two point params |
| A_Wait | JE | 0x5c8f00 | 0x5c8f7c | factory **kind 5** @0x5c8f67 + ctor `0x568420` (duration → action+0x1C per T-085 wait-ctor read) | handle cell |
| A_PlayAnim | JE | 0x5c8f90 | 0x5c901e | factory **kind 6 (ActionAnim)** @0x5c8ff7; +0x18/+0x1C name pair, +0x20 u16 mode, +0x22 bool | handle cell |
| A_PickupItem | JE | 0x5c9460 | 0x5c950f | factory **kind 3** @0x5c94f9 + `0x406a30(item)` | handle cell + item handle |
| A_SendMessage | JE | 0x5c5120 | 0x5c5294 | factory **kind 7 (ActionMessage)** @0x5c5276; +0x28 from-cell, +0x40 msg-id, payload via `0x5a9020(argc-2, buf)` | funneled to the **from** entity (`0x54be10` on the 'from' cell; fan-out DEFERRED to ActionMessage::start — T-080) |
| A_SendMessageToTeam | JE | 0x5c52b0 | 0x5c53f4 | kind 7 + **action+0x20 = team cell** @0x5c53de | same, team fan-out deferred to start |
| A_SendMessageToSquad | JE | 0x5c5660 | 0x5c57a4 | kind 7 + **action+0x24 = squad cell** @0x5c578e | same, squad fan-out deferred |
| A_FloatingSpeechText | JE | 0x5c2710 | 0x5c27c4 | factory **kind 8** @0x5c27a9; fields +0x18..+0x28 (text/color/pos) | handle cell |
| A_Kill | JE | 0x5c8e70 | 0x5c8ee8 | factory **kind 0 (Die)** @0x5c8ede — AFTER drain @0x5c8ec0 (§2c) | handle cell |
| TeleportParty | JE | 0x5ca3a0 | 0x5ca471 | factory **kind 1 (Idle)** @0x5ca46b per member — AFTER drain per member (§2c) + server map-move `JE::Server::meth_0x5563d0(ent, mapName, x/y cell, 5−i%6, 1)` | **PARTY iteration** (list via worldsub `[vft+0x68]` snapshot @0x5ca429, all members) |
| A_Attack | F3 | 0x459eb0 | 0x459f5d | **direct `new`+ctor `0x403e40` + `0x403fa0(target)` — no factory** (GameActionAttack) | attacker cell + victim cell |
| A_JustAttack | F3 | 0x459f70 | 0x45a025 | same ctor path + writes action bytes **+0x6C=0, +0x6F=0** (the "just" — no-anim/no-wait flags at those fields) | same |
| StartDialogue | F3 | 0x45a040 | 0x45a0f0 | direct `new(0x24)` + ctor `0x404b40` (InitiateDialogue); +0x1C=dialog id, +0x19=0 — AFTER drain @0x45a0c2 (§2c) | handle cell |
| A_ReloadWeapon | F3 | 0x45a380 | 0x45a3e6 **and** 0x45a3fb | direct `new(0x3C)` + ctor `0x4071c0` (GameActionReload — alloc 0x3C raw-confirmed, pairs KeenStorm's P-REV erratum note: NOT 0x2C) | handle cell; TWO funnel sites = the two weapon-path branches (both same action kind) |

The remaining 47 funnel sites are engine-side (action self-requeues ×~30 incl. GameActionAttack::OnServerUpdate ×5/MoveTo ×6/Wander ×3/Give ×2/Door ×2/Container ×2/PickUp ×4/UseSkill ×2/InitiateDialogue ×2, projectile, `sub_435480`/`sub_435DE0`/`sub_467640` (net command helpers), GameServer net-receive `virt_meth_0x46dd70` ×10, combat round `sub_45C140`, idle-rebase `0x551ba0`, server pump `virt_meth_0x556ea0` ×3, `0x551be4`, `0x401890`, TransitionTrigger) — **no other native reaches the funnel.** The server-pump indirection question is closed: `0x556ea0` has exactly 2 callers, both inside `GameServer::virt_meth_0x46dd70` (net receive) — natives do NOT reach the funnel through it.

### 2b. MAILBOX +0x37C — direct push `0x54EE60` (3 natives — the BARE trio, T-080)

| native | table | handler | push site | selection | delivery |
|---|---|---|---|---|---|
| SendMessage | JE | 0x5c5410 | 0x5c5503 | handle cell → single target | **immediate**: target's +0x37C, consumed next pump-frame pop, OnMessage script callback |
| SendMessageToTeam | JE | 0x5c57c0 | 0x5c5908 | `GetTeamMemberList_56a2f0` loop at CALL time | per-member immediate push |
| SendMessageToSquad | JE | 0x5c5930 | 0x5c5a7a | `GetSquadMemberList_569390` loop | per-member immediate push |

### 2c. THE DRAIN-AT-DIALOG PATTERN (+0x304 `meth_0x54c440`, 4 natives)

`ClearActions` @0x5c8a00 (the pure drain native — drain @0x5c8a50, nothing else), `A_Kill` @0x5c8ec0, `TeleportParty` @0x5ca44b (per member), `StartDialogue` @0x45a0c2 — drain-then-replace: kill/teleport/dialogue-interruption all clear the action deque before queueing the replacement. **Atlas consequence: `ClearActions` drains ONLY +0x304 — the +0x37C mailbox, +0x1A8 anim deque, and +0x2E0 inbox SURVIVE a ClearActions call** (no other drain primitive is invoked by any native; containment-proven). A script can wipe pending actions and the entity will still get its queued messages and finish its current animation timeline.

### 2d. ANIM DEQUE +0x1A8 and EFFECT INBOX +0x2E0 — **no native touches either directly**

Containment: neither `sub_5505F0` (267-site host list: all in the 0x462c50-0x4668db creature-command band) nor `meth_0x5506c0`/`0x5505F0`'s funnel-adjacent wrappers contains any table-root range. The QueueAnimEvent front door (`0x4673d0`) has 30 call sites total — **all engine action-lifecycle** (GameActionDie ×9, Attack ×3, Equip/Idle/MoveTo×3/OperateDoor/PickUp/Reload/SetActiveWeapon×2/Unequip/Wait×2, `sub_45C140`, `sub_467640`). Natives reach the anim deque only by creating actions that themselves queue anim events at start/update. **NONE rows, with the path named.**

### 2e. THE FIFTH QUEUE (named, not conflated)

`QueueFunction` @0x5ca4f0 / `ClearFunctionQueue` @0x5c9f80 push/clear a **script-function queue** (string callback names into the AMX-side queue retrieved via FOURCC param `1163219270`; push = `0x4c60a0`; verbatim "QueueFunction - Error in trying to retrieve function queue."). This is NOT an entity queue — it lives in the Script_Small/AMX layer (same object as `this->mbr_0xc0`-style state). Listed so the four-queue atlas boundary stays honest: script callbacks ride here, entity queues ride on the four.

## 3. THE T-070 × T-080 RESOLUTION (dual-path, split by `A_` prefix — both rows true)

The engine ships **two delivery models of the same message** and the naming prefix IS the axis (not table, not module — all six live in the 155-table contiguously @entries 99-106):
- **BARE trio = synchronous**: native resolves members NOW, pushes 12-byte records straight to each target's +0x37C; delivered at the target's next pump pop → OnMessage.
- **A_ trio = asynchronous/queued**: native creates ONE kind-7 ActionMessage funneled to the SENDER's +0x304; fan-out to team/squad/single is **deferred to ActionMessage::start** (T-080's @0x5a9107/0x5a91a7/0x5a9214 branches, reading action+0x20/+0x24 team/squad cells — exactly the fields the A_* natives write at @0x5c53de/@0x5c578e). Delivery order: at the sender's action-walk dispatch, after HOL/budget semantics apply — i.e. a sender whose deque is DRAINED before dispatch (A_Kill @0x5c8ec0, StartDialogue @0x45a0c2, ClearActions — the §2c set all erase a queued A_SendMessage) NEVER delivers; a bare-send always does.
- T-070's "SendMessage ×3 among the funnel callers" = the A_* trio ✓ true. T-080's "the trio pushes mailbox direct" = the bare trio ✓ true. **New axis: prefix-A = queued-async via +0x304; bare = direct-sync to +0x37C.** Suggested row annotations: one clause each at checkpoint (coordinator's call).

## 4. SELECTION-MODE DIET (column legend, EAs)

`handle-cell` = `sub_59AEF0(amx, params[n], &out, 1)` → world entity lookup by script handle (the dominant mode — 15/19); `from-entity` = `F3::GameActionEquip::meth_0x54be10_54be10` on the AMX-readout cell ('from' id via FOURCC 1179407181 @0x5c5152); `team` = `0x56a2f0` (immediate: bare ToTeam loop; deferred: action+0x20 → start-fanout); `squad` = `0x569390`; `party` = worldsub `[vft+0x68]` list snapshot @0x5ca429 (TeleportParty only); `bounds/points` = `sub_59AC40` point-params (Wander/move targets).

## 5. PREARM HOOKS (T-085 §6 crosswalk — the boot lane confirms rows WITHOUT re-deriving)

Native-side bps (additive to bp7-bp13, same rig, address-paired):
- **bpN1 `0x550720` entry** — every +0x304 push; `return-address ∈ table-root ranges` ⇒ native identified from the 63-site table above (§2a/2c rows incl. engine rows for exclusion).
- **bpN2 `0x54c440` entry** — the four drain natives (ClearActions/Kill/Teleport/StartDialogue) vs engine resets — a `StartDialogue` drain hit at boot would be the first visible latch-clearing-adjacent event (T-055 "clear at boot" candidate now has its NATIVE: dialogue start drains the deque; pair with bp9 latch-clear).
- **bpN3 `0x54EE60` entry** — bare trio only (T-080 §5); distinguishes sync-mailbox traffic from bpN1 kind-7 rows.
- **bpN4 `0x46f290`** (CreateGameAction factory) — the pushed `K` at the call site = engine kind (§2a map): live kind histogram per script; A_PlayAnim=6 and A_SendMessage=7 rows confirm in one bp.
- **bpN5 `0x5505F0` entry** — anim deque pushes; hit = engine Queue_* path (creature command band), NEVER a native (containment-proven NONE) — if it ever fires with a native return address, this row is wrong and says so loudly.
- Runtime identity chain for any script-side sighting: bp13 (sink) fields ↔ bpN1/kind ↔ bpN4 — three EAs, one sentence of attribution.

## 6. PORT RELEVANCE — NAMED (no invention)

- The 209-entry × {9,155,45} table map IS the script-VM port surface: names + handler EAs + arity prologues are mechanical; the port needs nothing invented here.
- The **A_ vs bare dual-path is a REQUIRED paired-observable** once scripts run: async A_SendMessage ordering (HOL/budget-gated, sender-death-kills-delivery) vs bare immediate — the digest seams never see either (+0x304 digest is count-based; a kind-7 action is just +1 to the count — T-085's R-family expectations unchanged), so this is **named future material, not a seam**.
- StartDialogue/TeleportParty's **drain-before-replace** pattern and the §2c survival matrix (mailbox/anim/inbox survive ClearActions) are behavioral specs the port's clear-primitives must mirror if scripts ever call them.
- The fifth-queue naming (QueueFunction/ClearFunctionQueue = AMX-side) keeps the port's queue inventory at four entity surfaces — prevents a false fifth queue from being minted.

## 7. POSTURE

Read-only honored (zero writes/saves; no IDB mutation). Raw-over-decompiler for every kind byte (`6a KK / ff 5x 14` sequences dumped per site, §2a); completeness by containment against FIVE all-`more:false` primitive censuses + three sentinel-walked tables — the method IS the acceptance bar, stated in §0/§1 rather than implied. Item 43: host names are live-IDB function attributions; no membership claims minted (the 155/45 table attribution is by registration data, not class guessing — no share-test needed since no class-membership was claimed). No collisions: KeenStorm/HappyEagle files untouched; T-070/T-080 conflicts resolved as ANNOTATIONS PROPOSED to the coordinator, not edits. Both prior rows stay true.
