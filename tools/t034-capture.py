#!/usr/bin/env python3
# T-034/T-052 live-capture harness v4 (HappyEagle, T-052 retry sitting).
# SUPERSEDES, in this same file:
#   * the previous tools/t034-capture.py (v2, RedXenon/NiceHawk --no-start shape), and
#   * /tmp/t034-run.py (v3 working harness, NiceHawk 01a15cb7) — re-landed here per the
#     T-052 acceptance item "harness v3 gets re-landed under tools/".
# RedXenon's CORRECTIONS ARE KEPT VERBATIM IN SPIRIT: Entity+0x314 is the LIVE COUNT and
# +0x310 is the HEAD INDEX (9bbdc9e / HappyKnight 633744d) — never sample +0x314 as an offset.
#
# THE v4 DELTA (T-052 outcome, superseding this file's own v4-first-draft theory):
# `winedbg --gdb --port N F3.exe` WITHOUT --no-start does NOT leave a plain LISTEN stub —
# it spawns the inferior AND a frontend gdb (cmd `gdb -x <prefix>/temp/gdbXXXX.tmp`) already
# connected and parked at the ntdll startup stop. Keep that gdb's stdin open (held fifo
# writer) and drive it via tools/t052-drive.py: the dance is N resumes (one per wine
# thread-start DbgUiRemoteBreakin SIGTRAP), not two. The --no-start + python-remote shape
# below still works for the MANAGED stub, but "never fired" judgments from it are void
# until the same N-resume dance is consumed (T-034's miss was this, not the port).
# After attach: continue -> expect ntdll startup stop -> continue again -> THEN arm
# breakpoints and judge "never fired". (v2 shape, kept for the managed --no-start stub.)
#
# Hard-won invocation rules (still true, carried from v2/v3):
#   * winedbg stubs are SINGLE-CLIENT: never TCP-probe before gdb — the probe eats the slot.
#   * One gdb client per stub. When gdb EXITS the listener dies but F3.exe keeps running free.
#     Kill leftovers ONLY with a /proc/<pid>/environ WINEPREFIX gate (+ own port in cmdline);
#     comm-grep (`pgrep -x F3.exe`) is NEVER an owner test (T-034 incident 320383).
#   * ImHex hardcodes 127.0.0.1:31337 — never evict it. The managed original stub is 31437
#     ($OPENVB_ORIG_GDB_PORT; read the env, do not quote memory); reimpl stub is 31338. This
#     harness defaults to scratch port 31441 for its own manual stub.
#   * `set architecture i386:intel` REQUIRED before target remote (winedbg 32-bit); harmless
#     try/except when the target is already reported 32-bit.
#   * Image base 0x400000, no ASLR => IDA EA == target VA. Verify MZ at 0x400000 after attach.
#   * Async is dead on this pair: `continue &` is accepted but `interrupt` no-ops and there is
#     no `wait`/`stepi`. Plain synchronous `continue` is the ONLY reliable resume (v3 cycles).
#   * FALSE NEGATIVE BY DESIGN: at the MAIN MENU the world pump never runs
#     ([0x70BD98]==0 => $esi NULL at 0x552bab), so R0 "never fired" at menu != attach failure;
#     R1 hits regardless. R3/R4/R4b need a live entity — drive movie->menu->New Game (xdotool,
#     windowactivate --sync on the real window id) while this loop runs.
#   * R1 reseed gate byte [0x707C64]; first-call seed = time32 via SetStartupTime; the drawn
#     value is in $eax at 0x48ccf6 (ret of sub_48CCC0).
import gdb, time, os, json, sys

PORT = int(os.environ.get("T052_PORT",
           sys.argv[-1] if len(sys.argv) > 1 and sys.argv[-1].isdigit() else 31441))
DEADLINE_S = int(os.environ.get("T052_DEADLINE", "1500"))
LOGPATH = os.environ.get("T052_LOG", "/tmp/t052-run.log")
LOG = open(LOGPATH, "w", buffering=1)
def log(*a):
    LOG.write("[%7.1fs] %s\n" % (time.time() - T0, " ".join(str(x) for x in a)))

gdb.execute("set pagination off"); gdb.execute("set confirm off")
try: gdb.execute("set architecture i386:intel")
except gdb.error: pass          # already-32-bit target rejects the re-set; harmless
gdb.execute(f"target remote 127.0.0.1:{PORT}")
T0 = time.time()

def rd(addr, n=4):
    try:
        return int.from_bytes(bytes(gdb.selected_inferior().read_memory(addr, n)), "little")
    except Exception:
        return None
