# The entity action-queue insert seam — `JE::Entity::meth_0x550720`

T-030. Everything below is read from the shipped binary unless it says **PORT**. Port code:
`F3::EnqueueEntityAction` / `F3::PushActionOntoEntity` in `src/F3/MapEntitySpawn.{hpp,cpp}`, drain in
`TickOneEntity` (same file), acceptance test in
`modules/openje/tests/je/ActionMoveTo/SpawnMove.cpp`.

## Why this row existed

T-028's recon inverted the assumption: the per-frame command FIFO at `client+0x1F0` carries **no
gameplay verbs** (134 registered commands, UI/camera/toggles/engine only, zero Attack/Move/Select).
Gameplay orders are **entity Action objects**, and `0x550720` is where they get attached. Without it our
`OnServerUpdate` ports were reachable only by a direct call, so a digest could never be produced *from an
order*.

## What the shipped seam does — three steps, all load-bearing

`0x550720..0x550741`, reached as a member call on the **entity** with the action as the argument:

```
55072e  cls_0x54e070::meth_0x54ea00(&this->mbr_0x304, &param_1)   push_back the action POINTER
550737  *(param_1 + 20) = this                                    OWNER back-link at action+0x14
550741  return JE::Entity::meth_0x54ef30(this)                     deferred refresh latch
```

### The container (Entity+0x304)

`cls_0x54e070::meth_0x54ea00` (0x54ea00..0x54ea74) is a **block-growing deque of pointers**:

- `mbr_0x4` = block array, `mbr_0x8` = block capacity, `mbr_0xc` = 4 elements per block (the `& 3` /
  `>> 2` pairs at 0x54ea31/0x54ea35), `mbr_0x10` = running element index;
- grows by allocating 0x10-byte nodes (4 pointers) at 0x54ea54 when the count crosses a block boundary;
- stores `*param_1` — the pointer *value*, not the address of the caller's variable.

**PORT:** our `SpawnedEntity::actions[4]` is therefore not "the list", it is **the first block**. The
original appends blocks; when our 4 slots are full the seam returns 0 instead of growing. Registered as
deviation **D-2**. Refusing is correct behaviour for the port (a dropped order is not), and the test
asserts the refusal rather than the silent loss.

### T-039: `Entity+0x314` is THIS DEQUE'S LIVE COUNT — not a busy flag, never was

The port declared `unsigned char busy0x314` and T-011 was about to build the turn gate
`+0x1E4 == 0 || +0x314 != 0` on it as a boolean. The DB types `Entity::mbr_0x314` as **size 4**, and the
field is the deque's own `mbr_0x10` (`0x304 + 0x10 = 0x314`). The writers settle it:

| EA | form | what it is |
|---|---|---|
| `0x54ea71` | **`++this->mbr_0x10`** | the push_back that funnel `0x55072e` calls — appends at `head + count`, then count++ |
| `0x54b637`, `0x54b672` | **`--count`, store** | `cls_0x54b5c0::meth_0x54b5c0`, the erase that walks the deque (both deques use it) |
| `0x5515e6` | `= 0` | `Entity::ctor_0x5513a0` |

A `++`/`--` pair on a 4-byte field with a zero-init is a **count**. There is no `mov reg, 0/1` writer
anywhere in the usage set. The readers say the same thing:

- `Entity::virt_meth_0x4084c0` is *one line*: `return Entity+0x314 == 0;` — an "is my queue empty" predicate;
- the pump `Entity::virt_meth_0x54c570` bounds its action loop by `head != head + count` (0x54c77c/0x54c788),
  and `meth_0x54c440` does the same at 0x54c447/0x54c4a0 before deleting the head action;
- **the gate readers the coordinator pointed at are readers, not writers**: `0x54ab30`
  (`virt_meth_0x54ab00`) and `0x54ab80` (`virt_meth_0x54ab50`) both *test* `+0x314`, and they test it
  **beside `mbr_0x2f0`** — which is the live count of a *second* deque of the same 20-byte shape at
  `Entity+0x2E0`. So that pair asks "does this entity have anything queued in either list".

⇒ **T-011 step 3's second term means "has queued actions"**, and the byte model was wrong twice: one
byte cannot hold a depth (256 aliases to 0, i.e. *idle*), and a non-zero test on it silently collapses
"1 pending order" and "4 pending orders" into the same value.

`Entity+0x1E4` was resized in the same edit (`mbr_0x1e4`, **size 4**, ctor `= 0` at `0x55147b`, unpacked
from the entity record at `0x54e739`) and renamed `flag0x1E4`: its readers gate **map-membership**
bookkeeping (`0x54bc3a` guards `--*(oldMap+280)` / `++*(newMap+280)`; `0x54947e` bails out of a movement
update when it is 0), so the old name `noTurn0x1E4` asserted a meaning the evidence does not support.
**What it means is still an open question and T-011 must not pick one** — the shape of the shipped test
is all the port may mirror until a read or a record-format doc settles it.

