---
status: delivered 2026-10-23 (splitseat persisted the deliverable page from these Notes:
discoveries/2026-10-23-r-codec-foreign-runtime-deliverable.md — verdicts credited, unchanged in
substance; open tails = per-frame bitstream (only for in-proc decoder reimpl), anchor-quad roles,
.rht sound-name payloads, inline code-RIFF ~0x4c6e80)
type: research (offline analysis of in-tree artifacts; discoveries/ DIRTY-until-checkpoint)
depends: nothing — everything needed is now in the tree
---

# R-CODEC — The two foreign runtimes: RadTools SDK audit + Bink 1.5u container spec

## Scope
Three sub-parts, all byte+doc grounded, one deliverable:

1. **RadTools SDK audit** (`dependencies/radtools-master/`, 3.1 MB, user-provided 2026-10-21):
   version-stamp the include set (Bink_Rev / MSS version macros vs the shipped binaries: binkw32.dll
   **1.5u**, granny2 2.1.0.5-era demo), inventory tools/ (binkplay/binkconv/smkinfo/radinfo/radutil —
   the *inspector* exes are as valuable as headers: they decode containers the same way for a human),
   and state completeness honestly (1.0 SDK vs 1.5u runtime = which APIs exist, which came later).
2. **Bink container spec, 1.5u** (`tools/scripts/bink15_facts.md` = coordinator seed facts, all verified in-tree: 4 SDK headers incl **mss.h**, 150+ inspector exes, the 1.5u BinkBuffer* export family, the open MSS-or-plain-dsound audio question): file layout,
   frame/kaput block structure, audio track format — cross-checked against the SDK docs where the
   version allows, against the shipped dll's export table where it doesn't. Deliverable: the lift-ready
   spec in R-VEG-E's style — a wire-format page the sitting's movie-era and any future content miner consume.
3. **The audio question (MSS?)** — F3 imports **DSOUND**, no mss32 in the import table, yet a
   Miles-Sound-style ASI/ASI-plugin tree exists in RadTools and F3's CSound epoch reads 'WAV/ACM' tags.
   Settle empirically: what does the shipped demo's audio path actually load (exports referenced by
   CSound/CPlayback, dsound buffer feed sites vs any plugin-load), and does the dsound-era F3 use **no**
   middleware at all? This determines whether the audio port era needs MSS knowledge or pure dsound.

## Method law (standard, restated)
Offline/raw over decompiler prose; export tables and bytes first; SDK docs are the *naming authority*
layer on top (their exact role, settled today); item 41 three-query answers for any negative; named-
open over guessed on version mismatches. Read-only; zero IDB writes.

## Acceptance
Discoveries file: SDK audit table (header/API/version), Bink 1.5u container spec (field-level,
consumer-ready), MSS question answered with cited call-sites. One page consumable by M-001's movie
era and the audio port alike; a claim-vs-truth table if the 1.0 SDK lies about 1.5u anywhere.

## Notes (SwiftMoon, research-complete 2026-10-21)
GATE-VERDICTS all bytes-first: (1) AUDIO SETTLED — NO MSS at three levels (import table, zero
strings mss32/.asi/Miles/AIL/WaveOut, no mss32.dll shipped); movie audio = Bink-internal via the
header's EXACT macro recipe BinkSetSoundSystem(&BinkOpenDirectSound,0) @0x5b530b; game volume =
mixer band 0x53a4xx (pairs btn*Volume GUI tags) + mci MCI_SET 0x808; DirectSoundCreate (#11) is a
DEAD import — sole ref = data literal at 0x5fe038 in a pointer table ending in 0xCC padding (the
'ff 25 jmp thunk' at 0x5fe036 proved a linear-disasm misalignment, dword-aligned read kills it).
(2) MOVIES ARE OLD-FORMAT 'BIKi' v1 WEARING .mve NAMES (both files; filelen=filesize-8 exact;
frame-count duplicated; 640x320; trailing quads DIFFER between the two files = two v1 revisions,
roles named-open not guessed); 'Interplay MVE' expectation refuted; 1.5u runtime +
BINKOLDFRAMEFORMAT consistent. (3) VERSIONS from resources/banners directly: binkw32 1.5.21.0,
granny2 2.1.0.5 (claim verified exact), SDK = bink.h 1.0p/smack 3.2f/mss.h 6.1a vs bundled
mss32.dll banner 6.0i w/ stale 3.0.0.0 resource (SDK internally delta'd too). (4) Header-vs-export
diff: ONE true lie — BinkGetTrackLargest shipped as BinkGetTrackMaxSize (rename trap); 30+
1.5-era adds; F3's 9 used APIs present in BOTH. (5) Doc-claim correction: 'CSound reads WAV/ACM'
refuted — all WAV hits are VFXWAVE_* + WAVFile/PresetFile schema vocabulary; ACM zero; inline
code-RIFF at ~0x4c6e80s unclassified; .rht 'RIFF/.ogg' hits = substring false positives
(SheriFF/Togggle); .rht carries a sound-NAME table (MonitorBeep, SlidingDoor*) with payloads
unlocated — tail for the content era. Method failures caught+reported: my PE helper had swapped
export tables + double-offset data dirs (fixed, hand-verified at 0x43700/0x42cbc); wine absent ->
chunk anatomy left OPEN with the exact missing instrument named. Deliverable STAGED per item 55
(stage-or-commit), never committed by this lane. Successor proposed: R-MVE-A (wine-gated radinfo
sitting). Row flip relayed.
