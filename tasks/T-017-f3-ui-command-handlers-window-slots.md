---
status: done
assignee: PureKnight `01a15cb7` (was HappyWolf, departed)
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

## Result (PureKnight 01a15cb7-e27e, pass 4, 2025-10-21)
All three open items CLOSED — `discoveries/2025-10-21-t017-pass4-blk-allocation-handlers.md`:
(1) **blk allocation**: virtual factory chain in `JE::Client::virt_meth_0x58f010` — vft+0xB0 override
`GameClient::virt_meth_0x45e8e0` news the 0x64-byte block (vft `0x656650`, 24 slots) and stores `client+0xA0` at
`0x58f1f5`; install is LAZY at first client-vft-slot-1 call (`virt_meth_0x4618f0`→`0x58f010` only direct caller),
not at ctor — pass 3's "no install store" resolved. blk ctor builds the 16 named windows itself
(`sub_4a7680(size)` + name→registry via `sub_440990` + SetActive(1)). Full slot table recovered (Inventory 0x18,
PipBoy 0x44, Options 0x4C, Actionbar/Feedback surface 0x10, EscapeMenu 0x1C, Mainmenu 0x04, five Options* +
MiniMap/Container/LoadingBar/LoadMap …); one MED-confidence pair flagged honestly (0x0C vs 0x10 Actionbar/Feedback).
(2) **11 un-sampled handlers classified**: five Options commands are 8-byte selector thunks (1..5) into the SHARED
handler `0x45ed10` = one Options window + panel arg `0x4433e0(iface,panel)` — the five Options* blk windows exist
but the command path toggles slot C with a selector; UseSkill goes through the `client+0xA4` dialog interface
(op 0x10), NOT any blk slot; AttackModeCycle/WeaponToggle/FeedbackWindow/ActionbarWindow are actionbar-surface /
action commands; engine trio left to spawn lanes per row.
(3) **§4 residuals**: two-globals re-confirmed resolved (every handler loads `ds:0x70bd90`); stub census unchanged.
