# SEH-era command triage: GetGlobal / SetGlobal (law #6, R-USERRE-A seat)

Date: 2026-10-22 (~07:0x) — author: SwiftOwl `01a16198-6cbb` — dispatched by QuickNova
as the law-#6 re-triage (`fe15ac1` relay): "those 'SEH-era disasm-only' natives are
very likely the same entry-vs-body pattern — check for stack-probe heads and census
the THUNK before accepting a disasm-only verdict."

Verdict up front: **BOTH are real bodies, not thunks.** The decompile failures are
the IDA SEH handler limitation on in-function `_CorExitProcess`-era frame setups, not
split-prologue indirection. Bodies then read COMPLETE by disasm (168 + 137
instructions, cursor done both) — the disasm-only tier was the right tool here, but
the TRIAGE distinguishes why: the law-#6 signature (a 4-instruction thunk head like
CConsoleInterface::virt_meth_0x59ec10's `sub ecx,[ecx-4] / jmp body`) is ABSENT at
both EAs; both open with the classic 5-instruction in-function SEH4 head
(`push -1 / push <fn>_SEH / mov eax,fs:0 / push eax / mov fs:0,esp`) — the same head
pattern that made the R-HEADERSAN era name the 'SEH4 frame' class.

## F3::CommandGetGlobal @ 0x58DFD0 (168 insns, fully read)

| step | EA | shape |
|------|------|-------|
| SEH head | 0x58dfd0-0x58dfde | push -1 / push F3__CommandGetGlobal_SEH / fs:0 install |
| argc guard | 0x58dfec-0x58e002 | cells = ((*(argv+8) - *(argv+4)) >> 2); require !=0 and ==2 |
| usage echo | 0x58e004-0x58e032 | "Usage: GetGlobal <variable name>" via sub_59EAF0([client+0x88]) — the SAME console-object slot the slice-1 EchoClient names (print sink), reached [[0x70BD90]+0x2C]+0x88 |
| name sink | 0x58e033-0x58e092 | two cls_0x50db20 temps built (SSO init pattern 0Fh/cap, strlen loop) — argv[1] and a zero literal |
| GAME lookup | 0x58e097-0x58e0b7 | obj = F3::global_cls_0x616328_0x70BD98; if null -> null-self; else ecx = [[obj+8]+4] + obj + 8 (thunk-adjusted sub-object); call [ecx-vft + 0x6C] (slot 27) with the name sink -> result handle |
| registered? | 0x58e0ba-0x58e0c3 | sub_5A9D50(handle) -> byte; 0 = not registered |
| miss echo | 0x58e0c5-0x58e0fa | "Global variable <%s> not registered." (argv[1] through %s) via the same [client+0x88] sink, then RETURN |
| value extract | 0x58e0fb-0x58e178 | sink + name + argv[1] appended (meth_0x401d20 = char append era); same slot-27 call again; cls_0x50db20::meth_0x5A9E60(handle) -> int value |
| hit echo | 0x58e178-0x58e1ae | "<%s> = %d" (argv[1], value) — third format through [client+0x88] |
| SEH tails | 0x639da0+ | destructor arms (string dtors) + __CxxFrameHandler stru_6EB350 — the decompiler's failure input |

## F3::CommandSetGlobal @ 0x58E1B0 (137 insns, fully read)

| step | EA | shape |
|------|------|-------|
| SEH head | 0x58e1b0-0x58e1be | same in-function SEH4 |
| argc guard | 0x58e1ce-0x58e1e2 | require cells == 3 (`Usage: SetGlobal <variable name> <value...` string @0x58e1f2) |
| usage echo | 0x58e1e4-0x58e212 | [client+0x88] sink (identical family) |
| sinks | 0x58e213-0x58e272 | name + value strings built (SSO init twice, meth_0x401bd0 assign x2) |
| GAME lookup | 0x58e277-0x58e29b | IDENTICAL slot-27 path via 0x70BD98 |
| registered? | 0x58e29d-0x58e2a4 | sub_5A9D50 — **warning-only**: al==0 prints "Global variable <%s> not registered. It..." @0x58e2bc and FALLS THROUGH @0x58e2ca into the set path: SetGlobal CREATES unregistered globals (the message's second sentence is cut in the IDB string table at "It" — full text = data-region read, open note, NOT guessed) |
| set | 0x58e2ca-0x58e31e | value = j_atol(argv[2]); name sink + value string; **GameClient::meth_0x58A9A0(client, sink...)** — the actual create/set sink (client-era, body unread — named) |
| SEH tails | 0x639dc0+ | dtor arms + frame handler stru_6EB374 |

## Law-#6 receipts banked

1. entry-vs-body census FIRST for any decompile-fail: SEH4 in-function head (5 pushes)
   vs split thunk (4-byte `sub ecx,[ecx-4]; jmp`) vs stack-probe head — all three
   shapes now have real specimens on record (probe family: the item-62 cmkr notes;
   thunk: 0x59ec10; SEH4: both here).
2. The decompiler fails on the SEH4 + sink-heavy pattern, not on size alone
   (comparable-size SEH-free bodies decompile fine).

## WAVE-4 manifest feed (IDB write-side candidates from this read)

- `F3::global_cls_0x616328_0x70bd98` role CONFIRMED beyond the pump's timer use: it
  is the GLOBAL-VARIABLE TABLE OWNER (the same object the pump ticks at state==3 via
  vft 0/+20/+24 also answers slot-27 name lookups returning value handles).
  Rename candidate: the cls_0x616328 family -> a game-state/globals owner name,
  live-vs-plan re-cut required (one object, two documented roles = check for a
  shared-adjacency mislabel before minting, item-54 style).
- `sub_5A9D50` -> predicate shape: is-registered (returns byte, consumed by cmp al,0)
  — annotation candidate `[out] bool IsRegistered(handle)`.
- `cls_0x50db20::meth_0x5A9E60` -> value-to-int extractor on the handle family —
  annotate as toInt (sibling of the toBool mislabel row already in the manifest —
  same class, same era).
- slot +0x6C (vft slot 27) on the globals sub-object: find-by-name returning handle —
  census siblings (Set uses the same slot; RequestNewGame-era fields now on the
  GameServer: +0x70A8 map-name string, +0x70C4 flag — layout feed rows).
- OPEN (not guessed): the SetGlobal message tail beyond "It" (data-region string
  read), GameClient::meth_0x58A9A0 body (set/create path — likely the globals-table
  insert; one disasm block when the wave touches this table), and the port status of
  both commands stays GATED (needs the game object at 0x70BD98 + [client+0x88] echo
  object — neither constructed in-port; bodies now fully read so the port is
  comments-to-code when those objects land, same posture as the 7c factories).
