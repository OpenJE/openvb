# S3-INJ deep-input readings — the S3 window's five pre-specified cells (2026-10-24, splitseat)

Source: the S3 injection window `run-window 2026-10-24T00:25:45Z | head=c8432a8 | drvrc=0`
(`/tmp/sitting-redquartz/s3a-winedbg.log`, 314 KB, 1005 breakpoint stops; `values-s2.log`, 543 KB).
Recipe: foreground window + xdotool 40-round mouse injection, zero SIGSEGV. This note works each of
the five readings the S3-INJ row pre-specified; two cells are answered, three residuals are named
with their next concrete step — nothing silently dropped.

## R1 — bp5 live F3::Attack sighting (0x459f5d): ZERO hits — negative datum, recorded

Breakpoint 5 (0x459f5d, the F3::Attack/A_JustAttack door band) **never fired** across the whole
window while 1005 other stops logged 200+ event-kind dispatches under 40 rounds of synthetic mouse
input. Consistent with the banked precedents: T-037 (script natives are a separate door from the
packet/script flow), T-071 run H ("the native fires — run H's never-fired observation stays
un-explained"), and the mouse-only input channel cannot produce the attack keypress. The datum is
the row's honest answer: **under mouse-only synthetic injection the attack native is not exercised;
sighting it live requires keyboard-channel injection, which this window did not attempt.**

## R2 — T-050 re-check at 0x5a955d (bp10): ZERO hits — falsification CONFIRMED live

bp10 (0x5a955d, T-050's predicted-live path) never fired under sustained injection. The static
falsification (bp10/T-050 note, commit `9167884`) predicted the path is not on the live input chain;
the live window confirms it dead. Cell closed.

## R3 — sustained kind-stream numerics: LIVE-traced, census landed

The event-kind band's five armed EAs fired 201/60/60/39/60 times respectively:
`0x5803e0 ×201 → 0x580417 ×60 → {0x5804e0 ×60 | 0x580435 ×39} → 0x58059a ×60`. The 201-vs-60 ratio
shows the table entry (0x5803e0) is hit ~3.4× per dispatched leg — the entry sees every input
sample, the legs see the DISPATCHED subset. This is the sustained kind-stream decode the row asked
for; the third leg's alternation (0x5804e0 vs 0x580435, 60 vs 39) is the live branch pair.

## R4 — SetCursor callers: live-sighted ×20, and the THIRD caller identified STATICALLY (closed)

Live: `0x437ab0 (GameCursorManager::SetCursor, vft 0x6550B4 slot 5) ×20`; every hit's post-stop read
returned the SAME string object `0x52c0398` — the default-cursor object every time — so under
mouse-only injection only the default-branch caller fires (beam cursor never demanded).

Static (new finding this note): the image references the `GUI_DefaultCursor.tga` literal
(`VA 0x6561ec`, fileoff 0x2561ec) from exactly TWO push sites:
- `0x58039c-0x5803a7` — the known T-061/T-062 facade default branch, and
- **`0x43964C-0x439652` — the THIRD caller: `CEngineInterface::meth_0x4395f0`** (fn spans
  0x4395f0..0x4396b0, bounded by `GameInterfaces::dtor_0x4396b0`). Decoded:
  `mov eax,[ecx+0x30] / cmp eax,1 / je +4 / test eax,eax / jne +0x15 / mov eax,[ecx] /
  push 0 / push 3 / push 2 / push 'GUI_DefaultCursor.tga' / call [eax+0x14]` — i.e.
  **SetCursor("GUI_DefaultCursor.tga", 2, 3, 0) via slot 5, taken when [this+0x30] is 0 or 1**
  (skipped when ≥ 2). Same arg shape as the facade's default branch — a second DefaultCursor
  demand site in a DIFFERENT class (an interface-side fallback), not a duplicate of it.

## R5 — 801-band cross-refs vs live sightings: INPUT recorded, membership check = named residual

T-084's residual pool is **801 LIVE/DELEG weak-evidence fns** (construction/embed evidence absent,
'used-by' not 'owned-by'; regenerable via `tools/ecx-flow.py hosts --cls <cls> --dump
discoveries/wavedumps/t064-postdump.json`). This window's live-sighted EAs (the cross-ref INPUT):
0x48ccf6 ×3, 0x552bb5 ×5, 0x550720 ×12, 0x550741 ×35, 0x56af50 ×5, 0x581810 ×1, 0x5a94a0 ×24,
0x54c833 ×10, 0x54c864 ×101, 0x549090 ×101, 0x54c9c1 ×126, 0x54c92a ×3, 0x548d80 ×101,
0x46f290 ×35, 0x5505F0 ×66, 0x5803e0 ×201, 0x580417 ×60, 0x5804e0 ×60, 0x580435 ×39,
0x58059a ×60, 0x437ab0 ×20. Whether any of these owners sit in the 801 pool (a live sighting
upgrades weak-evidence to live-confirmed) needs the pool regenerated per class — the residual.

## R6 — deque >4 depth (the D-2 live datum): sampling SPARSE — residual with the fix named

The driver's +0x314 extra reads (`x/1wx $ecx+0x314` at bp14/15) executed for **one serviced stop in
126** — the driver services one hit per chunk, so bulk-hit stops logged no extras. That one sample
read `[ecx+0x314] = 0x0a1e2030`, POINTER-shaped, not a 0-4 count — so the live layout nuance
(`Entity+0x314` = the +0x304 deque's `mbr_0x10`; what the pointer at that slot means when the deque
is empty/nonempty) needs a proper quad read (`+0x308/+0x30C/+0x310/+0x314`) at a serviced stop
under sustained input. Next window's driver leg must sample EVERY stop (or lower the caps) and read
the quad — that is the cell that produces the D-2 live datum.

## Verdict

Row S3-INJ stays OPEN: cells R2, R3, R4 closed this sitting (R4's third-caller identity was the
row's named static target); R1 negative-with-channel-reason recorded; R5 and R6 residuals carry
their next concrete steps (pool regeneration; quad-read driver leg). No strike until all five
pre-specified cells are answered.
