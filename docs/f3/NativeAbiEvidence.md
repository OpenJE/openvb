# The shipped native-ABI, from instruction text (2026-10-22)

Read-only RE pass commissioned by TrueRaven to convert gap 4 of `NativeWiringGaps.md` from a design
argument into byte evidence. Scope: the AMX native **invocation** side (registration was already known),
the producers of `params[0]`, the three C-side tables' relationship, and the product paths to
`JE::Script_Small::Load` (0x5a8880) in the shipped demo. Database: `artifacts/F3.exe.i64` via
`ida_original_f3`. **Every offset below was re-emitted from instruction text** (this lane's standing
rule); where I quote a decompiler line I give the instruction address beside it so a reader can
re-derive it. Nothing here is inferred from the port's shape, and `artifacts/F3.json` records **no**
named symbols for the `0x5CCxxx` AMX core — everything in §1–§3 is IDA-live analysis, not a DB lookup.

## 0. Verdict up front

Shipped's native calling convention is `int __cdecl(int amx, int params)` with **exactly two pushed
args, `amx` first, `params` second**, `params` = `dataptr + hp` (the VM's own data-stack pointer),
`params[0]` = block byte count, the native's `EAX` returned into the VM's **PRI** register, and a
separate error channel at `amx+0x84` that the VM clears before every call and reads after.

Therefore: the port's `NativeScriptFn = int(*)(Mover*, int, int, int)` (`modules/openje/src/JE/Scripting.hpp:18`)
is **not the engine's shape**. Gap 4 is a real typedef/adapter decision — but the call form it must
adapt between is now pinned by bytes, not by taste.

## 1. The invocation site

Three functions matter. None is named in the DB; all are in `.text` 0x5CC4xx–0x5CCE5x.

### 1a. `sub_5CC4F0` — the default native dispatcher (the `call` itself)

Installed as the callback at `AMX+0x08` by core init §1c **only if that slot is 0**. Full byte text:

    5cc4f0  mov     ecx, [esp+arg_4]      ; ecx = arg2 = native index
    5cc4f5  mov     esi, [esp+4+arg_0]    ; esi = arg1 = AMX struct ptr
    5cc4f9  mov     eax, [esi]            ; eax = AMX+0x00 = loaded image base
    5cc4fb  mov     edx, [eax+24h]        ; native-table offset, at image byte 0x24
    5cc4fe  lea     ecx, [ecx+ecx*4]      ; ecx = idx * 5
    5cc501  lea     ecx, [edx+ecx*8]      ; + idx*5*8 = idx*40  (stride 40, by bytes, not taste)
    5cc504  mov     edx, [esp+4+arg_C]    ; arg4 = params pointer
    5cc508  mov     eax, [ecx+eax]        ; eax = *(image + table_off + idx*40) = entry+0x00 = fn
    5cc50b  push    edx                   ; push 2nd arg FIRST  = params
    5cc50c  push    esi                   ; push 1st arg SECOND = amx
    5cc50d  mov     dword ptr [esi+84h], 0; clear error slot AMX+0x84 before the call
    5cc517  call    eax                   ; fn(amx, params) — two args, nothing else
    5cc519  mov     ecx, [esp+0Ch+arg_8]  ; arg3 = &PRI_out  (still pre-cleanup)
    5cc51d  add     esp, 8                ; CALLER cleans 2 pushes => __cdecl
    5cc520  mov     [ecx], eax            ; PRI = native's int return value
    5cc522  mov     eax, [esi+84h]        ; handler's own return = the error slot
    5cc529  retn

So the entire shipped ABI is: **`eax = fn(amx, dataptr+hp)`; `*PRI_out = eax`; error = `AMX[0x84]`.**
The native index is supplied by the caller (§1b); entries are addressed **inside the loaded .amx
image**, at `[image+0x24] .. [image+0x28)` (registration writes `entry+0` at 0x5cce2a, matcher arg is
`entry+4` = the script-side name string; `if ( !*v6 )` at 0x5cce10 skips already-filled entries).

### 1b. `sub_5CCE50` — the interpreter; two opcodes reach the dispatcher

The dispatch loop head, verbatim:

    5cd0d1  mov     ecx, [ebp+0]          ; ecx = raw opcode cell at pc
    5cd0d4  add     ebp, 4
    5cd0d7  dec     ecx                   ; switch variable = opcode - 1
    5cd0d8  cmp     ecx, 85h
    5cd0de  ja      def_5CD0E4            ; > 134 cases or opcode 0 -> error 6 path
    5cd0e4  jmp     ds:switchdata_005ce0c4[ecx*4]

Opcode numbering trap, noted explicitly because this lane has been burned by rendering twice today:
Hex-Rays labels cases by the **switch variable** (opcode−1); IDA's `jumptable case N` annotations on
the disassembly use the **raw opcode**. Anchor proving the annotation is raw: the `LOAD_PRI`-shaped
absolute load at 0x5cd0eb (`mov ecx,[ebp+0]; mov ebx,[ecx+edi]; ... local_34=ebx`) is annotated
`jumptable 005CD0E4 case 1` — raw AMX v2.2 opcode 1 *is* LOAD_PRI, so **asm annotation = opcode**.
The two native-invoking cases are therefore **opcode 122 (0x7A) and 123 (0x7B)**.

Case 122 — native index in PRI, block at hp (the classic SYSREQ shape):

    5cdcbe  mov     ecx, [esp+44h+local_18]   ; jumptable case 122; code base
    5cdcc6  mov     eax, ebp
    5cdcc8  sub     eax, ecx
    5cdcca  mov     ecx, [esp+44h+local_24]   ; stk
    5cdce2  mov     [esi+18h], ecx            ; persist stk -> AMX+0x18
    5cdce5  mov     [esi+14h], edx            ; persist frm -> AMX+0x14
    5cdcce  mov     [esi+10h], eax            ; persist pc  -> AMX+0x10
    5cdcd1  mov     eax, [esp+44h+local_2c]   ; hp
    5cdcd5  mov     [esi+20h], eax            ; persist hp  -> AMX+0x20
    5cdcd8  add     eax, edi                  ; edi = dataptr (AMX+0x04, §2)
    5cdcda  push    eax                       ; push 1st = arg4 params = dataptr + hp
    5cdcdf  push    &local_34                 ; arg3 = &PRI out
    5cdce0  push    ebx                       ; arg2 = PRI, the native index
    5cdce1  push    esi                       ; push last = arg1 = amx
    5cdce8  call    dword ptr [esi+8]         ; -> sub_5CC4F0 (§1a) unless replaced
    5cdceb  add     esp, 10h                  ; 4 args, caller-clean
    5cdcee  cmp     eax, 0Ch
    5cdcf1  jz      loc_5CE01A                ; 12 = suspend: save state, return 12 to host

(esi = AMX, ebx = PRI, edx = frm loaded at 0x5cdcc2/0x5cdd22.) Return handling: `12` → the loop
persists PRI/ALT/`hp`/`stk` into `AMX+0x88/0x8C/0x90/0x94` and returns 12 to the host
(0x5ce018–0x5ce03a) — the same fields the `param_3 == -2` **resume** path reads back at
0x5ccf11–0x5ccf53. Any other nonzero return aborts exec and propagates (0x5cdcf7 → LABEL_244).
Zero → execution continues, and PRI = the native's `eax` (via `local_34`, LABEL_262 at 0x5cdeef).

Case 123 — identical, except the index is the instruction operand, not PRI:

    5cdcfc  mov     eax, [ebp+0]              ; jumptable case 123; index = operand
    5cdd07  add     ebp, 4
    5cdd1c  mov     [esi+20h], ecx            ; persist hp
    5cdd1f  add     ecx, edi                  ; params = dataptr + hp
    5cdd21  push    ecx                       ; push 1st = arg4 params
    5cdd29  push    &local_34                 ; arg3 = &PRI out
    5cdd2a  push    eax                       ; arg2 = index (operand, not PRI)
    5cdd2b  push    esi                       ; push last = arg1 = amx
    5cdd2c  call    dword ptr [esi+8]
    5cdd2f  add     esp, 10h

Neither case body contains **any** store to `dataptr` or write of a byte count — see §2.

### 1c. `sub_5CC810` — core init; defaults and the installer's one call site

Byte-verified shape (all from `decompile` lines tied to instruction addrs, then re-rendered hex):
header test at 0x5cc8b5 (`magic halfword != 0xF1E0` → return 17 = ERR_FORMAT), version bytes
`[6],[7]` outside 6..8 → 18 (0x5cc8cb), then defaults:

    if (!amx[0x08]) amx[0x08] = sub_5CC4F0;   # native dispatcher      (0x5cc944)
    if (!amx[0x0C]) amx[0x0C] = sub_5CC530;   # debug hook -> return 21 (0x5cc950)

plus zero-walks of every 40-byte table span inside the image (`v7 += 40` loops at 0x5cc989–0x5cc9ef),
and `sub_5CC540` (the core scanner: `switch 122..130`-style reloc fixups; sets `flags AMX+0x28 |= 0x8000`,
the bit the exec entry tests at 0x5cce90 → "not initialized" error 22 = 0x16 if absent).

`sub_5CE320(amx, fn)` is a one-field setter — `*(amx+0x0C) = fn` at 0x5ce328. `Load` uses it to install
`Script_Small::Debugger` (debug build) or `sub_5A5860`, **never overwriting `amx+0x08`** unless it was 0,
so native dispatch is not interceptable by the debugger path. `JE::Breakpoint` proves the error channel
from the native side: `0x5c3431 push 0Ch; 0x5c3433 push esi(amx); 0x5c3434 call sub_5CE330`, and
`sub_5CE330` is `*(param_1 + 132) = param_2` at 0x5ce338 — **132 decimal = 0x84**, the slot §1a clears.
`JE::Breakpoint` also re-verifies the line counter claim: `0x5c3422 mov ecx,[esi+2Ch]` (PRI read for the
'LINE' public write at 0x5c342c), decimal 0x2C = the `amx+0x2C` line field the port's gap doc asserts.

### 1d. Registration (context, already known — bytes confirmed, not re-derived)

`sub_5CCDE0(&amx, table, count)`: start = `[image+0x24]`, end = `[image+0x28]`, `(end−start)/0x28` count
(0x5ccdf7 uses the 0xCCCCCCCD magic divide), per-entry `if (!*v6)` skip, miss → err 19 at 0x5cce2e.
Matcher `sub_5CCD60(scriptName, table, count)`: walks `{name, fn}` pairs stride 8
(`esi = [ebp+edi*8]` 0x5ccd89, `strcmp` loop 0x5ccd90, match → `return [ebp+edi*8+4]` = fn at 0x5ccdcd,
next-name-null terminates when count == -1). So `entry+0x00` = fn, `entry+0x04` = script-side native
name string pointer, stride 40 = 5 cells (3 unused/reserved — not established here).

Exec-side self-check: `sub_5CCE50` starts (0x5cce70) by calling `sub_5CCDE0(amx, 0, 0)` — a *validation*
pass (null table → any still-null entry ⇒ 19). **Nothing executes unless every native slot is filled.**

## 2. `params[0]`: producers named

Consumer (arity gate, the thing that needs a producer): `sub_59AF40(amx, params, n, name)` begins

    v4 = *param_2 >> 2 ; 0x59af4c     ; params[0] in BYTES / 4 == declared cells
    if (v4 == param_3) return 1;
    "Invalid number of paramters passed to function %s()." / "Expected %d parameters but recieved %d."

Its xrefs are the natives themselves (0x459ec6 Attack … 0x5c6661 CreateTeam, and every batch-B ported
function) — each native starts `push name; push n; push params; push amx; call sub_59AF40`
(byte text at 0x5c3c52–0x5c3c62 in `JE::DialogueCall`, 0x45a111 in `HideInterfaces`, etc.). Natives then
read `params[1..n]` (e.g. `a2[1]`, `a2[2]` in DialogueCall at 0x5c3cb0/0x5c3dba).

**Producer (engine side) — named: the `sub_5CCE50` prologue's stacked-argument builder.** When the host
calls a public by index (`param_3 = idx ≥ 0`, `param_4 = n`, `param_5 = cell*`), bytes
0x5cd000–0x5cd0b5 build the block the callee inherits:

    5cd056  lea     ebx, ds:0[ecx*4]      ; ebx = 4*n  (the byte count)
    5cd05d  sub     eax, ebx              ; new slot address = hp - 4n
    5cd061  mov     [local_2c], ebx       ; holds 4*n
    5cd06f  lea     ebx, [eax+edi]        ; dataptr + (hp - 4n)
    (loop 0x5cd080: copy param_5[0..n-1] to ascending cells — a1 first)
    5cd0a1  sub     eax, 4
    5cd0a4  mov     [eax+edi], ecx        ; *(hp - 4n - 4) = 4*n  <-- params[0] = BYTE COUNT
    5cd0a7  sub     eax, 4
    5cd0aa  mov     dword ptr [eax+edi], 0; one more zero cell; that becomes the callee's hp (0x5cd0b5)

Block layout, high→low: `aN … a1, bytecount(4n), 0 ← hp_entry`. The byte count sits **immediately below
a1**, i.e. `[count, a1, a2, …]` ascending — the same relative layout every native indexes
(`params[0]`=count, `params[1]`=first arg), so the engine builder and the script convention are one
convention. (The extra `0` cell that becomes the callee's initial hp is the call-frame sentinel; its
consumer is the callee's own prologue and I did not chase it further.)

**Producer (script side) — POSITIVELY TRACED (same day, follow-up pass): the compiler-emitted
push sequence before opcode 122/123.** 119 compiled `.int` AMX images exist inside the demo bundle at
`data/Scripts.grp` (staged copy: `.devenv/state/f3-reimpl-data/data/Scripts.grp`); they match the
engine's own header validation instruction-for-instruction (word `0xF1E0` at image+4 per 0x5cc8a4,
word `0x28` at image+0xA per 0x5cc8b0, version bytes ≤6/≥6 at +6/+7 per 0x5cc8c2/0x5cc8cb), each
carries its own stride-40 native table at `[image+0x24..0x28)` with **fn = 0 and the inline ASCII
native name at entry+4** — the exact layout §1d's registration writes. Decoding those images' code
streams with the interpreter's own operand arithmetic (opcode 36 PUSH_PRI @0x5cd519, 37 PUSH_ALT,
**39 PUSH_C** `hp-=4; *hp = operand` @0x5cd52f, 40 push dataptr[ofs], 41 push value@(frm+ofs) @0x5cd589,
44, 133 push `frm+ofs` address @0x5cdee3 — all `hp`-descending), every native call in shipped
bytecode ends with the same pattern:

    <param pushes…>  39 <4·arity>  123 <native-ordinal>

