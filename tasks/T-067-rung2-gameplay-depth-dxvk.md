---
status: done (wall BROKEN — DXVK stack found; depth data harvested; live-Attack/count>4 routed to input-injection)
assignee: HappyEagle (dispatched by LoudDragon off T-056 acceptance, 2026-10-21 — board line `tasks/README.md:172`)
depends: T-056 (six-row table + hybrid-stack recipe + the named bink wall = this row's opening premise), T-050 (re-entrancy observation is this row's payload), T-062 (bonus watches), T-068/T-069 (downstream consumers)
---

# T-067 — Rung-2 environment for GAMEPLAY DEPTH: past the bink-intro wined3d_cs wall

## Premise (from T-056 — do NOT re-derive)
Six lockstep rows are banked (commit `4202459` / doc `discoveries/2025-10-21-t056-xorg-dummy-six-rows.md`).
Healthy :96 boots die 2.5-4 min in: deterministic SIGSEGV in wine's own `wined3d_cs` thread at the
BINK-INTRO UPLOAD (`ucrtbase!sse2_memmove` fed a dead cs-packet handler pointer + ret 0x0).
`HKCU Direct3D cs=none` does NOT remove the cs thread on wine-11.0. Working stack is HYBRID:
mode metadata from Xorg-dummy :96 RandR (keep it), GL via host EGL/winewayland (this is what crashes).

## Targets (table-or-named-mechanism, one sitting)
1. **bp5 live**: `0x459f5d` (`F3::Attack` script-native call site) fires at least once; R3/R4/R4b
   re-captured AT THAT INSTANT (the T-056 deviation closes).
2. **R3 depth datum**: deque `[Entity+0x314]` LIVE COUNT > 4 observed (the D-2 dropped-order test).
3. **T-050 re-entrancy**: watch `0x5a94a0` / `0x5a955d` — confirmation OR falsification of the
   skip-advance seam, with the loud discipline: no evidence = gate stays, named why.
4. **BONUS-3**: slot16 repaint under a rate<0 skin (mode-1/2 interpolation quality) if the menu/areamap shows one.
5. **BONUS-4**: numeric window-event kinds {1,3,4,5,7} — one live dispatch trace at cursor-reload bps.
6. **BONUS-5**: writers of Window+0xD24/+0xD40 (cursor-sound names) + GameClient+0xA4 (GameCursorManager) fill site.

## Environment decision (coordinator ladder rung 2)
**DXVK native d3d8** (`WINEDLLOVERRIDES='d3d8=n'`, user-scoped nix dxvk, dlls copied into the PRIVATE
prefix only). Mandatory caveat: DXVK substitutes the runtime the six values were defined against —
state explicitly whether any captured datum's meaning depends on wined3d-vs-DXVK internals (mode
enumeration, present path); values read from ENGINE memory (RNG state, queues, entity fields) are
runtime-independent in definition but must be cross-checked against the T-056 wined3d-instant rows.
KNOWN RISK (name it before chasing it): DXVK may return an EMPTY EnumAdapterModes list; then shipped
FindMode `meth_0x502870` finds no mode and dies with the SAME fatal string via a different mechanism —
that outcome = named second-order mechanism, stop-early-and-report per acceptance.
Fallbacks IN ORDER: (a) DXVK works → harvest; (b) DXVK modes empty → try `DXVK_STATE_DIR`/config knobs
ONCE, else named wall; (c) fixed-wine route (rebuild wined3d dodging the cs-bink path) = beyond this
sitting's scope → escalate as devenv decision, do NOT edit devenv/** here.

## Recipe (warm start)
Xorg :96 ALREADY RUNNING (pid `/tmp/t056-xorg.pid`=158324, config `/tmp/t056-xorg.conf`); private
wineserver up (158368); `tools/t052-drive.py` carries all legs; session recipe = T-056 doc §"How the
wall fell" minus the WAYLAND clearing, plus `WINEDLLOVERRIDES` d3d8 native + dxvk win32 dlls in the
prefix. New bps to arm: `0x5a94a0`, `0x5a955d` (+ existing six).

## Hygiene (non-negotiable, row contract)
Private prefix only (`/tmp/t056-wineprefix` or fresh clone); recorded-pids + `/proc/<pid>/environ`
WINEPREFIX gates for every kill (`pgrep -f` in compound commands = banned after two self-SIGTERMs);
managed 31437/31338 + ImHex 31337 NEVER touched; user-scoped installs only, zero `devenv/**` edits;
scratch ports 31440+ only; IDA original MCP read-only.

