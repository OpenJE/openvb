# T-034 handover (NiceHawk → RedXenon) — **the original's gdb stub never binds: ImHex owns 31337.** Attach itself works.

Row moved off me mid-session. Nothing here is a value read — **R0–R4b are all still unread** — but the row has been
reported as "unreachable" by two lanes and the reason is a harness defect, not a breakpoint problem. Everything below
was measured, not inferred. No source edits; no IDB edits.

Target: ORIGINAL_REFERENCE (`$OPENVB_ORIG_EXE` = `…/f3demo-1.0/share/f3demo/F3.exe`, wineprefix
`.devenv/state/f3demo-wineprefix`).

## 1. Root cause of "can't attach to the original" (measured)

| evidence | content |
|---|---|
| `devenv/processes/imhex-headless.nix` | `imhexNetworkPort = "31337";` — hardcoded literal |
| `devenv/environment/openvb.nix:19,23,26` | `OPENVB_ORIG_GDB_PORT = "31337"; OPENVB_REIMPL_GDB_PORT = "31338"; OPENVB_IMHEX_MCP_PORT = "31337";` |
| `ss -ltnp` (live) | `127.0.0.1:31337 users:(("ImHex ????",pid=12337,fd=44))` — and **no** winedbg on 31337; the only live stub is `winedbg.exe --gdb --port 31338` (reimpl) |
| `/run/user/1000/devenv-664db37/processes/logs/ida-orig-gdb.stdout.log` | ends `Original demo GDB stub listening on 127.0.0.1:31337` … `WineDbg starting on pid 0170` — banner printed, **bind lost to ImHex, stub gone**; stderr log is empty |

`processes.ida-orig-gdb` (in `devenv/processes/f3-gdb-stubs.nix`) and `f3-lockstep-debug` (which starts *both* stubs)
both use `$OPENVB_ORIG_GDB_PORT`. Whichever starts second loses the port silently: winedbg prints its banner, the
`ready.exec = nc -z 127.0.0.1 31337` check **passes against ImHex's listener**, so devenv reports the process ready.
A lane then connects to ImHex's socket instead of a debug stub (or times out), and concludes the original won't attach.

**Fix, when someone holds a `devenv/**` slot:** move ImHex's Network Interface off 31337 (it is not in any contract;
`IMHEX_PORT` is exported from the same file that hardcodes it, so it is a one-line change in two places). Do **not**
move `OPENVB_ORIG_GDB_PORT` — docs and muscle memory quote 31337.

## 2. Workaround that actually attaches (verified, no `devenv/**` edit)

Same prefix, same exe, non-colliding port. `winedbg` is **not on the devenv PATH** — use the store path
(`pkgs.wine` = `/nix/store/hf94anvjgxy56ddx1arxrswpv2zwbzvi-wine-11.0/bin/winedbg` in the current shell).

```bash
export WINEPREFIX="$PWD/.devenv/state/f3demo-wineprefix" WINEARCH=win32 \
       WINEDLLOVERRIDES="mscoree,mshtml=" WINEDEBUG="-all,+err"
cd "$OPENVB_F3DEMO_ROOT"
nohup /nix/store/hf94anvjgxy56ddx1arxrswpv2zwbzvi-wine-11.0/bin/winedbg \
      --gdb --port 31437 --no-start F3.exe > /tmp/orig-gdb.log 2>&1 &
```

Observed (`/tmp/orig-gdb.log`): `WineDbg starting on pid 016c` → `create process '…F3.exe'` → `target remote
localhost:31437`, listener up on 31437, full DLL load list (incl. `d3d8.dll`/`wined3d.dll`, `granny2.dll@10000000`,
`binkw32.dll@30000000`). Then host **gdb 17.2** (`gdb` on PATH, Python 3.14 enabled):

```
(gdb) set architecture i386:intel
(gdb) target remote 127.0.0.1:31437
0x7bf95046 in process_rtl_work_item@8 () from …/f3demo-wineprefix/drive_c/windows/system32/ntdll.dll
(gdb) x/4bx 0x400000      →  0x4d 0x5a 0x90 0x00      # "MZ" — image mapped at 0x400000
```

**So IDA EA == target VA == `$pc` space**: the PE has image base `0x400000` and no ASLR, breakpoints at `0x48ccf6` /
`0x552bb5` / `0x550720` are settable directly. `info proc mappings` is *not supported* by the winedbg stub — read
memory with `x`/`read_memory`, don't chase mappings.

### 2a. Three lifecycle gotchas that will otherwise look like new failures

1. **One gdb per stub.** `--no-start` stubs accept a single connection; when gdb exits, the listener is gone and the
   next connect fails `could not connect: Connection timed out` (I hit exactly this). Drive everything — breakpoints,
   `continue`, reads — inside **one** gdb process (see §4 harness).
