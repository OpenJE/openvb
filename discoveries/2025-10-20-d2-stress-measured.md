# 2025-10-20 — D-2 severity MEASURED on the port seam (RedXenon 01a157ec)

Question: is `actions[4]` (block-of-four, 5th insert refused) a style deviation or a dropped
gameplay order? WildYak's statics answered the shipped half; this measures our half.

**Shipped (T-037 `docs/f3/ScriptNativeActionDoors.md`):** out-of-range `GameActionAttack::OnServerUpdate`
pushes TWO actions onto the SAME owner in ONE pass (`0x40470a` MoveTo + `0x40471a` new Attack, both
`call 0x550720`), the drain pops ONE front per pass (`0x550617..`), so depth grows +1/pass with **no bound
in the funnel** (`meth_0x54ea00` allocates another block at `0x54ea54` when `count & 3` wraps). Depth 5 is
the ordinary 3rd–4th approach pass of one melee entity. A retarget (door `meth_0x403fa0` branch `0x404038`)
pushes a third. ⇒ shipped deque is EXPECTED to exceed 4.

**Port, measured this sitting:** `wine tools/sim-driver.exe tools/scripts/d2-stress.txt` → **exit 1**,
`SIM-DRIVER FAIL line 19: entity brute queue block full (D-2 refusal)`. Five bare shipped-shape `attack`
verbs on one actor; the seam's return value (not damage) is the assertion, per WildYak constraint 1; the
five targets are enqueued front-to-back so nothing tests LIFO the original never does (constraint 2).

**Verdict:** D-2 is a real defect in `actions[4]`, not stylistic. HappyKnight's digest `q=` field is the
same count with the same name (shipped `Entity+0x314` LIVE COUNT), so in any future parity run a shipped
`q>4` beside our `q=4` is the same dropped order, now with a measured mechanism behind it.

**Boundary (this is a port-side stress, not R3):** the stress proves OUR refusal. It does NOT produce a
trace of a live shipped 5-deep deque — that is R3, and R3 needs the attach + menu→combat input that the
current shell lacks (no xdotool/wmctrl). Statics say depth>4 is reachable; the stress says our seam drops
it. Both stated, neither over-read. R4's +0x314 read, when someone attaches, confirms a depth (WildYak's
2-pushes/pass analysis), not a semantics — the semantics are already statics-settled.
