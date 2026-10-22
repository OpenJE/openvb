# Bink/RAD seed facts — VERIFIED IN-TREE (coordinator scan 2026-10-21; R-CODEC task file; starting material, not conclusions)

**What the user drop contains** (`dependencies/radtools-master/`, 3.1 MB):
- `include/`: **bink.h, mss.h, rad.h, smack.h** — the SDK carries **Miles Sound System headers**, so the
  audio-middleware question is now answerable API-to-API, not by folklore.
- `tools/`: binkplay / binkmake.dll / vic32.dll / radutil.dll / smkinfo / radinfo / binkconv /
  soundpr.exe (+ ~150 more) — the inspector/playback exes are human-readable decoders AND link targets
  for runtime comparison.
- Version stamps NOT yet extracted from headers (BinkRev constant pending — R-CODEC step 1).

**The shipped runtime** (`.../f3demo/binkw32.dll`, FileVersion **1.5u**):
- export family sample (first read): `BinkBuffer*` set (`BinkBufferOpen/Close/Lock/Unlock/Blit/Clear/
  SetDirectDraw/SetHWND/SetResolution/SetScale/GetError`) — the DirectDraw-buffer playback path;
  full ordinal/name table pending (R-CODEC step 2 seeds from it).
- SDK-vs-runtime version delta (**1.0 drop vs 1.5u shipped**) = the audit's first question: where the
  headers predate the runtime, export-table facts outrank docs; where they agree, docs are naming
  authority.

**Audio question (open, cheap):** F3's import table = DSOUND, **no mss32** — but SDK ships mss.h and
tools carry mss32.dll/mp3dec.asi plugins. Candidate answers to settle at call sites: F3 does plain
dsound buffers (CSound epoch's WAV/ACM tags fit that), OR loads MSS plugins privately via its own
resource system. Census targets named in R-CODEC step 3.
