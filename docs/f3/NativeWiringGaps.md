# Batch B's natives are reversed and tested, but nothing in the running port can call them (2026-10-22)

> **COUNT REFRESH (TrueRaven, B24, same day):** this doc was written when the batch had 27 natives and a
> 196-assert suite. It is now **36 natives / 243 asserts** (B22 task trio, B23 weapon-combo pair, B24
> quest/journal alias quad). **Nothing in the four gaps or gap 5 below changed** - the additions are more
> rows in the same unreachable table. Gap 1 also has its ANSWER now, in
> `docs/f3/NativeInstallerEvidence.md`: shipped installs native tables from `JE::Script_Small::Load`
> (0x5a8880), three tables per script load, fail-fast on any name that does not resolve. The counts inside
> the text below are left at their original values because they are dated observations, not a live status.

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
4. **The ABI does not match - now SETTLED BY THE CALL SITE, not by argument** (WildUnion,
   `docs/f3/NativeAbiEvidence.md`; dispatch re-verified here). Shipped's native form is
   `int __cdecl(int amx, int params)`, proven in the AMX default native handler `sub_5CC4F0`:

       5cc508 mov  eax,[ecx+eax]      ; fn = image native table[ordinal+0], stride 40
       5cc50b push edx                ; edx = params  <- pushed FIRST  = second argument
       5cc50c push esi                ; esi = amx     <- pushed SECOND = first argument
       5cc50d mov  dword[esi+84h],0   ; error channel cleared BEFORE the call
       5cc517 call eax
       5cc51d add  esp,8              ; exactly TWO args, caller-cleaned -> __cdecl
       5cc520 mov  [ecx],eax          ; return value -> PRI
       5cc522 mov  eax,[esi+84h]      ; the native's error code, read AFTER

   `NativeScriptFn` is `int (*)(struct Mover *self, int a2, int a3, int a4)` (`Scripting.hpp:18`), while every
   shipped native - and every ported one in batch B - is `int __cdecl (int amx, int params)` with `params[0]`
   as the parameter-block byte count and `amx+0x2C` the script line. So the mismatch is between the PORT'S
   OWN TYPEDEF and the engine, not between the port's natives and shipped: batch B already matches shipped.
   Installing the table is therefore not a loop over entries - it needs the typedef corrected in the openje
   lane, or two coexisting native forms - and that decision now rests on a cited push order rather than on
   inference. A second witness, free: `JE::DialogueCall`'s own prototype (`__cdecl f(int, _DWORD*)`,
   `push n=2` + gate at 0x5c3c52).

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

## Gap 4 has a second half: shipped has TWO script shapes, and the port's typedef matches neither
(NiceTiger `0cfb22a` §7; every number below re-derived from the PE at adoption time, per item 75)

Gap 4 as first written was about the **native** ABI, and it is settled: `sub_5CC4F0` pushes params then amx,
`call eax`, `add esp,8` - shape (a) `int __cdecl(int amx, int params)`. The pass that produced the OnDamage
witness shows a second shape the typedef conversation keeps collapsing into the first:

| shape | form | evidence |
|---|---|---|
| **(a) native** | `int __cdecl(int amx, int params)`, `params[0]` = block size in BYTES | dispatch `sub_5CC4F0`: `push edx`(params) @0x5cc50b, `push esi`(amx) @0x5cc50c, `call eax`, `add esp,8` |
| **(b) script entrypoint** | `vft(slot)(name, argc, cells[, &out])` on a script object fetched per entity | `call [edx+0x14]` @0x54b044 - slot 5, args `(name, 4, block, out)` forwarded verbatim |