**PORT CHOICE (stated, not hidden):** the count is **derived** from `actions[]`
(`SpawnedEntity::QueuedActionCount()`) rather than stored beside it. The shipped container keeps its own
counter because it is a real unbounded deque; our `actions[4]` is the only queue storage we have, so a
second number maintained at every push *and* at every one of the five auto-clear sites in
`MapEntitySpawn.cpp` could only drift from the slots it claims to count. Consequence for **D-2**: a
derived count cannot exceed 4 because our queue cannot, while the shipped `mbr_0x10` grows past one
block — so in a parity diff, **shipped `q>4` beside our `q=4` is deviation D-2 firing (a dropped order),
not a digest bug.** That is why `SimDigest` now prints `q=` as its own field.

### The owner back-link

`*(param_1 + 20) = this` is unconditional and type-free: every shipped action class carries the owner at
+0x14, so the *drain* never has to be told who queued what.

**PORT — the interesting limit, and it cost me a red suite before I saw it.** We cannot copy the store:

| our model | owner field | real port offset | can the seam link it? |
|---|---|---|---|
| `GameActionAttack` (kind 4) | `actor` | **0x00** — flat POJO; its `+0x14` comment is the *shipped* offset | **yes** |
| `JE::ActionMoveTo` (kind 1) | `mover` | shipped **+0x11**, set by the registration path | verified only, never overwritten |
| `GameActionWait` (kind 2) | — | `{startMs, durationMs}` | **no field exists** |
| `GameActionDie` (kind 5) | — | `{damageType, material, flagsByte0x18…}` | **no field exists** |
| `GameActionOperateDoor` (kind 3) | — | `{door, startMs}` | **no field exists** |

My first version wrote blindly at +0x14 through a header struct. Measured result: it **corrupted
`JE::ActionMoveTo`** (two pre-existing step-aside/door tests started failing) and **did not link the
attack** (whose `actor` is at 0). Both directions of the lie. The seam now links the kinds that model an
owner, *verifies* the one whose shipped offset differs, and **refuses** the rest — logging once, not per
call, because "some kinds were queued ownerless" must be visible to whoever reads the log. Adding an owner
member to Wait/Die/OperateDoor to make the store uniform would be inventing layout we have not verified, so
it is recorded rather than done.

### The refresh latch (0x54ef30) — NOT ported, NOT faked

`JE::Entity::meth_0x54ef30` raises a per-entity latch (`gap1AC[60]`), links the entity into a global list
(`byte_70BE24`), pokes `F3::global_gui_interface_0x70be20` through `meth_0x401dd0(1)`, and sets
`byte_70BDE8 = 1`. That is deferred GUI/net refresh, which this port does not have. The seam contains the
EA citation and **no** substitute flag — a boolean nobody reads would be a false model of a subsystem we
have not reconstructed.

## Who enqueues in the shipped game (xrefs to `0x550720`, `more: true` at 40)

Every `GameAction*::OnServerUpdate` re-queues follow-ons (Attack 0x40470e/0x40471e/0x404815/0x404829/
0x404b07, MoveTo six sites 0x4064bd..0x406659, OperateDoor 0x4068cb/0x4068f8, OperateContainer
0x406bb4/0x406be1, PickUp 0x40700f/0x40704c, UseSkill 0x407db5/0x407de6, InitiateDialogue
0x404cdc/0x404d0a, Give 0x405481/0x4054c1, plus `GameActionAttack::meth_0x401890`/`meth_0x403fa0`), the
**script natives** `F3::Attack` (0x459f5d), `F3::JustAttack` (0x45a025), `F3::StartDialogue` (0x45a0f0),
`F3::ReloadWeapon` (0x45a3e6/0x45a3fb), the **server packet path** `GameServer::virt_meth_0x46dd70` (four
sites), the **hit-reaction** function `sub_45C140` (0x45c626), `GameProjectile` (0x411e7a), effects
(`sub_467640` → `0x46776e`, i.e. the projectile/effect path queues actions too), and
`sub_435480`/`sub_435DE0`.

Read that list as the row's real finding: **the seam is the single gameplay-order funnel** — script,
server packet, AI/reaction and effect paths all converge on it. Our port currently reaches it from
`GameActionMoveTo.cpp` (step-aside dance) and `GameActionAttack.cpp` (die-on-kill); the *script natives*
and the *server packet* doors are still absent, which is what T-028's driver must build on.