Examples from `Scripts.grp` image @0x158fa (28-entry native table, ordinals match the inline names):
`41 16 · 41 12 · 39 8 · 123 0` → floatcmp, arity 2, count 8; `41 12 · 39 4 · 123 4` → GetCurrentHP,
arity 1, count 4; `39 12 · 123 3` → SetValue, arity 3, count 12; `39 4 · 123 5` → float, arity 1, count 4.
Because PUSH cells descend, the final `PUSH_C` lands at the LOWEST address — `dataptr + hp` at sysreq
time — so `params[0]` = the byte count and the arg cells lie ascending above it, exactly what the gate
at 0x59af4c (`*params >> 2`) and the natives' `a2[1..n]` indexing consume. The interpreter itself never
writes the count at call time (both sysreq cases, §1b, contain no dataptr store); the count is script
payload. Engine-side `sub_5CCE50`'s prologue (§ above) emits the identical convention for host-initiated
public calls.

## 3. The three tables' relationship (structure only — FastNova owns contents)

`JE::Script_Small::Load_5a8880` calls `sub_5CCDE0` three times per loaded image, in this order, and
this is the whole relationship:

    0x5a8ada  sub_5CCDE0(&this->mbr_0x26, &off_7004C0,            -1)   ; result ignored
    0x5a8ae7  sub_5CCDE0(&this->mbr_0x26, &small_script_functions, -1)  ; result ignored
    0x5a8afe  if (!sub_5CCDE0(&this->mbr_0x26, &off_6F3750,        -1)) ; ONLY the third gates success

