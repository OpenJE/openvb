# The script-native doors into the Action funnel — `0x550720`

T-037. Everything below is read from the shipped `F3.exe` (`ida_original_f3`, **read-only, no IDB
writes**). Funnel row: `docs/f3/EntityActionQueueSeam.md`. Field-name source for the runtime-filled
columns: `discoveries/2025-10-20-attack-state-machine-damage-formula.md` §1 (with its own
supersession note about IDA display skew — raw `mov [reg+disp]` displacements are what is quoted here).

## Boundary

This answers **"what does AMX's door look like"**. It does **not** answer "is the sim correct". The
server packet door (`GameServer::virt_meth_0x46dd70`, 4 sites), the AI/hit-reaction door
(`sub_45C140`), the effect/projectile doors and the follow-on re-queues inside
`GameActionAttack::OnServerUpdate_0x404320` all reach the same funnel and may fill the same record
differently. Those are separate rows (§ "doors that are not this row").

## Headline: the door is three instructions wide

No native builds a record inline and no native builds it in a helper either. Every one of the four is
the same shape:

```
arity check  ->  resolve each entity arg (script ref -> Entity*)  ->  operator new(size)
               ->  <class>::ctor (ALL the defaults)                ->  <= 2 arg-derived stores
               ->  0x550720(owner_entity, record)                   ->  return 1
```

Consequence for the driver: **the ctor is the record.** The argument-derived half of a shipped action
is one or two stores, and in one case zero. Everything else in the bytes we currently fill by hand is
engine-supplied default, not caller intent. Per native:

| native | EA of the `0x550720` call | record | `new(size)` | arg-derived stores before the funnel | size check |
|---|---|---|---|---|---|
| `F3::Attack` | `0x459f5d` | `GameActionAttack` | `0xC8` | **1**: `+0x38` | 200 == `GameActionAttack` size in `F3.json` ✔ |
| `F3::JustAttack` | `0x45a025` | `GameActionAttack` | `0xC8` | **3**: `+0x6C=0`, `+0x6F=0`, `+0x38` | 0xC8 ✔ |
| `F3::StartDialogue` | `0x45a0f0` | `GameActionInitiateDialogue` | `0x24` | **2**: `+0x1C`, `+0x19=0` | 36 ✔ |
| `F3::ReloadWeapon` | `0x45a3e6` **and** `0x45a3fb` | `GameActionReload` | `0x3C` | **0** | 60 ✔ |

`ReloadWeapon` is the extreme case and it is not a stub: it resolves one entity, allocates, runs the
ctor, and enqueues **with no field written at all**. Reload therefore cannot carry a target or a
weapon from the script; everything it needs it must derive from the owner back-link the funnel stamps.

## The shared preamble (this is the AMX-visible contract)

All four natives are `int f(int machine, int params)`:

1. **Arity** — `sub_59AF40(machine, params, expected, name)` at `0x59af4c` is literally
   `if ((*params >> 2) != expected) { error; return 0; }`. So `params[0]` holds the parameter block's
   **byte size** (4 × count) and argument *i* lives at `params + 4*i`:
   `*(params+4)` = arg 1, `*(params+8)` = arg 2. Attack/JustAttack/StartDialogue declare 2,
   ReloadWeapon 1. Failures print `"Invalid number of paramters passed to function %s()."` (shipped
   typo) + `"Expected %d parameters but recieved %d."` (second typo) and the native returns **0**.
2. **Entity resolution** — `sub_59AEF0(machine, cell, &out, 1)` at `0x59aef5` calls
   `meth_0x54be10`, which walks the entity-ref list rooted at `byte_70BE0C` comparing
   `*(candidate + 0x1DC)` against the raw cell, falling back to a map lookup at
   `F3::global_cls_0x50dbb0_0x70be14` (`meth_0x51f0f0`, returning `found[+0x10]`).
   **So the script argument is a ref key, not a pointer.** A miss returns 0 and the native errors
   (`"Attack() was unable to find the attacking entity."` etc.) and returns 0.
   The 4th argument (`1`) is an "insist on active" flag: `*(entity+0x3A0)==0` logs
   `"Attempt to use inactive entity in script."` and **still returns 1** — the check warns, it does
   not refuse.
