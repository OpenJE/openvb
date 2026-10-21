# 2026-10-21 — T-067: DXVK d3d8 BREAKS THE BINK WALL — shipped F3 ran 12 min CPU clean; seam bp observed 25×; honest limits (no live Attack, count caps at 3)

Row: T-067 (rung 2). Coordinator: LoudDragon. Opening premise: T-056 bink-intro `wined3d_cs` SIGSEGV.
**Verdict: the wall is ENVIRONMENTAL, not shipped — DXVK 2.7.1 native d3d8+d3d9 on wine-11 removes it entirely.**

[DISCOVERY-BATCH 2 ready]

## THE WORKING DXVK STACK (recipe — two non-obvious keys)

1. **DXVK's d3d8.dll IMPORTS `Direct3DCreate9` FROM d3d9.dll** (objdump: `DLL Name: d3d9.dll`).
   `d3d8=n` ALONE crashes at `d3d8.dll+0x1305a` (`D3D8Interface::D3D8Interface` reads NULL vtable —
   wine's BUILTIN d3d9 rejects the DXVK-passed SDK version and returns 0). **d3d9 must ALSO be DXVK native.**
2. **On this wine-11 build, WINEDLLOVERRIDES env does NOT make d3d9 native** —
   `+module` trace: `get_load_order got hardcoded default` for d3d9 while the same env made d3d8 native.
   **The HKCU registry override WORKS**: `wine reg add 'HKCU\Software\Wine\DllOverrides' /v d3d9 /t REG_SZ /d n /f`
   (same for d3d8) → `Loaded C:\windows\system32\d3d9.dll: native`. Env `WINEDLLOVERRIDES` for d3d9: silently ignored.
3. Dropped `LIBGL_ALWAYS_SOFTWARE/GALLIUM softpipe` for this stack (they had caused a 32-bit
   `LLVM ERROR: out of memory` EGL fallback storm when wined3d stopped owning GL).

Full launch env: private prefix `/tmp/t056-wineprefix` + registry DllOverrides (d3d8,d3d9=n) +
`DISPLAY=:96` (Xorg-dummy stays — FindMode still reads its RandR metadata) +
`VK_DRIVER_FILES=<graphics-drivers-32bit>/share/vulkan/icd.d/nvidia_icd.json` + winedbg:31440 + t052-drive.
DXVK dlls = `nix build nixpkgs#dxvk` x32 output (d3d8/d3d9/dxgi) copied to prefix system32.
RTX 3090 32-bit ICD verified working under winevulkan (`+vulkan` trace enumerated the device).

## RESULT — depth evidence table (run I, canonical, 19:00 EDT)

| probe | result | meaning |
|---|---|---|
| **Bink wall** | **GONE.** Engine alive **712 s CPU (~12 min wall)**, utime still climbing at reap; zero SIGSEGV/SIGABRT/Fatal | The T-056 crash was wined3d-11's own cs-thread bug at bink upload, NOT shipped behavior and NOT a Wine-impossibility. DXVK substitutes exactly the failing subsystem. |
| **R0 @5 pumps** | vft `0x66856C`, `[+0x18]=0x00544A90` — 4th independent confirmation (2 different GL runtimes) | T-045 pin holds ACROSS runtime classes. Runtime-invariant by evidence now. |
| **R1** | draw `0x70C2`, state `[0x707C60]=0xBEF0C228`, gate `0x01` | boot-seeded as always; pairing currency unchanged (state-at-instant) |
| **R2** | world `0x07E164A0`, greg `0x0A1F7DE8`; seconds 0→**1** at pump3 | **frame-0 clock leap REPRODUCED under DXVK** = shipped behavior, runtime-invariant |
| **R3/R4 depth** | 35 funnel posts; per-entity counts: `0x0A6C6060 → [1,2,3]`, all others 1; **max observed 3 — at 12 minutes** | count>4 did NOT occur in the demo's scripted rounds even crash-free. **The cap is the DEMO SCRIPT, not the wall.** Live count>4 needs interactive play (input events), not a better GL stack. |
| **bp5 live `F3::Attack` (0x459f5d)** | **0 hits in 12 min.** Same as every wined3d run. | Demo script never issues script-native Attack without player input. The T-056 deviation is now CONFIRMED as a harness/input limit, not a crash-truncation artifact. Closing it requires input injection (new capability — flagged for routing, not built here). |
| **T-050 seam bp9 `0x5a94a0`** | **25 hits** (driver cap 24 +1), `this=0x0A1F0440` CONSTANT (one object, the 36-byte cls_0x5a9330 struct), `$eax` seq `0x0, then 0x445×10, 0x6243×10, 0x7380×4` (monotone-ish counters) | OBSERVED, not interpreted: entry leg fires repeatedly with a stable receiver and advancing numeric args. |
| **T-050 seam bp10 `0x5a955d`** | **0 hits in 12 min** | The advance leg (inside meth_0x5a94a0, guarded past mbr_0x8 byte flag) was NEVER taken in a full clean demo boot. T-050's re-entrancy question now has LIVE grounding: skip-leg YES, advance-leg NO under default scripted play. Gate decision belongs to the coordinator/KeenStorm semantics pass — but it is NO LONGER blocked by the environment. |
| Net | `[server]` answered challenge; `Client received packet with unknown id 7` chatter = live session traffic flowing | first time shipped net traffic was observed alive > minutes |

## Divergence caveat (mandatory per board row — is any datum wined3d-defined?)

- **R0/R1/R2/R3/R4/R4b engine-memory reads: runtime-independent** — proven: identical semantics under
  BOTH stacks (0x544A90 ×4 boots; frame-0 0→1s leap ×2 stacks; funnel owner-store 35/35 ×2 stacks).
- **Addresses moved** (world 0x5A86538→0x07E164A0, queue arena 0x080Fxxxx→0x0A8Fxxxx): heap layout is
  allocation-order sensitive; DXVK's init allocates differently. **T-031/T-068 pairing must key on
  ADDRESSES WITHIN THE SAME ENVIRONMENT, never across wined3d↔DXVK sets.** The value-semantics (constants,
  flags, backrefs, class vfts) ARE cross-environment stable — pair on those.
- FindMode still validated against :96 RandR in both stacks (DXVK d3d8 delegates mode enum through DXVK
  d3d9 which reads the same X11/XWayland display) — mode metadata remains an ENVIRONMENT datum, pin :96 recipe.
- Present pacing/threading: wined3d_cs thread is ABSENT by construction (DXVK has its own internal
  device thread). Any datum defined by PRESENT timing (none in the six rows) would need re-derivation.

## Routing outcomes
- **T-050: gate UNBLOCKED for observation** — seam legs now have live hit/miss counts above; interpretation pass needed.
- **T-051/T-068: environment decision = recommend DXVK-registry stack for LONG-RUN captures** (crash-free
  indefinitely) and wined3d stack for re-deriving the original six-row instants; pairing rule: same-env addresses, cross-env constants.
- **Live Attack / count>4: needs INPUT INJECTION (xdotool key send into the :96 window, or win32 SendInput
  harness)** — cheap next step, new mechanism class, NOT built this sitting (out of row scope).
- Bonus B-3/4/5 still unattempted (cursor-reload watchpoints not armed this run) — same 12-min window now available.

## Hygiene log
Private prefix only (registry edits confined to /tmp/t056-wineprefix); user-scoped nix builds
(dxvk, vulkan-tools — no devenv, no system change); recorded-pid+environ gates on all kills
(one more `pgrep -f` self-SIGTERM incident LOGGED — pattern-match kill now forbidden even in
inspection pipelines); managed pairs untouched; :96 + wineserver still up (reused by this run).