2. **`detach`/gdb-exit leaves the demo running free**, it does not die with the debugger. I found two orphan
   `F3.exe` (one mine) in the shared `f3demo-wineprefix`. Kill leftovers before each attempt:
   `pgrep -a 'F3.exe'` and check `/proc/<pid>/environ` for `WINEPREFIX=…f3demo-wineprefix` so you do not kill a
   lane's reimpl run (`build/F3.exe` uses `f3-reimpl-wineprefix`).
3. Graphics init is noisy but non-fatal (`libEGL warning: pci id … driver (null)`, `egl: failed to create dri2
   screen`); the window still comes up on `DISPLAY=:0`.

## 3. Static decoding I did while the stub was coming up (closes R1's address question, sharpens R0)

Raw `objdump -d` on `$OPENVB_F3DEMO_ROOT/F3.exe` (`pei-i386`, `.text` vma `0x401000` = file off `0x1000`; `.data` vma
`0x6f1000`+`0x17000`).

**`sub_48CCC0` (the shipped LCG) — R1 has no address ambiguity:**

```
48ccc0  mov  al,[0x707c64]        ; reseed flag
48ccc5  test al,al
48ccc7  jne  0x48ccd9             ; already seeded -> skip
48ccc9  push 0x0
48cccb  call 0x618b05             ; time(NULL)
48ccd0  push eax
48ccd1  call 0x48cca0             ; seed the state
48ccd9  mov  eax,[0x707c60]       ; STATE
48ccde  imul eax,0x19660d         ; 1664525
48cce4  add  eax,0x3c6ef35f       ; 1013904223
48cce9  mov  [0x707c60],eax       ; store STATE
48ccee  shr  eax,0x8
48ccf1  and  eax,0x7fff
48ccf6  ret
```

⇒ **R1 = dword `[0x707C60]` at the first hit of `0x48ccf6`** (with `$eax` = the drawn value `(state>>8)&0x7fff`), and
the reseed gate is byte `[0x707C64]`. Both globals are in `.data`, i.e. zero-init: the state is seeded from `time()`
at run time, so the captured value is **per-boot** — which is exactly T-031 §1(B)'s capture-and-replay premise, and
means a second boot will *not* reproduce it.

**`JE::Server::virt_meth_0x552b60` (the pump) — R0's register contract at the breakpoint:**

```
552b99  mov  eax,[0x70bd98]       ; server object
552b9e  test eax,eax
552ba4  lea  esi,[eax+4]          ; WORLD VIEW = [0x70BD98] + 4   (the DB's "+1" is 4-byte pointer arithmetic)
552ba9  xor  esi,esi              ; …unless the global is null
552bab  mov  ebx,[esi]            ; VFTPTR of the world view
552bad  call 0x56af50             ; JE_TickMilliseconds == timeGetTime()
552bb2  push eax                  ; arg = absolute uptime ms
552bb3  mov  ecx,esi              ; this = world view
552bb5  call *0x18(%ebx)          ; vftable slot 6
```