They are not three tables the VM consults. They are **three name→function lookup sources merged into
the one 40-byte-stride native table inside the loaded image** (entries `[image+0x24..0x28)`): pass N
fills only entries still null (0x5cce10 `if (!*v6)`), so pass order decides precedence where names
collide; the first two passes are expected to miss (unmatched names stay null for later passes), and
only the final pass's leftover-miss (err 19) fails the load with
`"Script_Small::Load() Unable to locate a matching function for a Native in: %s"` (0x5a8b00-era log).
The dispatch index (§1a) is the **script's own native ordinal in the image table**, never an index into
off_7004C0/small_script_functions/off_6F3750. `0x6F3750` = the F3 game lib (45 entries, per the board) —
its content census belongs to FastNova; I verified only that `Load` references it (data xref 0x5a8aee)
and nothing else in the image reads it (xrefs_to 0x6F3750: that one instruction).

## 4. Product paths to `Script_Small::Load` in shipped — they exist

`Load` (0x5a8880) has exactly two recorded references — one code, one data — but it is a **virtual**:
`Script_Small`'s vftable `0x6a539c` slot 1 = `0x6a53a0` → `0x5a8880` (DB `f3_class_info`; value re-read
from the live image). Product callers, all byte-cited:

1. **`JE::DialogueManager::BeginDialogue_55edb0`** — builds the `.dlg` path (`sub_434600(..., ".dlg")`
   0x55edf6), `operator new(0xE0)` + `Script_Small::ctor_0x5a7240` (0x55ee2b), then
   `(*(v7->vftptr_0x0 + 1))(v7, path, 1)` at 0x55ee53 = **vft slot 1 = Load**; on success enqueues the
   `"OnStartDialogue"` entry (0x55efec). On failure logs `"DialogueManager::BeginDialogue() - Error
   loading script "` (0x55ee93) and destructs via vft slot 0. **This is dialogue start.**