3. **Return** — `1` on "the order was issued", `0` on arity or resolution failure. **No native returns
   the record pointer**; after the funnel call the record belongs to the entity's queue.

| native | arg 1 (`params+4`) | arg 2 (`params+8`) |
|---|---|---|
| `Attack` / `JustAttack` | attacker ref → enqueue owner (`this` of `0x550720`) | target ref → record `+0x38` |
| `StartDialogue` | source ref → enqueue owner | target ref → record `+0x1C` |
| `ReloadWeapon` | the entity → enqueue owner | — |

## Offsets WRITTEN, with provenance

### `F3::Attack` — `0x459ed0..0x459f6c`

```
459f33  operator new(0xC8)
459f41  GameActionAttack::ctor_0x403e40
459f53  GameActionAttack::meth_0x403fa0(record, target_entity)   ; arg 2
459f5d  0x550720(attacker_entity /*arg 1*/, record)
```

`meth_0x403fa0` (`0x403fa0`) is the *only* store this door makes. Raw form:

```
403fc5  mov  ebp,[ebx+1DCh]     ; ebp = target entity's OWN ref key (Entity+0x1DC)
403fcf  mov  eax,[edi+38h]      ; current record target
403fd2/fd6  if (eax != 0 && eax != ebp) -> retarget branch
404045  mov  [edi+38h],ebp      ; THE write: record+0x38 = arg2's ref key
```

* `+0x38` ← **arg 2**, stored as `Entity+0x1DC` (the ref key), **not** as an `Entity*`. Proven by the
  consumers: `OnCreate_0x4011d0` (`0x40120b`) and `meth_0x4012e0` (`0x401304`) and
  `OnServerUpdate` (`0x404409`) all feed `record+0x38` straight back into `meth_0x54be10` to re-resolve
  it. The record persists an identity that survives despawn; the pointer is re-derived per use.
* The **retarget branch is unreachable on a fresh native record** (the ctor zeroes `+0x38`, so
  `eax == 0` short-circuits). It exists for the already-queued case, and when it does run it writes
  `+0x58 = 1` (`0x403fdf`), allocates a **second** `GameActionAttack` and copies `+0x78`, `+0x48`,
  `+0x4C`, `+0x50`, `+0x54` into it (`0x404010..0x404035`), then enqueues that copy onto
  `record+0x14` (`0x404038 mov ecx,[edi+14h]`). Read that as the answer to "what is `+0x14` for":
  the re-queue door does not receive an entity argument, it reads its owner out of the back-link the
  funnel wrote.

### `F3::JustAttack` — `0x459f70..0x45a034`

Identical to `Attack` plus two raw byte stores between the ctor and the setter:

```
45a00c  mov byte ptr [esi+6Ch], 0      ; ctor wrote 1 here at 0x403f64
45a010  mov byte ptr [esi+6Fh], 0      ; ctor wrote 1 here at 0x403ecc
45a01b  meth_0x403fa0(record, target)  ; +0x38 as in Attack
45a025  0x550720(attacker, record)
```

**So `JustAttack` == `Attack` with exactly two ctor defaults cleared.** Their consumers, which is how
they should be named until proven otherwise:
* `+0x6C` gates `meth_0x4012e0` (`0x4012e6`): the resolve-target → normalised delta →
  `acos(...)*180/pi` → `Entity::meth_0x549fe0(owner, angle)` chain, i.e. **turn-to-face**.
* `+0x6F` gates `OnServerUpdate` at `0x404651`: the `JE::Vector3::EuclideanDistance` + `record+0x74`
  compare, and past it the `new(0x48) GameActionMoveTo` + `0x550720(owner)` at `0x4046a7..0x40470e`,
  i.e. **walk into range**. That is the shipped half of the deviation our port documents as
  "auto-move-to-range".
* JustAttack's *arity* error names `"Attack"`, not `"JustAttack"` (`0x459f7d`) — a shipped copy-paste.
  Cosmetic, but it tells you these two functions were cloned, so treat the pair as one door with a
  two-byte switch.

### `F3::StartDialogue` — `0x45a040..0x45a0ff`

