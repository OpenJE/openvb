# The world tick pump — and why the clock jumps on frame 0 (answered by statics)

T-026 surfaced `clock=29242` at `frame=0` in an app digest run. T-031's acceptance asked for that to be
settled "here, not in prose" via a debugger read. **It is settled without one** — the two reads that
decide it are in the shipped code's own init and pump. Cited by EA; nothing here is inferred from our port.

## The pump passes ABSOLUTE uptime

`JE::Server::virt_meth_0x552b60` (0x552b60, size 0x6a):

```
552ba4  v2 = F3::global_cls_0x616328_0x70bd98 + 1      ; the World subobject of the server instance
552bab  mbr_0x0 = v2->mbr_0x0                          ; its vftable pointer
552bad  v4 = JE_TickMilliseconds_0x56af50()            ; timeGetTime(): MACHINE UPTIME, ms
552bb5  (*(mbr_0x0 + 24))(v2, v4)                      ; vftable offset +24 = SLOT 6, arg = that tick
```

So `ms` is **absolute uptime**, not a delta and not a session-relative timer. `0x544a90` is recorded in
the F3 DB as a `World` virtual method (`virt_meth_0x544a90`), and slot +24 is the one the DB's World
vftable dump leaves unlisted — the tick-consuming single-int updater. (If slot 6 ever resolves to a
different EA, this page is wrong; that is the only assumption in it.)

## THE RESOLUTION (T-045, 2026-10-21): the pump passes the tick through the **+4 mixin table**, and slot 6 there IS `Update`

The falsification above was correct about the bytes and wrong about which table they belong to. `JE::World`
carries **four** vft pointers, and the ctor says so explicitly (`ctor_0x548a80`):

```
548abb  mov [esi],    offset World_vftable_691a1c      ; primary   -> this = X+0
548ac1  mov [esi+4],  offset ??_7World@@6B@_0          ; mixin +4  -> this = X+4
548ac8  mov [esi+0Ch],offset ??_7World@@6B@_1          ; mixin +C  -> this = X+0xC
548ae6  mov [eax+esi+4], ecx                           ; vbtable displacement (virtual base)
```

and the server holds the **`+4`** one, not the primary:

```
552ba4  lea  esi,[eax+4]        ; esi = the +4 subobject, NOT the primary
552bab  mov  ebx,[esi]          ; ebx = ??_7World@@6B@_0 = 0x6919e4 (engine) / 0x66856c (GameWorld copy)
552bb3  mov  ecx,esi            ; this = X+4
552bb5  call [ebx+18h]          ; slot 6 OF THE +4 TABLE
```

Read both +4 tables out of the image (`.rdata`, base `0x400000`, no ASLR):

| table | slot 4 `+0x10` | slot 5 `+0x14` | **slot 6 `+0x18`** |
|---|---|---|---|
| `0x6919e4` (`JE::World` +4) | `0x544a60` | `0x544a70` | **`0x544a90` = `World::Update`** |
| `0x66856c` (`F3::GameWorld` +4) | `0x544a60` | `0x544a70` | **`0x544a90`** (GameWorld does *not* override it) |

`.rdata` contains exactly two dwords equal to `0x544a90`: `0x6919fc` and `0x668584` — the two cells above.
And the primary table `0x691a1c`, the one R0 read, has its **own** slots 3 and 6 as `0x404b70`. So both readings
were right about their own table; `WorldTickPump`'s mistake was indexing the primary.

**So `ms` does reach `Update_0x544a90`, and through what:** `JE_TickMilliseconds_0x56af50()` →
`JE::Server::virt_meth_0x552b60` `0x552bb5` → slot 6 of the `+4` mixin table → `World::Update(X+4, ms)`.
The pump chain is intact; `0x404b70` is irrelevant to it (a 1-byte `retn` that ignores its argument, so it can
never have been the tick consumer — it does not even store the value it is handed).

