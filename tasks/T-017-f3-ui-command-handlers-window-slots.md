---
status: open
assignee: PureKnight `01a15cb7` (queued 3rd; was HappyWolf, departed)
---

# T-017 — F3 UI command surface: Command* -> window slot / root-Open mapping

**First pass delivered** (B39, `discoveries/2025-10-20-f3-ui-command-handlers-window-slots.md`).
Headline: F3 UI handlers do **not** route through the GameInterfaces singleton table. Runtime
windows live in a **client+0xA0 window-slot block** - blk[0x18] main, +0x44 companion, +0x4C
options - gated by client+0x9C, entity source client+0xE0 (the session-37 actionbar field), with
SetActive-based exclusive close/reopen. Console = name -> fixed handler -> **fixed slot**; the
texture path is name -> RbTree node. Two mechanisms; do not unify (relayed into T-013).

Exception: `CommandEscapeMenu`'s close path calls the session-27 root bootstrap `0x4b4f00`, so its
window participates in GUI-root modal/watcher machinery while inventory/pipboy/options use plain
client-slot exclusivity.

**Open:** blk allocation pass (likely `GameWorld::Initialize` UI wiring); 11 un-sampled handlers
(EAs listed in the doc); §4 residuals - two-globals ambiguity `*0x70BE08` vs `*0x70BD90` flagged,
not guessed. `StartMap`/`CreateEntity`/`Warp` deliberately left to the spawn-slice lanes.
Handler-size honesty: all four sampled are real code; the stub census stays the B38 four.
