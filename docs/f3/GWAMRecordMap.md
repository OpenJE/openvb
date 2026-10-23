# GWAM record map (working, stream order → builder locals → copy-ctor cells)

Status: DRAFT extraction from the creature-side reader `0x40dba0` (sole reliable
decompile of the shared builder; weapon twin `0x4148a0` renders differ by the
4-byte `push ebp @0x40dbc2` shift only, per verifier-certified pair-diff).
Every offset below is RE-EMITTED from instruction text / the saved decompile's
own frame comments, not from printed decimal. Builder-base = `&v8` (creature
esp+0x14; weapon esp+0x10). Copy-ctor = `sub_40A980` walk order.

## COMPLETED TABLE (var-name axis: stack_frame table offsets, base var_78@0x14
## = builder +0x00; reads from my windows @0x40dc01-0x40dd32; gated block from
## the creature decompile, cross-checked against ctor sub_40A980's +4k walk;
## verifier's six esp-based stamps re-verified same-base-pair 6/6 - cites below
## carry that construction, not her memory)

| cell | builder off | var (frame-table) | fills | source |
|------|-------------|-------------------|-------|--------|
| 0    | +0x00       | var_78 (0x14)     | type  | INIT-only mov @0x40dc0d; never read; passed to append @0x40e0ad |
| 1    | +0x04       | var_74 (0x18)     | u32   | r1 @0x40dc01 - anim-mode id (feeds 56..104 ladder) |
| 2    | +0x08       | var_70 (0x1C)     | u32   | r2 @0x40dc42 |
| 3    | +0x0C       | var_6C lo (0x20)  | u32   | GATED (version!=0) reach-back fill |
| 4    | +0x10       | var_6C+4 (0x24; table prints var_6C size 0x8, NO var_68 in either function's rendering - verifier's proposed name would make future greps structurally blind, adjudicated by table dump 0x40dba0 window @118) | u32   | NEVER WRITTEN: gated reach-back pushes exactly 4 @0x40ddab into var_6C's low dword; ctor copies the high survivor blind - shipped reality, mirror don't fix |
| 5    | +0x14       | var_64 (0x28)     | u32   | r3 @0x40dc6e (first after step-over) |
| 6..9 | +0x18..+0x24| var_60,5C,58,54   | u32   | r4-r7 @0x40dca6/bb/dc17/dcf7 |
| 10   | +0x28       | var_50 (0x3C)     | u32   | r8 @0x40dd0c |
| 11   | +0x2C       | var_4C (0x40)     | u32   | r9 @0x40dd2d |
| 12   | +0x30       | var_48 (0x44)     | u32   | r10 (next window; decompile v19) |
| 13   | +0x34..+0x37| var_44 (0x48, printed size 1 - metal wins: lea+push 4 fills the whole u32 @0x40dd5d) | u32 | r11, UNCONDITIONAL (11th read; verifier's "gated" was her reading the stream-FAIL exit jz@0x40dd6f as a version gate - the version gate is cmp edi,1/jb @0x40dd9d/40dda4, AFTER this read) |
| bytes| +0x38..+0x3B  | var_40,3F,3E,3D (0x4C..0x4F) | 4x u8 | +0x38=flag byte, UNCONDITIONAL store @0x40dda0 (a2!=0 chain, BEFORE the gate jb); +0x39=CLAIMED, +0x3A=v22, +0x3B=v23 - these three inside the version!=0 extension, survivors when gate closed (A2 90h-pair/B2 FCh-pair verified) |
| 15   | +0x3C       | var_3C (0x50)     | u32   | gated v24 = COUNTER (+3 in kind-3 consumer arms; 90h-pair verified) |
| 16   | +0x40       | var_38 (0x54)     | u32   | anim ladder target + version>=2 v25 = KIND (creature 8Ch-pair, weapon 88h-pair verified) |
| 17   | +0x44       | var_34 (0x58)     | u32   | version>=3 v26 |
| str  | +0x48       | var_30..(0x5C..)  | SSO   | version>=4 v27 via sub_49C8D0 = StreamReadString_0x49c8d0; cap/len init 15 |
| tail | +0x64..+0x66  | var_14,13,12 (0x78..0x7A) | 3x u8 | version>=5 v31,v32 (RV2 presets (1,0,0); 84h-pair verified) |

MESH RESULT: the gated block lands EXACTLY on the three role-cells certified
independently from consumers (+0x39/+0x3C/+0x40), and string/trailing cells
match the ctor walk end-for-end - the map and the census agree from opposite
ends. Cell 4 never-written is NEW to this map: ctor copies it blind; the port
mirrors (shipped reality, unnamed cell, offset-named only).

ONE BOUNDARY OPEN before code: ctor treats +0x34 as the LAST u32 of its
14-u32 run, but the reader's u8 flag sits AT var_44(+0x34) - so the flag is
cell 13's LOW BYTE and +0x35..+0x37 are ctor-copied unwritten stack. Resolve
by walking r10/r11 EAs + the init stores to 0x49..0x4B; until then the byte
group's left edge is unnamed by rule.

## Metal facts (verifier raw-disasm 0x40dc01-0x40dd17 + my confirming window)
COORDINATES - RULE, NOT ORIGIN (my previous commit's translation advice is
RETRACTED: "subtract 4 from the verifier's numbers" presumed they had a single
origin; they did not. IDA prints [esp+BASEh+var_Y] and BASE drifts with live
pushes - the failed sequence spanned 8 instructions at 3 different printed
bases (94h/98h/90h); esp-absolute arithmetic across those is arithmetic on a
SAWTOOTH, and no constant translation rescues it). The invariant axis is the
FRAME VAR NAME: this doc's builder coordinates are var-name deltas (base cell
var_78 = +0x00 by definition; r1 = var_74, etc.), which are stable regardless
of printed base. VALIDITY BOUNDARY (verifier's own ruling, certified at her
+0x40 re-check): single-origin esp arithmetic IS valid WITHIN a constant
live-push depth - the +0x40 pair (var_38 vs var_78) printed under one base on
each side and was checked BEFORE the push @0x40e0b1 that separates depths;
the trap is the multi-PC drifting-base sequence, not the two-PC same-base
pair. DEFAULT: var-name deltas.
On this doc's (var-name) axis, from my window at her EAs:
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
