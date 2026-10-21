# T-013 static tail: Fragment +0x2C consumers, the +0xC0 broadcast blocks, and the +0xDC probe-key bytes

**Agent:** KeenStorm · **Date:** 2026-10-21 · **Task:** T-013 (static tail row a/b/c), parent claim `43cd194`
**Method:** `artifacts/F3.json` member/vftable indexes first, then RAW BYTES off
`$OPENVB_ORIG_EXE` (`/nix/store/…f3demo-1.0/share/f3demo/F3.exe`, `file_off = VA - 0x400000`,
disassembled with `objdump -b binary -mi386 -Mintel`) plus `ida_original_f3` READ-ONLY cross-checks.
Per the coordinator's note 1, every F3.json-listed usage was re-verified against raw bytes; per the
devenv-respawn advisory, all conclusions below stand on bytes re-read after the 8745 restart, and
nothing here is anchored to an IDA name (names under these regions churn; EAs do not).

## Verdicts up front

1. **(a) +0x2C HAS semantics — found, with a consumer shape.** It is the **animation/frame
   index** of the piece. Slot 15 advances it by wall-clock (mod the frame count at +0x30, at the
   rate at +0x28); slot 16 multiplies it by the authored frame width (+0x20) and adds it to the
   UV/pixel origin floats. The earlier "parse and store, no interpretation" port rule was correct
   *for the reader*, but the field is NOT dead: it is the state variable of the animated-piece
   machinery. The shipped wire value is the **starting frame**.
2. **(b) The broadcast trio is per-vertex derived geometry, NOT "per-part clones".** +0xC0/+0xDC/
   +0xF8/+0x114 are four **identical 0x1C-byte vertex blocks** (corners of the piece quad). The
   ctor initialises each block's float pair to the unit-quad pattern (u,v) ∈ {(0,0),(1,0),(0,1),
   (1,1)} with color 0xFF808080; slot 14 REWRITES the tail (0x10..0x18) of every block with the
   derived rect. The reader's "broadcast" writes the wire +0xC0 dword into **field 0 of all four
   vertices** — it is not copying vftptrs and it is not four separate records.