The cluster is now a **vtable of one interface**, not four entry points: slot 4 `0x544a60` = `mov
[this+0x60], arg; retn 4` (a setter), slot 5 `0x544a70` = the gate (reads state at `this+0x34` and
`this+0x68`, `retn 4`) — that is the `vft3[+0x14]` gate our `Update` already calls at `0x544ad0`, which is
itself the proof: `Update` dispatches `[edx+14h]` on the *same* table it was entered through — and slot 7 =
`0x545100` (a `new(0x24)` Combat-family factory), slot 11 = `0x545930` = `PackState`.

## Second, independent reason there is no leap: **the shipped tick is not `timeGetTime`**

Four pages state that `JE_TickMilliseconds_0x56af50` *is* `timeGetTime()` (machine uptime). It is not. The
function is two instructions:

```
56af50  mov eax, F3__global_elapsed_time_0x70c2b8     ; NOT timeGetTime
56af55  retn
```

and that global has exactly one writer, `F3::ProcessMessagesAndUpdateTime` (`0x56afb8..0x56afd7`), which
accumulates **deltas**: `diff = timeGetTime() - SYSTEM_TIME_MS_0x70c2bc + elapsed`; `SYSTEM_TIME_MS = now`;
`elapsed = diff`. `WinMain_0x56b810` seeds the reference **at startup** (`0x56b8a0 call timeGetTime` →
`0x56b8aa mov F3__SYSTEM_TIME_MS_0x70c2bc, eax`), and `0x70c2b8` sits in the zero-filled tail of `.data`
(`.data` VA `0x6f1000`, `vsize 0x200e0`, `rawsize 0x17000` — the dword is past the file-backed part, so it
loads as **0**). A dword scan of `.text` finds no other reference to either global. So the shipped tick is
**elapsed-since-process-start, beginning near 0** — structurally incapable of carrying machine uptime.

## This needs no live sample: the writer's own algebra closes it

The proposed confirmation — read `[0x70c2b8]` twice, ~200 frames apart — is unnecessary, and a foreign
`winedbg.exe` is alive on port 31338, so it is not free to spend. The single writer telescopes:

```
elapsed_n = elapsed_(n-1) + (now_n - now_(n-1))     ; now = timeGetTime(); 0x56afd2 keeps last = now_n
          = now_k - now_0                            ; every intermediate term cancels
```

with `now_0` the `WinMain` seed at `0x56b8a0`. So `[0x70c2b8]` is **`timeGetTime() − timeGetTime at WinMain`**
by construction; it can equal machine uptime only if the second term were absent, and the writer's own
operands exclude that. Two consequences worth recording:

1. **A first sample of `0` is uninformative.** Before the pump has run once, `elapsed` is `0` under *both*
   hypotheses. And the discriminating sample is not safe under a hit-and-hold window either: `now_n - now_0`
   includes stopped time, so a long debugger pause inflates the accumulator in exactly the way uptime would.
   That is an argument for the telescoping proof rather than against it.
2. **Blast radius, measured — and I have overstated it twice, so here is the trace that ends it.** The fold
   runs `0x544aab` accumulator → step-6 carry → `Gregorian::AddSeconds` → `totalSeconds`. Following every
   consumer of that value:
   - `ClockSeconds()` (`src/F3/GameWorld.cpp:118`) has exactly two call sites, both in the digest
     (`src/F3.cpp:842` base capture, `:851` the printed value).
   - So **`clockDelta` cancels it**: the base is snapshotted on the first tick and the line prints
     `clockSec - base`. The compared field cannot see this bug — which is a limitation of the digest, not
     reassurance about the accumulator.
   - **`turnDeadline0x29C` is NOT reached.** I asserted it was, twice, and was wrong both times. The live
     `accMs_` is a **round-based virtual clock** — `accMs_ += 1000 * secondsPerRound`, once per dispatched
     queue event (`CombatManager.cpp:188`, `GameWorld.cpp:53-58`) — and the `accMs_ += clockSeconds*1000`
     line that *would* have carried the fold is the **C-23 placeholder, retired by T-011**
     (`CombatManager.hpp:114-116`, canonical fact sheet §13). I quoted a retired placeholder as live code.

   Net: **the fold is invisible in the compared set and touches no arm, no deadline, no capture baseline.**
   What survives is smaller and still real: (a) our absolute `clock` prints uptime-shaped where the
   original's prints session-shaped — a fidelity defect in a CONTEXT number and in the pages explaining
   it; (b) `CombatManager.hpp:38` still says the port "feeds accMs from `JE::Gregorian::TotalSeconds()`",
   contradicting both the code and its own comment twelve lines below. One-line stale-comment fix, in that
   lane's hands, not mine.

   Correction history, kept because it is the useful part: v1 "three lanes are building on a false premise,
   deadlines contaminated" → wrong about deadlines; v2 "permanent in the deadline, cancels in `clockDelta`"
   → right about `clockDelta`, still wrong about the deadline; v3 (this) traced by consumer rather than
   inferred. Both errors came from citing a path I had read about in a comment instead of at the call site.

