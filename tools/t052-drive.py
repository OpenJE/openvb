#!/usr/bin/env python3
# T-052 startup-dance driver (HappyEagle) — RE-LANDED harness v3 successor.
# Supersedes /tmp/t034-run.py (v3) and this sitting's /tmp/t052-drive{,L}.py.
# tools/t034-capture.py documents the TARGET SHAPE; this file is the working CLIENT for it.
#
# WHY THIS EXISTS (T-052 findings, discoveries/2025-10-21-t052-startup-dance-retry.md):
#   `winedbg --gdb --port N F3.exe` WITHOUT --no-start spawns the inferior AND its OWN
#   frontend `gdb` (child of winedbg.exe, cmd `gdb -x <prefix>/.../gdbXXXX.tmp`) connected
#   to the 127.0.0.1:N stub, parked at ntdll process_rtl_work_item (wine startup stop).
#   Keep that frontend's stdin OPEN (fifo with a held writer) or it EOF-quietly `detach`s
#   and the inferior runs debugger-LESS (the T-034 "stub died" family of ghosts).
#   The dance is NOT two continues: every thread wine creates posts a DbgUiRemoteBreakin
#   SIGTRAP that ALL-STOPs the debuggee at the gdb prompt — you must resume once per event
#   until the loader is done. This driver consumes those stops and services the six
#   T-034 read breakpoints while parked at each.
#
# SHAPE OF USE (see the discoveries doc for the full recipe):
#   1. fifo with held writer; winedbg stdin=fifo, e.g.:
#      env WINEPREFIX=<prefix> WINEARCH=win32 WINEDLLOVERRIDES='mscoree,mshtml=' \
#          WINEDEBUG='-all,+err' DISPLAY=<X> GALLIUM_DRIVER=softpipe LIBGL_ALWAYS_SOFTWARE=1 \
#          winedbg --gdb --port 31440 F3.exe <fifo >log 2>&1 &     (cwd = f3demo root)
#   2. echo 'set pagination off' >fifo ; six `break *<ea>` lines >fifo
#   3. python3 tools/t052-drive.py <fifo> <log>   (env T052_DEADLINE=s ; then echo continue >fifo)
#
# BREAKPOINT MAP (ORIGINAL F3.exe, image 0x400000, IDA EA == VA):
#   bp1 0x48ccf6 R1 F3::start_time draw-return  (state dword [0x707C60], draw in $eax,
#        reseed gate byte [0x707C64])
#   bp2 0x552bb5 R0 first world pump ($ebx = World vftptr; [ebx+0x18] = tick target —
#        T-045 static expects 0x544a90; LOUD-FLAG if live disagrees)
#   bp3 0x550720 R3/R4 funnel entry  ($ecx = entity; action = [esp+4];
#        deque: +0x308 blocks / +0x30C cap / +0x310 HEAD INDEX / +0x314 LIVE COUNT
#        — RedXenon 9bbdc9e / HappyKnight 633744d; NEVER sample +0x314 as an offset)
#   bp4 0x550741 R4 post-store ([action+0x14] should == entity; owner back-link)
#   bp5 0x459f5d F3::Attack script-native call site ($ecx ent, $esi action, pre +0x14)
#   bp6 0x56af50 TICK probe (JE_TickMilliseconds heartbeat; elapsed [0x70C2B8])
#   bp7 0x581810 JE::Graphics::Initialize entry (captures $gfx=$ecx)
#   bp8 0x5818d0 the "Cannot find requested mode." pre-fatal stop ($gfx members =
#        requested adapter/mode/w/h/refresh at +0x2c..+0x44; the live Gfx_D3D mode list is
#        at *(int*)0x708380 -> +1072 groups -> +228 count / +232 modes[32B]: w,h,REFRESH,?,FMT)
import sys, time, re, os
FIFO, LOG = sys.argv[1], sys.argv[2]
DEADLINE = time.time() + int(os.environ.get("T052_DEADLINE", "1200"))
OUT = os.environ.get("T052_VALUES", "/tmp/t052-values.log")
pos = int(os.path.getsize(LOG))
vals = open(OUT, "a", buffering=1)

def send(*cmds):
    with open(FIFO, "w") as f:            # held writer keeps the frontend from EOF
        for c in cmds:
            f.write(c + "\n")

def readnew():
    global pos
    n = os.path.getsize(LOG)
    if n <= pos:
        return ""
    with open(LOG, "rb") as f:
        f.seek(pos); chunk = f.read(n - pos).decode("utf-8", "replace")
    pos = n
    return chunk

R1C = ("print/x $eax", "x/1wx 0x707c60", "x/1bx 0x707c64",
       "set $srv=*(int*)0x70bd98", "print/x $srv")   # + R2 chain via R2AT helper