## VERDICT 2026-10-21 (HappyEagle) — WALL BROKEN; DXVK RUNS THE ENGINE 12 MIN CLEAN
Full evidence: `discoveries/2026-10-21-t067-dxvk-rung2-depth-harvest.md`. Two keys: (1) DXVK d3d8 IMPORTS
Direct3DCreate9 from d3d9.dll — d3d9 must ALSO be native (d3d8 alone = NULL-d3d9 vtable AV at d3d8+0x1305a);
(2) wine-11 IGNORES WINEDLLOVERRIDES for d3d9 (`get_load_order got hardcoded default`) — the
HKCU\Software\Wine\DllOverrides REGISTRY values (d3d8=n, d3d9=n) take effect. With that + :96 kept + 32-bit
nvidia Vulkan ICD (and the softpipe env REMOVED — it caused a 32-bit LLVM OOM once wined3d stopped owning GL):
engine alive 712s CPU (~12 min wall), ZERO crashes — **the bink wall was wined3d-11's cs-thread bug, not shipped**.
Depth datum: R0 0x544A90 confirmed 4th time (cross-runtime invariant); R2 frame-0 leap 0→1s REPRODUCED under
DXVK; funnel counts grew 1→2→3 (max 3 at 12 min — **>4 needs interactive input, not a better GL stack**);
bp5 live Attack 0 hits (same — confirms T-056 deviation as input-limit, not crash-truncation);
**T-050 seam: entry leg 0x5a94a0 fired 25× on one receiver (0x0A1F0440, args advancing) — the advance leg
0x5a955d NEVER fired in a clean full boot.** Observation gate UNBLOCKED; interpretation with T-050 static
semantics is the coordinator's pass. Divergence ruling: engine-memory values pair CROSS-stack (proven
identical semantics both runtimes), but HEAP ADDRESSES differ between stacks — T-031/T-068 must pair on
same-env addresses, cross-env constants. Next cheap step (routed, not built): input injection for live
Attack/count>4 + bonus cursor watches on this now-endless window.

## T-077 CAPTURE-PLAN ADDENDUM (from KeenStorm T-070/T-071 framing + this lane's data; commit-ready when staffed)
Injection is only the trigger; the PAYLOAD is one shape read at the first funnel hit on an injected order:
  `x/2wx <entity+0x308>,<entity+0x314>` + slot0 action `[action+0x00]`
- SHAPE A (predicted): slot0 stays 0x6497C4 (Idle RESIDENT), injected order lands slot1+, frame-over-frame
  unchanged, never advances → HOL live-confirmed; T-056/R-H "Attack never fired" upgrades from gap to
  DEMONSTRATED designed behavior; T-071 push-verb HOL stands.
- SHAPE B (THE ONLY TRUE FALSIFIER, KeenStorm refined): slot0 STILL 0x6497C4 (idle PRESENT) while the order
  behind it ADVANCES — count decreasing, or the order's Update bp hits, or idle skipped/erased with no
  meth_0x54c440 drain. Pump eating PAST a never-finishing head. LOUD in-session ping, verb split revised.
- SHAPE C (CONSISTENT — do NOT alarm): (1) count hits 0 and STAYS 0 — nothing re-feeds idle on empty
  (pump's empty branch 0x54c849+ is the anim-level default meth_0x548d80, not a re-enqueue; only rebase
  sites present idle); (2) mid-round the EXECUTING order IS the head — self-re-queue sites (MoveTo
  waypoints 0x4064bd x6, attack self-re-queues 0x40470e x6) enqueue non-idle heads with no drain at that
  instant; (3) PRACTICAL TRAP: injecting into an entity whose queue is EMPTY fires the order IMMEDIATELY —
  that is C, not B. Stall only holds while the resident lives.
- READING RULE: slot0-vft + count as a PAIR at every advance observation; B is idle-PRESENT-and-consumed-past,
  never advance-per-se. Frame-over-frame stands.
- Banked by KeenStorm as independent R4b confirm (third boot, second runtime, third entity pointer):
  scripted rounds' count 1→2→3, Idle-first/Anim-after on 0x0A6C6060.
Also arm (T-070 §5): [idle+0x16]/[idle+0x18] across frames (expect 1/0), [entity+0x184] before/after rebase
(world-ms stamp), drain @0x551bbe vs create @0x551bde same-frame cadence, count>4 (D-2 minus resident) rides
along. Re-opener watch: bp10 0x5a955d — zero across sustained boot is consistent with raiser-exclusion
(+0x319 never armed); ANY post-injection bp10 fire = T-050 closure falsifier, report immediately.