3. **T-011 arming: unaffected — under the port's current operand.** `accMs_` is fed by
   `m_secondsPerRound`, never by the clock, so the fold cannot reach `turnDeadline0x29C` today. This is a
   statement about the code as it stands, not about the shipped line: `docs/DEFERRED-NOTES.md:354-356`
   records an older reading of the same consumer as `acc_ms += 1000*worldSimSeconds` off
   `World+0x70`/`IWorldServer` slot 0 (`0x46F250`), while T-010d/T-011 read the operand as `world+0x40`,
   the immutable `[World]` config. **If that older reading is the correct one and the port moves back to a
   clock-derived operand, the fold does reach every deadline** and this bullet flips. So the withdrawal is:
   not contaminated *by the current port*, and the operand question — already T-011's — decides the rest.
   Worth settling once, because the two readings differ on whether `accMs` is a virtual round clock or a
   wall clock, and that is exactly the axis a parity diff turns on.

RedXenon verified both load-bearing facts independently — the bytes `a1 b8 c2 70 00 c3` and
`vsize 0x200e0 > rawsize 0x17000` — and their own T-034 capture note had already named `0x70C2B8` as the
tick value: the mis-inference was call-site-equals-global, and the mis-numbered table is why R0 looked
live-only. That pair of independent reads is why this section states the algebra rather than a request.

That is the mechanism of our own artifact: `src/F3.cpp:786` is
`int TickMilliseconds() { return (int)timeGetTime(); } // 0x56AF50` — labelled as the shipped accessor while
returning the raw API value. So `tickMs=29242857 clock=29242` at `frame=0` is our port putting uptime into an
elapsed slot, on top of never stamping the field `Update` subtracts.

## What this takes away: **the shipped code does not leap on frame 0. We do.**

Because `Update`'s `this` is `X+4`, its displacements are **not** primary offsets. Translating them:

| `Update` reads | as primary offset | who initialises it |
|---|---|---|
| `[this+0x34]` (state) | `X+0x38` | `ChangeState_0x5454f0` uses `X+0x38` directly — **same field, one state** |
| `[this+0x44]` (accumulator) | `X+0x48` | `0x5466b5` → **0** |
| `[this+0x48]` (**the stamp it subtracts**) | `X+0x4C` | `0x5466bd` → **`JE_TickMilliseconds_0x56af50()`, SEEDED** |
| `[this+0x58]` (pause credit) | `X+0x5C` | ctor `0x548b30` → 0 |

and the same holds for the client pump: `Tick_0x544b30` is slot 0 of the `+0xC` table (`??_7World@@6B@_1` =
`0x6919c0`), whose `[this+0x48]` is primary `X+0x54` — the **second** seeded stamp — and it proves its own
base twice over with `mov edx,[eax-0Ch]` / `lea ecx,[eax-0Ch]` at `0x544b60/0x544b66`.

So the world-entry path seeds **both** stamps (`X+0x4C` for the server pump, `X+0x54` for the client pump)
and zeroes **both** accumulators (`X+0x48`, `X+0x50`). The first shipped `delta` is
`uptime − uptime-at-world-entry` = **one frame**, not 29242 seconds. `docs/DEFERRED-NOTES.md`'s claim that
"`ctor_0x548a80` zeroes the range covering `World+0x48`" read a mixin displacement as a primary one: the
field `Update` subtracts is the *seeded* stamp.

**Consequences, stated before anyone builds further on them:**

