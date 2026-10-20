---
status: done
assignee: unassigned (suggest implementer-B, engine side)
depends: nothing; required by T-028's driver half
---

# T-030 — Port the entity action-queue insert seam (`0x550720` equivalent)

## Why
T-028's recon answered the input question in the **inverse** of how this row was framed: the per-frame
command FIFO at `client+0x1F0` carries **no gameplay verbs** — zero Attack/Move/Select among the 134
registered commands, UI/camera/toggles/engine only. **Gameplay orders are entity Action objects**: the
`F3::Attack` / `JustAttack` natives allocate a 0xC8 `GameActionAttack` and enqueue it on the attacker via
`0x550720`, drained by the per-round `OnServerUpdate` pump; the server packet path
(`GameServer::virt_meth_0x46dd70`) builds the same object.

HappyWolf's grep of `src/F3` for any action-queue insert (`SetAction` / `m_pAction` / `actionQueue`)
returns **0 hits**. So today our `OnServerUpdate` ports are reachable only by direct test calls, not by
an *order*. That is a missing engine seam, and it is exactly the insertion point the driver needs:
without it, T-026 digests can only be produced from hand-built state, never from input.

## Scope
1. Establish what `0x550720` actually does: queue shape (per-entity linked list? ring?), what it stores,
   whether it is the same container the round pump drains, and who else calls it (spawn-time actions, AI,
   party orders).
2. Port the insert and the drain **as one pair**, so we never grow a queue that only our tests feed.
3. Cite the original's EAs for both ends and name the seam in `docs/PROGRESS.md`, engine-side.

## Acceptance
- A test enqueues a `GameActionAttack` through the ported seam and the **round pump** executes it — pump,
  not a direct method call, because the point is proving the drain side is the same container.
- T-028's driver becomes buildable: an order arriving through this seam changes T-026 digests.
- If the container turns out to be shared with something we stub, record which and do not fake it.

## Refs
- `discoveries/2025-10-20-input-path-architecture-command-fifo-actions.md` (two front doors)
- `discoveries/2025-10-20-spawn-path-reachability.md` (kind=4 attack enqueue we already pump)
- `tasks/T-028-...`, `tasks/T-026-...`

## Landed (implementer-B, 01a15802)

Scope 1 (what `0x550720` is): three steps - `push_back` onto the **block-growing deque** at
Entity+0x304 (`cls_0x54e070::meth_0x54ea00`: 4 pointers per 0x10-byte node, slot = count & 3, block =
count >> 2), the **type-free owner store** `*(param_1+20) = this` at 0x550737, and the deferred
GUI/net refresh latch `JE::Entity::meth_0x54ef30` at 0x550741. Callers include every
`GameAction*::OnServerUpdate`, the script natives (`F3::Attack` 0x459f5d, `JustAttack` 0x45a025,
`StartDialogue` 0x45a0f0, `ReloadWeapon` 0x45a3e6/0x45a3fb), the server packet path
`GameServer::virt_meth_0x46dd70` (4 sites), and the hit-reaction function `sub_45C140` - i.e. it is
the single gameplay-order funnel, which is the answer T-028's driver needs.

Scope 2 (insert+drain as one pair): `F3::EnqueueEntityAction` in `src/F3/MapEntitySpawn.{hpp,cpp}`;
the drain was already `TickOneEntity` reading the same array, and the new test proves the pair by
going through the **pump** (`F3::TickWorldEntities`) with no direct `Update()` call anywhere.

Scope 3 (naming): the row asks for `docs/PROGRESS.md`, which does not exist in this repo - named in
`docs/f3/EntityActionQueueSeam.md`, indexed in `docs/f3.md`, and in the `EnqueueEntityAction` header
comment instead. Flagging rather than creating a parallel progress file.

Acceptance, measured: an order queued through the seam executes in the round pump (damage lands, slot
cleared by the drain); the `+0x14` back-link is written BY THE SEAM (the test starts the action with a
deliberately wrong owner at x=900 and asserts that entity never moved); a fifth insert is refused, not
dropped.

Two things I did not fake, both in deviation D-2: the refresh latch (`0x54ef30`) is cited and absent,
because a flag nobody reads would model a subsystem we have not reconstructed; and the owner store is
**per-kind, not uniform**, because our action models are not uniformly shipped-layout
(`GameActionAttack::actor` is at port offset 0, `JE::ActionMoveTo::mover` is the shipped +0x11, pump-side
Wait/Die/OperateDoor have no owner field at all). My first attempt wrote blindly at +0x14 and corrupted
`JE::ActionMoveTo` - two pre-existing tests went red, which is what caught it: the blind store was both
wrong (didn't link the attack) and destructive.

Note for T-031: the seam is where an *input* becomes state, so it is the precondition for producing
digests from orders rather than hand-built state. Script-native and server-packet doors are still
missing; only the in-engine re-queue sites and the new test path go through it.

