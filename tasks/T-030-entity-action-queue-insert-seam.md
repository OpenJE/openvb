---
status: open
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