def reg(r):
    try: return int(gdb.parse_and_eval("$" + r)) & 0xFFFFFFFF
    except Exception: return 0

log("ATTACHED pc=%08x utc=%s port=%d" % (reg("pc"), time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()), PORT))
mz = rd(0x400000, 2)
log("MZ@0x400000 = %s (IDA EA == target VA confirmed: %s)" %
    (mz, "YES" if mz == 0x5A4D else "NO - WRONG TARGET?"))

START_TIME, RESEED, WORLD = 0x707C60, 0x707C64, 0x70BD98
TICK_ELAPSED = 0x70C2B8
res = {"r0": [], "r1": [], "r2": [], "funnel": 0, "funnel_post": 0, "r34": [],
       "attack_site": [], "dance": [], "stops_nonimage": 0}

def clock_sample(tag):
    srv = rd(WORLD); w = (srv + 4) if srv else 0
    clk = rd(w + 0x3C) if w else None
    secs = rd(clk + 0x34) if clk and 0x10000 < clk < 0x7FFFFFFF else None
    log("R2[%s] server=%08x world=%08x [w+0x3C]=%s [clk+0x34]=%s" %
        (tag, srv or 0, w or 0, "None" if not clk else "%08x" % clk,
         "None" if secs is None else "%08x" % secs))
    if len(res["r2"]) < 8: res["r2"].append((tag, srv, w, clk, secs))

def deque_walk(ent):
    # +0x304 anchor / +0x308 block-array ptr / +0x30C block capacity /
    # +0x310 HEAD INDEX / +0x314 LIVE COUNT (RedXenon 9bbdc9e — do not regress).
    blocks = rd(ent + 0x308); cap = rd(ent + 0x30C)
    head = rd(ent + 0x310); count = rd(ent + 0x314)
    slots = []
    if blocks and cap and 0x10000 < blocks < 0x7FFFFFFF and count is not None and head is not None:
        for k in range(min(count, 24)):
            idx = head + k
            bi = (idx >> 2) % cap
            blk = rd(blocks + 4 * bi)
            if not blk or not (0x10000 < blk < 0x7FFFFFFF):
                slots.append((idx, None, None)); continue
            ptr = rd(blk + 4 * (idx & 3))
            vft = rd(ptr) if ptr and 0x10000 < ptr < 0x7FFFFFFF else None
            slots.append((idx, ptr, vft))
    return blocks, cap, head, count, slots

bps = {a: gdb.Breakpoint("*0x%x" % a, internal=False) for a in
       (0x48CCF6, 0x552BB5, 0x550720, 0x550741, 0x459F5D, 0x56AF50)}
tick_hits = [0]