## The pair, proven

`SpawnMove` now contains a block that:

1. starts an attack whose actor is **deliberately wrong** (`t30Stranger`, parked at x=900),
2. queues it through `EnqueueEntityAction( &t30Atk, &order, 4 )` and asserts `order.actor == &t30Atk` —
   the seam overwrote the constructor's claim, exactly as 0x550737 does,
3. registers only the attacker and drives `F3::TickWorldEntities(ms)` — **no direct `Update()` call** —
   asserting damage landed, the action finished, the slot was cleared by the drain (same container), and
   `t30Stranger` never moved (the action acted as the *seam's* owner, since the pump-side
   `GameActionAttack::Update(ms)` takes no entity argument and reads `actor` at 103/130/157),
4. fills the block and asserts the fifth insert is **refused**, not dropped.

Step 3 is the acceptance: the drain that ran the order is the same array the insert wrote.

## T-071 addendum: the idle resident and the shipped kind numbering

The seam now carries a seventh kind. Full mapping (port kind → shipped class; the SHARED AXIS
is the class NAME — the port numbers are NOT shipped `GameActionType` values, because our empty
sentinel is 0 and shipped kind 0 is GameActionDie; the shipped factory
`F3::GameWorld::CreateGameAction_0x46f290` dispatches
`0=Die 1=Idle 2=MoveTo 3=PickUp 4=Wander 5=Wait 6/7=JE::World::CreateAction passthrough
8=FloatingSpeechText 9=Equip 10=Unequip 11=SetActiveWeapon`, and GameActionAttack is not a
factory kind at all — direct-constructed by `F3::Attack` at 0x459f33):

| port kind | class name | shipped source |
|---|---|---|
| 0 | (empty slot) | port sentinel |
| 1 | GameActionMoveTo | factory case 2 |
| 2 | GameActionWait | factory case 5 |
| 3 | GameActionOperateDoor | engine passthrough |
| 4 | GameActionAttack | script native |
| 5 | GameActionDie | factory case 0 |
| 6 | **GameActionIdle** | factory case 1 — the RESIDENT (T-070 doc) |

Key facts the resident contributed (all EA-pinned in
`discoveries/2025-10-21-t070-idle-resident-rootcause.md`): the shipped queue is NEVER empty at
rest (feeder `Entity::SetInMap_0x551ba0` drains then enqueues idle; the stop-packet case 0x3B
rebases identically); the idle predicate byte `+0x18` is never written by idle code (its update
writes `+0x16`), so the resident head-blocks later orders until a drain — and orders arrive on
TWO shapes with different drain semantics: script natives do NOT drain (run H's "Attack never
fired"), packet-pump commands DO (cases 0x1A/0x3C). The port models both: `attack/attack-pos/wait`
take the command shape, `push` takes the native shape. The +0x319 raiser is NOT armed by the
resident (the shipped raiser at 0x54c833 belongs to the anim-event deque; arming it from an idle
action-head would freeze the combat manager's skip-advance consumer in a state shipped never
reaches). D-2's capacity re-expresses as **3 orders + resident** filling the first block.

### Pre-rebase window and the port's edge-timing approximation (T-077 exchange, 2026-10-21)

Idle is presented ONLY at rebase sites - `Entity::SetInMap_0x551ba0` (membership ADDTOMAP, from
`Server::meth_0x555750` / `Client::virt_meth_0x58c660`) and the packet-pump cases 0x1A/0x3B. Between
ctor and the first rebase the deque is genuinely all-zero (`Entity::ctor_0x5513a0` zeroes the count
at 0x5515e6): an entity in that window has NO head at all, and a native order enqueued there takes
slot 0 and fires IMMEDIATELY - no HOL, because there is nothing to block on. T-056's
0x79B3600 first-insert observed exactly that pre-state (`x/5wx 0,0,0,0,0` before count 1), and the
first insert being GameActionIdle is the rebase itself winning the race; 12/12 idle-first means
every observed entity reached SetInMap before any native order.

The port rebases at the SPAWN/creation edge (map-load + harness spawn), while shipped rebases at
the MEMBERSHIP edge that follows creation over the net. In a headless harness the two edges
coincide (spawn == join == AddToMap, no transport in between), so this is a TIMING approximation,
not a semantic gap: no order-advances-past-idle state is reachable in either model, and the
differences are only observable pre-rebase, where both models agree the order simply fires.
Recorded per the stated-rather-than-hidden rule.

Fourth nail (unstaffed, testable in archives or any boot): dump a spawn-to-first-order timeline.
Any entity whose FIRST funnel insert is a REAL order instead of Idle proves the native beat the
rebase in the pre-join window - completing the presentation-sites census from the other side.
