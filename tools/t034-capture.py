#!/usr/bin/env python3
# T-034 capture harness (redxenon 01a157ec). gdb -nx -batch -x tools/t034-capture.py --args? No:
#   gdb -nx -batch -ex "python PORT=31441" -x tools/t034-capture.py
# or edit PORT below. Targets a winedbg --gdb --no-start stub of the ORIGINAL F3.exe.
#
# Hard-won invocation rules (2026-10-21 sitting, see the attempt report):
#   * winedbg stubs are SINGLE-CLIENT: never TCP-probe before gdb — the probe eats the slot.
#   * RSP has no 'run': the first command to start a --no-start inferior is 'continue'.
#   * 31337 may be held by another debugger (ImHex/IDA); pick a free port and pass it here.
import gdb, time, sys

PORT = int(sys.argv[-1]) if len(sys.argv) > 1 and sys.argv[-1].isdigit() else 31441
DEADLINE_S = 420  # boots are slow under headless wine; R0 needs the first pump

gdb.execute("set pagination off"); gdb.execute("set confirm off")
gdb.execute(f"target remote 127.0.0.1:{PORT}")

def rd(addr, n=4):
    try:
        return int.from_bytes(bytes(gdb.selected_inferior().read_memory(addr, n)), "little")
    except Exception:
        return None

res = {"r1": [], "r2": [], "r0": [], "funnel": 0, "r34": []}
bp1, bp0, bpf = (gdb.Breakpoint(a, internal=True) for a in
                 ("*0x48ccf6", "*0x552bb5", "*0x550720"))
bpfun = gdb.Breakpoint("*0x459f5d", internal=True)  # F3::Attack native -> funnel call site

START_TIME = 0x707C60   # pinned from bytes at 0x48cce9 (A3 60 7C 70 00)
WORLD = 0x70BD98        # embedded GameWorld object

gdb.execute("continue")  # --no-start: first continue spawns
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
        vft = rd(WORLD); tgt = rd(vft + 24) if vft else None
        res["r0"].append((hex(vft) if vft else None, hex(tgt) if tgt else None))
        if res["r0"]: break
    elif pc == 0x550720:
        res["funnel"] += 1
        ent = int(gdb.parse_and_eval("$ecx")) & 0xFFFFFFFF
        idx = rd(ent + 0x314); cap = rd(ent + 0x30C); blocks = rd(ent + 0x308)
        act = int(gdb.parse_and_eval("$eax")) & 0xFFFFFFFF if res.get("a") else None
        if len(res["r34"]) < 12:
            res["r34"].append((hex(ent), idx, cap, blocks))
    elif time.time() - t0 > DEADLINE_S * 0.6 and res["funnel"]:
        break

print("RESULT", res)
try: gdb.execute("detach")
except Exception: pass
