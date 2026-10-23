# R-CODEC — foreign-runtime deliverable: RadTools SDK audit + Bink container (file-level) + the MSS question (SETTLED)

**Provenance:** research-complete by SwiftMoon `01a16092` 2026-10-21 — the GATE-VERDICTS
below are THEIR bytes-first receipts, recorded in `tasks/R-CODEC-radtools-bink-mss.md`
Notes but never persisted as this deliverable page (the lane's "staged" work was lost;
the research survived only in the task file). This page LANDS it, credited, unchanged in
substance. The BIKi-v1 file anatomy (§2) is the splitseat companion delivered by
R-MVE-A (`discoveries/2026-10-23-r-mve-a-biki-v1-anatomy.md`, a4580c6). No battlemage
models used by either lane.

## 1. RadTools SDK audit (dependencies/radtools-master/, 3.1 MB user drop)

| component | version (bytes-first) | notes |
|---|---|---|
| shipped `binkw32.dll` | **1.5u** (banner 1.5.21.0) | the runtime the game actually loads; DirectDraw-buffer path (BinkBuffer* family) |
| SDK `bink.h` | **1.0p** (2000-06-26) | predates the shipped runtime |
| SDK `smack.h` | 3.2f | Smacker side |
| SDK `mss.h` | 6.1a | vs the SDK's OWN bundled `mss32.dll` banner 6.0i (stale 3.0.0.0 resource) — the SDK is internally delta'd too |
| shipped `granny2.dll` | 2.1.0.5 | claim verified exact |
| SDK `lib/` | binkw32.lib, mss32.lib, smackw32.lib | link targets for runtime comparison |
| SDK `tools/` | ~150 exes incl binkplay/binkconv/smkinfo/radinfo/radutil + mss32.dll + mp3dec.asi | inspector exes are windowed/trial-grade: **all refuse headless** (radinfo AVI-only; smkinfo stubbed `radutil._GFileOpen@92`; binkplay silent headless, hangs under Xvfb — receipts in the R-MVE-A page) |
| SDK `samples/` | examddp.c / exammss.c / examw32.c | the decode-side usage patterns, human-readable |

**Completeness honesty (1.0 SDK vs 1.5u runtime):** the headers predate the runtime;
export-table facts outrank docs where they disagree. Header-vs-export diff: **ONE true
lie** — `BinkGetTrackLargest` is shipped as **`BinkGetTrackMaxSize`** (rename trap);
30+ 1.5-era exports exist that the 1.0p headers do not declare; F3's 9 used APIs are
present in BOTH.

## 2. Bink container, file-level — consumer-ready (movie era + content miner)

The wire anatomy is closed and proven in the companion page
(`2026-10-23-r-mve-a-biki-v1-anatomy.md`): 8-byte prefix (magic `BIKi`/BINKMARKER4 +
filelen==size−8 exact) → 7-dword header (frames / max_frame_size / frames-dup / 640 /
320 / rate-num / rate-den — the two shipped fixtures are `15/1` and `125000/8341`,
same nominal ~15 fps from two encoder revisions) → 5 byte-identical anchor quads
(`[0, 1, 0x10e00, 0x70005622, 0]`, roles named-open) → (nframes+1) offset table whose
terminal entry == filesize exactly → **headerless old-Bink frame payloads sized purely
by offset deltas** (a per-frame `[size][flags]` walk is demonstrably wrong).

**Named successor (honest limit):** the per-frame codec BITSTREAM (BCIQ/v1 block
internals) is NOT decoded and is deliberately NOT in scope here — the movie era
consumes playback via `binkw32.dll` 1.5u (the game's 9-API surface), so bitstream
knowledge is only needed if someone re-implements the decoder in-proc. Fixture
surgery (the recipe in the companion page) is the content-mining tool that does NOT
need the bitstream.

## 3. The MSS question — SETTLED (SwiftMoon's three-level verdict, verbatim substance)

**No MSS.** Three independent levels:
1. F3's import table = DSOUND, no `mss32`.
2. Zero strings for `mss32` / `.asi` / `Miles` / `AIL` / `WaveOut` anywhere shipped; no mss32.dll shipped.
3. Movie audio = **Bink-internal** via the header's exact macro recipe
   `BinkSetSoundSystem(&BinkOpenDirectSound, 0)` @0x5b530b.

Game volume = mixer band 0x53a4xx (the `btn*Volume` GUI-tag pairs) + `mci` `MCI_SET`
0x808. `DirectSoundCreate` (#11) is a **DEAD import**: sole ref = data literal at
0x5fe038 in a pointer table ending in 0xCC padding (the `ff 25` "jmp thunk" at
0x5fe036 was a linear-disassembly misalignment — a dword-aligned read kills it).

**Doc-claim correction:** "CSound reads WAV/ACM" is refuted — all WAV hits are
`VFXWAVE_*` + `WAVFile`/`PresetFile` schema vocabulary; ACM: zero. Inline code-RIFF
at ~0x4c6e80s unclassified. `.rht` "RIFF/.ogg" hits are substring false positives
(SheriFF / Togggle). `.rht` carries a sound-NAME table (MonitorBeep, SlidingDoor*)
with payloads unlocated — content-era tail.

**The audio port era needs pure dsound + Bink-internal audio. No MSS middleware
knowledge required.**

## 4. Claim-vs-truth table

| claim (docs/folklore) | truth (bytes) |
|---|---|
| movies are "Interplay MVE" | refuted — old-format `BIKi` v1 wearing .mve names; `filelen==size−8` exact; frame count duplicated |
| `radinfo`/`smkinfo` can inspect them | refuted headless — radinfo is AVI-only; smkinfo dies on the drop's stubbed radutil |
| CSound reads WAV/ACM | refuted — VFXWAVE_*/schema vocabulary only; ACM zero |
| `.rht` contains RIFF/OGG | substring false positives (SheriFF/Togggle) |
| BinkGetTrackLargest (per 1.0p header) | shipped as BinkGetTrackMaxSize — the one true rename trap |
| DirectSoundCreate import is used | dead — data literal at 0x5fe038, misalignment artifact |
| MSS present (mss.h in SDK) | absent at all three levels — pure dsound + Bink-internal |

## 5. Method + open tails

Method law as scoped: offline/raw over decompiler prose; export tables and bytes
first; SDK docs as the naming-authority layer on top; item-41 three-query answers
for negatives; named-open over guessed on version mismatches; read-only, zero IDB
writes (both lanes).

**Open tails (successor record):** (a) per-frame Bink bitstream — only for in-proc
decoder re-implementation; (b) anchor-quad roles (0x10e00, 0x70005622) — named-open;
(c) `.rht` sound-name payloads — content era; (d) inline code-RIFF ~0x4c6e80 —
unclassified.