```
45a0c2  JE::Entity::meth_0x54c440(source)        ; SIDE EFFECT before the record exists
45a0c9  operator new(0x24)
45a0d7  GameActionInitiateDialogue::ctor_0x404b40
45a0e4  mov  [eax+1Ch], ecx    ; +0x1C = arg 2's RESOLVED ENTITY POINTER
45a0e7  mov  byte ptr [eax+19h], 0               ; ctor wrote 1 at 0x404b4f
45a0f0  0x550720(source /*arg 1*/, record)
```

Two things the driver must not miss:
1. **The pointer/ref split is inverted versus Attack.** Dialogue hands over a live `Entity*`; its
   `OnCreate` (vft+4, `0x404b80`) then converts it: `mov eax,[eax+1DCh]; mov [ecx+20h],eax` — the
   persisted key is `+0x20`, and `OnUpdate_0x404be0` re-resolves `+0x20` → `+0x1C` **every tick**
   (`0x404c08..0x404c18`), setting `+0x18 = 1` (finish) if the key no longer resolves. A scripted
   dialogue therefore survives the target disappearing and dies cleanly. Attack persists the key at the
   door; dialogue persists it one stage later. Same rule, different site.
2. `Entity::meth_0x54c440` (`0x54c440`) walks the deque at `Entity+0x304` — the *same* container
   `0x550720` appends to — and destroys every element in it (vft+8 per element, then vft[0] with
   `delete=1`), then `if (!vft[+27](entity)) Entity::meth_0x54bf00(entity)`. In door terms:
   **a scripted dialogue flushes the source's pending orders before issuing its own.** No other native
   in this set has a side effect of that size, and our driver cannot express it today.

### `F3::ReloadWeapon` — `0x45a380..0x45a408`

```
45a3cc  operator new(0x3C)
45a3dc  GameActionReload::ctor_0x4071c0
45a3e6  0x550720(entity, record)              ; success path
...
45a3f4  mov ecx,[local_4]; xor eax,eax; push eax
45a3fb  0x550720(entity, 0)                   ; FAILURE PATH PUSHES A NULL ACTION
```

Zero argument-derived stores. `GameActionReload` reads `+0x14` at `0x407292`, `0x407386`, `0x407427`,
`0x407590`, `0x4075a2` — i.e. the funnel-stamped owner is its only input. Note the second site:
on allocation failure the shipped native **still enqueues**, with `param_1 = 0`, and the funnel then
executes `mov [ecx+14h],esi` against `ecx = 0`. That is a guaranteed access violation in the shipped
binary. Do **not** mirror it; the port's "refuse, don't drop" (D-2) stays.

## Offsets LEFT UNTOUCHED — the valuable half

"Untouched" below means *no store by the native, and no store by the ctor*. It does not mean
"uninitialised and ignored": the runtime fills most of them later, from the owner link.

### `GameActionAttack` (0xC8), both `Attack` and `JustAttack`

