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
