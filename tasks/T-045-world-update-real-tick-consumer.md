---
status: open
assignee: WildYak (01a15802) — accepted 2026-10-21
depends: R0 answered from the image (discoveries/2025-10-20-r0-world-vftable-slot6-read-from-image.md)
---

# T-045 — Slot +24 is a shared stub, so how does `World::Update_0x544a90` actually receive `ms`?

## Why this is urgent rather than interesting
`docs/f3/WorldTickPump.md` built the tick-pump story on one inference: `JE::Server::virt_meth_0x552b60` calls
`[World vft + 24]` with `timeGetTime()`, so that must be `Update_0x544a90`. Reading the shipped table shows slot 6 is
`0x404b70`, a stub shared with `DialogueManager`. The doc flagged this exact failure, so the correction is clean — but it
leaves the pump chain **unnamed**, and the frame-tick story is load-bearing for **T-011's deadline arming**, **T-026's
digest clock**, and **T-031's parity fields**.

## Questions, in order of evidence available
1. **Which of World's four vft pointers** (`0x548ab8..0x548ad2`, per the ctor stores) does `JE::Server` hold in `$ebx` at
   `0x552bb5`? `+24` means a different function per table, and the wrong table makes the question moot.
2. **Who calls `0x544a90`?** Enumerate which tables reference it, plus non-virtual callers. `0x544a60` / `0x544a70` /
   `0x544a90` / `0x544b30` sit adjacent, which usually means a small dispatcher cluster, not four entry points.
3. **What does `0x404b70` do with its argument?** If it stores or forwards the tick, it may be the actual dispatch shim
   into `Update` — in which case the pump story survives with one extra hop and nothing else changes.
4. Only if 1–3 stay ambiguous: **one** live `info registers` at `0x552bb5` to name `$ebx`'s table. A register read, not
   the x64 watchpoint hunt this row began as.

## Acceptance
- Corrected `docs/f3/WorldTickPump.md`, call chain cited EA by EA, slot-6 finding **integrated rather than appended**.
- An explicit statement of whether `ms` reaches `Update_0x544a90` at all, and through what.
- If the answer changes T-011's arming or T-026's clock fields, say so **before** those lanes build further on the old
  reading — that is the only reason this outranks other statics work.

## Boundary
`clock` comparability does **not** depend on this: `+0x48` zeroed while `+0x4C`/`+0x54` are seeded is established
independently, and T-038's `totalSeconds = 0` is unaffected. Unsettled is *which code consumes the per-frame tick*, not
whether the leap exists.