Three call sites, **one object shape, live from both sides of the engine**:
- net-dispatch case 0x51: predicate `[script+16]`, invoke-with-out `[script+24]` (JadeBear's id-map)
- combat apply: `[script_vft+0x14]` @0x54b044, **no predicate at all**, and its return is **unconsumed** -
  verified as an absence with a bound: apply's next instruction is `8b 46 48` `mov eax,[esi+0x48]` @0x45c57a,
  a full reload, and nothing reads `eax` between the call and the return. **Fire-and-forget is structural,
  not inferred.**
- the script object itself is **fetched by handle**: `call [edx+0x24]` @0x54af38 - i.e. **36 bytes = slot 9** -
  on `GetWorldServerInstance_0x401150`, applied to `entity+0x1DC`.

That last point closes a loop this doc opened: slot 9 was already the census's script-handle slot (what
`Entity::Create` stamps at 0x55190c and what the resolver compares). It is now also the accessor by which a
script is *looked up*, so the same singleton slot both defines and retrieves the handle, from the combat side
as well as the spawn side. Two independent derivations (this PE read and NiceTiger's IDB walk) agree on slot 9.

**NOTATION TRAP, recorded because it is the lane's most-recurring one**: the fetch is `+0x24` **hex** = 36
**bytes** = slot **9**. "`vft+24`" written without a base is therefore ambiguous in exactly the way that has
bit this batch repeatedly (cells-vs-bytes at 0x45ac5f, 0x46ece6, 0x54b044, and our own
`MapEntitySpawn.hpp:115 "(vft[8])"` label). Write `+0x24` or "slot 9", never "+24".

**Consequence for the port, quotable at the decision**: `NativeScriptFn` is
`int (*)(Mover*, int a2, int a3, int a4)` (`Scripting.hpp:18`), which matches **neither** (a) nor (b). Fixing
gap 4 for the natives (typedef to `(amx, params)`) does NOT give the port script *execution*: that needs shape
(b) - a per-entity script object plus `(name, argc, cells[, out])` dispatch - and the two must not be fused
into one function-pointer type. Also named-deferred, not guessed: the script object's exact class identity, and
slots 0/+4/+0xC of its vft (`GetScript`'s own `[vft+4](script, &name, 1)` @0x54af5f looks like an exists/attach
probe on the freshly fetched script; not chased). And `0x54aff3` throws the shipped SEH message "Entity::RunScript()
method may only be called by the server." behind the same world-server flag guard as `0x462620` - the
guarded-selection/unguarded-use family now has a fifth member, inside the script-invoke path.

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

## Refinement of gap 5 (same day, after a peer proposed the bridge was free): the two worlds do NOT provably share a key

The suggestion was that `SpawnedEntity.netId` already carries the handle the natives speak, so a resolver is a
dozen-line scan. Checking the shipped stamping sites says otherwise, and the difference decides what the
resolver must key on.

**The port's own label.** `src/F3/MapEntitySpawn.hpp:115` documents `netId` as
"**GameEntity mbr_0x28**: world-server-assigned serial (vft[8])" - not as `Entity+0x1DC`.

**What `GameEntity+0x28` actually is** (bytes, `GameEntity::ctor_0x4678a0`, gated on the area-map-icon flag bit 0):

    46799f  call GetWorldServerInstance_0x401150
    4679a4  mov ecx,[eax+4]        ; delta table
    4679a7  mov edx,[ecx+4]        ; the delta
    4679aa  lea ecx,[edx+eax+4]    ; the virtual-base subobject
    4679ae  mov eax,[ecx]          ; its vftable
    4679b0  call dword ptr [eax+8] ; SLOT 2
    4679b3  mov [esi+28h], eax     ; -> GameEntity+0x28

**What `Entity+0x1DC` is** (`Entity::Create`-era at 0x5518f7): the same adjustment shape, then
`mov [a1+1DCh], eax` after `call [eax+36]` - **slot 9** - and Create mirrors that value into the `+0x168`
object at `+4` (0x55192a) and into `global_area_map_icons[0]->int_0x2c + 228` (0x551a60).

**And they are the same object:** `GetWorldServerInstance_0x401150` returns `global_0x70bd98 + 1` (0x40118a).
Which yields a three-slot census of one server singleton, every site byte-verified:

| slot | call bytes | where used | what it yields |
|---|---|---|---|
| 2 | `[eax+8]` @0x4679b0 | GameEntity ctor | the value stored at `GameEntity+0x28` - what the PORT calls `netId` |
| 9 | `[eax+36]` @0x55190c | Entity::Create | the handle stored at `Entity+0x1DC`, mirrored to `(+0x168 obj)+4` and map-icons`+228` |
| 26 | `[edx+68h]` @0x45a9a5 / 0x45a343 | Protagonist, AddCNPCToParty | the `Party` object |

The script-facing handle is unambiguously the **slot-9** value: it is what the resolver compares
(`*(entity+476)` = `+0x1DC`, in `meth_0x54be10`), what `SetHP` resolves through, and what `Protagonist` returns.
So a resolver keyed on `netId` is keyed on the field the port labels to **slot 2**, which nothing here proves
equals the slot-9 handle.

**Resolved to (b), statically - by JadeBear, verified independently here, and it retires the live read as a
blocker.** The member census settles it without a debugger: `GameEntity+0x28` has **two usages in the shipped
image and both are writes** - `mov [esi+28h],eax` (0x4679b3, the success branch) and `mov [esi+28h],ebx`
(0x4679b8, the client/editor branch storing the else value) - with **no reader anywhere**. A field nothing in
the exe reads is not the script handle by consumer definition, so the port's `netId` label needed a correction
rather than a bridge, and it now carries one in `src/F3/MapEntitySpawn.hpp:115` (including the old `(vft[8])`
note, which was the cells-vs-bytes ambiguity sitting in OUR OWN comment: byte 8 is **slot 2**).

`Entity+0x1DC` is the opposite shape: reads by the handle resolver (comparing `*(entity+476)`), by SetHP's
chain (0x45b36d), by Protagonist (0x45a9e0), plus the two mirrors above. Precision note, applied to a peer's
number this time rather than to my own: the "nine consumers" figure does not reproduce as a single list - the
DB records 7 usages against `JE::Entity`'s own member and attributes the native/resolver sites to other class
keys, so the count depends on class attribution. What is verified is the ASYMMETRY (reads on one, writes only
on the other), and that is what the design call rests on - a good example of checking the claim that supports
a conclusion you agree with.

So the unlock is unchanged in shape but honest in cost: a spawn-stamped field with `+0x1DC` semantics (or a
mirror into the natives' registry), not a `netId` scan - and the decision no longer waits on a run.

Census bonus from inside the same negative result, credited to JadeBear: the singleton's slot 26 is not a
server packet dispatch - `0x667dd4` slot 26 (`0x46dd70`) and `0x6927bc` slot 26 (`0x556ea0`) refute themselves
against the observed Party-returning behavior, which means the singleton's real class is **unattributed** in
the DB. Inside `0x46dd70` sits an id-map worth its own pass: case `0x49` (73) constructs
`GameActionInitiateDialogue` and funnels through the T-030 seam `0x550720` - the server-side twin of
`StartDialogue`, which gives a wiring-blocked row its counterpart - cases `0x64`/`0x65` build packets for the
weapon/reload family (siblings of SetHP's id 88), and `0x6A`-`0x70` attach the demo's rain/snow/wind VFX.
