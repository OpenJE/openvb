# Gap 1 has an answer: shipped installs native tables in `Script_Small::Load` (2026-10-22)

`docs/f3/NativeWiringGaps.md` gap 1 said the port's native table has zero callers and named the missing thing
as "which array registers into which script context, and when". Shipped answers all three parts, and the
answer is one function. Read from `JE::Script_Small::Load_5a8880` (0x5a8880, size 0x332), which is also the
**only** xref to the F3 game-lib table @0x6F3750.

Scope discipline: this file records what the bytes say about the INSTALLER. The native invocation ABI
(gap 4) is WildUnion's pass and is deliberately NOT concluded here; the other two tables' contents are
FastNova's dump. No port code changed because of this doc — the decision it informs belongs to the
openje/spawn lanes, as the gap doc says.

## 1. The registration sequence, cited

After the module is read and `sub_5CC810` (amx_Init) succeeds, Load registers THREE tables in a fixed order,
each with `param_3 = -1`:

    0x5a8ada  sub_5CCDE0( &this->mbr_0x26, &off_7004C0,            -1 )
    0x5a8ae7  sub_5CCDE0( &this->mbr_0x26, &small_script_functions, -1 )
    0x5a8afe  sub_5CCDE0( &this->mbr_0x26, &off_6F3750,             -1 )   <- the F3 game lib

so the answer to "which array registers into which context, and when" is: **all three tables into every
script's own AMX, at that script's load time**, not once per process. Two consequences for the port:

- Registration is PER-Script-instance (`&this->mbr_0x26` is the AMX inside the Script_Small object). A
  port installer that registers globally into one shared VM is not modelling shipped.
- There is no "F3 installs its natives at startup" site to find, because shipped has no startup install:
  the install happens inside the loader, and the loader runs when a script name is handed to it
  (`0x5a88ee` `FileStreamOpen(global_resource_system, ArgList, 1)`). The gap doc's "the caller was not
  written" is therefore accurate but incomplete — the caller in shipped is the loader, and the loader's
  caller is the script-binding era, not a boot hook.

## 2. The registration is FAIL-FAST, and the failure is a named message

`sub_5CCDE0`'s return is tested at 0x5a8afe. Nonzero (a name in the table matched no native in the module)
goes to:

    0x5a8b0c  LogDebugString("Script_Small::Load() Unable to locate a matching function for a Native in: %s\n")
    then free(this->mbr_0xd0); this->mbr_0xd0 = 0; return 0

and the load is abandoned. The per-entry matcher is the 0x5CCD60-era function with miss error 19
(`mov ebp,0x13` @0x5CCE2E), already cited in `modules/openje/src/JE/Scripting.cpp`.

**Why this matters to the batch:** a native table is only installable if EVERY name in it resolves in the
module. So a port that installs a 36-name table into a VM whose module declares fewer natives will hit
shipped's own failure path and lose the script. This makes the table's name set load-bearing in a way the
"it's just a lookup array" framing hid, and it is the reason FastNova's name audit (port 36 vs shipped 45)
is a prerequisite for any installer attempt rather than a nice-to-have.

## 3. After registration, Load publishes PUBLIC VARIABLES — this is gap 3's other half

On success, Load calls `sub_5CCD00` (amx_SetPubVar) with 32-bit ASCII tags. Decoded here from the decimals
the decompiler printed (`v.to_bytes(4,'little')`, so the byte order is the one a string reader sees):

| call site | decimal tag | hex | tag string | value published |
|---|---|---|---|---|
| 0x5a8aca (only when `mbr_0x25 == 1`) | 1279415366 | 0x4c425446 | **FTBL** | `&this->mbr_0xd4` |
| 0x5a8b30 | 1179407181 | 0x464c534d | **MSLF** | `this->mbr_0x4` |
| 0x5a8b4b | 1413567571 | 0x54415453 | **STAT** | `dword_70C724`, itself set at 0x5a8b46 to `(this->mbr_0x24 != 0)` |
| 0x5a8b69 | 1163219270 | 0x45555146 | **FQUE** | `&this->mbr_0xc0` |

That is a correction of my own first draft, which refused to name these pending a byte split. The split is
one command and the tags are unambiguous, so "pending" was a hedge that cost nothing to remove — and the
endian question is the one to state plainly: read little-endian the tags are MSLF/STAT/FQUE/FTBL, which are
all pronounceable engine abbreviations; read big-endian they are FLSM/TATS/EUQF/LBTF, which are not. The
LE reading is also the one that reproduces the discovery note's `FQUE`, so the two sources agree on the
method.

`discoveries/2025-10-20-scripting-abi-native-tables.md` describes these same publishes as tags
"`myself`/debug/`FQUE`". Only `FQUE` survives the decode: there is no tag whose bytes spell `myself`, and
`MSLF` is a four-character name, so that note's `myself` is an INTERPRETATION of MSLF rather than a literal
in the image. Recorded as a correction owed to the discovery lane, with the caveat that it does not change
what MSLF carries (`this->mbr_0x4`, the script's own identity) — only what it is called.

What is solid on top of the decode: the natives' `amx` argument is the same object Load initialized
(`&this->mbr_0x26`), and the reporter's two `sub_5CCCC0` lookups (the port's
`ScriptNativeReportError` cites tags 1179407181 / 1279415366, i.e. **MSLF** and **FTBL**) read back exactly
what Load published. So a script's name and file — the two fields the shipped error prefix
`"!Run-time error in script %s, file %s%s, line %d: "` prints — are per-script pubvars, set at load time,
not globals. The port currently fakes those two fields as `"<Unknown>"`/`"<unknown>"`; this says precisely
what a real implementation must store and where.

## 4. The load path also shows what a "running script" is

`this->mbr_0xd0` = the malloc'd module image (52-byte header read first, then the body), `this->mbr_0x26` =
the AMX struct (0x98 bytes memset at 0x5a89e3), `this->mbr_0x24` = debug-hook flag (set from the module
header's bit 4 at 0x5a8a5a), `this->mbr_0x25` = a second header-driven mode (bit 1) that installs a
different callback (`sub_5A5860` instead of `JE::Script_Small::Debugger`, both via `sub_5CE320`).

The two warning strings on the early exits are shipped's, worth keeping if the port ever mirrors the loader:
`"Warning: attempt to load NULL script."`, `"Warning: attempt to load unknown script %s."`,
`"Warning: error reading header from script %s.  Script will not be loaded.\n"`,
`"Warning: seek error reading script %s.  Script will not be loaded.\n"`,
`"Script_Small::Load() was unable to initialize the AMX.\n"`.

## 5. What this does NOT settle

- **The native ABI (gap 4).** Load registers; it does not show the call form. That needs the OP_NATIVE
  handler in the exec loop — WildUnion's pass. No claim here about `(amx, params)` vs the port's
  `(Mover*, a2, a3, a4)` beyond what the natives' own bodies already prove (they take amx/params).
- **Whether anything in the port should call Load today.** The port has no AMX module table, no .int
  loading path in product code, and `JE::Script::Run` still has no product callers (gap 2/3 unchanged).
  This doc narrows the design space; it does not license inventing a loader.
- **The other two tables' contents** (`off_7004C0`, `small_script_functions`) — FastNova's dump, including
  whether their entry counts (claimed 9 and 155) survive a walk to the {0,0} terminator.
