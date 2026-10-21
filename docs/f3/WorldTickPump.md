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

## Conclusion: the jump is shipped behaviour — mirror it, do not "fix" it

`Update` (0x544a90, our `World::Update_0x544a90`) computes `delta = ms - [this+0x48]` and only then
stores `ms` into `+0x48`. On the first call after entry that is `uptime - 0`, folded into the accumulator
`+0x44`, and the step-6 carry (`+0x44 / 1000`) adds it to `Gregorian::AddSeconds` as world seconds. The
shipped code initialises `+0x48` to **zero** while seeding `+0x4C` and `+0x54` with the tick, so it takes
the same leap our port takes. **Our behaviour is faithful here**, and the mirror rule applies: no
"initialise the stamp to the first tick" change, because that would silently improve the original.

Two consequences that matter beyond this row:

1. **`clock` is a valid comparison field for the parity oracle** — both sides start from the same leap, so
   a divergence in `clock` means a divergence in *elapsed* sim time, not in an epoch mismatch.
2. The seeded stamps `+0x4C` and `+0x54` are what the *other* tick consumers read (0x546690 seeds both, and
   `+0x54` is the field our `Update` subtracts as pause credit). Why `+0x48` alone is left at 0 is still
   open — it is either deliberate (the first delta is meant to be the uptime, because the world clock is
   calendar-based and `AddSeconds` is later clamped/ignored) or a shipped quirk. Not a defect claim: a
   question with a location.

## What still needs the debugger (the remaining T-031 reads)

Written as address / value / when, for whoever runs the window — see
`docs/f3/ParityOracleDesign.md` §Reads. Read (a) from the coordinator's list — what `ms` the pump passes —
is **closed by this page** and should not be spent on.
