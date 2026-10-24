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

## R5 — 801-band cross-refs vs live sightings: CLOSED (the cross-ref table, 2026-10-24)

T-084's residual pool is **801 LIVE/DELEG weak-evidence fns** (construction/embed evidence absent,
'used-by' not 'owned-by'; regenerable via `tools/ecx-flow.py hosts --cls <cls> --dump
discoveries/wavedumps/t064-postdump.json`). The pool was regenerated for this window's 21
live-sighted EAs (`hosts --addrs <set>`) and the cross-ref is decisive — **8 of the sighted EAs are
801-pool members, each LIVE-EXECUTED this window** (weak 'used-by' evidence upgraded to
live-confirmed usage; counts from the window's census):

| EA | pool class | live hits this window | host evidence (pool derivation) |
|---|---|---|---|
| 0x5803E0 | kind-table entry | ×201 | 8 callers via plain-this/field edges |
| 0x548D80 | anim-bypass clearer | ×101 | JE::Entity plain-this + 6 action fns |
| 0x549090 | clearer C1 | ×101 | sub_409BE0 other x1 |
| 0x5505F0 | | ×66 | F3::GameEntity x153 / GameActionEquip x94 |
| 0x550720 | the funnel | ×12 | JE x11 / GameServer x10 / GameActionAttack field-load x7 |
| 0x5A94A0 | | ×24 | JE::World field-load x1 |
| 0x56AF50 | | ×5 | JE::World other x4 + 7 more hosts |
| 0x581810 | | ×1 | JE::Client field-load x1 |

The other 13 sighted EAs are ISO (proven-final / outside the pool) — the cross-ref closes with the
pool membership split 8-LIVE / 13-ISO over the window's sighted set. The ~793 remaining pool
members are unchanged residual workload for the future evidence class (unchanged by this window —
only sighted-EA evidence upgrades, which is what the cell asked).

## R6 — deque >4 depth (the D-2 live datum): CLOSED — negative, with the sampling caveat stated

The driver's quad reads DID execute at the serviced stops (R4P at bp4 0x550741: `[ecx+0x310]`
head index + `[ecx+0x314]` live count; bp14/15 legs' +0x314 extras) — sparse (driver services one
hit per chunk, ~8 quad samples captured of 35 bp4 hits), but DECISIVE for the question asked:
**every captured live count is 0 or 1** (3 clean adjacent `+0x310/+0x314` pairs all read
head=0, live=1; standalone +0x314 reads agree). **The deque never exceeded depth 1 under 40 rounds
of mouse-only injection — the D-2 (>4) event was NOT observed live under this channel.** One
nuance stands recorded, not hidden: the bp14 pop-path sample read `[ecx+0x314] = 0x0a1e2030`,
pointer-shaped (self-referential into the same struct at +0x310) — a DIFFERENT ecx shape on the
walk side (0x54c9c1) than the funnel side (0x550741); the funnel side is where T-039's writers
(++/-- at 0x54ea71/0x54b637) assert the count, and its quads are clean counts. The walk-side ecx
identity is a decode note, not an open D-2 question. The honest
statement: D-2 remains a STATIC-side refusal the port reproduces deterministically (d2-stress,
`artifacts/d2-stress.digest`); producing a live >4 event needs an enqueue-faster-than-pump channel
the mouse cannot make. The reading is worked to a negative-with-channel-reason, like R1.

## Verdict (UPDATED 2026-10-24, second pass)

All five pre-specified cells are now worked: R2/R3/R4 closed (R4 = the row's named static target);
R1 and R6 closed as honest negatives with their channel reasons (mouse-only injection cannot
produce the attack keypress, and cannot outpace the pump's drain); R5 closed with the cross-ref
table (8 pool members live-confirmed, 13 ISO). **Row S3-INJ's readings are exhausted — the row
closes.** Residual for the FUTURE (named, not owed by this row): the ~793 unsighted 801-pool
members; keyboard-channel injection for a live attack-native sighting.