| offsets the door never writes | who does | why the driver inventing it is a fidelity risk |
|---|---|---|
| `+0x14` | **the funnel only** (`0x550737`) | see §"+0x14" below |
| `+0x18`, `+0x1C` | `OnServerUpdate` `0x404392`, `0x4043FC` (RTTI casts **derived from `+0x14`**) | the driver's notion of "actor" is the funnel's, not an input |
| `+0x20`, `+0x24`, `+0x28` | weapon id / resolved `GameWeapon*` / target-list group, filled in `OnServerUpdate` (`0x404446..0x40446B`) | **the shipped door passes no weapon at all** |
| `+0x2C`, `+0x30`, `+0x34` | attack-mode record + resolved target entity (runtime) | the mode row comes from the actor's equipped weapon, never from the script |
| `+0x3C`, `+0x40` | target creature / target inventory (runtime, `0x40443D`) | derived from `+0x38`, so it must not be set independently |
| `+0x44` (and its byte at `+0x48`), `+0x4C`, `+0x50`, `+0x54` | **only** `meth_0x401100` — `mov [ecx+48h],al` + copy `param_2[0..2]` into `+0x4C/+0x50/+0x54` | this is the *explicit-position override* group (`0x401315`, `0x404410`, `0x4046D1` read it). **No script native ever sets it**: `Attack()` can only attack an entity, never a location. Whoever calls `meth_0x401100` is a different door |
| `+0x58` | retarget branch of `meth_0x403fa0` (`0x403fdf`) and ammo bookkeeping | ctor default 0; the door leaves 0 |
| `+0x59`, `+0x5A`, `+0x5D`, `+0x5E`, `+0x60`, `+0x64`, `+0x65`, `+0x68` | anim-beat / first-strike / series timing, written during `OnServerUpdate` | the driver's `firstStrikeDueMs = nowMs` is a **port-side choice**, not shipped door data |
| `+0x6C`, `+0x6F` | **JustAttack writes these two** (Attack leaves them 1) | if the driver has one `attack` verb it cannot express the difference at all |
| `+0x70`, `+0x71`, `+0x72`, `+0x73`, `+0x74` | ranged / unarmed / melee-range / reach — resolve stage (`0x402410`, `0x40250E`, `0x40256A`) | our port's `unarmed0x71 = true` default is a stand-in for an unported registry, and the shipped door never sets it |
| `+0x78` | ctor **9** ("unset"), copied by the retarget path; written by resolve | the driver's `calledShotPart` 9→0 "preflight" is port policy, not door policy |
| `+0x88`, `+0x8C`, `+0x9C`, `+0xA4`, `+0xA9..0xAB`, `+0xB5..0xB7`, `+0xBD..0xBF`, `+0xC5..0xC7` | **nothing writes them at door time** — heap residue; first written inside `OnServerUpdate`/finisher (`0x4048B1`, `0x404931`, `0x404979`, `0x4048CB`, `0x40491D`) | the byte-level risk: a state hash that covers these bytes is comparing **uninitialised memory** on the shipped side |

Also ctor-side, worth knowing because it is engine-supplied and invisible in the arg list:
`sub_401690` (`0x403ee6`) allocates a **0x28 node** and stores it at `+0x80` with `+0x84 = 0`;
`JE_RbTree_InitSentinel_0x56e0b0` (`0x403f1c`) builds the ordered container at `+0xAC`
(`+0xB0` sentinel, `+0xB4` count 0); `sub_57C2E0` (`0x403f40`) allocates a **0x20 node** at `+0xC0`
(`+0xC4` count 0). A driver that hand-zeroes a record instead of running a real ctor leaves those
three containers dead — which is exactly the ODR/layout landmine T-035 is about.

Non-zero ctor defaults in `GameActionAttack::ctor_0x403e40` (all engine-supplied, none caller-visible):
`+0x04 = 1`, `+0x10 = 1`, `+0x5C = 1`, `+0x6C = 1`, `+0x6D = 1`, `+0x6F = 1`, `+0x78 = 9`,
everything else in the scalar range `0x18..0x74` is 0, and `+0x14` is **never written**.

`GameActionReload::ctor_0x4071c0`: `+0x04 = 1`, `+0x10 = 1`, `+0x30 = 6`, `+0x34 = 1`, zeros at
`0x18..0x2E`, `0x38`. `+0x14` never written.
`GameActionInitiateDialogue::ctor_0x404b40`: `+0x04 = 1`, `+0x10 = 1`, `+0x19 = 1` (then the native
clears it), zeros at `0x18`, `0x1C`, `0x20`. `+0x14` never written.

**The same fact three times is the point: not one of the three ctors writes `+0x14`.**

## `+0x14` — the answer from statics

The native does **not** write it. Neither does the ctor. The **callee** does, unconditionally:

```
550720  push esi
550728  lea  ecx,[esi+304h]        ; Entity+0x304 action deque
55072e  call cls_0x54e070::meth_0x54ea00   ; push_back(&param_1)
550733  mov  ecx,[esp+4+param_1]
550737  mov  [ecx+14h], esi        ; <-- the ONLY writer on this path: owner back-link
55073c  call JE::Entity::meth_0x54ef30     ; deferred refresh latch (unported, see T-030)
```

So the field is **uninitialised heap between `operator new` and `0x550737`**, and every record on this
path has a readable `+0x14` only after it is queued. Statics cannot say *what* it reads before the
call, because `new(0xC8)` may land on recycled memory — that is exactly NiceHawk's T-034/R4 question
and the two lanes are complementary, not redundant:

* statics: no door-side writer exists; the funnel owns the back-link; consumers (`0x404038`, `0x4011f4`,
  `0x404378`, `0x407292`, `0x404c26`, `0x404db3`) all read it *after* enqueue.
* live (R4): is the pre-call value zero or garbage?

**Coordination note, per the disagreement protocol.** IronLion held R4 and left the mesh; it is
NiceHawk's now. If R4 measures a *defined* `+0x14` before the funnel call, statics say either (a) heap
reuse, which is not a model, or (b) a writer we have not found — and I want that reported as a result.
If it measures garbage, we agree, and the port's per-kind linking stays justified rather than
cosmetic.

## What `tools/sim-driver.cpp` is inventing today (and what I propose instead)

The current `attack` verb (`tools/sim-driver.cpp:156-186`, RedXenon's file) does:

```cpp
F3::AttackModeRecord m = F3::GameActionAttack::MakeUnarmedMode( dmin, dmax, 1, tohit );
m.flatDamageBonus = bonus;
o.Start( ea, et, (unsigned)g_clockMs, &m );   // actor, target, mode, timers, seriesRemaining
```

Against the shipped door this is wrong in five separate ways, and only one of them is a bug:

1. **Four caller arguments (`dmin dmax tohit bonus`) have no counterpart in the shipped door.**
   `Attack()` is 2 cells. Damage/to-hit/attack-count come from the actor's equipped weapon's mode row
   (`OnServerUpdate` → `ResolveWeaponAndAttackMode_0x40b380`, which reads the actor's
   `GameInventory`, i.e. the registry this port documents as unported). They must be labelled as
   *port-side mode injection standing in for an unported resolve*, not as script parameters.
2. **`actor` is written by the caller and then overwritten by the seam.** Shipped never writes it —
   the funnel does. The port's assert (`o.actor != ea`) is a fine guard, but the *write* in `Start()`
   is invention, and `EnqueueEntityAction` is the thing that makes it true.
3. **`target` is stored as a pointer where shipped stores a ref key.** Shipped `+0x38` =
   `Entity+0x1DC` and is re-resolved per use; the port stores `SpawnedEntity*`. This is a
   representation deviation (register it as **D-x**), not an error — but it must be registered,
   because it is what makes "despawn mid-attack" reachable in the original and unreachable in the sim.
4. **`calledShotPart 9 -> 0` preflight and `firstStrikeDueMs/nextSubAttackDueMs = nowMs`** are
   port policy. The shipped door leaves `+0x78 = 9` (unset) and `+0x5E/+0x65 = 0`.
5. **The driver cannot express `JustAttack` at all**, because the entire difference is two bytes.

Proposed shipped-shaped verb (for RedXenon to apply, not me):

```cpp
} else if ( strcmp( verb, "attack" ) == 0 || strcmp( verb, "justattack" ) == 0 ) {
    // T-037: shipped door = new(0xC8) + ctor_0x403e40 + ONE store (meth_0x403fa0):
    //   record+0x38 = target's ref key. Damage/to-hit are NOT script data (0x459f5d).
    //   justattack == attack with the two ctor defaults +0x6C/+0x6F cleared (0x45a00c,0x45a010).
    int just = ( verb[0] == 'j' );
    ... sscanf( line, " %*s %63s %63s %d %d %d %d", ... ) ...
    F3::GameActionAttack& o = g_atks[ g_atkN++ ];   // default ctor == shipped ctor defaults
    o.StartScripted( et );                          // ONLY +0x38; touches nothing else
    if ( just ) { o.turnToFace0x6C = false; o.approach0x6F = false; }
    if ( n >= 6 ) { o.SetModeForTestOnly( dmin, dmax, tohit, bonus ); }  // deviation, logged
```

plus a one-line `SIM-DRIVER NOTE` when the override branch runs, so a digest reader can tell a
shipped-shaped order from a mode-injected one. Diff versus today: `Start(who, against, nowMs, &m)`
→ `StartScripted(against)` (no actor write, no timer stamps, no called-shot preflight, no mode), with
the actor still asserted post-enqueue as seam-owned.

## Doors that are NOT this row

* **Server packet** — `GameServer::virt_meth_0x46dd70`, 4 sites into `0x550720`. Unexamined here; may
  fill `+0x38`/`+0x20`/`+0x30` differently (a networked order is exactly where an id-vs-pointer
  difference would show up). Separate row.
* **Explicit-position override** — `GameActionAttack::meth_0x401100` (writes `+0x48/+0x4C/+0x50/+0x54`).
  Callers unknown; no script native in this set calls it.
* **Follow-on re-queues** — `OnServerUpdate` `0x40470e`/`0x40471e` (MoveTo + the next attack),
  `meth_0x403fa0` `0x40403c` (retarget copy), `meth_0x404270` `0x4042b4..0x4042f8` (clone-and-copy
  `+0x38`/`+0x5B`/`+0x78`/`+0x48`...), `GameActionInitiateDialogue::OnUpdate` `0x404d0a`. These all
  depend on `+0x14` being set — consistent with the funnel owning it.
* `sub_45C140` (hit reaction), `GameProjectile` `0x411e7a`, `sub_467640`, `sub_435480`, `sub_435DE0`.

### The measurable consequence this map hands to D-2

`Entity+0x314` is the deque's **live count** (T-039, `9bbdc9e`: `++` at `0x54ea71` inside the push the
funnel performs at `0x55072e`, `--` at `0x54b637`/`0x54b672`, zero at `0x5515e6`), so the depth question
this row raises is *readable*, not inferable. Statics already shows the growth mechanism: a single
`OnServerUpdate` pass **consumes one action and pushes two onto the same owner** — `0x40470a mov
ecx,[esi+14h] / push edi /*GameActionMoveTo*/ / call 0x550720` and then `0x40471a mov ecx,[esi+14h] /
push eax /*meth_0x404270's clone*/ / call 0x550720` — while the drain pops exactly one element per pass
(`sub_5505F0` 0x550617..0x550647). An out-of-range melee actor therefore grows the deque by ~1 per pass,
and the container has **no bound**: `cls_0x54e070::meth_0x54ea00` allocates another 4-pointer block node
at `0x54ea54`. So shipped *expects* >4 live actions, and our one-block `actions[4]` (D-2) drops an order
the original would run.

