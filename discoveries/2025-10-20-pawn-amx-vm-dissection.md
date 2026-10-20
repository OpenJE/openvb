# 2025-10-20 — Pawn/AMX VM dissection (C-17): interpreter ABI, core struct, tables, error+debug codes

Target: ORIGINAL_REFERENCE. Read-only. Answers chat/from-A.md **C-17** ("if you take a
session on the script VM — sub_5CC* family, the 5CCA80 lookup table format, 5CE310 invoke
ABI, 5CCE50 result codes"). ⚠ First correction: the fourcc state cells 'MPS?'/'MEVE' do
NOT exist in the binary (byte-searched both byte-orders; only ANIMEVENT matches
'MEVE*' as a substring @0x266030). The real packed tags are `MSLF/STAT/FQUE/FTBL`
(scripting-abi doc §1 step 5) + AMX magic 0xF1E0.

## 0. Identity

Stock-shaped **Small/Pawn 2.x bytecode interpreter** (134-opcode switch dispatch, not JIT),
with ONE nonstandard extension: **all symbol records are 40 bytes** (publics, natives, and
the three debug tables swapped in CoreInit) — vanilla Pawn uses 8-byte publics/natives
records. Consequence: you cannot read shipped `.amx`/`.int` blobs with a stock amx parser;
the 40-byte stride must be honored (A: this is your (b) runtime-blob-reading answer).

File container (Script_Small::Load, scripting-abi doc): first read = **52 bytes**, but the
in-image `size` field must equal **40** (byte 4, u16 next to magic 0xF1E0 at byte 2 — classic
AMX_HEADER), version bytes [6]≤6,[7]≥6 else err 18. Header+52? ⇒ 12 extra bytes consumed
before/behind the standard fields — parse fields by OFFSETS in §2 table, don't use a stock
header struct.

## 1. Function map (sub_5CC*/5CE* family)

| EA | role | evidence |
|---|---|---|
| 0x5CC460 | one-time std-native bootstrap (guarded by byte_7004BC = "already inited") | Init prologue call |
| 0x5CC4D0 / 0x5CC4B0 | bswap-dword / bswap-word (endianness fixups of header fields & tables; identity on x86 path taken) | Init walks header fields +4..+48 & 40B-tables with them |
| 0x5CC4F0 | **default native dispatcher** (amx+8): `fn = *(*(code)+36 + 40*idx + *code)`; `*ret = fn(amx, &stk)`; returns `amx[33]` (=+0x84 abort code, cleared on entry) | decompiled |
| 0x5CC530 | default debug callback: `return 21` | decompiled |
| 0x5CC540 | post-init auto-registration (amx_BuildAutoNative-family) | Init tail |
| 0x5CC7B0 | relocation fixup pass (flags&4 branch in Init): walks reloc table adding base delta | Init 0x5CC90B |
| 0x5CC810 | **amx_Init/CoreInit** — magic/version/flags checks, defaults install, reloc tables, err 17/18 | this session |
| 0x5CCA10 | natives_count getter (amx+N?) | called by FindPublic prelude |
| 0x5CCA30 | symbol record name→cstring (40B stride calc `*5`, `*8`) | FindPublic inner |
| 0x5CCA80 | **amx_FindPublic** — binary search over 40-byte publics records `{+0 code_addr, +4 name_off, ...}`; returns 20 on bad amx/null args, 0x14 overflow | disasm tail (0x5CCBA1 mov eax,0x14) |
| 0x5CCB40 | amx_GetPubAddr-style: strcpy name out + return record value dword | tail string-copy loop |
| 0x5CCBE0 | amx_FindPubVar (vars table, 40B) | SetGlobals caller (ge94/cutscene §8) |
| 0x5CCD60 | single native resolve (name→fn compare; caller Register loop) | Register inner call |
| 0x5CCDE0 | **amx_Register(table)** — loop `{name,fn}` caller stride 0x28; per-entry 5CCD60; on miss err **19** | `mov ebp,0x13` @0x5CCE2E |
| 0x5CCCC0 | amx_GetGlobal (tag+dword out) — MSLF/STAT/FQUE/FTBL consumers | prior docs |
| 0x5CCD00 | amx_SetGlobal/DefineGlob (tag,dword) | prior docs (Load step 5) |
| 0x5CCE50 | **amx_Exec** — full interpreter (see §3) | this session |
| 0x5CE310 | amx_Exec 5-arg wrapper: stores arg5 at [esp+0x10] then tail-jumps — preserves classic `amx_Exec(amx,&ret,code)` vs extended (index,args) forms | disasm (jmp 0x5CCE50) |
| 0x5CE320 | amx_SetDebugCallback: `[amx+0x0C] = fn` | disasm |
| 0x5CE330 | **amx_RaiseAbort**: `[amx+0x84] = code` (the abort channel read by 0x5CC4F0) | disasm |
| 0x5CE350 | amx_Address/GetAddr: bounds vs `[amx+0x18]` (mem size), err 20/… | head disasm |
| 0x5CE5F0.. | float lib natives (floatstr/mul/div/add/sub/fract/round/cmp) — FPU x87 wrappers | scripting-abi §3.1 |

## 2. amx core struct (`Script_Small+0x26`, 152 B) — field map (byte offsets)

| off | field | proven by |
|---|---|---|
| +0x00 | blob base ptr (image in Script+0xD0) | Init `param_1[0]=blob` |
| +0x08 | native callback (default 0x5CC4F0) | Init; exec native case |
| +0x0C | debug callback (default 0x5CC530, ret 21) | Init; SetDebugCallback |
| +0x10 | saved cip (resume slot) | exec -2 path; native case saves |
| +0x14 | saved heao/frm pair slot | 0x5CCF1C/0x5CDCE5 |
| +0x18 | memory size limit (dat+hea+stk bound) | amx_Address cmp |
| +0x1C | pri (primary reg mirror) | 0x5CDCA7 reads |
| +0x20 | heao mirror / `hp` | exec init `local_2c = *(+32)`… |
| +0x24 | frm mirror (`local_28`) | exec init *(+24) |
| +0x28 | stk top (`local_2c` initial = *(+32)) | exec |
| +0x2C | (stk base/limit) | *(+28) lower-bound check err 8 |
| +0x30 | heao live | *(+32) save/restore |
| +0x34 | flags (bit0 BYTEORDER-reject, bit1 DEBUG (debug hooks on), bit13 cleared-per-run, **bit15 REQUIRED** else err 22) | 0x5CCE83..90 |
| +0x38 | mem-size ceiling vs heao (err 7) | 0x5CCF98 |
| +0x44/+0x48 | op-arg mirrors for debug state 5 (op 125 writes both) | 0x5CDD62 |
| +0x4C/+0x50 | name ptr / aux (op 125 area) | — |
| +0x50 | **debug state** (`local_14` gates: 2=break,3=line,5=call?,7=return/abort-note,8=?) | case 0x7C/7D/7E saves + exec tail 0x5CDFF0 state 7 |
| +0x54 | state detail (code offset / idx) | same sites |
| +0x58..+0x64 | abort/resume channel fields | 0x5CDDE2-0x5CDDE9 |
| +0x84 | **abort code** (RaiseAbort write; default-native return) | 0x5CE330/0x5CC520 |
| +0x88/+0x8C/+0x90/+0x94 | saved registers {stk,frm,pri? …} for `index==-2` RESUME | 0x5CCF1C..43 |

## 3. amx_Exec ABI (0x5CCE50) — exact contract

`exec(amx, &retval, code, argCount, argv)` — classic 3-arg form arrives via 0x5CE310
(thunk stores argv=0):
- `code = -1` → cip = **`hdr[+28]`** directly (a "default entry" slot at the classic stk
  offset in this fork — the 52-byte read confirms a reinterpreted header); err 20 if <0;
- `code = -2` → **RESUME** from saved fields §2(+0x10/14/20/88..94) — this is the
  step/abort resume the debug UI uses (Script_Small::Execute state-3 path, scripting-abi §2);
- `code >= 0` → publics[10*idx] record → start cip; **publics @ hdr-byte-32, natives @
  hdr-byte-36** (classic offsets — verified two ways: exec's record count
  `(natives-publics)/40` and the native dispatcher's `*code + 36 + 40*idx`);
- **arg push**: `argCount` packs *count in low 16* + *dest-offset-high 16* variant
  (`(param_4 & 0xFFFF0000)!=0` branch — two push forms, both write cells to heap, then
  `heao-4 = byteSize; heao-8 = 0` (PROC header), stack-fence check `frm+64 > heao → err 3`.
- dispatch: `op = *cip++; switch(op-1)`, 134 cases, jump table **@0x5CE0C4** (extracted —
  case→handler map persisted in this repo via this doc; full mnemonic annotation = next pass,
  anchors verified: op 121 bounds-fence(err 4), op 123 **NATIVE** (`native(amx,idx,&pri,&stk)`
  via amx+8; result==12 → propagate abort 12; sets [amx+0x88..] resume save + state 7),
  op 125/126/127 = debug-notify forms (states 5/2/3 + callback via amx+0xC when flags bit1),
  op 128 = jump `cip = &code[pri]`, op 129 = jump-with-record (switch-table), op 130 = **err 6**
  (invalid), op 131/132 = swap pri↔[stk] / alt↔[stk], op 133 = push C (`stk-4 = heao+arg`),
  op 134 = RETN-adj skip `cip+=4`, op 78 shares handler with 29-branch group (0x5cd3d1).
- halt/return path writes `[amx+16/20/24/32]` + state 7 + returns; **result 12 = ABORT is the
  "clean stop" code** (Script_Small::Execute `rc==0 || rc==12 → success`, ge94/ABI docs) —
  and op 120 (0x5cdfc3) is the halt/exit return site.

## 4. Error codes (observed constants)
`0 OK · 3 STACKFENCE · 4 BOUNDS(op121) · 6 INVALID OP · 7 MEM/heap-exceed · 8 CODE-fence ·
9 NO-IMAGE · 12 ABORT(clean-stop semantics) · 17 FORMAT(magic/size/ver-field) · 18 VERSION
(byte[6]>6||byte[7]<6) · 19 NATIVE-LINK (Register miss) · 20 PARAMS (bad index/null) ·
21 DEBUG-default (default hook return) · 22 FLAGS (bit15 missing)`. (Names per classic amx.h
mapping; **12 = ABORTED confirmed by flow**, others by position — mark *(inferred)* where
name matters.)

## 5. What A asked, direct answers
(a) **parity checks against native scripts**: use §3 arg-push layout — natives receive
`fn(amx, stkPtr)` and read cells at `stkPtr[1..n]`; RET via `*(&stk[1])`… i.e. classic
`GET_CELL/PUSH_C` frame layout with 2 hidden header cells (hep, frm). Debug states: only
fire when flags bit1; your stand-in registry can ignore 5/2/3/8 unless you port the debugger.
(b) **reading shipping blobs at runtime**: honor 40-B records (§0) — publics record
`{+0 addr, +4 nameOff}` at `[hdr+32]`..`[hdr+36]` window; natives `{+0 fnIdx, +4 nameOff}`
from `[hdr+36]`; the three 40B tables swapped at `[8]/[11]/[12]`-derived
offsets in Init = debug names/tags/lines — parse defensively (bounds = `[hdr+3]` imagesize).
The `rc==12` tolerance in Script_Small::Execute means script-side `abort()` (via
amx_RaiseAbort → +0x84 → native return 12) reads as "handled, no result" — replicate.

## 6. Queue
1. Per-opcode mnemonic annotation of the 134-case table (anchors done; ~30 cases decompile
   mechanically — next session if wanted).
2. `sub_5CC7B0` reloc-table format (fixup chain) — needed for A's runtime blob loader.
3. `Script_Big`/ScriptListener (reimpl named them): same AMX core? (they share +0x26-style
   embedding — verify offsets before use).
4. Float-native precision semantics (fixed-point vs FPU? 0x5CE610 floatstr will show it).
   ~~pending~~ **DONE — §7.2.**

## 7. Session follow-ups (same day)

### 7.1 Reloc/record decompression — `sub_5CC7B0` (A: this gates your blob loader)
When header `flags & 4`: Init calls `unpack(hdrsize-cip_delta, tables_delta, blob+code_off)`.
Body = **reverse walk, LEB128-style delta decoding**: per record, consume 7-bit groups
(continuation bit 0x80), sign-extend when bit 0x40 set at group end (fills 0xFF high bytes
to 32 bits), then `*--dst = value` writing records DOWNWARD — i.e. the compressed table sits
BEFORE its expanded 40-byte-record destination; deltas accumulate into absolute-ish cells
(call-site passes two size deltas + destination). Net effect: the on-disk blob stores the
publics/natives/debug records LEB-packed; amx_Init expands them in place before Exec.
Reimpl must run this expansion before using the 40B-stride tables of §0.

### 7.2 Float representation = **raw IEEE-754 float bits in the cell** (NO tag, NO fixed-point)
`0x5CE5E0 decodeCell = fld dword[esp+4]`; `0x5CE5D0 encodeCell = identity passthrough`;
floatmul (0x5CE650) = fld·fld·fmulp·fstp·push. So `float x` cells are just `bit_cast<u32>(f)`.
⚠ This breaks the stock Pawn `float.inc` tagged-fixed-point convention — script-side float
literals and the 9 float-lib natives operate on raw bits. Any reimpl float interop must match
(bitcast), not the classic tag scheme. (floatstr 0x5CE610 formats via decode+sprintf path;
floatcmp 0x5CE7C0 compares decoded doubles.)

### 7.3 ⚠ Correction for A: there is NO `Script_Big` / `ScriptListener` class
F3.json full sweep (`.?AU*Script*@@`): only **Script_Small (0xE0)**, **ScriptTrigger (0x50)**,
**Scripting (0x04)**, **GameScripting (0x04)**. The "Script_Big/ScriptListener" names in
C-17 appear to be a memory slip — nothing of that shape exists; the script-VM dispatcher is
fully covered by §1-§3.

### 7.4 Scripting / GameScripting = the script error+output sinks (4-byte vft singletons)
* `JE::Scripting` (vft 0x59BF40-ish; ctor 0x59C040, dtor 0x59BFB0,
  virts 0x59BFD0/0x59C030/0x59C080/0x5B1510): `0x59BFD0(fmt,...)` = vsprintf(1KB) →
  `LogDebugString` — the **log-only sink**.
* `F3::GameScripting` (virts: dtor-tail 0x45E0E0, **0x46CD60**, 0x45E100): `0x46CD60(fmt,...)`
  = vsprintf → **on-screen message** via `cls_0x558740::meth_0x5583E0(msg, 0x80, 128, 0x80)`
  (the same overlay-message primitive seen in Script_Small::Load's debugger-hook branch and
  GameWorld::ReloadConfig) → plus
  LogDebugString("%s\n"). This is the script-visible `printf`/`Error()` destination — A: when
  your VM stand-in surfaces script errors, mirror the two-channel shape (log always, overlay
  only in client/GUI context; the `->int_0x2c + 140` overlay owner is the GUI manager, see
  gameinterfaces-singleton-table doc).

### 7.5 queue remaining
1. Per-opcode mnemonic annotation (Appendix A mechanical walk — ~20 cases left to name).
2. `Scripting` remaining virts 0x59C030/0x59C080/0x5B1510 (likely abort/warning variants —
   5-min pass).
3. floatstr exact format spec (which %f width the sprintf uses) — trivial, only if UI diffs.

## Appendix A — opcode→handler table (jump table @0x5CE0C4; index = raw cell value − 1
### as decoded: `switch(op-1)`; listed `op: handler`, all inside amx_Exec 0x5CCE50..0x5CE0C0)

1:5CD0EB 2:5CD0FD 3:5CD10F 4:5CD127 5:5CD13F 6:5CD154 7:5CD169 8:5CD184 9:5CD19F
10:5CD1C2 11:5CD214 12:5CD223 13:5CD232 14:5CD247 15:5CD25C 16:5CD26A 17:5CD278
18:5CD290 19:5CD2A4 20:5CD2B5 21:5CD2C6 22:5CD2E1 23:5CD2F8 24:5CD317 25:5CD366
26:5CD38E 27:5CD3BD 28:5CD3C9 29:5CD3DC 30:5CD40E 31:5CD442 32:5CD4BA 33:5CD4F0
34:5CD4FB 35:5CD506 36:5CD519 37:5CD524 38:5CD53D 39:5CD52F 40:5CD569 41:5CD57A
42:5CD591 43:5CD5A4 44:5CD5B7 45:5CD622 46:5CD653 47:5CD679 48:5CD6BE 49:5CD724
50:5CD76A 51:5CD863 52:5CDD50 53:5CD7AB 54:5CD7BB 55:5CD7CB 56:5CD7DB 57:5CD7EB
58:5CD7FB 59:5CD80B 60:5CD81B 61:5CD82B 62:5CD83B 63:5CD84B 64:5CD85B 65:5CD86B
66:5CD878 67:5CD885 68:5CD892 69:5CD8A3 70:5CD8B4 71:5CD8C5 72:5CD94C 73:5CD8D6
74:5CD910 75:5CD94C 76:5CD958 77:5CD97B 78:5CD3D1 79:5CD99E 80:5CD9A9 81:5CD9B8
82:5CD9C3 83:5CD9CE 84:5CD9D9 85:5CDAE6 86:5CD9EB 87:5CD9F6 88:5CDA07 89:5CDA1B
90:5CDA26 91:5CDA31 92:5CDA43 93:5CDA5B 94:5CDA7E 95:5CDAA1 96:5CDAB3 97:5CDAC5
98:5CDAD4 99:5CDAE2 100:5CDAF1 101:5CDAFF 102:5CDB11 103:5CDB23 104:5CDB35 105:5CDB47
106:5CDB63 107:5CDAF5 108:5CDB79 109:5CDB83 110:5CDB95 111:5CDBAE 112:5CDBB6 113:5CDBC0
114:5CDBCA 115:5CDBDC 116:5CDBF5 117:5CDBFD 118:5CDC30 119:5CDC55 120:5CDFC3
121:5CDC91 122:5CDCBE 123:5CDCFC 124:5CDD50 125:5CDD5C 126:5CDDC6 127:5CDE1B 128:5CDE5B
129:5CDE67 130:5CDF06(err 6) 131:5CDEBD 132:5CDECB 133:5CDED9 134:5CDEF3

Shared handlers of note: 52&124 (5CDD50), 72&75 (5CD94C), 78 lives out-of-sequence
(5CD3D1, between ops 28/29), 51 sits between 49/50 block-wise (5CD863 — small-op group).
