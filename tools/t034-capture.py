#!/usr/bin/env python3
# T-034 capture harness (redxenon 01a157ec). gdb -nx -batch -x tools/t034-capture.py --args? No:
#   gdb -nx -batch -ex "python PORT=31441" -x tools/t034-capture.py
# or edit PORT below. Targets a winedbg --gdb --no-start stub of the ORIGINAL F3.exe.
#
# Hard-won invocation rules (2026-10-21 sitting, see the attempt report):
#   * winedbg stubs are SINGLE-CLIENT: never TCP-probe before gdb — the probe eats the slot.
#   * RSP has no 'run': the first command to start a --no-start inferior is 'continue'.
#   * The managed original stub listens on $OPENVB_ORIG_GDB_PORT = **31437** (it moved off 31337,
#     which ImHex's Network Interface hardcodes and will always squat - harness item 39). A manual
#     `winedbg --gdb --port N` stub may still be on another number; pass the port as argv.
#   * Never TCP-probe the stub before gdb: it is single-client and the probe eats the slot.
import gdb, time, sys

PORT = int(sys.argv[-1]) if len(sys.argv) > 1 and sys.argv[-1].isdigit() else 31441
DEADLINE_S = 420  # boots are slow under headless wine; R0 needs the first pump

import gdb, time, sys

# 2026-05 handover merge (NiceHawk b77625e, adopted by row holder RedXenon):
#  * `set architecture i386:intel` REQUIRED before target remote (winedbg 32-bit).
#  * Image base 0x400000, no ASLR => IDA EA == target VA (no slideware).
#  * At 0x552bb5: $ebx IS the World vftptr already -> R0 = `x/1wx $ebx+0x18`;
#    $esi = [0x70BD98]+4, $eax = tickMs. Keep the memory-read fallback below.
#  * FALSE NEGATIVE BY DESIGN: 0x552bab derefs [esi] unconditionally; esi is NULL when
#    [0x70BD98]==0 -> a demo parked at the MAINMENU hits 0x48ccf6 (R1) but NEVER 0x552bb5
#    (R0). "never fired" != "attach failed". Same for R3: menu has no combat.
#  * R1 reseed gate: [0x707C64] byte; first-call seed = time32 via SetStartupTime.
#  * One gdb per stub: when gdb EXITS the listener dies but F3.exe keeps running free.
#    Kill leftovers (match /proc/<pid>/environ WINEPREFIX to spare reimpl runs) before attach.
#  * R3/R4 mid-combat needs INPUT the shell lacks (xdotool/wmctrl absent). If unreachable,
#    settle D-2 from T-037's record map + own-seam stress test; report R3 as
#    unreachable-by-tooling, not unread.
gdb.execute("set pagination off"); gdb.execute("set confirm off")
try: gdb.execute("set architecture i386:intel")
except gdb.error: pass          # already-32-bit target rejects the re-set; harmless
gdb.execute(f"target remote 127.0.0.1:{PORT}")
try:                              # stub may be RUNNING-FREE from a previous attach
    gdb.execute("interrupt")
    time.sleep(1.5)
except gdb.error: pass

def rd(addr, n=4):
    try:
        return int.from_bytes(bytes(gdb.selected_inferior().read_memory(addr, n)), "little")
    except Exception:
        return None

res = {"r1": [], "r2": [], "r0": [], "funnel": 0, "r34": []}  # r34 rows: (ent, COUNT@314, head@310, cap, blocks)
bp1, bp0, bpf = (gdb.Breakpoint(a, internal=True) for a in
                 ("*0x48ccf6", "*0x552bb5", "*0x550720"))
bpfun = gdb.Breakpoint("*0x459f5d", internal=True)  # F3::Attack native -> funnel call site

START_TIME = 0x707C60   # pinned from bytes at 0x48cce9 (A3 60 7C 70 00)
WORLD = 0x70BD98        # embedded GameWorld object

try:
    gdb.execute("continue")  # --no-start: first continue spawns
except gdb.error:
    pass  # already running: breakpoints below still arm
t0 = time.time()
while time.time() - t0 < DEADLINE_S:
    try:
        gdb.execute("continue", from_tty=False)
    except gdb.error as e:
        print("CONT-DONE", e); break
    try:
        pc = int(gdb.parse_and_eval("$pc"))
    except Exception:
        break
    if pc == 0x48ccf6:
        st, eax = rd(START_TIME), int(gdb.parse_and_eval("$eax")) & 0xffffffff
        clk = rd(WORLD + 0x3C)
        ts = rd(clk + 0x34) if clk and 0x10000 < clk < 0x7FFFFFFF else None
        if len(res["r1"]) < 4: res["r1"].append((st, eax))
        if len(res["r2"]) < 4: res["r2"].append((hex(clk) if clk else None, ts))
    elif pc == 0x552bb5:
        try:
            ebx = int(gdb.parse_and_eval("$ebx")) & 0xFFFFFFFF
        except Exception:
            ebx = None
        vft = ebx if ebx and ebx > 0x400000 else rd(WORLD)
        tgt = rd(vft + 24) if vft else None
        res["r0"].append((hex(vft) if vft else None, hex(tgt) if tgt else None))
        if res["r0"]: break
    elif pc == 0x550720:
        res["funnel"] += 1
        # 9bbdc9e correction: +0x314 is the LIVE COUNT (empty-test 0x4084c0, push ++ at
        # 0x54ea71, pump bound head!=head+count at 0x54c447) — +0x310 is HEAD INDEX.
        ent = int(gdb.parse_and_eval("$ecx")) & 0xFFFFFFFF
        count = rd(ent + 0x314); head = rd(ent + 0x310)
        cap = rd(ent + 0x30C); blocks = rd(ent + 0x308)
        if len(res["r34"]) < 12:
            res["r34"].append((hex(ent), count, head, cap, blocks))
    elif time.time() - t0 > DEADLINE_S * 0.6 and res["funnel"]:
        break

print("RESULT", res)
try: gdb.execute("detach")
except Exception: pass
