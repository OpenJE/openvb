# 2025-10-21 — T-015 remainder: GWAM full slot table (frame reconstitution), creature-twin parity, mode 0-vs-3 buckets; T-019 promoted to final

PureKnight 01a15cb7-e27e. ORIGINAL_REFERENCE (`ida_original_f3` disasm of `sub_4148A0` =
`GameWeapon::ReadAttackModeData` + raw-EXE objdump diff of the twin). Statics only, no IDB writes.
Supersedes B11 §2/§4 (the open questions there are all answered here; B11 §1 counts re-verified).

## 1. Frame reconstitution — the definitive slot table (weapon parser `sub_4148A0`, 0x4148a0..0x414e42)

Decompile fails on this frame (the known push-shift trap) — table built from the instruction stream; IDA's
normalized `stack_frame` names are given as anchors. "Frame" = the normalized frame base IDA uses (locals at
0x10..0x75). Block passed to `sub_40C970` at `0x414e02` starts at frame 0x10 (mode word) and the 21 wire fields
land as:

| wire # | gate | width | frame slot | payload byte-offset | note |
|---|---|---|---|---|---|
| #1  | always | u32 | 0x14 `local_74` | 0  | **attack style (1..34)** — 34-way clip switch input `0x414c6c` when #18==0 |
| #2  | always | u32 | 0x18 `local_70` | 4  | stored; not consulted in-branch |
| #3  | always | u32 | 0x24 `local_64` | 8 | |
| #4  | always | u32 | 0x28 `local_60` | 12 | |
| #5  | always | u32 | 0x2c `local_5c` | 16 | |
| #6  | always | u32 | 0x30 `local_58` | 20 | |
| #7  | always | u32 | 0x34 `local_54` | 24 | |
| #8  | always | u32 | 0x38 `local_50` | 28 | |
| #9  | always | u32 | 0x3c `local_4c` | 32 | |
| #10 | always | u32 | 0x40 `local_48` | 36 | |
| #11 | always | u32 | 0x44 `local_44` | 40 | |
| #12 | always | u8  | →bool frame 0x48 `local_40` | 44 | `0x414a75→0x414a99` setnz-normalized |
| #13 | v≥1 | u32 | 0x1c `local_6c` | 45 | read `0x414ab1`; **un-aligned dword (byte 45)** |
| #14 | v≥1 | u8  | →bool frame 0x49 `local_3f` | 49 | `0x414afa` write |
| #15 | v≥1 | u32 | 0x4c `local_3c` | 50 | read `0x414afa` (after bool#14 stored) |
| #16 | v≥1 | u8  | →bool frame 0x4a `local_3e` | 54 | setnz `0x414b38`/store `0x414b42` |
| #17 | v≥1 | u8  | →bool frame 0x4b `local_3d` | 55 | setnz `0x414b53..0x414b61` |
| #18 | v≥2 | u32 | 0x50 `var_38` | 56 | explicit **anim-clip override**; if !=0 skips the style switch |
| #19 | v≥3 | u32 | 0x54 `var_34` | 60 | |
| str | v≥4 | {u16 len+ascii} | frame 0x58.. (std::string `local_2c/1c/18`) | 64.. | `sub_49C8D0` at `0x414bc8`, cleans `add esp,8` |
| #20 | v≥5 | u8 | 0x74 `var_14` | post-string | |
| #21 | v≥5 | u8 | 0x75 `var_13` | +1 | |

