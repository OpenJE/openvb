# Batch B's natives are reversed and tested, but nothing in the running port can call them (2026-10-22)

Found while scoping `StartDialogue`, and worth writing separately from the per-native notes because it is a
statement about the **whole batch**, including the 27 rows already committed. Everything below is a
`grep`/`sed` result against the tree, with the location named; nothing here is inferred from intent.

## The four gaps, each with its evidence

1. **No installer.** `ScriptNativeGameCreatureTable` (`src/F3/ScriptNatives.cpp:1172`, declared
   `ScriptNatives.hpp:275`) is the only way the 27-entry table leaves that file, and it has **zero callers** in
   `src/`, `modules/openje/`, or `modules/tracing/` apart from the suite that asserts its contents. The table
   exists; nothing asks for it.
2. **The registry installer is test-only, by its own comment.** `JE::RegisterNativeScript` is *defined* at
   `modules/openje/src/JE/Scripting.cpp:57`, and its header comment at `Scripting.hpp:26` says the access is for
   "tests + F3 game script installation". The only call in the tree is `modules/openje/tests/je/Scripting/Scripting.cpp:39`.
   F3-side installation is **intended and absent** - which is the honest way to describe it: the seam was made,
   the caller was not.
3. **Nothing executes scripts.** `JE::Script::Run` (`Scripting.hpp:23`) has **no callers** anywhere in product
   code (`grep -rn "Scripting::Run\|\.Run(" src modules --include=*.cpp` finds none), and no product path loads a
   `.int` (`src/F3.cpp` matches for `.int` are an unrelated VFX parameter struct at :649/:664/:677). The
   `GUILoading` suite loads GUI `.int` files in a test, so the loader exists; the executor does not.
4. **The ABI does not match.** `NativeScriptFn` is `int (*)(struct Mover *self, int a2, int a3, int a4)`
   (`Scripting.hpp:18`), while every shipped native - and every ported one in batch B - is
   `int __cdecl (int amx, int params)` with `params[0]` as the parameter-block byte count and `amx+0x2C` the
   script line. Installing the table is therefore not a loop over entries: it needs an adapter, a typedef change
   in the openje lane, or a decision that both shapes exist.

## A fifth gap that is the one `StartDialogue` actually died on

The port has **two unrelated entity types** and no bridge between them:

- `JE::Entity : public DynamicObject` (`modules/openje/src/JE/Entity.hpp:10`) is what the natives' stand-in
  registry stores, and what every batch-B native resolves;
- `JE::Mover` (`modules/openje/src/JE/ActionMoveTo.hpp:22`) is a *layout-mirroring excerpt* - its comment says
  it "derives from the fully NAMED JE::Entity ... unit tests use this EXCERPT directly" - and the live world
  object is `struct SpawnedEntity : public JE::Mover` (`MapEntitySpawn.hpp:30`), which owns the action queue that
  `EnqueueEntityAction` (304) and `ClearActionQueue` (326) operate on.

The only lookup is `SpawnedEntity *FindSpawnedEntity( const char *prefix )` (`MapEntitySpawn.hpp:397`) - by
**name prefix**, not by handle. There is no netId→object resolution anywhere in the port, and no
`static_assert` or cast tying `Mover` to `Entity`. So a native whose observable is *on the action queue* -
`StartDialogue` (0x45a040: flush the source's queue at 0x45a0c2, then enqueue through the T-030 funnel 0x550720),
and by the same argument `Attack` (0x459f5d), `JustAttack` (0x45a025), `ReloadWeapon` (0x45a3e6) - cannot be
ported faithfully from this side of the split, because the handle in hand names an object in a world the queue
does not live in.

## What that means for batch B's status, stated plainly

- The 27 natives are **verified reversal with enforced behavior**, and their suite (196 asserts) is real:
  offsets, messages, check orders, clamp holes, case rules, and the shipped defects are pinned against bytes.
- They are **not runtime parity**, and no commit in the batch changes what the running port does, because
  nothing can reach them. That was implied by the stand-in-registry deviation but never written as a status, and
  a reader assuming otherwise would be misled by my own summaries.
- The remaining rows split into two different kinds of blocked: **era-blocked** (dialogue interface, world
  server, GUI hide, AMX object reference - where the shipped behavior is not yet fully read) and
  **wiring-blocked** (the queue-touching natives above, where the behavior IS fully read and the port has no
  path to act on it).

## What a fix requires - and why it is not mine to make silently

Closing gaps 1-4 means deciding which script layer is authoritative: whether `JE::Scripting` grows an
`(amx, params)` native form, whether F3 installs `ScriptNativeGameCreatureTable` at startup behind a real or
emulated VM, and whether `Entity`/`Mover`/`SpawnedEntity` become one type or an index. Each is a design decision
in the openje and spawn lanes. I am deliberately **not** adding a handle→`SpawnedEntity` bridge or an adapter in
this file, because a shim invented inside a natives TU would make the split look solved while hiding which world
is authoritative - the same class of move as silently mapping shipped's null-deref to a friendly message, except
with no declared collapse to point at.

Cheapest real behavior-parity step, if the coordinator wants one: gaps 1-4 together - an installer plus an
executable script path - because every native already reversed becomes observable the day they close, and the
test suite becomes a regression net instead of the only consumer of the work.