1. **T-026 / the digest: `clock=29242` at `frame=0` is a PORT ARTIFACT, not shipped behaviour.** Our
   `World::Update_0x544a90` computes `delta = ms - mbr_0x48` with `mbr_0x48` never seeded, so we fold machine
   uptime into the accumulator on frame 0 and the original does not. The `WorldTickPump` verdict below
   ("mirror the leap, do not fix it") is **reversed by this row** and stands corrected: fidelity here is
   *seeding the stamp*, and the fix is ours (`GameWorld::Initialize` should stamp the tick into the same
   field our `Update` subtracts, mirroring `0x5466b8/0x5466bd` — which the shipped code does in the world-entry
   path and we do nowhere).
2. **T-031 parity:** `clock` was kept comparable on the argument "both sides take the identical leap". That
   premise is false, so the field is comparable *only after* the port seeds the stamp; until then a shipped
   capture and our run differ by ~8 hours of folded uptime on frame 0 and by that much of `totalSeconds`
   thereafter. T-038's conclusion that the **year** cannot reach `clock` is untouched (different argument,
   `totalSeconds` is still 0 at construction) — what changes is the *other* half: our baseline is uptime, theirs
   is world-entry.
   **And `tickMs` itself is mis-described upstream.** `docs/f3/ParityOracleDesign.md:47`,
   `docs/f3/SimDigest.md:173-180` and `docs/f3/OrigCapture.md:63` each say `JE_TickMilliseconds_0x56af50` is
   `timeGetTime()` = machine uptime, and use that to justify putting `tickMs` in CONTEXT. The shipped accessor
   returns an *accumulated session delta* (zero-filled global + a WinMain seed), so in the original `tickMs`
   is process-relative and would in fact be comparable — the incomparability being designed around is ours.
   Those three pages are ZenRaven's/RedXenon's lanes; flagged here, not edited.
3. **T-011 arming:** any deadline armed from a first frame in our run absorbs the false 29242 s, so a
   round-1 timing difference found against the open demo is a suspect against this bug, not a round-math
   finding. Re-check after the stamp is seeded.
4. **The old ledger note about `vft[5]` is also a numbering artifact, and my T-038 slot-5 claim stands:**
   `0x5466b0` is `call [eax+14h]` on the **primary** table (`[esi]`, `esi` = primary), whose slot 5 is
   `0x546b50` = the calendar seeder. `0x404b70` sits at primary slots **3 and 6**. Two notes, two tables, no
   contradiction — resolved from the image rather than by picking a winner.

## The init zeroes `+0x48` and seeds the OTHER stamps

`JE::World::virt_meth_0x546690` (vftable slot 1; reads `globals.ini`, i.e. the world-entry path), with
`ebx = 0` from `xor ebx,ebx` at 0x5466b3:

```
5466b5  mov [esi+48h], ebx      ; World+0x48 = 0            <- the stamp Update subtracts
5466b8  call JE_TickMilliseconds_0x56af50
5466bd  mov [esi+4Ch], eax      ; World+0x4C = tick          <- SEEDED
5466c0  mov [esi+50h], ebx      ; World+0x50 = 0
5466c3  call JE_TickMilliseconds_0x56af50
5466c8  mov [esi+54h], eax      ; World+0x54 = tick          <- SEEDED
```

## Conclusion (T-045): the jump is **ours**, and the mirror verdict below is reversed

`Update` (0x544a90, our `World::Update_0x544a90`) is entered with `this = X+4` and computes
`delta = ms - [this+0x48]` — i.e. `ms - X+0x4C`, and `X+0x4C` is **seeded with the tick at world-entry**
(`0x5466b8/0x5466bd`). The shipped accumulator `X+0x48` starts at 0, the shipped *stamp* does not. So the
first shipped delta is one frame and there is **no 8-hour leap in the original**.

The text below was this page's verdict before the base was known; it is kept, marked, because the reasoning
is what produced the wrong answer and a reader should see it:

> `Update` computes `delta = ms - [this+0x48]` and only then stores `ms` into `+0x48`. On the first call
> after entry that is `uptime - 0` … **Our behaviour is faithful here**, and the mirror rule applies: no
> "initialise the stamp to the first tick" change.

