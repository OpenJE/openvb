---
status: done
assignee: WildYak (01a15802) — closed 2026-10-21, statics only (read-only IDB + a raw PE parse of the shipped
# image). Questions 1–3 answered; question 4 was unnecessary — the ctor's four vft stores plus the image settle
# which table the pump loads, with no process attached.
depends: R0 answered from the image (discoveries/2025-10-20-r0-world-vftable-slot6-read-from-image.md)
---

> **1. Which vft is in `$ebx`? The `+4` one, `0x6919e4` — and its slot 6 IS `Update_0x544a90`.**
> `JE::World` carries four vft pointers (`ctor_0x548a80`: `[esi]=0x691a1c` `0x548abb`, `[esi+4]=0x6919e4`
> `0x548ac1`, `[esi+0Ch]=0x6919c0` `0x548ac8`, vbtable write `0x548ae6`), and the pump loads the `+4`:
> `0x552ba4 lea esi,[eax+4]` → `0x552bab mov ebx,[esi]` → `0x552bb3 mov ecx,esi` → `0x552bb5 call [ebx+18h]`.
> `0x6919e4+0x18 = 0x544a90`. `F3::GameWorld`'s copy of the same interface (`0x66856c+0x18`) is also
> `0x544a90`, and `.rdata` holds `0x544a90` in exactly two cells (`0x6919fc`, `0x668584`) — so the live
> concrete class cannot change the answer. R0's `0x404b70` is `+0x18` of the **primary** `0x691a1c`: both
> reads are right about their own table.
> **2. Callers of `0x544a90`:** none direct (`E8` scan finds zero), so it is virtual-only; and the
> "adjacent cluster" is one interface's slots, not a dispatcher — `+0x10 0x544a60` setter, `+0x14 0x544a70`
> the gate `Update` itself calls at `0x544ad0` via `[edx+14h]` (the callee dispatching on the table it entered
> through is the independent confirmation of the base), `+0x18 Update`, `+0x1C 0x545100` factory, `+0x2C
> 0x545930 PackState`.
> **3. `0x404b70`:** raw bytes `c3` — a bare `retn`. It neither stores nor forwards, so it was never a
> candidate consumer of the tick. The pump story needs no extra hop.
>
> **The consequence nobody asked for, and it moves three lanes: the frame-0 leap is OURS.** With the `+4`
> base, the field `Update` subtracts is primary **`+0x4C`**, which `0x5466bd` **seeds** from the tick; only
> the accumulator `+0x48` is zeroed (`0x5466b5`). Independently, `JE_TickMilliseconds_0x56af50` is
> `a1 b8c270 00 c3` = `mov eax,[0x70c2b8]; ret` — **not** `timeGetTime()`. `0x70c2b8` has exactly one writer,
> `F3::ProcessMessagesAndUpdateTime` (`0x56afcf..0x56afd7`, `elapsed += now - [0x70c2bc]`), `0x70c2bc` is
> seeded by `WinMain` at `0x56b8aa`, and both globals sit past `.data`'s file-backed end (`vsize 0x200e0` vs
> `rawsize 0x17000`) so `elapsed` starts at 0. **The shipped tick is session-relative elapsed.** So
> `clock=29242` at `frame=0` is produced by our `src/F3.cpp:786` returning raw `timeGetTime()` under
> `0x56AF50`'s label, twice over — and this row's predecessor page, which said "mirror the leap, do not fix
> it", is reversed by it. Full write-up: `docs/f3/WorldTickPump.md`.
>
> **Flagged, not edited** (other lanes' files): `ParityOracleDesign.md:47`, `SimDigest.md:173-180`,
> `OrigCapture.md:63` all assert `JE_TickMilliseconds == timeGetTime() == machine uptime`, and `tickMs`'s
> CONTEXT parking rests on it. `SimDigest` is ZenRaven's. T-038's own conclusion survives (the year still
> cannot reach `totalSeconds`), but its *rationale* sentence about the leap was corrected in the same commit.

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
