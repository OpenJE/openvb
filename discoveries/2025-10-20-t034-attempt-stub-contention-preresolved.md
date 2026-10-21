# 2025-10-20 — T-034 attempt report (redxenon 01a157ec): stub contention; everything resolved except the attach

> **SUPERSEDED-IN-PART by NiceHawk's handover (b77625e, `…t034-handover-orig-gdb-port-collision.md`):**
> attach itself works once the ImHex collision is respected (`set architecture i386:intel`, store-path
> winedbg, one-gdb-per-stub lifecycle); R0's read is a REGISTER at 0x552bb5 ($ebx = World vftptr →
> `x/1wx $ebx+0x18`); main-menu demo NEVER reaches 0x552bb5 (esi-null at 0x552bab) — menu-phase
> "never fired" is by design, not attach failure; R3/R4 mid-combat is gated by MISSING INPUT
> TOOLS (xdotool/wmctrl absent from the shell), making R3 a candidate for unreachable-by-tooling
> with D-2 settled from T-037 + own-seam stress instead. All merged into tools/t034-capture.py.

Status: **R0–R4 NOT READ — reported as misses per the row's own rule.** All statics are pre-resolved
so the read session is one attach away; the blocker is debugger ownership, not knowledge.

## What blocked it (the attempt, honestly)
- `127.0.0.1:31337` accepts TCP but every gdb handshake returned "Remote connection closed";
  `ss -ltnp` shows the listener owned by an **ImHex process (pid 12337)** — a live debugger client
  (or bridge) already holds the single-client winedbg stub. Evicting another lane's session is not
  mine to do.
- Fresh private stubs (`winedbg --gdb --port 31438/31440/31441 --no-start F3.exe`, own Xvfb :99):
  first died when my script sent `run` (unsupported by RSP — the correct start is `continue`, learned);
  later ones accepted TCP but never completed the RSP handshake inside the harness timeout —
  most likely wineprefix contention (`f3demo-wineprefix` already has NiceHawk's orphaned 31437 stub
  plus another lane's running instance 13035). A `/dev/tcp` pre-probe ALSO eats winedbg's single
  client slot — do not probe before gdb, attach first.

## Tooling landed with the attempt (resumable by anyone)
`tools/t034-capture.py` — gdb-Python batch runner: connects, sets the three spec breakpoints + a
funnel-count probe, STARTS WITH `continue` (never `run`), polls `[0x707C60]`, `[0x70BD98+0x3C]` →
clock`+0x34`, `[World vft]+24`, counts `0x550720` hits, prints a RESULT line. Port via argv.
Prereq (any lane can satisfy): a free `winedbg --gdb --port N --no-start F3.exe` under the demo
prefix with no other client, xvfb present, then
`gdb -nx -batch -ex "py ..." tools/t034-capture.py <N>`.

## All read targets, pre-resolved (this is the useful half of the sitting)
| # | exact read | derivation |
|---|---|---|
| R0 | `x/xw [ *(0x70BD98) + 24 ]` at first hit of `0x552bb5` | World object embedded AT the global (decompile pattern `global + *(global[2]…)`), so vft = `[0x70BD98]`; expect 0x544a90 per WorldTickPump assumption |
| R1 | `x/xw 0x707C60` at `0x48ccf6` (first return) | **NEW: EA pinned from raw bytes** — 0x48cce9 is `A3 60 7C 70 00` = `mov [0x707C60],eax` (the store; 0x48ccde = imul 0x19660d / add 0x6EF35F, the LCG consts). `byte_707C64` (adjacent) = init flag: seed comes from `time32(NULL)`+`SetStartupTime` on FIRST call → **the pinned state is process-start-epoch dependent; capture must record wall time too** |
| R2 | clock = `*(0x70BD98+0x3C)`; seconds = `[clock+0x34]` | openje `Gregorian.hpp:24 totalSeconds +0x34` (mirror); read at same instant as R1 and at frame 1 |
| R3 | `Entity+0x304` deque: blocks`[+0x308]`, cap`[+0x30C]`, 4`[+0x310]`, **index`[+0x314]`** at `0x550720` entry, mid-round | T-030 docs §container (`cls_0x54e070` mbr_0x4/8/c/10) |
| R4/R4b | `[action+0x14]` before/after the call; `[action]` = vft per slot | WildYak: `0x550737` sole writer → pre-value is the datum (heap-reuse vs zero) |

## Flag for whoever reads R3 (potential finding before the debugger runs)
The deque **element index at Entity+0x314** is the same dword our own digest line prints as
`busy0x314` (SpawnedEntity field, sim digest `st=` third value). If the shipped index lives at
+0x314, then "busy" in our model and "queue depth" in the original are **the same dword** — mid-round
R3 read settles it: an index >0 while our `busy0x314` is boolean-ish would mean our port reads a
counter as a flag. Not a conclusion — a named question R3 answers for free.

## Boundary (row's discipline kept)
No values were sampled; nothing here is a guess dressed as a read. A single-sample caveat still
applies once R3 runs (capacity-at-instant ≠ capacity). ImHex holder: if you detach, say a word and
this lane reruns `tools/t034-capture.py <port>` in one command.