2. **`sub_55F3E0`** (entity-scoped script loader; stores the Script_Small at `[owner+0x14]` 0x55f4fe) —
   `call dword ptr [edx+4]` at **0x55f4de**, IDA's operand-value comment resolves the slot to
   `5a8880` (`5a8880, 5a8880, 5a8880`); ctor call at 0x55f4b5.
3. **`JE::DialogueCall`** (itself a native — §1d) — the only *direct* `call 5a8880` in the image, at
   0x5c3e15, loading the callee dialogue's `.dlg` (name+`".dlg"` composed at 0x5c3da9), then executing
   the named node via **vft slot 6 = `0x5a58c0`** at 0x5c3e73. So shipped nests: a running script's
   `DialogueCall` loads+runs another script, installing the same three tables via `Load` each time.
4. **`JE::World::virt_meth_0x544b80`** — constructs a `Script_Small` (0x544bba); slot 5 (`0x5a5a90`,
   the OnUpdate runner — compares the public name against `"OnUpdate"` at 0x5a5b25–0x5a5b33) executes
   per world tick. **World scripts run shipped scripts on the tick pump.**

Execution of loaded scripts goes through the two exec wrappers §1b serves: `sub_5CCE50`
(amx, &retval, index|−1|−2, stacked-param count, params) called at 0x5a5a57 (slot 6, run-by-name) and
0x5a5b9f (slot 5, OnUpdate), with `sub_5CCA80` resolving the name first; `sub_5CCCC0(amx, tag, &out)` /
`sub_5CCD00(amx, tag, ptr)` are the wrapper-level named-variable get/set the `Load` prologue uses to
register the script's entity handle (`[amx+0x04] ← this->mbr_0x4`, 0x5a8b30) and line/statistics
variables before any call.

