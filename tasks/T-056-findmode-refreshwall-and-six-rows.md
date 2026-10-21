---
status: done (branch a — six values delivered; second-order mechanism named+sampled)
assignee: HappyEagle (dispatched by LoudDragon `01a15f82`, 2026-10-21 — decision (a) from T-052 §5)
depends: T-052 (the named mechanism + working harness are this row's foundation), T-031/T-050/T-051 (all three wait downstream on THIS row, not on tooling)
---

# T-056 — Get shipped F3 past the FindMode `RefreshRate>=60` wall, then read the six rows

## The wall (established by T-052 — do NOT re-derive)
`JE::Gfx_D3D::meth_0x502870` (gate `0x50292b`) rejects any mode with `RefreshRate < 60`;
wined3d-11's X11/GLX backend publishes `RefreshRate=0` for EVERY mode on EVERY reachable display
(proven by live mode-list read at the `0x5818d0` fatal stop). Client-side `xrandr --newmode` and
`ForceVideoModeList` are proven inert. Boot now reaches Graphics::Initialize, maps a real window,
and dies there — TICK fires, engine pumps, RNG seeds (`[0x707C60]=0x6AD906E5`, boot-phase value,
NOT the R1 draw).

## Decision (coordinator ruling, ladder in order)
1. **Xorg `-dummy` with an explicit `Modeline ... "60.00"` + monitor `HorizSync/VertRefresh` ranges** —
   a real X server whose RandR reports carry a nonzero vrefresh (Xvfb is proven to bring the wined3d
   adapter UP, but its RandR metadata is exactly what FindMode rejects; -dummy keeps the working
   adapter class and fixes only the metadata). Run with the proven `LIBGL_ALWAYS_SOFTWARE=1
   GALLIUM_DRIVER=softpipe` stack, PRIVATE cloned prefix (never the managed one), fifo-held-stdin
   winedbg per `tools/t052-drive.py`. N-resumes dance already automated.
2. **DXVK d3d8** (only if 1 fails): dxvk via `wine` dll-override in the private prefix; note this
   substitutes the runtime the six values were defined against — if the RNG/pump values differ in
   meaning under DXVK, SAY SO; do not silently pair DXVK-side values against wined3d-defined rows.
   If dxvk is not in the pinned nixpkgs, message the coordinator (devenv changefile is the
   coordinator's lane) — do NOT edit devenv/** from this row.
3. **Real-GPU X session** — requires the USER; escalate, do not attempt.

## Steps
1. One sitting per T-052's tradition: get past `0x5818d0` OR name the next terminal mechanism.
2. Past the wall: drive movie->menu->New Game (xdotool on the real window), then read ALL SIX
   rows (R0/R1/R2/R3/R4/R4b) at their instants, cold reads forbidden, table format per T-034.
   R0 loud-flag rule stays: live `[vft+0x18]` vs T-045 static `0x544a90` — disagreement is shoutable news.
3. Values + instants into the same discoveries file lineage; harness updates to `tools/t052-drive.py`
   /`t034-capture.py` v4 stay committed in tools/ (the writer lane for this row includes both).
4. Environment hygiene NON-NEGOTIABLE (T-052 incident re-briefed): private prefix only; recorded
   pids + /proc/<pid>/environ WINEPREFIX gates for ANY cleanup; NEVER the managed 31437/31338 pair;
   pgrep-pattern cleanup is banned (self-SIGTERM incident this wave).

## Acceptance
- EITHER the six-row table WITH values (pairing ready for T-031), OR named-and-sampled next
  mechanism under 1+2 attempted. Xvfb/:0/DXVK-null results already banked in T-052 §3 — do not re-walk.

## COORDINATOR ADDENDUM — bonus observations for whoever crosses the wall (post-`70c86f3`)
KeenStorm's static tail made the six rows CHEAPER and added two new lockstep asks for the same
run (they cost nothing once you're past the mode gate and driving to combat):
- BONUS-1 (was T-013's last open key question): statically DECIDED (stem-keyed registry, verbatim
  probe) — confirm only which shipped names carry dots if the census (157/165) needs live backing.
- BONUS-2: **RESOLVED STATICALLY by T-062 — no lockstep needed**: corner field 0 is a FLOAT u0
  written by GUI::Fragment slot12 (0x4b1610 setter / 0x4b1640 getter) via the child-notify float; not an index.
- BONUS-3: mode-1/2 frame-pair interpolation quality (linear vs blend weight) — slot16 repaint
  path under a rate<0 skin, if the menu/areamap surface shows one.
- BONUS-4 (from T-062): the NUMERIC identity of window event kinds {1,3,4,5,7} (gates+targets are
  statically pinned; producer-side kind values need one live dispatch trace at the cursor-reload bps).
- BONUS-5 (from T-062): who WRITES Window+0xD24/+0xD40 cursor-sound name strings, and the
  GameClient+0xA4 (GameCursorManager) fill site — both cheap watches if already armed nearby.

## VERDICT 2025-10-21 (HappyEagle) — SIX ROWS DELIVERED; full evidence: `discoveries/2025-10-21-t056-xorg-dummy-six-rows.md`
Ladder rung 1 executed: Xorg-dummy :96 (server-computed vrefresh 67.20 in RandR ModeInfo) cleared the FindMode gate — BP8 `0x5818d0` NEVER hit on any :96 boot. HYBRID caveat: wined3d reads mode metadata from :96 RandR but the GL context/present rides **host EGL via wine's winewayland driver** (inherited WAYLAND_DISPLAY) — pure X11 on :96 fails `Direct3DCreate8` (`Failed to Create D3D!`; no mesa `libglxserver_mesa` exists in pinned nixpkgs). If :96 dies or host EGL stalls the boot freezes as a `wglSwapBuffers` deadlock in wined3d_cs (runs D/F/G) — keep :96 alive during capture.
Six values (canonical run H, cross-checked run C — boots are ADDRESS-IDENTICAL): **R0** `[World vft 0x66856C +0x18] = 0x544A90` — T-045 documented assumption LIVE-CONFIRMED (no loud-flag; agreement). **R1** first-return draw `$eax=0x2D81`, state `[0x707C60]=0x65AD8170`, gate `0x01`; varies boot-to-boot (wall-clock seed) — comparable datum = STATE at instant (coordinator pairing rule banked). **R2** `[[0x70BD98]+0x3C]=0x079CD8D8`; seconds `0` at first pump → `1` by pump #3 (frame-0 leap IS shipped behavior); 21s at R1 instant. **R3** deque @Entity+0x304: count at `+0x314` semantics HOLD (RedXenon kept); observed growth 0→1→3 (max 3 pre-crash; no >4 datum). **R4** `[action+0x14]` pre = **0 (not garbage)** → post = **entity pointer**, 35/35 stores on script-native-path callers. **R4b** slot/action vfts: `0x6497C4=`**GameActionIdle**, `0x6A6414=`**ActionAnim**; zero UNKNOWN_VT (resolved vs artifacts/F3.json). DEVIATION (honest): bp5 `0x459f5d` live `F3::Attack` never fired in any boot — pre-crash demo script enqueues Idle/Anim only; R3/R4/R4b captured at the same funnel from those scripted callers.
SECOND-ORDER MECHANISM (named+sampled; one-sitting stop honored): deterministic **SIGSEGV in wine's own wined3d_cs at the bink-intro upload** (`ucrtbase!sse2_memmove`, ecx≈1.8MB, edi=0, frame1 = misresolved nearby symbol + ret 0x0 ⇒ dead cs-packet handler pointer), 2.5–4 min into every healthy boot, AFTER all six instants, BEFORE interactive combat. `HKCU Direct3D cs=none` does NOT remove the cs thread on wine-11.0 (finding). Gameplay-depth (count>4, live Attack, bonus-3/4/5 cursor watches) requires dodging it — rung 2 (DXVK d3d8, divergence caveat) or a fixed-wine; routed onward, not attempted (six values already met).
BONUS disposition: B-1/B-2 need no live work (statically decided — per addendum). B-3/4/5 = one extra armed-bp pass; recommend KeenStorm/T-062 owner on a :96 capture run (bps cheap, window 2.5-4 min, cursor-reload sites already known statically). T-050 gate HOLDS (bink wall prevents re-entrancy evidence either way); T-031/T-051 pairing READY with this table.
