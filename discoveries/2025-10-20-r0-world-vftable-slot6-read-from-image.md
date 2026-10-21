# R0 answered from the shipped image, not a debugger — World vftable slot +24 is `0x404b70`, not `Update_0x544a90`

**Coordinator, 2026-10-21. Method:** with `image base 0x400000` and no ASLR, a vftable is just data in `.rdata`.
Mapping `0x691a1c → raw 0x291a1c` from the PE section table and reading 20 dwords answers the structural half of
T-034's R0 **with no attach at all**.

## Validation first
Fourteen slots reproduced the database **exactly** (`0:0x548cf0`, `1:0x546690`, `2:0x5455b0`, `4:0x5460d0`,
`5:0x546b50`, `7:0x545250` … `17:0x545ff0`) — that agreement is what makes the four *new* values trustworthy rather than
an offset bug, the same self-test discipline `tools/shipped-literals.txt` applies to strings.

| slot | image value | DB | note |
|---|---|---|---|
| **3** | **`0x404b70`** | hole | shared stub |
| **6** | **`0x404b70`** | hole | **the `+24` the pump calls** |
| 18 | `0x5a9320` | hole | duplicated |
| 19 | `0x5a9320` | hole | duplicated |

## Consequence: `docs/f3/WorldTickPump.md`'s one stated assumption is FALSIFIED
That page declared — correctly, and to its credit — that if slot 6 resolved to a different EA, the page was wrong.
**It resolved to a different EA.** `JE::Server::virt_meth_0x552b60` calls `[vft+24]` at `0x552bb5`, and slot 6 holds
`0x404b70`, which the DB attributes as **`DialogueManager::virt_meth_0x404b70`** (also vftable `0x6945cc` slots 0 and 1
— one function shared across class tables, i.e. exactly the "engine no-op" shape the old ledger note described).
**So the tick is not delivered to `World::Update_0x544a90` through that vcall.**

**This also settles the vftable contradiction WildYak reported.** The old `DEFERRED-NOTES` line said World's `vft[5]`
was the no-op `0x404b70`; the DB says slot 5 is `0x546b50` (the year loader). Both were half right — `0x404b70` really
is in this table, at slots **3 and 6**, not 5. No second vft pointer is needed to explain the disagreement.

## Boundary — what this does NOT prove
1. **I read the table, not the register.** The answer holds only if `$ebx` at `0x552bb5` is World's vftptr. Two lanes
   asserted that independently (NiceHawk's handover; RedXenon's pre-resolution: `$esi = world = [0x70BD98]+4`, `$ebx`
   its vftptr), so the premise is well-supported but **still an inference**. One `info registers` at that breakpoint
   converts it to observation — and it is now a read, not a hunt.
2. **The time-base verdict survives.** `+0x48` zeroed at world-entry while `+0x4C`/`+0x54` get the tick, and
   `Update` computing `delta = ms - [this+0x48]`, are unchanged. `clock` stays a valid comparison field; what moves is
   *which call site consumes the tick*.
3. `0x544a90` remains a real `World` virtual method in the DB. Open question: **how it is reached** (T-045).

## Also learned about the environment, the hard way
Cleaning up this session's attach debris, I ran `pkill -f "winedbg --gdb --port 31437"` and then
`pgrep -f "port 31437"` — **and both matched the shell running the command itself, killing my own script twice**
(exit 143). A pattern-based process kill in a tree where the pattern is also an argument you just typed is
self-directed. Correct form: resolve the PID from the socket owner or a recorded launch PID, and exclude `$$`, or
match on the executable name (`pgrep -x winedbg.exe`) rather than a command-line substring. Related: the gdb listener
is held by **wineserver** (291356), not by the `winedbg` process, so "who owns the port" and "who is the stub" are
different PIDs — the exact confusion that made this morning's attaches look like prefix corruption.