What statics cannot do is prove the demo ever *reaches* 5 — that is one read of `Entity+0x314` in T-034,
and the argument for preferring a **count read** over an x64 hardware breakpoint (harness lesson 12).
Handed to RedXenon/NiceHawk on exactly that basis; not claimed here.

## Routed questions (rule 3 — undeterminable here, not coded as guesses)

1. **What exactly is `Entity+0x1DC`?** It is the script-visible ref key (`meth_0x54be10` matches on it;
   `UnpackState_54e690` writes it at `0x54e6e9` from the loaded stream; the dialogue path uses it as
   `DialogueManager::BeginDialogue_55edb0`'s argument at `0x404db6`/`0x404df0`). Whether it is a
   form-id, a load-serial, or a handle index is **not** settled by these sites. Needs the
   entity-load/ref-registry row.
2. **Who calls `meth_0x401100`**, i.e. which door attacks a *location* rather than an entity?
3. **`GameActionAttack::OnServerUpdate` +0x6C gate reachability**: `meth_0x4012e0`'s caller set was not
   enumerated (offline DB has no xrefs for free functions and I kept the IDB read-only). Needed before
   naming `+0x6C`/"turn-to-face" definitively; same for `+0x6F`/"approach".
4. **`ctor_0x403e40` writes a byte it never initialises into `record+0xBC`**
   (`0x403f27 mov al,[esp+20h+local_11]` → `0x403f3e mov [edi],al`, `edi = record+0xBC`, then
   `sub_57C2E0` builds the node at `+0xC0`). `local_11` is a slot in the ctor's own frame and **no
   store in the ctor writes it**, so `+0xBC` is indeterminate at door time. Is that a real field (a
   queue/allocator flag) or a compiler artifact? Matters for any byte-level digest: our port zeroes it.
5. **Server packet door** (§ above).
6. Whether `meth_0x54be10`'s `byte_70BE0C` list walk and the `global_0x70be14` map can disagree
   (two resolution paths, one ref key) — relevant to "target despawned mid-attack".
