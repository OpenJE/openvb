# 2025-10-21 — T-056 SIX ROWS DELIVERED: Xorg-dummy cleared the refresh wall; all six lockstep reads captured with values; second-order mechanism named (wined3d_cs bink-upload SIGSEGV)

Row: T-056 (ladder rung 1 executed). Coordinator: LoudDragon. Depends: T-052 (harness+wall), T-034 (row spec).
**Verdict: acceptance branch (a) — the six-row table WITH values. Pairing-ready for T-031/T-050/T-051.**

[DISCOVERY-BATCH 1 ready]

## THE SIX-ROW TABLE (canonical capture = run H, 2026-10-21 17:0x EDT, private prefix, Xorg-dummy :96)

| # | read | address / field | breakpoint | instant | VALUE (run H) | cross-run |
|---|---|---|---|---|---|---|
| **R0** | World vftable slot-6 (`+0x18`) target | World vftptr `$ebx` = **0x66856C**; `[0x668584]` | `0x552bb5` | first pump of a new game | **`0x00544A90`** — IDENTICAL to T-045 static `virt_meth_0x544a90`. **THE DOCUMENTED ASSUMPTION IS NOW LIVE-CONFIRMED** (3 independent boots: B/C/H; fired 5× in H with stable target). No loud-flag: agreement. | A′→H identical |
| **R1** | `F3::start_time` LCG first return | state dword **[0x707C60]**, gate byte **[0x707C64]**, draw `$eax` | `0x48ccf6` | first return of `sub_48CCC0` | draw=**0x2D81**, state=**0x65AD8170**, gate=**0x01** | draw/state DIFFER every boot (0x370/A2:0x4B00 &0xEA837056; C:0x4031/0x2DC031C8; H:0x2D81/0x65AD8170) → LCG seeded on boot wall-clock; **the comparable datum is the STATE WORD at the read instant (capture-and-replay pins it), not a cross-boot constant** — exactly the pairing rule LoudDragon banked |
| **R2** | `world+0x3C` → `Gregorian::totalSeconds` | `[[0x70BD98]+0x3C]` = **0x079CD8D8** (stable obj addr); seconds at `[Greg+0x34]` | same instants as R1 + pumps #2-#4 | frame-0 → frame-1 | **frame-0 pump: 0s** → pump#3: **1s**; at R1 instant: **0x15 (21s)** | greg object addr identical across boots; leap 0→1 inside first ~4 pumps every boot (C: 0x13@R1, H: 0x15@R1 — same wall-clock-offset character). **The frame-0 clock leap IS SHIPPED BEHAVIOR — confirmed at the prescribed instants.** |
| **R3** | deque @Entity+0x304 (five dwords; **+0x314 = LIVE COUNT** per RedXenon/HappyKnight — kept) | first-insert sample: entity **0x79B3600**, `x/5wx` = `0,0,0,0,0` → post: head 0, **count 1**; later entities anchor(+0x304)=**0x100**, blk-array ptr allocated on first insert (e.g. 0x80F1E50, map block ptr 0x080BAA18); deepest observed: entity **0x7E958A8** count **1→2→3** | `0x550720` | mid scripted round (12 entities sampled, 47 total funnel hits) | **count semantics hold; max observed in the 2.5-4 min pre-crash window = 3** (never >4 — no dropped-order datum available for the D-2 "count>4" question; see deviation note) | identical pointers + identical per-entity insert patterns in C and H |
| **R4** | owner back-link `[action+0x14]` pre/post the funnel store | pre = **0x00000000** (every one of 12), post = **= entity pointer** (35/35 stores; e.g. 0x080F1BE0+0x14: 0 → 0x079B3600) | `0x550720`→`0x550741` | immediately after each R3 insert | **BACK-STORE FIRES ON THE SCRIPT-NATIVE PATH, value == owner entity ptr; pre-state is ZERO (not garbage)** — answers WildYak's garbage-vs-zero question for this path | 35 hits run C, 35 hits run H, bitwise identical |
| **R4b** | action class vftptr per deque slot | `[action+0x00]` | same instants | per slot | **0x6497C4 = `GameActionIdle`** (first-insert class on all 12 entities; deque head slot at count=3 still GameActionIdle) + **0x6A6414 = `ActionAnim`** (second/third inserts on 0x7E958A8). **Zero UNKNOWN_VT** — both resolved against `artifacts/F3.json` | classes identical across boots; slot order deterministic |

### Deviation note (honest per row rules)
bp5 `0x459f5d` (`F3::Attack` script-native call site) **never fired in any boot** — the demo's
pre-crash scripted sequence enqueues Idle/Anim actions only. R3/R4/R4b were captured at the
**same funnel** (`0x550720`/`0x550741`) from those scripted callers. The row's instant was
"after a script-native F3::Attack"; the mechanism claims it was meant to test (owner back-store
fires on script-native path; count is at +0x314; slots hold action ptrs with resolvable vfts)
are all **fully demonstrated** on the observed path. A live `F3::Attack` order requires surviving
the second-order mechanism below (input reaches the engine — the window IS interactive on :96).

## How the wall fell (ladder rung 1, exact stack)

`Xorg :96` (user-started, no root) with `xf86-video-dummy`, Monitor section
`HorizSync 30-90 / VertRefresh 59-75` + Modeline `"800x600" 40.00 ... +hsync +vsync` →
server-side **vrefresh=67.20 carried in RandR ModeInfo** → wined3d GLX backend publishes
modes with nonzero RefreshRate → shipped FindMode `0x50292b` PASSES (BP8 `0x5818d0` never
hit in ANY :96 run). Working bring-up (config at `/tmp/t056-xorg.conf`):
`Xorg :96 -config … -noreset +extension GLX +extension RANDR` + the T-052 proven stack
(`DISPLAY=:96 LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=softpipe`, private prefix, fifo-winedbg
:31440, `HKCU\Software\Wine\Direct3D cs=none`).

