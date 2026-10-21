---
status: in-progress
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