**Coordinator conclusion (gap 2/3):** shipped has product callers — dialogue start, entity script load,
world OnUpdate, and native-nested `DialogueCall`. The port therefore needs no new mechanism to justify
an installer: `Script_Small::Load` = core-init + the three `sub_5CCDE0` passes, and the port's existing
hooks (BeginDialogue-shaped entry, a per-entity script holder, the world tick) are where shipped
attaches them.

## 5. Where shipped faults instead of degrading (stated per the standing rule)

* `JE::DialogueCall` handle resolution: `0x5c3cf5 call meth_0x54be10` (the `Entity+0x1DC`-keyed resolver)
  **returns 0 on a miss** (explicit `return 0` at 0x54be68), and `JE::DialogueCall` stores it
  (`0x5c3d09 mov esi, eax`) then **dereferences without any test** at
  `0x5c3d18 mov eax, [esi+1ECh]` (= pseudocode's `*(v6 + 492)`; 0x1EC = 492 ✓, cross-checked against the
  resolver's own compare `*(result+476)` at 0x54be39). Fault state: calling `DialogueCall` from script
  while its character/handle argument does not resolve to a live entity in the `byte_70BE0C` list.
  The apparent graceful branch — `"DialogueCall() was unable to find the character with target dialogue
  name attached."` at 0x5c3d49 — guards a **different, later** object (`v9`, built through the
  vft-slot-84/32 chain at 0x5c3d2d), not the resolver's null. Do not port that else-branch as a
  "handles-missing" path; the real handles-missing path is a read violation at 0x5c3d18.
* Graceful (for contrast, all byte-verified): missing dispatcher slot → error 9 (0x5cce59:
  `mov eax,[esi+8]; test; return 9`); unfilled native slot → err 19 both at install (0x5cce2e) and at
  exec entry (0x5cce70 recheck) → load failure + free (no fault); stack headroom → error 3 checked
  before the loop (0x5cd0b9 `add ecx,40h; cmp ecx,hp; return 3`); opcode 0 / >134 → error 6 path.
  `call eax` at 0x5cc517 on a null `entry+0` would fault, but §1d's pre-exec validation is what keeps
  it unreachable for images obtained through `Load`; bypassing the installer (the port's temptation)
  removes exactly that protection.

## 6. What this pass does NOT establish (absence honesty)

* Entry-reserved cells of the 40-byte native entries (bytes 0x08–0x27) — unused on these paths, unread here.
* Which of the three C-side tables registers `DialogueCall`/`Attack`/etc.: xrefs to 0x5c3c40 returned no
  data references in the live IDB, and the DB is an excerpt — that is missing tooling, not evidence of
  absence; the gate-pattern is what ties them to this ABI.
* `off_7004C0` and `small_script_functions` contents/counts — FastNova's census; §3 claims order and
  merge semantics only, neither count.
* Opcode names (SYSREQ vs an engine-specific "NATIVE" variant): behaviors cited by case bytes
  (index-in-PRI = 122, index-in-operand = 123); no name-level claim.
* `byte_7004BC` (a 64-bit-cell/compression flag consulted by the SHL_C/SHR_C cases at 0x5cd3f2 and the
  init zero-walks) — noted, unexplored.
* The callee-side interpretation of the prologue's extra `0` cell (entry hp sentinel) — layout cited
  exactly, semantics left to the port's call-frame work.
* Whether any shipped script uses opcode 122 (index-in-PRI) vs 123: every traced call site in the
  sample images used 123 (operand index); 122 exists and is dispatched identically, but a 122-using
  script was not located in the sampled windows (absence from a sample is not absence from the bundle).

## 6.5 `.int` verification method (repeatable)

Scan any `.grp` for `e0 f1` at image+4 with `28 00` at image+0xA and version bytes at +6/+7; parse
header cells at +0x0C code, +0x10 code-size/end, +0x14 dat, +0x18 dat-size, +0x1C entry (−1 here —
scripts are run by public ordinal through the `[image+0x20..0x24)` stride-40 publics table, 11
entries in the first image, name inline at entry+4), +0x24/+0x28 native span. Code is cell-addressed:
opcode word then operand cells; the operand count per opcode comes from the interpreter case bodies
at 0x5cd519–0x5cdee8, NOT from a remembered Pawn table. Tools in this repo already walk the `.grp`
TOC (`tools/data_census.py`, layout B `{id, off, size}`).

### 6.6 Exact grep encoding, and a full-bundle caller-side arity census (2026-10-22, after a peer
reproduction attempt failed on encoding width)

TrueRaven scanned for `39 08 7b 00` (byte operands) and `39 00 08 00 7b 00 00 00` (16-bit cells) and
got zero hits — correct result, wrong encoding. **Code cells are full 32-bit LE words**, so the
`PUSH_C k ; SYSREQ idx` pair is SIXTEEN bytes:

    27 00 00 00 <k:4> 7b 00 00 00 <idx:4>

Re-verified over the whole staged bundle (`.devenv/state/f3-reimpl-data/**/*.grp`) with regex
`\x27\x00\x00\x00(....)\x7b\x00\x00\x00(....)` confined to each image's code span (start = dword at
image+0x0C, per `mov ecx,[eax+0Ch]` @0x5ccea8; end = dword at image+0x14):