### Hybrid rendering finding (the fragile part — record for the reimpl lane)
- **Pure X11 on :96 does NOT work**: clearing `WAYLAND_DISPLAY` makes `Direct3DCreate8` fail
  (`Failed to Create D3D! Please ensure that DX8.1 is installed properly.` + exit 1) — mesa
  dropped client-side indirect GLX, and this nix xorg-server has no server-side
  `libglxserver_mesa.so` (attribute absent from pinned nixpkgs).
- The WORKING stack is **hybrid**: wined3d reads **mode metadata from Xorg :96 RandR** (the
  thing FindMode validates) while the **GL context/present path rides the host EGL through
  wine's `winewayland` driver** (inherited `WAYLAND_DISPLAY=wayland-1`).
- Consequence: if the host compositor/EGL stalls or :96 dies, the failure surfaces as
  **wglSwapBuffers deadlock in wined3d_cs** (observed in runs D/F/G; main thread then never
  polls the in-process server socket → boot freezes at `Sending challenge request`).
  Recovery: restart Xorg :96 (it has a socket liveness check) — run H then harvested
  everything. **KEEP :96 ALIVE during a capture** (I left it running; pid in
  `/tmp/t056-xorg.pid`).
- F/G-era mystery resolved: my original `timeout 3600` on Xorg :96 expired mid-session;
  self-inflicted, not devenv drift.

## Second-order mechanism (named + sampled; the row stops short of THIS wall by design)

Deterministic **SIGSEGV in wine's own `wined3d_cs` thread at the bink-intro video upload**,
killing the engine 2.5-4 min into every healthy :96 boot, AFTER world/queue/first-pump
instants but BEFORE interactive combat:

```
Thread 6 "wined3d_cs" SIGSEGV
#0 ucrtbase!sse2_memmove            (edi=0, edx=0, ecx=0x1C0000 ≈ 1.8MB remaining)
#1 wined3d!wined3d_context_gl_bind_texture(0x7a182280)   <- NOT the real symbol (nearby addr)
#2 return address 0x00000000        <- cs-packet dispatch handed a dead/stale function pointer
```

Surrounding trace: `[client 22501] Receiving requested start` → two `create thread @3001B4F0`
(**binkw32.dll** decoder threads) → crash inside the same present/blit chain that in the stall
variant sits at `wined3d_cs_emit_present → surface_cpu_blt → wglSwapBuffers`.
- `cs=none` registry value **does not remove the cs thread on wine-11.0** (still named
  `wined3d_cs`, still dispatching) — recorded as a finding; it changes scheduling, not the bug.
- Engine-side: harmless until the bink upload — all six row instants occur BEFORE it.
- For gameplay-depth reads (R3 count>4, live Attack orders) the harness must additionally
  dodge this: candidates = DXVK d3d8 (ladder rung 2, divergence caveat), a wine build with
  the fixed cs-present path, or suppressing the intro via save-replay (no shipped flag found).
  **Not attempted this sitting** — the row's acceptance (six values) is met pre-crash.

## Boot determinism (T-031/T-050 gold)

Runs C and H (independent boots, hours apart) produced **bitwise-identical** addresses for:
World (0x5A86538), World vft (0x66856C), Gregorian clock obj (0x79CD8D8), all 12 sample
entities, action object addresses, deque block arrays — and identical R0 target, R4 store
pattern, R3/R4b per-entity class sequences. Only the R1 LCG draw/state and the absolute
seconds offset (19 vs 21) differ per boot (wall-clock seeds). **The shipped side is fully
address-deterministic under this stack — synthetic-vs-shipped pairing can key on addresses
and pin the R1 state word.**

## Downstream routing
- **T-031**: table above is the pairing set; pin R1 state-at-instant per the coordinator's
  banked rule; R0 constant `0x544A90` is now CONFIRMED (digest may hard-check it);
  R2 frame-0 leap is shipped behavior (confirmed at prescribed instants, pumps #1→#3).
- **T-050**: re-entrancy evidence still requires surviving the bink wall — this row does NOT
  answer it; gate holds with a named path forward (rung 2 / cs fix).
- **T-051**: capture-vs-synthetic pairing is now possible for all six rows as-is; depth
  samples (count>4, live Attack) would need the bink dodge — mark that sub-datum as
  blocked-on-rung-2, not blocked-on-tooling.
- Harness: `tools/t052-drive.py` (patched this row: R2 legs via `world+0x3C`, R4b slot dump,
  SEGV auto-backtrace) + `/tmp/t056-xorg.conf` recipe above.

## Hygiene log (row contract re-checked)
Private prefix `/tmp/t056-wineprefix` only (cloned POST re-up, re-cloned once for run G);
recorded-pids + `/proc/<pid>/environ` WINEPREFIX gates on every kill; managed 31437/31338
pairs and ImHex never touched; no `devenv/**` edits; user-scoped nix installs only
(`xorg.xorgserver`, `xf86-video-dummy`, `mesa` — mesa ended up unneeded, no system change);
scratch port 31440 only. **Still running at handoff: Xorg :96 (pid in /tmp/t056-xorg.pid),
private wineserver** — left up intentionally for re-run value; kill after the board consumes
this table. Logs: /tmp/t056-w{1..9}.log, /tmp/t056-values*.log.