R2AT = ("x/1wx $srv+0x3c", "x/1wx *(int*)($srv+0x3c)+0x34")  # ROW FORMULA: world+0x3C -> Gregorian obj, [Greg+0x34]=totalSeconds (srv=[0x70bd98] IS the world ptr — no +4; A' run garbage-6 lesson)
SEGV = ("bt 18", "info registers eax ebx ecx edx esi edi esp", "x/4wx $esp", "x/4wx $esp+4")
R0C = ("print/x $ebx", "x/1wx $ebx+0x18", "print/x $esi", "print/x $eax",
       "x/1wx 0x70bd98", "x/1wx 0x70c2b8")
R3C = ("print/x $ecx", "x/1wx $esp+4", "x/5wx $ecx+0x304")
R4PRE = ("x/1wx *(unsigned*)($esp+4)", "x/1wx *(unsigned*)($esp+4)+0x14")
R4P = ("print/x $esi", "x/1wx *(unsigned*)($esp+8)", "x/1wx *(unsigned*)($esp+8)+0x14",
       "x/1wx $ecx+0x310", "x/1wx $ecx+0x314",                       # post-insert head / LIVE COUNT
       "set $blk=*(int*)($ecx+0x308)", "x/4wx $blk",                 # slot-array window (decode offline)
       "set $hd=*(int*)($ecx+0x310)", "x/1wx $blk+4*$hd",             # HEAD slot -> action ptr
       "x/1wx *($blk+4*$hd)")                                         # R4b: [action] vftptr
R5C = ("print/x $ecx", "print/x $esi", "x/1wx $esi", "x/1wx $esi+0x14")
GFX1 = ("set $gfx=$ecx", "x/8wx $gfx+0x28", "continue")
GFX2 = ("x/8wx $gfx+0x28", "x/1bx $gfx+0x48", "set $ad=*(int*)0x708380",
        "set $grp=*(int*)($ad+1072)", "x/1wx $grp+228", "set $md=*(int*)($grp+232)",
        "x/8wx $md", "x/8wx $md+32", "continue")

tick = [0]; fun = [0]; r1 = [0]; post = [0]; segv = [0]; pump = [0]
while time.time() < DEADLINE:
    chunk = readnew()
    if not chunk:
        time.sleep(0.4); continue
    vals.write("----CHUNK----\n" + chunk)
    if "received signal SIGSEGV" in chunk:
        # wined3d_cs / bink-path guard (T-056 run A): DIAGNOSE, don't blind-continue past 2.
        segv[0] += 1
        send(*SEGV)
        if segv[0] <= 2: send("continue")
        else: send("continue"); vals.write("SEGV-LOOP-CAP at %d\n" % segv[0]); break
    # gdb prints hits as "Breakpoint N hit ..." or "Breakpoint N, 0x..." (no symbols) —
    # match BOTH or you silently mis-service stops (T-052 session J lesson).
    hit = {n: (("Breakpoint %d hit" % n) in chunk or
               re.search(r"Breakpoint %d[, ] 0x" % n, chunk) is not None or
               ("hit Breakpoint %d," % n) in chunk) for n in range(1, 9)}
    if hit[7]:
        send(*GFX1)
    elif hit[8]:
        send(*GFX2)                      # stays stopped on purpose: inspect, then resume
    elif hit[1]:
        r1[0] += 1; send(*R1C, *R2AT, "continue")
        if r1[0] >= 3: send("disable 1")
    elif hit[2]:
        pump[0] += 1
        if pump[0] == 1:
            send(*R0C, "set $srv=*(int*)0x70bd98", *R2AT, "continue")      # R0 + clock at frame-0 pump
        elif pump[0] <= 4:
            send("x/1wx 0x70c2b8", "set $srv=*(int*)0x70bd98", *R2AT, "continue")  # R2 frame-1..3 legs
        else:
            send("disable 2", "continue")
    elif hit[3]:
        fun[0] += 1; send(*R3C, *R4PRE, "continue")
        if fun[0] >= 12: send("disable 3")
    elif hit[4]:
        post[0] += 1; send(*R4P, "continue")
    elif hit[5]:
        send(*R5C, "continue")
    elif hit[6]:
        tick[0] += 1
        if tick[0] >= 6: send("disable 6", "continue")
        elif tick[0] == 1: send("print/x $eax", "x/1wx 0x70c2b8", "set $srv=*(int*)0x70bd98", *R2AT, "continue")  # R2 frame-1 leg (unconditional)
        else: send("print/x $eax", "x/1wx 0x70c2b8", "continue")
    elif "received signal" in chunk or "SIGTRAP" in chunk:
        send("continue")                 # THE DANCE: consume thread-start stops
    elif "exited" in chunk and "exit thread" not in chunk:
        vals.write("PROCESS-EXITED\n"); break
vals.write("DRIVER-END r1=%d tick=%d pump=%d funnel=%d post=%d\n" % (r1[0], tick[0], pump[0], fun[0], post[0]))
