# R-MVE-A — BIKi-v1 chunk table closed, trailing-quad roles named, fixture recipe proven (2026-10-23)

Closes PARK R-MVE-A ("wine-gated radinfo sitting"): the R-CODEC lane (SwiftMoon
01a16092) left the chunk anatomy OPEN because wine was absent then — a wine-capable
runner EXISTS now (proven: the sim-driver suite boots under wine 30+ times this
session, G-FLAKY 0719361), so the gate premise was testable. Verdict: the gate was
satisfiable but the instruments are NOT — receipts below — and the cells close
bytes-first instead, tool-independent.

## 1. The instruments refuse (receipts, each named)

| instrument | attempt | verdict |
|---|---|---|
| `radinfo.exe` (SDK tools/) | `wine radinfo.exe Movies/IPlogo.mve` | AVI-only: `err:avifile:AVIFILE_LoadFile : not an AVI!` ×3, silent exit 0 — wrong instrument for BIKi entirely |
| `smkinfo.exe` | `wine smkinfo.exe Movies/IPlogo.mve` | aborts on the drop's own stub: `Call ... to unimplemented function radutil.dll._GFileOpen@92` — the SDK drop's radutil.dll has a placeholder export table |
| `binkplay.exe` | with file; headless; under Xvfb (`xvfb-run -a`) | zero console output headless; under Xvfb opens and HANGS (>180 s, killed) — playback GUI, not a batch inspector |
| `bink.exe` / `binkconv.exe` | no-arg usage probe | silent (GUI/windowed) |

So "GATED on a wine-capable runner" was TRUE but UNHELPFUL: the SDK inspector exes
are windowed trial-grade tools. Everything below is raw-byte derived and reproducible
without them.

## 2. BIKi-v1 chunk table — CLOSED (both fixtures)

Files: `$OPENVB_ORIG_EXE/Movies/{BISlogo,IPlogo}.mve` (2,962,008 / 4,035,056 bytes).

Verified structure (all checks re-runnable from the printed numbers):

```
[0x00] 'BIKi' magic                          (= BINKMARKER4 'iKIB' per bink.h 1.0p:276,
                                              LE u32 read; 'even newer' tag carrying
                                              OLD-format frames — consistent with the
                                              BINKOLDFRAMEFORMAT 0x8000 flag, bink.h:299)
[0x04] u32 filelen                           == filesize - 8  (EXACT, both files)
[0x08] u32 frames                            185 / 230
[0x0c] u32 max_frame_size                    0xb2d8 / 0x6c2c
[0x10] u32 frames (duplicated)               == frames (exact, both) — replay-count guard
[0x14] u32 width                             640 (both)
[0x18] u32 height                            320 (both)
[0x1c] u32 framerate numerator               15 / 125000   ← THE differing quads
[0x20] u32 framerate denominator             1 / 8341      ← (see §3)
[0x24] 5 u32 anchor quads                    [0, 1, 0x10e00, 0x70005622, 0]
                                             BYTE-IDENTICAL across both files
[0x38] u32 × (nframes+1) offset table        first-frame-offset ... terminal; the LAST
                                             entry == filesize EXACTLY (end sentinel)
[0x38+4·(n+1) .. EOF]                        frame stream — NO per-frame chunk headers
```

**The chunk-table close: this is the OLD Bink frame format — frame payloads are
headerless.** A naive `[u32 size][u32 flags]` per-frame walk is WRONG (demonstrated:
went OOB within 3 frames). Frame i's bytes = `[tbl[i] .. tbl[i+1])` where `tbl[0]`
is the first frame offset and `tbl[n]` == filesize. Every size is a delta; the
sum of deltas == `filesize - tbl[0]` (verified: both files monotonic within the
offset run; terminal == filesize exact).

Frame starts are NOT 4-aligned (BIS frame0 @0x321) — byte-granular, matching a
raw-BCIQ/v1 stream fed straight to the decoder.

## 3. Trailing-quad roles — NAMED (the "two v1 revisions" resolved)

R-CODEC observed "trailing quads DIFFER between the two files = two v1 revisions,
roles named-open not guessed". Closed: the differing header quads @0x1c/@0x20 are
the **frame-rate numerator/denominator pair**:

- BISlogo: 15 / 1 = **15.0000 fps** (185 frames → 12.3 s)
- IPlogo: 125000 / 8341 = **14.9862 fps** (230 frames → 15.3 s)

Same nominal ~15 fps encoded by two encoder revisions as different rationals —
a rationalization artifact of the encoder version, NOT two distinct wire layouts.
The header's `frames`/`max_frame_size`/`filelen` quads differ trivially with content;
the 5 anchor quads are byte-identical, so no other structural revision exists in
these two fixtures.

## 4. Fixture-build recipe — PROVEN by surgery (binaries kept in /tmp, not committed)

Recipe (the exact script that ran green):

1. Read the real capture; locate the terminal (first u32 == filesize, ≤ 0x400 scan).
2. Choose KEEP frames; cut the payload at `offsets[KEEP]`.
3. **Rebase** every kept offset: `new = o - frames_start + new_hdr_len` where
   `new_hdr_len = 8 + 28 + 20 + 4·(KEEP+1)`. Keeping original absolute offsets is
   the surgery bug to avoid (caught here: terminal==size broke until rebased); the
   terminal entry must be the NEW filesize.
4. Patch: `frames` (0x08) = `frames-dup` (0x10) = KEEP; `max_frame_size` (0x0c) may
   be clamped to the max kept delta (not load-bearing for the invariants); `filelen`
   = new size − 8. Anchor quads copied byte-identical.
5. Validate the result against §2's invariants:
   magic, filelen==size−8, frames-dup==KEEP, terminal==size, offset-chain monotonic,
   anchors byte-identical, frames-field==entries−1, deltas-sum==body.

Demonstration output (BISlogo → `/tmp/bislogo-3frame.mve`, 14,831 bytes, 3 frames):
**ALL-GREEN: True** on every invariant above.

## 5. Honest limits

- The per-frame Bink codec BITSTREAM (BCIQ/v1 block internals) is NOT decoded here —
  that is R-CODEC's 1.5u container-spec deliverable, not this row's. This page closes
  the CHUNK/TABLE anatomy and the header quad roles only.
- The 5 anchor quads (0x10e00, 0x70005622) are byte-identical across both fixtures
  but their ROLES are unproven (candidates: keyframe interval, codec flag bitmask);
  named-open, not guessed. 0x10e00 = 69,632 = 640·320/2.94 — no clean derivation;
  left open.
- Audio-track presence inside the frame stream (R-CODEC: Bink-internal via
  BinkSetSoundSystem(&BinkOpenDirectSound,0) @0x5b530b) is not byte-verified here.
- Two fixtures = the whole shipped corpus for this cell; a third v1 revision could
  exist in user content.

## 6. Method + repro

Raw bytes only (python3 + struct). The §2 structure, §3 rate arithmetic and the §4
surgery all reprint from the same seated script — the fixture build REPROVES the
chunk table by consuming it (a wrong table grammar cannot pass `deltas-sum==body`
plus `terminal==size` on a trimmed file).
