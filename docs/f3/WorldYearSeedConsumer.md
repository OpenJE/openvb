# `[World] Year = 2253` — who consumes the shipped clock seed?

T-038. Statics only (`ida_original_f3`, read-only, no IDB writes; `artifacts/F3.json` for the vftable
and member-usage lookups). Feeds T-031 (parity oracle) and T-026 (`clock` in the digest).
**ZenRaven owns the digest format**: this row states the finding and one proposal, and does not touch
`docs/f3/SimDigest.md`.

## Verdict in one line

The key **has a consumer** — the world's calendar block is copied into the `JE::Gregorian` clock at
world-init — **but it does not reach `Gregorian+0x34 totalSeconds`**, which is the quantity our digest
prints as `clock`. `clock` stays origin-comparable; the date lands in the *year/month/day/hour/minute/
second* fields only, and nothing in the combat path reads those. So neither of ZenRaven's two branches
is quite right: not "cosmetic data", and not "the hashed clock field has a different baseline".

## The cited chain: INI key → reader → fields written → first consumer

| stage | EA | evidence |
|---|---|---|
| INI key ships | `data/Engine.grp` plaintext `[World] Year = 2253`, `Month = 10`, `Day = 18`, `Hour/Minute/Second = 13`, `Seconds Per Round = 6` | RedXenon, T-016 §2 (not re-verified here) |
| **the reader** | `JE::World::virt_meth_0x546b50` = our `ApplyWorldClockParams` | tree from `sub_56AF40()`; section `"World"` (`0x546b96`), key strings `"Seconds Per Round"` `0x546c35`, **`"Year"` → `j__atol` → `mov [this+14h],eax` at `0x546d1d`**, `"Month"` `0x546da5` → `+0x18`, `"Day"` `0x546e2d` → `+0x1C`, `"Hour"` `0x546eb5` → `+0x20`, `"Minute"` `0x546f3d` → `+0x24`, `"Second"` `0x546fc5` → `+0x28`; `+0x40 = 6` written first at `0x546b5b` |
| **when** | at **world-entry**, first statement of `JE::World::virt_meth_0x546690` (Initialize): `0x5466b0 call dword ptr [eax+14h]` → slot 5 of `World_vftable_691a1c` = **`0x546b50`** | immediately followed by `0x5466b5 mov [esi+48h],0`, `0x5466b8 call JE_TickMilliseconds_0x56af50` → `[esi+4Ch]`, `0x5466c0 mov [esi+50h],0`, `0x5466c3` → `[esi+54h]`, then the `globals.ini` strings at `0x5466f5` |
| **the consumer** | `JE::Gregorian::ctor_0x544950` (`0x54497d`): `qmemcpy(&this->+0x04, param_1, 0x18)` — a **24-byte = 6-int block copy**, then `0x54499d virt_meth_0x5a3740(this, param_1)` which does `0x5a3752 qmemcpy(&this->+0x1C, &this->+0x04, 0x18)` | the running fields are `+0x1C` year, `+0x20` month, `+0x24` day, `+0x28` hour, `+0x2C` minute, `+0x30` second, `+0x34` totalSeconds (`docs/f3` + `modules/openje/src/JE/Gregorian.hpp`) |
| factory | `F3::GameWorld::virt_meth_0x46fda0` = `sub ecx,[ecx-4]; sub ecx,230h; jmp sub_544D90`, and `sub_544D90` does `new(0x38)` + `Gregorian::ctor_0x544950(v2, this-340)` | the only caller-side seed source in sight |

### Why the 24-byte source is the World calendar block (and what is still inferred)

Three independent shape matches, plus one absence:

1. `ApplyWorldClockParams` fills exactly **six consecutive ints at `World+0x14..+0x28`** in the order
   year, month, day, hour, minute, second. The copy reads exactly **24 bytes = six ints**.
2. `JE::World::ctor_0x548a80` initialises that block to **`0, 1, 1, 0, 0, 0`** (`0x548af6` `+0x14=0`,
   `0x548af9` `+0x18=1`, `0x548afc` `+0x1C=1`, `0x548aff..0x548b08` `+0x20/+0x24/+0x28=0`), and
   `Gregorian::ctor_0x544950` initialises its running block to **the same `0,1,1,0,0,0`**
   (`0x544985..0x544994`). Field-for-field identical defaults is not a coincidence you get from two
   unrelated structs.
3. **Nothing reads `World+0x14` directly**: `f3_member_index World +0x14` lists exactly two usages,
   `0x546d1d` (the INI write) and `0x548af6` (the ctor zero write). Same for `+0x18`
   (`0x546da5`, `0x548af9`, and one `+0x18` hit at `0x54811d` which is a *stack-slot* store inside a
   `net::buffer::underflow` throw — an IDA member-alias, not a read). So the block's only possible
   consumer is a copy through a computed pointer, which is what `0x54497d` is.
4. ⚠ **Not resolved here**: the pointer arithmetic `this-0x230` (thunk) `-0x154` (`sub_544D90`) was
   not reduced to a `World`-relative offset, so `param_1 == &World+0x14` is a *strong shape inference*,
   not a verified identity. If it is wrong, `World+0x14..+0x28` has **no consumer at all** and the
   verdict degrades to cosmetic shipped data — which is the *safer* conclusion for the digest, not the
   riskier one. Settling it needs one live read of `World+0x14` beside `Gregorian+0x1C` (a debugger),
   which this row deliberately did not use.

## What the seed does NOT touch — the part that decides the digest