def handle(pc):
    if pc == 0x56AF50:   # JE_TickMilliseconds getter = main-loop heartbeat probe
        tick_hits[0] += 1
        el = rd(TICK_ELAPSED)
        log("TICK#%d elapsed[0x70C2B8]=%s" % (tick_hits[0], "None" if el is None else "%08x" % el))
        if tick_hits[0] >= 8:
            bps[0x56AF50].enabled = False
            log("TICK probe done, bp disabled")
    elif pc == 0x48CCF6:
        if len(res["r1"]) < 6:
            st, eax, flag = rd(START_TIME), reg("eax"), rd(RESEED, 1)
            log("R1 STATE=%08x DRAW=%x flag=%s utc=%s" % (st or 0, eax, flag,
                time.strftime("%H:%M:%S", time.gmtime())))
            res["r1"].append((st, eax, flag))
            if len(res["r1"]) == 1: clock_sample("at-R1")
            if len(res["r1"]) == 6:
                bps[0x48CCF6].enabled = False
                log("R1 done x6, bp disabled")
    elif pc == 0x552BB5:
        if len(res["r0"]) < 3:
            vft, w, tick = reg("ebx"), reg("esi"), reg("eax")
            tgt = rd(vft + 0x18) if vft and 0x400000 < vft < 0x80000000 else None
            log("R0 world=%08x vft=%08x [vft+0x18]=%s tick=%08x  (T-045 static expects tgt=544a90)" %
                (w, vft, "None" if not tgt else "%08x" % tgt, tick))
            if tgt and tgt != 0x544A90:
                log("!!! R0 LOUD-FLAG: live [vft+0x18]=%08x DISAGREES with T-045 static 0x544a90" % tgt)
            res["r0"].append((w, vft, tgt, tick))
            clock_sample("at-R0-%d" % len(res["r0"]))
            if len(res["r0"]) >= 2:
                bps[0x552BB5].enabled = False
                log("R0 done, bp disabled")
    elif pc == 0x459F5D:
        ent, action = reg("ecx"), reg("esi")
        ok = action and 0x10000 < action < 0x7FFFFFFF
        pre14 = rd(action + 0x14) if ok else None
        avft = rd(action) if ok else None
        log("ATTACK-SITE ent=%08x action=%08x vft=%08x [act+0x14]_pre=%s" %
            (ent, action or 0, avft or 0, "None" if pre14 is None else "%08x" % pre14))
        if len(res["attack_site"]) < 12: res["attack_site"].append((ent, action, avft, pre14))
    elif pc == 0x550720:
        res["funnel"] += 1
        ent = reg("ecx"); action = rd(reg("esp") + 4)
        ok = action and 0x10000 < action < 0x7FFFFFFF
        pre14 = rd(action + 0x14) if ok else None
        avft = rd(action) if ok else None
        blocks, cap, head, count, slots = deque_walk(ent)
        log("FUNNEL#%d ent=%08x action=%08x vft=%08x [act+0x14]_pre=%s | "
            "blocks=%08x cap=%s head=%s COUNT=%s" %
            (res["funnel"], ent, action or 0, avft or 0,
             "None" if pre14 is None else "%08x" % pre14, blocks or 0, cap, head, count))
        if slots:
            log("  slots: " + " ".join(("%d:p=%08x:vt=%08x" % s) if s[1] else "%d:-" % s[0] for s in slots))
        if len(res["r34"]) < 16:
            res["r34"].append((ent, action, avft, pre14, blocks, cap, head, count, slots))
        if res["funnel"] >= 120:
            bps[0x550720].enabled = False
            log("FUNNEL entry bp off at 120 hits")
    elif pc == 0x550741:
        res["funnel_post"] += 1
        ent = reg("esi")
        action = rd(reg("esp") + 8)
        ok = action and 0x10000 < action < 0x7FFFFFFF
        post14 = rd(action + 0x14) if ok else None
        log("FUNNEL-POST#%d ent=%08x action=%08x [act+0x14]_post=%s match=%s" %
            (res["funnel_post"], ent, action or 0, "None" if post14 is None else "%08x" % post14,
             post14 == ent))

def finished():
    return len(res["r0"]) >= 2 and res["funnel"] >= 8 and len(res["attack_site"]) >= 4 and len(res["r1"]) >= 6

# ---------- THE DANCE (T-052 core) ----------
# continue #1: WITHOUT --no-start the stub has already spawned F3.exe; gdb's first resume
# lands on (or passes through) wine's startup DbgBreakPoint. Log both stops explicitly.
try:
    gdb.execute("continue")
except gdb.error as e:
    log("DANCE continue#1 error:", e)
pc1 = reg("pc")
res["dance"].append(("stop1", hex(pc1)))
log("DANCE stop#1 pc=%08x  %s" % (pc1, "ntdll startup stop" if pc1 > 0x70000000 else "in-image or other"))
if 0x400000 <= pc1 < 0x700000:
    log("DANCE note: stop#1 already inside game image — startup stop consumed before attach; arming now")

# breakpoints are ARMED here (all six), while parked, before the consuming resume.
for a in sorted(bps):
    bps[a].enabled = True

try:
    gdb.execute("continue")
except gdb.error as e:
    log("DANCE continue#2 error:", e)
pc2 = reg("pc")
res["dance"].append(("stop2", hex(pc2)))
log("DANCE stop#2 pc=%08x" % pc2)

# ---------- sampling loop ----------
n = 0
while time.time() - T0 < DEADLINE_S:
    pc = reg("pc")
    if 0x400000 <= pc < 0x700000:
        if pc in bps: handle(pc)
        else:
            if n < 60 or n % 10 == 0:
                try: ins = gdb.execute("x/i $pc", to_string=True).strip().splitlines()[-1]
                except Exception: ins = "?"
                log("STOP#%d pc=%08x %s" % (n, pc, ins[:80]))
    else:
        res["stops_nonimage"] += 1
        if n < 60 or n % 10 == 0:
            log("STOP#%d pc=%08x (outside image)" % (n, pc))
    n += 1
    if finished():
        log("ALL EARLY TARGETS SATISFIED at stop#%d" % n); break
    try:
        gdb.execute("continue", from_tty=False)
    except gdb.error as e:
        if "Interrupt" in str(e) or "interrupt" in str(e):
            log("INTerrupted at stop#%d pc=%08x" % (n, reg("pc"))); continue
        log("CONT-DONE", e); break

log("RESULT " + json.dumps(res, default=str))
try: gdb.execute("detach")
except Exception as e: log("detach:", e)
