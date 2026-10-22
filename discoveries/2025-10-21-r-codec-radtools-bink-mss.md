# R-CODEC — the two foreign runtimes: RadTools SDK audit, shipped-Bink container truth, and the audio-middleware settle

**Agent:** SwiftMoon `01a16092` · **Date:** 2026-10-21 · **Row:** R-CODEC (claim `a6f6e2a`) ·
**Method:** offline/raw over the md5-pinned demo bundle + the user-provided SDK drop
(`dependencies/radtools-master/`, 3.1 MB) + full linear disasm `/tmp/t078-disasm.txt`. Zero IDB
writes, zero src/ edits. Item-41 three-query posture on every negative (import table + ASCII
strings + archive content). PE parsing via `/tmp/peinfo.py` (self-written; two bugs found and
fixed in-session: swapped Names/NameOrdinals export tables, double-offset data directories —
results below are post-fix and cross-checked by hand-decoding at 0x43700/0x42cbc).
**WINE LIMITATION STATED:** no wine on this host tonight → SDK inspector exes (radinfo/smkinfo)
could not be RUN; every container claim below is from direct bytes or the shipped dll's own
markers, and the payload-chunk anatomy is left NAMED-OPEN rather than folklore-filled.

## VERDICTS (front)

1. **AUDIO: there is NO Miles Sound System anywhere near F3.** Decisive at three levels:
   (a) F3.exe import table has NO mss32 (full DLL list below); (b) ZERO strings matching
   `mss32`, `.asi`, `Miles`, `AIL`, `WaveOut` in the entire F3.exe; (c) the shipped bundle contains
   NO mss32.dll file at all (share/ ships exactly `binkw32.dll` + `granny2.dll` as foreign
   runtimes). **The audio port era needs ZERO MSS knowledge.**
2. **The shipped demo movies are OLD-FORMAT BINK ('BIKi' v1) containers wearing `.mve` names** —
   `Movies/BISlogo.mve` and `Movies/IPlogo.mve` both begin `42 49 4B 69 'BIKi'`; the "Interplay
   MVE" expectation is FALSE by bytes. The 1.5u runtime carries the `'BIKi'` marker ×2 internally
   and bink.h 1.0p exposes `BINKOLDFRAMEFORMAT` ("using the old Bink frame format — internal") —
   old-format support is consistent through the stack.
3. **Version delta confirmed and quantified:** SDK headers = Bink **1.0p (2000-06-26)**,
   Smack **3.2f**, mss.h **6.1a (2001-03-06)**; shipped runtime = binkw32 **1.5.21.0** (file
   version resource), granny2 **2.1.0.5** (resource — the coordinator's '2.1.0.5-era demo' claim
   VERIFIED exactly); SDK-bundled mss32.dll = banner **"Version 6.0i"** with a stale 3.0.0.0
   version RESOURCE — the SDK drop is internally inconsistent too (header 6.1a vs dll 6.0i).
   Where header and dll disagree, **the shipped export table outranks the header** (applied
   throughout; the one real collision is named in §1c).

## 1a. F3.exe import table (complete, 12 DLLs, post-fix parse)