Answers B11 §4's open question: **the v≥4 string does NOT overwrite the v≥2/v≥3 slots** — #18/#19 live at
frame 0x50/0x54, the string object begins at 0x58 (its `local_2c` data ptr at 0x5c); the ±8/±12 drift was the
string-call cleanup + u8-vs-u32 dest-size confusion, not slot reuse. The frame 0x20 hole (between #2 and #3) is
genuinely untouched by reads — B11's "13 contiguous dwords 0x1c..0x4c" should read "the block 0x1c..0x4c with
wire order #13,#_,#3..#11 and bool lane 0x48..0x4b" — the per-instruction table above is the canon.

## 2. FatalError guard — corrected mapping, T-019 promoted to FINAL

Trip condition (weapon parser): `bool(#17) && bool(#16)`, i.e. **wire payload bytes +54 and +55, each read as
its own u8** — NOT "bytes 2/3 of the #10 dword" (B11 §2 mapping refuted by the instruction stream: #16 is read
at `0x414b10`, normalized+stored at `0x414b42` before #17's read even begins at `0x414b46`; both operands have
parser writers, contradicting "0x4a never written").
String at `0x64d860` ("GameWeapon::ReadAttackModeData() - A weapon …").

Census on the corrected mapping: all **57/57 shipped GWAM chunks have (#16,#17) = (0,0)** → 0 trips, now
**unconditionally** (B11 §3's verdict survives with its CONDITION DROPPED). Histogram {(0,0)×57} over the
committed `artifacts/census/chunks.csv` GWAM set; the earlier {(0,0)×54,(0,1)×3} was the mis-mapped pair.
T-019: shipped data never trips; reimpl mirrors-and-cites; the general rule for custom records is now byte-exact:
**trip iff payload[54]!=0 && payload[55]!=0**.

## 3. Mode derivation & the "style 0-vs-3" buckets — byte-exact

After the guard, the parser computes the record's MODE word (frame 0x10, the head of the block appended to
`GameWeapon+0x40C` list via `sub_40C970`, which is an intrusive node-append — `0x40c979 [esi+4]` link dance,
NOT a plain dword write; one node per GWAM chunk ⇒ multi-mode weapons are lists):

```
0x414c18  if bool(#17):  mode = 2                      ; thrown-flag override wins
0x414c2a  else c = [this+0x3B0]  (weapon attack-style field)
          idx = c-1; if idx > 9:  mode slot NOT WRITTEN (stack residue path, see below)
          else map = switchdataD_00414e54[idx]  (10 bytes: 00 00 00 00 00 01 01 01 02 02)
          jump = ds:00414e48[map] = { 0x414c48: mode=1, 0x414c52: mode=0, 0x414c5c: mode=3 }
```

⇒ buckets: `[this+0x3B0] ∈ {1..5}→1`, `{6..7..8}→0` (map bytes 5,6,7 = 01), `{9,10}→3` (map bytes 8,9 = 02);
`0x3B0 == 0` or `>10` ⇒ **mode = frame residue** (never initialized — the same uninitialized-lane hazard class as
the attack-record heap residue, flagged, not named). The three-way style→clip jump (`0x414c48/52/5c → fall into
0x414c60`) means mode only selects the record label; the clip always comes from #18 or the 34-way table.
Semantic naming of mode {0,1,3} stays numeric pending the attack-side comparators (next hop: readers of the
+0x40C list nodes — B9's `modeRec = recordBase + 0x18` cross-checks remain the anchor).

## 4. Creature-twin parity (`F3::GameWeapon::ReadAttackModeData_0x40dba0`, called from `GameCreature::ReadData` GWAM loop `0x40e481`)

Raw objdump identity of the two bodies (normalized, address-stripped): **89.7%** with ALL residual diff lines
being register-allocation (`cmp [b],bl` vs `mov cl;test`) and prologue ordering — i.e. the T-036 `R36-d`
clone family (two inlined copies of one source function). Grammar verdict:

- **Identical**: 22-read stream (21 integer/u8 `MemStreamRead` push-4/1 sites in both + 1 shared string read
  `0x49c8d0`; same gates `cmp [v],1/2/3/4/5`), same
  #16/#17 trip guard against the SAME string operand `0x64d860` (`0x40de62..0x40de73`), same 34-case style→clip
  switch with identical constants (0x35,0x38,0x3b…0x68) into its own jump table `0x40e0f4`, same string-read
  (`0x49c8d0`) and same `sub_40C970` node-append epilogue.
- **Different**: the twin has **no caliber→mode derivation at all** — no `[+0x3B0]` read, no 10-entry map, no
  3-way jump; its mode slot (block head, base esp+0x14) is NEVER stored to anywhere in the body — it is only
  ever address-taken (`lea ecx,[esp+0x14]` at `0x40e0ad` for the node-append) ⇒ creature mode word is
  pure stack residue on EVERY path (weapon path residue only occurs on out-of-range calibers).
- **Different**: append target `this+0x6F8` (`0x40e0b6 add ecx,6f8h`, GameCreature) vs `this+0x40C`
  (GameWeapon) — same list-of-modes structure, class-dependent offset.

Consequence for the port: creature attack-mode records must be modelled as *parsed-identical records with a
residue mode word and no style table consult*, or the mode byte must be pinned to whatever the shipped residue
observationally behaves like for consumers (mirror-and-cite; do not invent a creature mode derivation — there is none).

## 5. Supersession ledger

- B11 §1 (22-read counts/gates): re-verified, stands.
- B11 §2 (operand frame 0x4a/0x4b as "bytes 2/3 of #10"; "0x4a never written"): **refuted** — independent u8
  reads #16/#17 at wire +54/+55, both written by the parser.
- B11 §3 (0/57, conditional): **promoted to final** on the corrected mapping (this §2).
- B11 §4 open items (string-vs-slot overlap, creature twin): closed by §1/§4 here.
- B9 prose "21 fields": canon = B11 §1 table (22 stream reads = 21 numeric/u8 + 1 string), re-affirmed.