* **25,369 well-formed sites in 118 of 119 images, 115 distinct natives-by-name called.**
* Caller-side arity (`k/4`) is a singleton per native and matches the C-side gate immediates: e.g.
  `floatcmp 2`, `GetCurrentHP 1`, `SetValue 3`, `float 1`, `DisplayStringRef 4`, `RandomNumber 2`,
  `Protagonist 0`, `AddHP 2`, `UnlockDoor 1`, `MessageGetArg 1`, `StartDialogue 2` — the last checked
  directly against the shipped prologue this pass (`push 2` @0x45a052 in `F3::StartDialogue`).
* Only exception seen in the spot list: `SendMessage {2,3,4}`. Plausible cause is per-image native
  declaration divergence (ordinals and names are per-image; one C function can face differently-declared
  scripts), but that is a HYPOTHESIS — `SendMessage`'s gate was not read in this pass (no DB symbol).
  The audit gate should treat it as the one manual-review row.
* Scanning OUTSIDE the code span (whole image) is where false negatives/near-misses breed; anchoring
  on the magic means remembering it sits at image+4 — a peer read my `[image+0x24]` offsets as
  magic-relative and mistook the publics table for a natives table with nonzero fn cells. Both tables
  are stride-40 and adjacent ([0x20..0x24) publics, [0x24..0x28) natives); the discriminator is that
  natives carry fn=0 + inline name on disk, publics carry a nonzero code offset.

One-liner for the audit hook (arity-per-ordinal from callers, an independent population for
COVERAGE-BEFORE-VERDICT):

    rg -aU '(?s)\x27\x00\x00\x00(.{4})\x7b\x00\x00\x00(.{4})' <image-code-region>   # then K=le32(g1)/4, ord=le32(g2)

### 6.7 Global cross-join: 209 table entries × 115 called names (2026-10-22, second follow-up)

Merged the caller-side census (§6.6) with FastNova's callee-side extraction
(`tools/native-table-dump.py --json`, run read-only here; its `--selftest` gates green) across all
three registered libs (45 + 155 + 9 = 209 name slots). The partition is exact (111+4+80+14=209):

| bucket | n | meaning |
|---|---|---|
| AGREE | 111 | gate cells == unique caller `k/4` (13 of these are ungated float/time names where only the caller publishes) |
| **DISAGREE** | **4** | see below |
| gated, never called | 80 | C prologue is the only arity evidence (incl. GetLevel, SetHP, UnequipItem, Get/SetWeaponCombo, the Passed-family except SecondsPassed) |
| ungated AND never called | 14 | zero evidence except the entry + stub: OpenQuest/CloseQuest/AddQuestEntry/AddJournalEntry (all → `return 1` @0x420990), TransferItemToParty, (A_)SendMessageToTeam/Squad variants, Year/Month/Day/Second, Debug, floatstr |

The four DISAGREE rows, with site histograms — the first two are **shipped defect candidates that
neither population shows alone**:

> **What these rows are evidence FOR (per TrueRaven's banking at 9c5004a, adopted):** a
> gate-vs-caller mismatch is only a DEFECT if the body's own cell reads disagree with its gate —
> the gate is one instruction, the contract is the reads; **`owed: body re-read`** stands on both
> rows. What the mismatch DOES establish without any further reading is about **script provenance,
> not the engine**: the shipped `.int` payload carries calls that could never have executed — the
> gate rejects them before the body is entered, prints "Expected N parameters but recieved M",
> returns 0. So these artifacts were compiled against a *different declaration* than the shipped
> exe ships (authored before a signature change, or never recompiled after one). A reader landing
> on "DISAGREE 4" should read payload archaeology / version skew, not engine bug.

* `GetGlobal` — gate 1, callers {1: 864, **2: 1**}. Exactly one demo-script call site publishes 2
cells; in shipped that call always fails its own gate ("Expected 1 … but recieved 2", returns 0 —
the gate at 0x59af4c → 0x59af63/0x59af70 message pair is byte-known). A live DOA typo sitting in
the shipped payload.
* `FloatingSpeechText` — gate 6, callers {3: **14**, 6: 211}. Fourteen sites declare 3 cells against
a 6-cell gate: also permanently DOA in shipped. The sibling `A_FloatingSpeechText` (gate 6) is
called with {6: 21} consistently — looks like the 3-arg variant was an earlier signature the demo\scripts were not fully migrated off. (Both readings are CENSUS+dump only; I did not re-read those
two bodies' prologues beyond FastNova's extractor.)
* `SendMessage` — ungated, callers {2: 1, 3: 6, 4: 80}; and `SendMessageToTeam` — ungated, callers
{2: 1, 4: 5}. §6.6's note stands: with no gate and (per the dump) no params count read in the body,
per-image declaration divergence is the working hypothesis; the multi-arity itself is NOT evidence
of a bug, only of divergence — an ungated native facing k smaller than what its body reads would
consume stack garbage, and nobody has yet counted that function's param reads.
**SUPERSEDED by §6.8: these two are MIN-GATED (≥2), multi-arity is legal by design.**

### 6.8 The second arity gate (FastNova 248f842, independently verified here) and the re-partition

There are **two arity gates, not one**:

* `sub_59AF40` — exact-match, 4 args: `*params >> 2 ; cmp ; je`, "Expected %d parameters but
recieved %d."
* `sub_59AF90` — **minimum**, 5 args (5th = `&out_actual`): `*params >> 2` @0x59af9b,
`*out_actual = cells` @0x59afa5, `cmp ; **jge**` @0x59afa7, "Expected at least %d parameters but
recieved %d." (@0x6A0694 string). Verified live in the IDB this pass, not adopted: the min gate
re-decompiled in full, and `JE::SendMessage` @0x5c5410 shows the call-site signature byte-for-byte
— `push 2` @0x5c5427, `call sub_59AF90` @0x5c542b, `add esp,14h` @0x5c5430 (note `add esp,14h` =
5 args vs `10h` = 4 args for exact; the stack cleanup is a second, independent discriminator).

Six engine-core SendMessage-family natives are **min-gated at 2**: A_SendMessage 0x5c5120,
A_SendMessageToTeam 0x5c52b0, A_SendMessageToSquad 0x5c5660, SendMessage 0x5c5410,
SendMessageToTeam 0x5c57c0, SendMessageToSquad 0x5c5930 (FastNova objdump; the two I re-checked
match). Consequences, applied to §6.7:

* **DISAGREE 4 → 2.** `SendMessage {2:1,3:6,4:80}` and `SendMessageToTeam {2:1,4:5}` are legal
min-gate use, not divergence and not bug — the stack-garbage open question is CLOSED for them
(no row can have k < 2 without failing the gate first). The residual DISAGREE rows are exactly
`GetGlobal` (gate 1 vs {1:864, 2:1}) and `FloatingSpeechText` (gate 6 vs {3:14, 6:211}), both still
`owed: body re-read`, both still provenance-not-engine-bug.
* **Re-partition of the 209**: AGREE-or-min-admitted **113**, DISAGREE **2**, gated-never-called
**84** (+A_SendMessageToTeam/A_SendMessageToSquad/SendMessageToSquad/Debug moved out of the
zero-evidence bucket — they have gates), ungated-AND-never-called **10** (4 quest aliases,
TransferItemToParty, Year/Month/Day/Second, floatstr). 113+2+84+10 = 209 exact.
* **FastNova's corrected callee-side numbers** (§9.3 of NativeTableAudit.md): engine-core ungated
is **9** (Year Month Day Hour Minute Second GetTimeStamp TransferItemToParty MessageArgCount),
not 16; engine-core total **17** with **21** error-name divergences — `Debug` (0x5c3710) gates
late at body offset **+0x86**, outside the old 96-byte window, and prints **"PointDistance"**
(0x6AD37C) in its arity error; gate coverage 209 = exact **181** / min **6** / none **22**.
* Of the 15 "ungated but called" rows in §6.7, **12** (float family, GetTimeStamp, Hour, Minute,
MessageArgCount) are genuinely ungated; the 3 SendMessage variants are min-gated.
* **Audit rule (supersedes the allow-multi suggestion)**: the six min-gated names form a DISTINCT
GATE CLASS (detect: `call 59AF90` / `add esp,14h`), everything else is exact-arity — a multi-arity
sighting outside the six is now a REAL signal, not an expected exception.

### 6.9 Stating the bound of every negative result (adopted rule; my census audited against it)

Item 72's lesson pointed at my own instrument: my "never called" rows rested on a `PUSH_C · SYSREQ123`
regex that **structurally cannot see the 122 form** (index-in-PRI). Explicit audit run this pass over
all 119 images' code spans: 32 cells equal 122 exist, **zero** are preceded by the in-range
LIN-const-load form `[11][idx] · [122]` — all are operands/data coincidences. So no demo script uses
the PRI-index call form via constant index, and §6.7/§6.8's never-called rows stand. Bound stated
honestly: a 122 call whose PRI was loaded by any *other* means (computed index) was not excluded —
dismissing that fully requires walking all code with the complete per-opcode operand table, which
nobody has written. Every negative in this doc now carries its instrument's bound:
"no 123-pattern site and no LIN+122 site in each image's `[image+0x0C, image+0x14)` code span",
never a bare "never called".

Method note for anyone re-deriving: caller-side name resolution goes through EACH image's own
ordinal table ([image+0x24] stride 40), so one name can hide several ordinals across images; the
join was done on names precisely because that is the only key both instruments can share.

**Bounding consequence for the oracle program (pulled out of the partition):** 80 of 209 slots are
gated-but-never-called — two-thirds of shipped's native surface has no demo caller — so **any
`.int`-parity oracle can ever cover at most 129/209 slots**; the remaining 80 rows are provable
only from the C prologue (and the 14 zero-evidence stubs only from the table itself). This bounds
what the installer decision can be *validated* by, independently of whether it is correct, and it
means "make the port's natives reachable" is chasing an engine API far broader than the demo ever
exercised. **And (TrueRaven's parting refinement, e6a5744): "all bytes pinned" must not be read as
"validation complete" in the decision record.** The pinned bytes settle the CALL FORM; they do not
settle the typedef call itself — correcting `NativeScriptFn` leaves the openje lane's existing
Mover*-shaped consumers owing a migration story, and with 80/209 slots uncallable-by-demo, parity
for the never-called majority can be *argued* from prologues and only *tested* for the ≤129-slot
minority. A disjoint count from TrueRaven's lane agrees in shape: 16 of the port's batch-B 36
rows have no shipped caller either.

## Appendix — verified AMX struct field map (all hex, each from instruction text)

| field | evidence (instruction @ EA) | role |
|---|---|---|
| AMX+0x00 | `mov eax,[esi]` @0x5cc4f9 | image base (written by init: `*param_1 = param_2` @0x5cc914) |
| AMX+0x04 | `mov edi,[esi+4]` @0x5ccead | dataptr (fallback `image + [image+0x10]` @0x5ccebe) |
| AMX+0x08 | `call dword ptr [esi+8]` @0x5cdce8 | native dispatcher (default sub_5CC4F0 @0x5cc944) |
| AMX+0x0C | `call dword ptr [esi+0Ch]` @0x5ccfe3 | debug hook (default sub_5CC530 → 21) |
| AMX+0x10/14/18/20 | persists @0x5cdce8-window | pc / frm / stk / hp |
| AMX+0x1C | `[eax+1Ch]` @0x5ccef2 | image entry-point offset (run-from-start) |
| AMX+0x20 | `mov ecx,[esi+20h]` @0x5ccec3 | hp (again: this is the slot §1b persists/reads) |
| AMX+0x24 | `mov edx,[eax+24h]` @0x5cc4fb | **native-table byte offset within image** (image header) |
| AMX+0x28 | `and edx,0FFFFBFFFh` @0x5cce83 | flags (bit 0x8000 init'd, 0x4000 debug) |
| AMX+0x2C | `mov ecx,[esi+2Ch]` @0x5c3422 | script line (also reported by sub_59AD20 as `line %d`) |
| AMX+0x84 | `mov dword ptr [esi+84h],0` @0x5cc50d | native error slot (writer: sub_5CE330 @0x5ce338) |
| AMX+0x88/8C/90/94 | `mov edx,[esi+8Ch]` @0x5ccf3d | saved PRI/ALT/hp/stk for the 12-yield + resume path |
| publics | `[image+0x20 .. +0x24)`, /40 @0x5ccf5f-6e | stride-40 run-by-index entry table, entry+0 = code offset |