WINMM(10) · KERNEL32(111) · USER32(35) · ADVAPI32(3) · **d3d8.dll(1: Direct3DCreate8)** ·
**granny2.dll(43)** · **DSOUND.dll(1: ordinal #11 = DirectSoundCreate)** ·
**binkw32.dll(9)** · WS2_32(17) · GDI32(14) · SHELL32(1) · ole32(3).
No smackw32, no mss32, no ddraw import (device path is d3d8-only + the T-083-era device wrapper).

## 1b. The nine Bink functions F3 uses = the entire movie-era API surface

`BinkOpen` (8), `BinkWait`, `BinkDoFrame`, `BinkNextFrame`, `BinkCopyToBuffer` (28),
`BinkGetError`, `BinkOpenDirectSound`, `BinkSetSoundSystem` (8), `BinkClose` — **9/9 present in
BOTH the 1.0p header and the 1.5u export table.** The full 1.5u table = 85 exports (incl. the
`BinkBuffer*` DirectDraw-buffer family F3 does NOT use — the Buffer API exists in the header too;
adjacency ≠ usage). The classic loop lives at **0x5b5990-0x5b5b3f**: `Wait→DoFrame→(surface from
the engine global [0x707CE4] via vft[+0xD8])→CopyToBuffer(surface,0,0,0,pitch…,0xf0000003
format-flag)→NextFrame`, opened at 0x5b566e `BinkOpen(path,0)` with the hard-coded paths
`movies\BISlogo.mve` @0x69c754 (push sites 0x588d25/0x58a6fb) and `movies\IPlogo.mve` @0x69c918
(push 0x58a6b4), error string `"Movies: %s"` @0x69c9e0. Consumer rule for the port's movie era:
this 9-function surface + the loop shape is ALL F3 semantics require; no GetSummary/GetRects/
BinkService calls exist anywhere (audio services run INSIDE the 1.5u dll).

## 1c. SDK-vs-runtime completeness table (claim-vs-truth, header = naming authority ONLY where both agree)

| claim surface | 1.0p header | 1.5.21.0 dll | ruling |
|---|---|---|---|
| Bink* prototypes | 61 | 85 exports | dll = truth for linkage |
| `BinkGetTrackLargest` | declared | **absent — shipped as `BinkGetTrackMaxSize`** | RENAMED API — the one true header lie; port from the export table |
| `BinkGDSurfaceType`, `BinkMacOpen`, `BinkOpenSoundManager` | declared | absent | GDI/Mac era retired in the win dll |
| `BinkDX8SurfaceType`, `BinkCopyToBufferRect`, `BinkSetMixBins[/Volumes]`, `RADSetMemory`, `RADTimerRead`, `radmalloc/radfree`, `YUV_blit_*` ×24, `YUV_init` | absent | exported | 1.5-era additions + internal blitters the 2000 header predates |
| `BinkSoundUseDirectSound/WaveOut/Miles` | `#define` macro aliases | not exports | header's SOUND RECIPE: `BinkSetSoundSystem(BinkOpenDirectSound, param)` — exactly what F3 executes (below) |

## 2. The audio path, fully attributed with cited call sites

- **Movie sound:** `BinkSetSoundSystem(&BinkOpenDirectSound, 0)` at **0x5b530b** — arg1 loaded via
  `mov eax,ds:0x647338` at 0x5b5302 (the IAT SLOT ADDRESS of BinkOpenDirectSound = the macro's
  first parameter verbatim), arg2 = 0 (ebx). This is bink.h's
  `#define BinkSoundUseDirectSound(lpDS) BinkSetSoundSystem(BinkOpenDirectSound,(u32)lpDS)` with
  lpDS=0 — the runtime then resolves DirectSound internally (binkw32 statically imports ONLY
  WINMM waveOut* as its always-available fallback — USER32/GDI32/KERNEL32/WINMM are its four
  import DLLs; no dsound import in the dll either: it LoadLibraries at runtime).
- **Game volume control:** winmm **mixer** band at 0x53a412 (`mixerOpen`), 0x53a446
  (`mixerGetLineInfoA`) — the GUI options cluster `btnMasterVolume/btnMusicVolume/btnSoundEffects/
  btnSpeechVolume` @0x659644-0x659662 sits exactly on this epoch's purpose; **mci** used twice
  (0x53a65e, 0x53a6f0) with first observed command **0x808 = MCI_SET** on a device handle.
- **DirectSoundCreate (#11 static import) is NEVER CALLED:** its only image-wide reference is as a
  DATA LITERAL — pointer `0x00647010` stored at **0x5fe038**, the last entry of a pointer table
  (0x5fd5f1..0x5fd669 stride-0xa0 entries) terminated by 0xCC padding; the apparent `ff 25` "jmp
  thunk" at 0x5fe036 is a LINEAR-DISASM MISALIGNMENT artifact across that data (proved by the
  dword-aligned read). Shipped-linkage residue, same family as the R-VFM-2 0x4F9A00 precedent.
- **Doc-claim correction (item 43 turned on a prior doc note): 'CSound reads WAV/ACM tags' does
  NOT hold against F3.exe bytes.** 'ACM' = ZERO hits; every 'WAV' hit is **VFX vocabulary**:
  `VFXWAVE_SINE/SAW/SQUARE/NONE` enum strings @0x6795d1-0x6795d8 and the VFX schema pair
  `WAVFile`/`PresetFile` @0x67960f-0x679612 (the R-VFM enum family!). The one inline `RIFF`
  immediate in code sits at ~0x4c6e80s (UNCLASSIFIED, named-open, 28 bytes from a
  `push 0x04; lea; e8` resource-call pattern). Port-era consequence: **no ACM codec, no MSS,
  no WAVEFORM structure needed**; the only audio-adjacent knowledge the port ever needs is (i) the
  Bink SetSoundSystem recipe above and (ii) mixer/mci volume control if the options UI is ever
  paired.
- **Demo audio payload census (negative, three-query):** resource.rht (346,410 B) has ZERO real
  audio payloads — the two '.ogg' and one 'RIFF' raw hits are **substring false positives**
  ('T**ogg**le', 't**ogg**le', 'Sheri**ff**' under case-insensitive scan). What the .rht DOES carry
  at ~0x2453f is a **sound-NAME table** (`MonitorBeep`, `SlidingDoorClose`, `SlidingDoorOpen`,
  `Toggle`) sitting among texture-group names — sound resources exist as identities with no
  located payloads in this demo; **named tail for the audio-content era, not a gap in this row.**

## 3. Container truth for the shipped movies (static-verified; chunks left open honestly)

BISlogo.mve (2,962,008 B) / IPlogo.mve (4,035,056 B), both 'BIKi' v1:
`u32 magic 'BIKi'` · `u32 filelen = filesize−8` (BIS 2,962,000 ✓ exact) · `u32 frame_count` ·
`u32 crc` (BIS 45784) · `u32 frame_count AGAIN` (equal in both files) · `u32 width,height`
(640×320 in both) · trailing quad @0x28: **BIS [15,1,0,1] vs IP [125000,8341,0,1]** — the two
files carry DIFFERENT old-format revisions (if the field is ms-per-frame, BIS=15ms≈66fps is
plausible and IP's 125000 cannot be — variants, roles named-open, NOT guessed); `u8`-ish tail
`01` in slot 3 shared by both (revision marker candidate). Frame/audio payload chunks were NOT
decoded statically (compressed tables; the honest tool for this is radinfo.exe on Windows — no
wine tonight). **Consumption rule for the movie era: the container layout UNDER the shipped 1.5u
runtime is what matters, and F3 proves the 9-API runtime path IS the decode — a port can consume
these exact two files through a vendored/compatible Bink, or replay through the shipped dll; a
from-scratch v1-chip decoder is a SEPARATE, larger era the bytes here say nothing about yet.**

## 4. SDK inventory (audit table, abbreviated)

`include/`: bink.h (1.0p), mss.h (6.1a — naming authority for the Miles API F3 does NOT use),
rad.h, smack.h (3.2f — Smack NOT imported by F3; header kept for SDK tools). `lib/`:
binkw32/smackw32/mss32 .lib. `samples/`: examw32.c (Bink classic+buffer loop), exammss.c (Miles
player), examddp.c. `tools/` 28 entries incl. **inspector/playback trio (binkplay, radinfo,
smkinfo)** for human decode on Windows, **producer chain (binkmake.dll + binkconv + binkmix +
smkcompi + smackscw) — as valuable in reverse: the port's movie-era FIXTURES can be BUILT from
raw assets with these**, plus the Miles stack (mss32.dll 6.0i-banner/3.0-resource, mp3dec.asi
plugin, milesp32/milesstu configurators, vort_dls.dll DLS synth — all SDK-side, none shipped in
the demo bundle).

## 5. Consequences for the port eras (named, no invention)

1. **M-001/movie era:** 9-API surface (signatures in bink.h, linkage truth in the 85-export
   table, `GetTrackMaxSize` rename noted) + the 0x5b5990 loop shape + `[0x707CE4]`-surface blit
   target with the `0xf0000003` format flag. Fixture movies: the two shipped 'BIKi' `.mve` files
   verbatim; NEW fixtures buildable via binkmake/binkconv on any Windows box.
2. **Audio era:** needs NOTHING from Miles. Volume = mixer lines; movie audio = Bink internal;
   if game SFX ever pair, start from the .rht sound-name table (MonitorBeep et al.) and the
   0x4c6e80s RIFF probe — both cited here, neither resolved by this row.
3. **Content miner:** `.mve` ≠ MVE is now corpus law; a claim-vs-truth note belongs on any older
   doc saying "Interplay movies" (this row is LIST-not-edit: relayed, T-079 lane applies).

## 6. Posture & closure limits

Read-only honored; every negative is 3-query; the two PE-helper bugs and the 0x5fe036 misalignment
trap are reported as method failures caught, not hidden; v1 chunk anatomy + trailing-quad field
roles + sound-payload location are NAMED-OPEN with the exact missing instrument (wine + radinfo,
or any Windows box + the SDK tools in-tree). Self-gate: every version number above read from
RESOURCES or BANNERS directly (1.5.21.0 / 2.1.0.5 / 6.0i / 1.0p / 3.2f / 6.1a), zero inherited
from prose. Successor proposal per contract §3: **R-MVE-A** (small, gated): 'run radinfo/smkinfo
under the boot's wine when a wine-capable runner exists — v1 chunk table + trailing-quad roles
close in one seating; the same session produces the port's movie-fixtures recipe'. Otherwise
DONE-NO-SUCCESSOR — the audio settle is terminal for middleware purposes.
