---
status: open
assignee: SwiftMoon (seated by standing order; decline-then-reseat logged as the system working; re-decline honored once, then re-slotted)
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
