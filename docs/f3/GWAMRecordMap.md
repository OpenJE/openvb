# GWAM record map (working, stream order → builder locals → copy-ctor cells)

Status: DRAFT extraction from the creature-side reader `0x40dba0` (sole reliable
decompile of the shared builder; weapon twin `0x4148a0` renders differ by the
4-byte `push ebp @0x40dbc2` shift only, per verifier-certified pair-diff).
Every offset below is RE-EMITTED from instruction text / the saved decompile's
own frame comments, not from printed decimal. Builder-base = `&v8` (creature
esp+0x14; weapon esp+0x10). Copy-ctor = `sub_40A980` walk order.

## Stream order (reads in sequence; sizes are push immediates @EA)

| seq | size | creature local | builder off | gate | note |
|-----|------|----------------|-------------|------|------|
| init| -    | v8 (type)      | +0x00       | -    | `mov [esp+14h],3` @0x40dc0d (RV2-era arms mutate; readers pin 3) |
| r1  | 4    | v9             | +0x04       | always | anim-mode id (feeds the 56..104 ladder: `switch(v9)`) |
| r2  | 4    | v11[0..3]      | +0x0C       | version!=0 (v4) | extension block head |
| r3  | 4    | v10            | +0x14?      | version!=0 | |
| ... |      |                |             |      | |

UNRESOLVED IN THIS DRAFT (the honest gaps; do not port over them):
1. The creature locals' exact frame offsets between +0x14 and +0x4C must be
   tabulated from `v10,v12..v19,v20[5]` printed offsets before use - the rows
   above are PARTIAL; `v10` and `v11` printed as esp+0x28 / esp+0x20 (not
   monotonic with read order), which means the builder is NOT dense in stream
   order at every gap. Gaps are where the copy-ctor's `+4k` walk supplies the
   truth: cell k reads param_1+4k, so cell order = RECORD order, and the
   LOCAL↔CELL join is the map proper.
2. u8 r-read @0x40dcf0-era → `v20[4]=a2!=0` flag placement vs the ctor's
   byte-cells +0x38..+0x3B (4 bytes; +0x39=claimed, +0x38/+0x3A/+0x3B unnamed).
3. Extension block (version!=0): `v11(4),u8→v21, v24(4), u8→v22, u8→v23` - the
   consumed(XOR)thrown pair = v22/v23 (FatalError @0x40de73 reads both).
4. Version-gated: >=2 → v25 (+0x54 region; anim-override cell == record +0x40
   ladder target via the ctor's 3-u32 group); >=3 → v26; >=4 → v27 STRING via
   `sub_49C8D0` = ported `StreamReadString_0x49c8d0`; >=5 → v31,v32 two u8s
   (ctor trailing +0x64..+0x66 triple; RV2 presets (1,0,0)).

## Certified anchors this map must reproduce
- `sub_40A980` copy walk: 14 u32 (+0..+0x34), 4 bytes (+0x38..0x3B), 3 u32
  (+0x3C..0x44), SSO string (+0x48), 3 bytes (+0x64..0x66). [verifier-stamped]
- kind cell = +0x40 (dual-frame: glyph store esp+0x54 vs base 0x14; weapon
  0x50 vs 0x10). [verifier + my dumps]
- claimed = +0x39 (second byte, payload base = node+8 proof @0x40e1fa/0x40c430).
- counter = +0x3C, kind-3 += 3 dual-site (A2 @0x40e248-5b, B2 @0x41506c).

## Metal facts (verifier raw-disasm 0x40dc01-0x40dd17 + my confirming window)
COORDINATES FIRST, because the seats' numbering differs and this exact confusion
already bit the v10/v11 line: BUILDER-BASE = var_78 @ esp+0x14 (this doc's
coordinates); the verifier's message measured from esp+0x0C (+4 below builder
base) - subtract 4 to translate. On THIS doc's axis, from my window at her EAs:
- Unconditional stream reads ASCEND: +0x04 @0x40dc01, +0x08 @0x40dc42, then the
  sequence STEPS OVER +0x0C/+0x10 to +0x14 @0x40dc6e. So order is clean UNTIL
  THE GATE: the if(v4) extension block reaches BACK to the skipped +0x0C/+0x10
  region (var_6C[8]) - non-monotonicity exists exactly (1) in the decompiler's
  v-numbering layer and (2) at that gated reach-back, NOT in the unconditional
  stream. My commit direction ("v10 read before v11") was right about SEQUENCE
  but attached it to a general disorder that does not hold - the verifier's
  correction stands and this line is the corrected form.
- CONFIRMED, stronger form: cells +0x0C/+0x10 receive NO unconditional read -
  they are gated fills only (type cell +0x00 is init, never read). A
  stream-order map would INVENT reads at those coordinates. The draft's
  r3 = "+0x14?" row is now RESOLVED: +0x14 IS read unconditionally (third
  read); the "?" it pointed at lives one cell down.
- Per-function frame renders still need reconciliation (creature prints
  var_6C[8] for the gated pair; weapon prints single-size labels at its 4-shifted
  base) - named port-blocking leg below.
- RULE ENDORSED WITH RAW EVIDENCE: read sequence != cell order (at the gate);
  cell order != cell EXISTENCE (gated fills). The ctor's fixed +4k walk is the
  ONLY join. Unnamed cells stay offset-named, never guessed.

## Port-blocking before this file turns into code
Complete rows r2..r10 with printed offsets from the creature decompile (in-session
context or re-decompile 0x40dba0), join LOCAL→CELL by ctor walk order, name any
cell whose consumer role is already certified (range +0x2C per batch-B
GetWeaponRange two-hop), and mark unnamed cells by offset only - never by guess.

Census/provenance: GameWeapon.hpp family block; commits 61bf2ea (roster/trio),
59cf672 (removal correction), verifier thread stamps cited per fact there.