⇒ at a breakpoint on `0x552bb5` nothing needs to be re-derived: **`$ebx` = World vftptr and the answer to R0 is
`x/1wx $ebx+0x18`**; `$esi` is the world view (`+0x3C` is R2's `Gregorian` pointer), `$eax` is the same `tickMs` the
digest prints. One instruction, one read.

**And the reason `0x552bb5` may never hit:** `0x552bab` dereferences `[esi]` unconditionally, and `esi` is the null
path when `[0x70BD98] == 0`. This pump body therefore only executes while a **world object exists** — i.e. after the
demo has hosted/entered a game. A demo sitting at the intro movie or main menu will hit `0x48ccf6` (menus draw, menus
roll dice) and will **not** hit `0x552bb5`. If R0 reads as "breakpoint never fired", that is the explanation to
record, not "attach failed".

## 4. Reusable harness (one gdb process; breakpoints never stop the target, they log)

Save as `/tmp/nh_reads.py`, run `gdb -q -batch -x /tmp/nh_reads.py` **immediately after** starting the stub. Extend
`HITS` rather than restarting the experiment. It records the R0/R1/R2 datum and counts funnel (`0x550720`) traffic —
the funnel count is itself a fact for R3: it says whether the demo enqueues gameplay orders at all without input.

```python
import gdb, struct, time
LOG = open("/tmp/nh-reads.log", "w", buffering=1)
def log(*a): LOG.write(" ".join(map(str, a)) + "\n")
def rd32(a):
    try: return struct.unpack("<I", bytes(gdb.selected_inferior().read_memory(a, 4)))[0]
    except Exception: return None
def rs(r):
    try: return int(gdb.parse_and_eval("$" + r))
    except Exception: return 0
class Hit(gdb.Breakpoint):
    def __init__(self, ea, name, limit):
        gdb.Breakpoint.__init__(self, "*0x%x" % ea, internal=True)
        self.name, self.limit, self.hits = name, limit, 0
    def stop(self):
        self.hits += 1
        if self.hits <= 8:
            log("### %s hit %d" % (self.name, self.hits))
            log("  eax=%08x ebx=%08x ecx=%08x esi=%08x edi=%08x esp=%08x" %
                (rs("eax"), rs("ebx"), rs("ecx"), rs("esi"), rs("edi"), rs("esp")))
            if self.name == "R1":       # 0x48ccf6 = ret of sub_48CCC0
                log("  R1 DRAW=%08x STATE[0x707c60]=%08x flag[0x707c64]=%08x" %
                    (rs("eax"), rd32(0x707C60), rd32(0x707C64)))
            if self.name == "R0":       # 0x552bb5 = call *[ebx+0x18]
                w, vft, tick = rs("esi"), rs("ebx"), rs("eax")
                log("  R0 WORLD=%08x VFT=%08x [VFT+0x18]=%08x tick=%08x" % (w, vft, rd32(vft + 0x18), tick))
                w3c = rd32(w + 0x3C) if w else None      # R2: Gregorian behind world+0x3C
                log("  R2 [world+0x3C]=%08x" % (w3c or 0))
                if w3c and 0x10000 < w3c < 0x80000000:
                    log("     deref:" + " ".join("%x:%08x" % (o, rd32(w3c + o) or 0)
                                                 for o in (0, 4, 8, 0xC, 0x10, 0x14)))
                log("  [0x70bd98]=%08x" % (rd32(0x70BD98) or 0))
            if self.name == "FUNNEL":   # 0x550720 entry; R3 context
                esp = rs("esp")
                log("  FUNNEL ecx=%08x a1=%08x a2=%08x  entity=%08x [ent+0x304+4]=%08x cap=%08x idx=%08x" % (
                    rs("ecx"), rd32(esp + 4) or 0, rd32(esp + 8) or 0,
                    0, 0, 0, 0))   # fill entity/0x304 fields per EntityActionQueueSeam.md once ecx's role is known
        return False                    # never stop
gdb.execute("set pagination off"); gdb.execute("set confirm off")
gdb.execute("set architecture i386:intel")
gdb.execute("target remote 127.0.0.1:31437")
log("ATTACHED pc=%08x" % int(gdb.parse_and_eval("$pc")))
bps = [Hit(0x48ccf6, "R1", 6), Hit(0x552bb5, "R0", 8), Hit(0x550720, "FUNNEL", 200)]
gdb.continue_for(45)                    # gdb >= 14; else `continue &` + sleep + interrupt
for b in bps: log("TOTAL %s = %d" % (b.name, b.hits))
for b in bps: b.enabled = False
gdb.execute("interrupt") if False else None
gdb.execute("detach")                   # NB: leaves the demo running — kill it (see 2a.2)
```

## 5. What was NOT reached, and why (the honest miss list)

- **R0, R1, R2, R3, R4, R4b: no values.** The row was reassigned after attach was proven; I did not run the window to
  completion. Nothing here should be read as a measurement of any of them.
- **R3 / R4b (and R4's pre-call sample) are blocked by a second, distinct gap: input driving.** They need a live
  entity **mid combat round**, which needs the shipped demo to be played from the menu into a hosted game and into a
  round. `xdotool` and `wmctrl` are **not installed** in the devenv shell (`which` → not found), so there is no CLI
  route to click/keys the wine window; and there is no headless/`--auto` switch in the demo dir (`readme.txt`,
  `data/`, `Demo/` show none). Options for whoever picks this up: (a) add `xdotool` to `devenv/packages` — a
  `devenv/**` edit, two-lane cap; (b) drive the window from inside gdb via `call SendMessage`/`keybd_event` — that is
  writing into the shipped process, which T-034 explicitly forbids without a ruling; (c) accept R3 as "unreachable
  without input automation" and settle D-2's severity another way (e.g. T-037's record map + a stress test on our own
  seam). I did not attempt (a)–(c); (c) is the one I'd argue for if the input gap stays.
- **The `[World vftptr + 24]` question itself is still open.** `docs/f3/WorldTickPump.md`'s single declared assumption
  survives *unfalsified* — nothing I found contradicts it, and §3 pins exactly which register to read — but it is not
  confirmed, so the page keeps its assumption marker.
