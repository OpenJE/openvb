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

## 5. The exact accounting: shipped's 45 vs the port's 36 (B24, same sitting)

Both sides computed programmatically, not by eye — shipped's table walked to its {0,0} terminator from the PE,
the port's rows parsed out of `s_gameCreatureNatives[36]`:

- **shipped F3 game-lib entries: 45. Port rows: 36.**
- **Name drift: ZERO.** Every one of the port's 36 names exists verbatim in shipped's table. That is worth
  stating because this batch has been burned by names before (the `je->f3` header corruption, the IDB's
  `GameActionEquip::` label contamination on server functions) — here the port's registered names are clean.
- The 9 shipped names the port does not register, with their table fn EAs:

| name | fn | why it is not in the port |
|---|---|---|
| `A_Attack` | 0x459eb0 | wiring-blocked: builds `GameActionAttack` (new 0xC8) and funnels through 0x550720 — gap 5 |
| `A_JustAttack` | 0x459f70 | same family |
| `A_ReloadWeapon` | 0x45a380 | same family (`GameActionReload`) |
| `StartDialogue` | 0x45a040 | wiring-blocked (queue flush + funnel), plus the dialogue interface era |
| `HideInterfaces` | 0x45a100 | era-blocked (GUI, `sub_46EDF0`) |
| `SetMoveMode` | 0x45a140 | era-blocked (AMX object reference, `sub_59AA50`) |
| `GetSquadAttitude` | 0x45aa90 | era-blocked (world-server singleton vft slot 34 + `meth_0x5692e0`) |
| `GetTeamAttitude` | 0x45aaf0 | same, slot 33 + `meth_0x56a240` |
| `DisplayStringRef` | 0x45a410 | **was never in the batch's inventory at all** — see below |

### 5a. The inventory had a filter-shaped hole, and this is the finding

Batch B's row list (39 entries) was built as "every `F3::`-NAMED function in the live IDB whose address is not
cited in src/". `DisplayStringRef` 0x45a410 does not match that filter, because the IDB stores it with NO
namespace prefix — the disassembly header reads `DisplayStringRef (.text @ 0x45a410)`, not
`F3::DisplayStringRef`. So a native that shipped registers and a script can call was invisible to an
inventory keyed on a NAME PATTERN, and the batch's "39 rows" total was quietly under-counting the F3 game lib.

Standing rule 7 again, in a new costume: a listing built by a filter is an excerpt of the image, and the
filter's own criterion is part of the claim. The table at 0x6F3750 — read from bytes — is the authority for
"which natives does the F3 game lib expose", not a symbol query.

### 5b. Registered name vs gate ArgList - TWO fields, and they disagree on 21 natives

> **COUNT CORRECTION (FastNova `248f842`, verified here)**: this section said 20 and the true figure is **21**
> (engine-core 16 -> 17). The cause is not a missed string, it is a missed FUNCTION: shipped has **two** arity
> gates - `0x59AF40` EXACT (`cmp/je`, 4 args, `add esp,10h`) and **`0x59AF90` MINIMUM** (`cmp/jge`, 5 args, the
> 5th an `&out_actual` written at `0x59afa5`, `add esp,14h`, its own message
> "Expected at least %d parameters but recieved %d." @0x6A0694, both shapes re-read from instruction text).
> FastNova's extractor knew one EA and used a guessed 96-byte window, so six engine-core natives counted as
> "ungated" were actually MIN-gated (real ungated engine-core count: 9, not 16 - Year/Month/Day/Hour/Minute/
> Second/GetTimeStamp/TransferItemToParty/MessageArgCount). The 21st divergence is `Debug` 0x5c3710, which
> gates at **+0x86** - outside that window - and prints "PointDistance" @0x6AD37C.
> **No F3 game-lib native uses the min gate**, so this lane's 36-row audit is unaffected; `tools/native-gate-audit.py`
> now resolves both EAs, reports gate kind, widened its window to 140, and prints the bound on every negative.

Shipped registers `A_Attack` but the body pushes `"Attack"` into the arity gate (0x459ed0-era), and the failure
lines read `"Attack() was unable to find the attacking entity."` / `"..the target entity."`. The prefix is the
REGISTRATION name; the bare word is the MESSAGE name.

**This is a law, not a curiosity - from FastNova's image-wide pass, byte-verified: registration name and
arity-error name are two independent fields that DIVERGE ON 20 NATIVES across the three tables.** The prologue
shape that makes it decidable is `push name-ptr; push cells; push reg; push reg; call 0x59AF40`, and that
name-ptr is a separate `.rdata` string, NOT the table's name pointer.

| registered as | arity error prints | evidence |
|---|---|---|
| `A_Attack` / `A_JustAttack` | `"Attack"` (both) | first 22 bytes identical **displacement and all**, 0xC0 apart; they diverge only past the gate, where ENTITY messages are name-specific while the ARITY message is not. Two bodies, one shared arity string - NOT one native with a flag |
| `A_ReloadWeapon` | `"ReloadWeapon"` | 0x45a380, cells 1 |
| `UnequipItem` | `"EquipItem"` | 0x45a580 / 0x45a6f0 both reach the gate at +0x32 with `68 f4 db 65 00` + `6a 03` - **already ported faithfully**, kept at `ScriptNatives.cpp:1024` ("SHIPPED: wrong name") |
| `Debug` | `"PointDistance"` | gates at +0x86 (0x5c3710); prints 0x6AD37C - the 21st, found only after the window grew |
| `DecGlobal` | `"IncGlobal"` | 0x5c8200 / 0x5c8360 push the SAME 0x6AD81C |
| `GetSquadID` + `GetSquadRank` | `"GetSquad"` | both push 0x6B4BC4 - a name in NO table, so the arity string is not even a valid native name |
| `GetSquadMEmber` | `"GetSquadMember"` | the capital-E typo is in the REGISTERED name (scripts must call it that) while the error prints the correct spelling - so "sic" marks the WIRED spelling, not the pretty one |
| 16 more, engine core | various | `A_MoveToEntity`->"MoveToEntity", `A_Kill`->"Kill", `GetNavPointCount`->"GetNavpointCount" (CASE drift), `SetWorldMapAreaVisible`->"WorldMapSetAreaVisible" (WORD ORDER), `FloatingDamageText`->"AddFloatingDamageText" |