`virt_meth_0x5a3740` (`0x5a3740`) is a *catch-up* routine: after copying the seed into the running
fields it sets `0x5a375a mov [this+34h], 0` (totalSeconds = 0) and then, for each component,
`if (param_1[i] > running[i]) AddYears/AddDays/AddHours/AddMinutes/AddSeconds(delta)`. At construction
the two are equal by construction, so **every branch is dead and totalSeconds ends at 0**. The frame
delta from the world pump is what later grows it (`Gregorian::AddSeconds` `0x5a3c60`, which is the only
thing that adds to `+0x34`).

Consequences:

* The **year never enters `clock`**. `GameWorld::ClockSeconds()` = `m_clock->TotalSeconds()`
  (`src/F3/GameWorld.cpp:110`) mirrors shipped `Gregorian+0x34`, and `+0x34` is 0 at init in the
  original too. The `clock=29242` frame-0 leap is caused by `[World+0x48]=0` vs an absolute
  `timeGetTime` delta (`0x5466b5` + `Update_0x544a90`), which we already mirror — see
  `docs/f3/WorldTickPump.md`. **Baseline shared, and the year adds nothing to it.**
* The date *is* real state (year/month/day/hour/min/sec on the clock object), just not state anything
  in the sim reads. So it must not enter the hashed set.

## Persistence (question 2, second half)

`JE::World::PackState_545930` writes, in order: `+0x44` (`0x545949`), **`+0x48`** (`0x545958`), `+0x6C`,
`+0x70`, `+0x74`, `+0x7C`, `+0x80`, `+0x84`, then iterates the entity list packing each
`entity+0x1DC` (the script ref key) and `entity+0x15C`, then the map list at `+0xE4`.
**`+0x14..+0x28` is not packed.** So the calendar is re-seeded from the INI every run and does not
survive a save; the *tick stamps* do. That is worth remembering for T-031's capture: a restored save
carries `+0x44/+0x48` (which is exactly the pair that produces the frame-0 leap) but re-reads 2253
from disk. Also note `PackState` requires state 3 (`0x545934`, fatal otherwise) — same gate shape as
the `UnpackState() method may only be called` guard at `0x54e6b6`.

## Does our port reproduce it? (question 3)

| shipped | port | status |
|---|---|---|
| `0x546b50` reads `[World] Year/Month/Day/Hour/Minute/Second` + `Seconds Per Round` into `World+0x14..+0x28/+0x40` | `JE::World::ApplyWorldClockParams` (`modules/openje/src/JE/World.cpp:22`) | **mirrored**, and tested: `StringSystem.cpp:119` asserts 2253/10/18/13:13:13 from the real `sys.ini` |
| called as the **first** statement of `Initialize` (`0x5466b0`) | called from `GameWorld::Initialize` before the clock is built (`src/F3/GameWorld.cpp:89`) | mirrored (order vs the `+0x4C/+0x54` stamps is not modelled, and those stamps are not modelled at all) |
| Gregorian seeded by a **24-byte block copy** from the calendar block, `totalSeconds = 0` | `new JE::Gregorian(m_calendarYear ?: 2253, m_calendarMonth ?: 10, m_calendarDay ?: 18, h, mi, s)` with `totalSeconds = 0` | **seeded, but with different fallbacks** — see the bug below |
| catch-up chain in `virt_meth_0x5a3740` (Add* to reach a target date) | not implemented; the running fields are simply initialised | **not mirrored, and unreachable at init** (dead branches, § above). Only a later `setdate`-style caller would need it |

**Filed as a bug, not a deviation:** the `?: 2253 / ?: 10 / ?: 18` fallbacks at
`src/F3/GameWorld.cpp:90-92` are *our* invention. Shipped's defaults for a missing key are the World
ctor's `(0, 1, 1, 0, 0, 0)` — the literal `2253-10-18` comes from `sys.ini`, never from code. With the
key present (the demo) both sides agree; with the key absent the sim says 2253-10-18 and the original
says year 0, month 1, day 1. One-line fix: pass the `m_calendar*` fields through unchanged (they are
already initialised to 0/1/1 in `JE::World`'s ctor — `modules/openje/src/JE/World.hpp:13`, which should
be checked to match `0,1,1,0,0,0`, not all-zeros) and delete the literals. `docs/DEFERRED-NOTES.md`
carries the row.

## Proposal to ZenRaven (decision-maker, not mine to execute)

1. **Keep `clock`/`clockDelta` in the compared set unchanged.** This row removes the counterfactual:
   the year cannot reach `totalSeconds`, so there is no hidden baseline.
2. **Do not add a `year` field to the hashed set.** Nothing gates on it. If the date is ever wanted,
   print `date=` under **CONTEXT** alongside `tickMs`, with the reason stated as "INI-derived, not
   sim-derived, and not persisted by `PackState_545930`".
3. If the oracle ever captures a **saved game** rather than a fresh boot, `+0x44/+0x48` are packed and
   the calendar is not — that asymmetry belongs in `ParityOracleDesign.md` §3, and I did not edit that
   file either.

## Numbering disagreement, recorded not reconciled

`docs/DEFERRED-NOTES.md` says "vft[5] at Initialize start (0x5466B0) is a no-op in engine (0x404b70)".
The primary table contradicts that: `World_vftable_691a1c` slot 5 (`+0x14`) is `0x546b50` in
`artifacts/F3.json`, and `0x5466b0` is literally `call [eax+14h]` — i.e. Initialize's first action is
the calendar-params call, not a no-op. `0x404b70` is a real no-op (`retn`) and `JE::World` carries four
distinct vft pointers (`0x548abb/0x548ac1/0x548ac8/0x548ad2`), so the two statements are probably about
*different tables* (primary `0x691a1c` vs the mixin at `0x691A20`). Not mine to reconcile here; flagged
because a reader using the old note would expect no seed at init and find one.