3. **(c) The +0xDC probe key is statically decidable and the answer is asymmetric.** The probe
   compares the **verbatim wire string** (raw bytes read by `0x50b880`, extension and case
   exactly as authored) against each node's key string with **byte-exact, case-sensitive memcmp**
   (`0x41e4b0` = `repz cmps`). But nodes are INSERTED (via +0xC4) under the name `Demand_0x49b750`
   stored on the Res — and `sub_499F10` **rewrites the caller's name string in place to the
   extension-STRIPPED stem before the Res is built** (0x499f89 `mov ecx,esi; call 0x401b20`,
   esi = the caller's string). So on any `.int`/`.ini` name that carries an extension, the +0xDC
   probe structurally CANNOT hit a node inserted by this same chain. The registry is keyed by
   **stem** (case preserved, no folding); the GUI probes it with **name.ext**.

## (a) Fragment+0x2C — the frame index

### Owner map (vft 0x66ea1c, from F3.json, slot = byte offset/4)

| slot | +bytes | EA | role established here |
|---|---|---|---|
| 7 | 0x1C | 0x4b20a0 | reader (`Fragment::ReadFrom`, T-007 G1) |
| 8 | 0x20 | 0x4b1e50 | **re-loader**: same chain via transfer wrappers (new, below) |
| 9 | 0x24 | 0x4b2300 | **save-writer** (`0x618e7c` stores field-by-field) |
| 14 | 0x38 | 0x4b1830 | derived-rect / vertex-block writer |
| 15 | 0x3C | 0x4b1720 | **animation tick** (timeGetTime-driven) |
| 16 | 0x40 | 0x4b1b00 | texture-geometry update (repaint callee) |

All ten F3.json `mbr_0x2c` usages, each re-verified at raw bytes:

| EA | owner | raw bytes | meaning |
|---|---|---|---|
| 0x4b1750 | slot15 | `8b 4e 2c` `mov ecx,[esi+2Ch]` | read frame: `add eax,ecx` (elapsed ticks + frame) |
| 0x4b1760 | slot15 | `89 56 2c` `mov [esi+2Ch],edx` | **write**: `edx = (frame + elapsed_ticks) mod [esi+30h]` |
| 0x4b1c28 | slot16 | `8b 46 2c` | read frame × `[esi+20h]` (`0f af 46 20`) = pixel offset |
| 0x4b1cdf | slot16 | `8b 56 2c` | same product on the second corner path |
| 0x4b1d43 | slot16 | `8b 46 2c` | mode-1 path: frame and **frame+1** (`40 inc eax`) |
| 0x4b1d92 | slot16 | `8b 46 2c` | mode-2 path: frame and frame+1 |
| 0x4b1ee6 | slot8 | `8d 4e 2c` `lea ecx,[esi+2Ch]` | transfer READ (4 bytes, `call 0x619089`@0x4b1eec) |
| 0x4b213a | slot7 | (reader store) | wire item #5 → frame initial value (T-007 G1) |
| 0x4b2384 | slot9 | `8d 4e 2c` | transfer WRITE (`call 0x618e7c`@0x4b238a) — SaveTo side |
| 0x4b24ec | ctor | `89 5e 2c` | init 0 |

### The tick, byte for byte (slot 15 = 0x4b1720)

```
4b1723  mov eax,[esi+0x28]        ; rate
4b1726  test eax,eax / jz exit    ; rate==0 -> no animation at all
4b172c  call timeGetTime          ; -> edi (now)
4b1732  mov edx,[esi+0x34]        ; last-advance clock (NOT on the wire)
4b1739  sub ecx,edx               ; elapsed = now - clock
4b173b  mov eax,3E8h / idiv [esi+0x28]  ; ms_per_step = 1000 / rate (truncating)
4b1746  cmp ecx, ebx / jbe exit   ; elapsed < ms_per_step -> nothing to do
4b174a  div ebx                   ; eax = whole steps, edx = remainder
4b1750  mov ecx,[esi+0x2c]        ; ecx = current frame
4b1753  mov [esi+0x34],edi        ; clock = now
4b1756  mov byte [esi+0x18],1     ; DIRTY flag
4b175a  add eax,ecx / idiv [esi+0x30]   ; (frame + steps) mod frame_count
4b1760  mov [esi+0x2c],edx        ; frame := remainder
4b1765  mov al,[esi+0x18] / test / jz
4b176c  mov eax,[esi] / call [eax+40h]   ; slot 16 (0x4b1b00) repaint
```
(raw: `8b 4e 2c … f7 7e 30 89 56 2c` and the dirty byte `c6 46 18 01` / `8a 46 18`, read at
`file_off 0xb174a`; note `idiv [esi+0x30]`'s divisor is the **frame count** — `rate==0` at +0x28
disables the whole path, `count==0` would divide by zero: shipped behaviour, not ours to fix here.)

### The consumer (slot 16 = 0x4b1b00), per the wire mode at +0x1C

Slot 16 branches on `[+0x1C]` (0 @0x4b1c19-26 `cmp/jne`, 1 @0x4b1d38 `cmp eax,1`, 2 @0x4b1d89
`cmp eax,2`) — the same 0/1/2 that T-007 G1 found the reader validating to 0..7 and the apply
switching on. In each branch the frame index is turned into an **origin offset**:

```
4b1c28  mov eax,[esi+2ch] ; imul eax,[esi+20h]   ; frame * frame_width
4b1c37  fild / fadd dword [esi+38h]              ; + float bias (+0x38)
4b1c4a  fdiv (texture extent from obj+0x24 -> +0x1C/+0x20)
```
and in the mode-1/2 paths (0x4b1d43, 0x4b1d92) both `frame` and `frame+1` (`40 inc eax`) feed the
pair of corners — i.e. the animation slides the UV window between successive sheet frames across
the two u-corners (or v-corners for mode 2), which is exactly how a frames-strip texture is
sampled. The texture extent arrives via `vft+0xD8` (handle→object) at 0x4b1b25; on an invalid
handle slot 16 zeroes +0xCC/+0xD0/+0xEC/+0x104 and leaves the ctor's unit-quad floats alone
(0x4b1b2f-0x4b1b5e: `mov eax,0x3f800000`; four `fstp`s to +0xE8/+0x108/+0x120/+0x124).

### Consequences for the port

- `d38` (our port name) is NOT a parse-and-ignore field **as state** — it is frame. The .int wire
  value is the initial frame; the shipped reader legitimately stores it and slot 15/16 advance it.
- The clock (+0x34), dirty flag (+0x18), and the two origin-bias floats (+0x38/+0x3C, ctor-zero
  @0x4b265f/0x4b2662) are **derived state with no wire bytes** — same category as dDC/dF8/d114.
- Slot 8 (`0x4b1e50`) is a second entry point into the SAME load chain (probe @0x4b1f66 `+0xDC`,
  +0xC4 @0x4b1fc8, clamp 0x4b1fd2, `Release 0x498eb0` @0x4b1fdd, id store `[ecx+0x130]` @0x4b1ff8)
  but reaches it through per-field transfer wrappers (0x619089) over a stream at global 0x707DA8 —
  i.e. the save game / reload lane. Slot 9 mirrors it for SAVE (0x618e7c stores +0x1C/2C/30, name,
  +0xC0, +0x154, then Base rect via 0x4b2830). Any registry port must assume +0xDC/+0xC4 are
  driven from BOTH lanes.

## (b) +0xC0 and the trio: four vertex blocks, not clones

The reader block (T-007 G1 items 8/9) is, raw at `file_off 0xb2250`:

```
4b2250  lea edi,[esi+0xc0] / push edi / call 0x499220   ; wire u32 -> frag+0xC0 (1 stream read)
4b2260  lea eax,[esi+0xdc] ; mov ecx,3
4b2270  mov edx,[edi] ; mov [eax],edx ; add eax,1Ch ; dec ecx ; jne   ; COPY to +0xDC,+0xF8,+0x114
```
The four 0x1C-stride bases +0xC0/+0xDC/+0xF8/+0x114 are exactly what the ctor initialises, raw at
`file_off 0xb260d`:

```
4b260d  lea eax,[esi+0xc4] ; mov edx,3f800000 ; loop ecx=0..3:
4b2623    mov [eax+4],0xff808080   ; block base+8  = default colour
4b262a    mov [eax],edx            ; block base+4  = 1.0f           (cl&1 path)
4b262c    mov [eax-4],ebx(0)       ; block base+0  = 0
4b2631/6  mov [eax+8],edx/0        ; block base+0C = (cl&1) ? 1.0f : 0
4b263e/3  mov [eax+0C],0/edx       ; block base+10 = (ecx>=2) ? 1.0f : 0
4b2647    add eax,0x1C
```
so each block starts life `{int 0, float 1.0, colour FF808080, u, v}` with (u,v) cycling
(0,0),(1,0),(0,1),(1,1) — the four corners of the unit quad. The "broadcast" then overwrites
**corner field 0** (the int) of all four blocks with the one wire dword; slot 14 (mode bit 2
gated @0x4b1866 `test [esi+1Ch],4`) rewrites the block tails +0x14/+0x18 (ints at
0x4b1a0b/0x4b1ac6, floats at 0x4b1a15-0x4b1af0, 0x4b1d15-0x4b1d2f) from the parent-dims
`call 0x4b2890` result and the window scale read through the **skin registry global 0x707E48**
(0x4b1848 `mov ecx, offset unk_707E48`). **Answer: derived rect per corner of one quad; no clone
table, no vftptrs in these bytes.**

## (c) The +0xDC probe key — what bytes actually reach the manager

Reader path, raw at `file_off 0xb2187`:

```
4b2187  call 0x50b880                 ; JE::String stream read = the wire name, VERBATIM
4b21a9  call 0x4e1900                 ; copy-ctor: dest@esp, src = the stream string ([esp+0x30])
4b21bb  call [vft+0DC]                ; +0xDC, single pushed ptr = &copy (the 0x5c0970 ret-0x20 thunk)
```
`0x5c0970` copies the name into a local JE::String (`push ecx; lea eax,[esp+0x18]; call 0x51f570`
with out-slot zeroed @0x5c0996) and frees it on the cap>=0x10 branch (0x5c09a5-0x5c09b6) — a
pass-by-value string, no transformation. The comparison loop is `0x51f570` (raw at
`file_off 0xb1f570`… re-verified from objdump): walk list `[[this+0x28]]`, per node
`mov ecx,[edi+0x10]` (the node key string @ node+0x10) then `call 0x41e4b0` — and **0x41e4b0 is
the JE::String memcmp**: length gate + `f3 a6` (`repz cmps byte`) + length-equality fixup, i.e.
**case-sensitive, byte-exact**. There is no extension strip and no case fold anywhere on the
probe side.

The insert side is where the asymmetry lives. `0x4b21dd` calls `Demand_0x49b750(name, 1)` and the
+0xC4 argument string at 0x4b21e8-0x4b2208 is built from `[res+0x20]/[res+0x2c]` — the Res that
Demand returned. But `Demand_0x49b750` (0x49b750) first builds its search key by calling
`sub_499F10` on a copy of the caller's string, and `sub_499F10` — the shipped resolver,
decompile-confirmed post-respawn and byte-confirmed:

```
499f49  mov byte [esp+…],2Eh          ; the '.' separator probe (0x458a20 = strrchr-like)
499f55..61  0x458ef0(src, dot+1, …)   ; substring AFTER the dot = extension -> local_48
499f66..70  0x458ef0(src, 0, dot, …)  ; substring BEFORE the dot = stem -> local_2c
499f89  mov ecx,esi ; call 0x401b20   ; ***ASSIGNS THE STEM OVER THE CALLER'S STRING (esi=param_1)***
499fb6..  stricmp extension vs table {dds,wav,fnt,gr2,rlz,amx,dor,sst,…} -> ftype (0x200 etc.)
49a815  mov eax,64h                   ; "no dot / unknown" default
```
mutates the caller's string **in place to the extension-less stem**, and only after that does
Demand build/copy the Res record — the Res's own name buffer therefore holds the **stem, original
case** (0x401c70/0x401bd0 are plain strlen+memcpy, no folding). `LoadTextureFromBuffer_0x521ca0`
inserts the node under exactly that string (0x521d42 `meth_0x401b20(&v4->mbr_0x4, &param_2, 0, -1)`
→ `meth_0x5212e0` tree insert), and it also **probes first with the same stem** (0x521cc4 call to
`meth_0x51f570`) — which is why re-demanding an already-loaded texture never duplicates the node
even though the GUI's own +0xDC call can never see it.

### The statically decidable key form

- **Registry node key (node+0x10): extension-stripped stem, authoring case preserved** — the
  bytes `[res+0x20]` carries into +0xC4, never mutated afterwards.
- **+0xDC probe at the GUI call sites (reader 0x4b21bb, slot 8 0x4b1f66, LoadDefault 0x52363b):
  the full authoring string, extension included, compared byte-exact case-sensitive.**
- Therefore, for every authored name that contains a `.`, the GUI-side probe is a **permanent
  structural miss** and the id can only ever arrive via demand→+0xC4→(stem self-probe hit).
  For a name with no `.`, both sides carry the same bytes and the probe hits.
- Nothing on this evidence needs the debugger: the "stem vs name.tga" open question is **answered
  statically** — the registry is keyed by stem; the GUI simply probes it with the wrong form.
  T-052's remaining lockstep job shrinks to confirming which authoring strings actually carry
  dots (the .int census we already have says all 157 resolvable shipped names do).

**Caveats (stated, not papered over).** (1) RESOLVED since first draft: the dot split is a
**last-dot** (`rfind`) split. `sub_499F10` seeds `0x458a20` with pos = string length (0x499f41
`push edi`, edi = `[esi+0x14]` = length) and `0x458a20` scans **backward** (`458a9a 48 dec eax;
458a9b eb d3 jmp 0x458a70`, hit returns `eax - bufStart` @0x458aa9, miss returns -1 @0x458aaf).
So "a.b.bmp" would stem to "a.b", ext "bmp". Moot for shipped data anyway — the census over 165 .int names + 5 ini skins
(recon §8a) contains no double-dotted names. (2) `[res+0x20]/[res+2c]` as the +0xC4 name/data
source is byte-verified at 0x4b21e8-0x4b21f9; the forwarder's inner arg mapping (the `ret 0x20` /
14-arg thunk shapes) stays T-013 port scope — what matters here is the STRING CONTENT, which is
fully decided above. (3) Case: `0x50bb00`'s RHT lookup was not byte-read here; the probe-vs-node
comparison was (`0x41e4b0` = `repz cmps`), and that is the one T-013's `Gfx_Texture_Manager` must
reproduce.

## Coordinator-facing summary

- (a) `+0x2C` = animated **frame index**: tick (slot 15) advances `(frame + elapsed/(1000/rate))
  mod count` and slot 16 turns `frame × frame_width(+0x20)` into the UV origin. NOT dead.
  Port implication: our `SkinFragment` can keep the stored wire value, but a faithful GUI
  runtime needs the frame clock (+0x34), the dirty byte (+0x18) and the bias floats (+0x38/3C) as
  DERIVED state. Second chain entry (slot 8 reload) documented.
- (b) The trio = the four **corner vertices of one 0x1C-stride block array**; the broadcast is
  "wire dword → corner.field0 ×4"; slot 14 fills the corner rects. COPY port rule still satisfied
  by dDC/dF8/d114; their reader-side semantics are now on record.
- (c) Decided statically: **node key = stem (extension stripped in place by 0x499F10 @0x499f89),
  GUI probe bytes = full authoring name, comparison = case-sensitive memcmp**. The T-013 port
  should key `Gfx_Texture_Manager` by stem; the GUI-side probe either passes stems (matching the
  engine's own +0xC4 usage) or reproduces the shipped structural miss — but the KEY FORM ITSELF
  is no longer a lockstep question, so T-052's observation list can drop it.