Three shapes of divergence, not one bug: **prefix stripped** (the `A_` family), **one string shared between two
natives** (Inc/DecGlobal, GetSquad*, Attack/JustAttack), **string re-cased or reordered** (GetNavpointCount,
WorldMapSetAreaVisible). The third is exactly what a "just strip the prefix" rule would miss.

Port implication, concrete: `ScriptNativeEntry` carries ONE name today. The faithful model needs the
registration name in the row and the error name at the gate call - the port already does this by hand for
`UnequipItem`, so the pattern exists; what it lacks is the stated rule that a row may legitimately need both,
plus an audit of all 36 rows against this table rather than trusting that only UnequipItem diverges. FastNova's
tool checks the port's `cells` column against the shipped gate immediate on all 32 gated rows (0 mismatches)
and confirms the 4 `cells = -1` rows are exactly the 4 shipped ungated natives - the arity half is byte-right,
the name half is open work.

**Port-side status, now AUDITED rather than trusted (added same sitting).** The open question above was
whether only `UnequipItem` diverges. It is answered by recomputation, not by sampling: for each of the 36
rows, the shipped native's own prologue immediates (`68 <name-ptr>` + `6a <cells>` before the
`call 0x59AF40`) were read from the PE and compared against what the PORTED body passes to
`ScriptNativeCheckArity` / `ResolveNativeEntity` - the latter resolving through the shared helper, whose own
gate cell (1) is folded in so helper-mediated rows are not silently skipped.

    rows parsed 36 (asserted - a 0-row parse would have looked identical to "0 findings", which is the exact
    vacuous-green trap this lane filed days ago, so the count is an assert, not a print)
    gate-comparable rows 36, divergences 0

=> every ported native's GATE NAME and GATE CELLS equal shipped's immediates, and the one shipped defect in
that field (`UnequipItem` printing `"EquipItem"`) is present in the port ON PURPOSE and asserted, so it does
not show up as drift. The alias quad's four rows carry `-1` (no gate) and were checked for exactly that. The
`cells` column and the name column are therefore both byte-right for all 36 rows, which is what makes the
table safe to hand to an installer once the typedef decision lands.

Two facts from the same pass that foreclose whole classes of speculation: counts walked to terminators are
**9 + 155 + 45 = 209**, and **no name and no fn value appears in two tables**, so shipped's registration order
never shadows. A flat port table cannot introduce a shadowing bug - there is no shipped shadowing behavior to
reproduce.

### 5c. Entry EAs vs inner-site EAs

Earlier rows in this batch cite `Attack` at 0x459f5d, `JustAttack` at 0x45a025 and `ReloadWeapon` at 0x45a3e6.
Those are the ENQUEUE CALL SITES inside the bodies (e.g. 0x459f5d is
`net::critical::section::meth_0x550720(local_4, v4)`), not the natives' entries, which the table gives as
0x459eb0 / 0x459f70 / 0x45a380. Both numbers are correct for what they cite; recorded here because a reader
matching "Attack 0x459f5d" against the table would find no such entry and conclude the table disagrees with
the doc. It does not — one is an entry, the other an interior site.

## 6. What this does NOT settle

- ~~The native ABI (gap 4)~~ **CLOSED while this doc was being written.** See `docs/f3/NativeAbiEvidence.md`
  (WildUnion) and gap 4 of `docs/f3/NativeWiringGaps.md`, where the dispatch is re-verified from bytes: the
  default native handler `sub_5CC4F0` pushes params @0x5cc50b, then amx @0x5cc50c, `call eax` @0x5cc517,
  `add esp,8` — two cdecl args, **amx first**, return into PRI, and the error channel `[amx+0x84]` cleared
  before the call and read after. So shipped's form is `int __cdecl(int amx, int params)` proven at the call
  site; batch B's 36 natives already match it, and what does not match is the port's own `NativeScriptFn`
  typedef. Three more installer facts from that pass, recorded here because they change the design: dispatch
  indexes the **image's own** native table (image+0x24/0x28, stride 40, entry+0 = fn, entry+4 = the script's
  name), so the three C arrays are merge SOURCES at load and never a dispatch table; the load-time merge
  IGNORES the first two results and only the third gates the load (err 19); and a product path to Load EXISTS
  in shipped — dialogue start (`BeginDialogue_55edb0` → vft slot 1 @0x55ee53), entity script loader
  (`sub_55F3E0` → `call [edx+4]` @0x55f4de), `JE::DialogueCall` itself (only direct call @0x5c3e15), and
  `JE::World::virt_meth_0x544b80`'s Script_Small slot 5 (`0x5a5a90`, the "OnUpdate" runner @0x5a5b25). So
  gaps 2/3 are "the port lacks callers", not "the engine has no caller concept".
- **Whether anything in the port should call Load today.** The port has no AMX module table, no .int
  loading path in product code, and `JE::Script::Run` still has no product callers (gap 2/3 unchanged).
  This doc narrows the design space; it does not license inventing a loader.
- **The other two tables' contents** (`off_7004C0`, `small_script_functions`) — FastNova's dump, including
  whether their entry counts (claimed 9 and 155) survive a walk to the {0,0} terminator.