**That was wrong in both halves**: the subtraction target is the seeded field, and the "initialise the stamp
to the first tick" change is exactly what the shipped code does (`0x5466bd`) and what we do not. The mirror
rule still applies — *to the seeding*.

Two consequences that matter beyond this row:

1. ~~**`clock` is a valid comparison field for the parity oracle**~~ — **superseded by T-045**: it is
   comparable only once the port seeds the stamp; today our `clock` carries ~8 h of machine uptime the
   original does not. `clockDelta` is unaffected in *shape* (both sides then advance per frame), but its
   frame 0 value differs, so a first-frame sample must not be used to validate the frame-0 path.
2. The seeded stamps `+0x4C` and `+0x54` are what the *other* tick consumers read (0x546690 seeds both, and
   `+0x54` is the field our `Update` subtracts as pause credit). Why `+0x48` alone is left at 0 is still
   open — it is either deliberate (the first delta is meant to be the uptime, because the world clock is
   calendar-based and `AddSeconds` is later clamped/ignored) or a shipped quirk. Not a defect claim: a
   question with a location.

## What still needs the debugger (the remaining T-031 reads)

Written as address / value / when, for whoever runs the window — see
`docs/f3/ParityOracleDesign.md` §Reads. Read (a) from the coordinator's list — what `ms` the pump passes —
is **closed by this page** and should not be spent on.

---

## T-079 batch-1 annotation (2026-10-22) — W1/W2 from the T-072 sweep

Per `discoveries/2025-10-21-t072-doc-errata-sweep.md` §1/§4. This page's BODY is the current authority
(T-045); two tails needed pointers, re-verified against `$OPENVB_ORIG_EXE` at apply time.

- **W1 — the section above is SUPERSEDED by T-052/T-056**: the six T-031 reads are no longer
  debugger-owed — they were TAKEN with values (T-056 six-row table; R0 `0x544A90` live-confirmed on
  three boots, matching this page's static claim — no loud-flag owed). Remaining depth reads re-based
  to T-067 (R5/R6, rung-2 list; folded by T-068/T-070). Same section's environment note about a
  foreign `winedbg.exe` on port 31338 is stale context: T-056 ran a private clone prefix + fifo-held
  `winedbg --gdb` on :31440, and the ImHex contention was named-solved in the T-034 handover
  (stub moved to 31437 for the ORIGINAL, ImHex keeps 31337 — env vars are the source, not memory).
- **W2 — the Conclusion headline needs the magnitude qualifier** (second leg of the §1 three-way
  collision): "the first shipped delta is one frame and there is no 8-hour leap in the original" is
  TRUE for the FOLD (the ~8 h machine-uptime artifact is ours, `src/F3.cpp:786` vs the session-
  elapsed `[0x70c2b8]` read here), but T-056 R2 LIVE-CONFIRMED a small shipped leap of a different
  magnitude: the world-seconds clock carries 0s→1s inside the first pumps (pump#1 0s → pump#3 1s;
  19–21s boot-offset character at the R1 instant). T-068: that carry is ported+correct in the app
  path, unobservable in the harness (driver clock starts 0). So "we do [leap]" on this page means
  the FOLD; the shipped 0→1s CARRY is real and nobody should read this headline as denying it.
- **Apply-time receipt (T-079)**: re-verified in the image — 0x56af50 bytes `a1 b8 c2 70 00 c3`;
  world-entry seed block `0x5466b5..0x5466c8` exactly as tabulated above (`mov [esi+48h],ebx` /
  `call 0x56af50` / `mov [esi+4Ch],eax` ...); `[0x691a1c+0x14]=0x546b50`, `+0x18` and `+0x0c` =
  `0x404b70` (slot-5 seeder, slots 3/6 retn — this page's own table identity). One catch against
  the BODY, logged not rewritten: the init-zeroes section's inline comment "World+0x48 = 0 <- the
  stamp Update subtracts" contradicts this page's Conclusion (the subtracted stamp is the SEEDED
  primary `+0x4C` via `this=X+4`; `X+0x48` is the ACCUMULATOR). The conclusion and the seed table
  are byte-true; read the inline note as the pre-correction wording it is.
