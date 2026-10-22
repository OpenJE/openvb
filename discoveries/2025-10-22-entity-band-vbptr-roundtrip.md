# Entity band 0x1DC/0x1E0/0x1E4: the vbptr round-trip, closed statically (2025-10-22)

Lanes: JadeBear (census + proof), WildKnight (port consequence, commits `0440439`/`22f1f29`,
suite floor 58, run-all 13/0). This note consolidates the RE-side chain so the B4 deferral's
proof obligation has one citable artifact. Every byte claim was re-read from
`$OPENVB_ORIG_EXE` (image base 0x400000) at write time.

## The two questions that looked contradictory

1. Census question: what writes `Entity+0x1E0`? **Answer: the lifecycle STATE dword.**
   Image-wide raw modrm scan (89/88/C7/C6/D9/DD, mod=10 disp32=0x1E0, SIB decoded,
   ~24 `[esp+idx+1E0h]` stack forms excluded): exactly 20 object stores. Three are
   Entity-era and all three are state writers: `ctor_0x5513a0@0x551475`, `ChangeState`
   `meth_0x5507f0@0x550945` (five verbatim `"Invalid state change in Entity::ChangeState()."`
   strings + transition table 0..8), `UnpackState_54e690@0x54e733`. The other 17 belong to
   Gfx/VFX/dialogue/interface classes (all attributed; full list in the T-079 session
   relay). Export agrees: `.?AUEntity@@` HAS `mbr_0x1e0` (base:true, size 4, 9 usages incl.
   getter 0x549030 = `[this+1E0h]==7`). NOTE the key family: plain `Entity` exists ONLY under
   the mangled key; cls_-style queries miss it — that is how "F3.json records no member at
   +0x1E0" was concluded and it was wrong.
2. SetHP question: `F3::SetHP` (0x45b2c0) resolves its subject via
   `8b 50 04 / 8b 52 04 / 8b 84 02 e0 01 00 00` (0x45b367-6d) — `[obj+4] -> [+4] -> [obj+disp+1E0]`.
   If `+0x1E0` is state, the resolver would be fed a 0-8 lifecycle value. Ugly; wrong model.

## The resolution: the 0x1E0 is NOT Entity-relative

`0x45b347: __RTDynamicCast(Entity -> GameEntity)`. The read base is a **GameEntity**, and
`GameEntity::ctor_0x4678a0` installs a vbptr at `+4`:

- `0x4678d4: c7 46 04 34 61 66 00` = `mov [esi+4], 0x666134` (vbptr; gated on the
  complete-object flag `param_2_00`).
- vbtable `0x666134`: `[-4 self-marker, +4 entry = 0xa0 = 160]` — read raw at 0x666138.
  The primary vftable `0x666130` is exactly ONE slot (`0x4080c0` — GetMaxHP, i.e. the
  B3 vft-slot-0 finding is this single-slot primary), and the vbtable follows it immediately.
- `0x4678fb/fd: 8b 48 04 / c7 44 31 04 44 60 66 00` = `mov ecx,[eax+4]` then
  `mov [ecx+esi+4], 0x666044` — the compiler itself defines Entity-base ≡ `this + delta + 4`
  (the secondary vftable install; no data read needed to prove the arithmetic).
- Witness: the birth idiom at `0x46797e` calls `Entity::virt_meth_0x550750(this+4+160, 5)`
  = `this+0xA4` = Entity-base (`&this->gap9D[7]` = 0x9D+7 = 0xA4), ChangeState(5).

Then SetHP's subject = `this + 160 + 0x1E0` = `this + 0x280` = **Entity-base + 0x1DC**.
The opcode's `0x1E0` = `0x1DC + 4`: MSVC folded the vbptr-slot offset into the disp32
(the ctor adds `+4` to the delta; SetHP adds delta to `this` and compensates in the
immediate). **Convention rule for this binary: a vb-displaced read at `[this+d+disp]`
means subobject-relative offset `disp - 4`.**

Secondary-table detail: `0x666044` slots 1..3 (`0x54dea0/0x549240/0x549cd0`) are the SAME
bodies as plain-Entity `0x692084` slots 1..3, and slot 1 (`0x54dea0`) is one of the nine
`mbr_0x1e0` usages — the state semantics carry into GameEntity by identical dispatch,
not just identical layout.

## Identity: the resolve is a provable round-trip, kept on purpose

`meth_0x46ee10(handle) -> meth_0x54be10` list-walk compares `*(entity+476)` (= 0x1DC)
against the handle; `0x54be41` is the MISS path only (map probe, node+0x10 or 0).
Handle is stamped once at `Create 0x55190c` (T-043) and restored 1:1 by UnpackState, so
`e -> e.handle -> e` is identity for every registered entity; **a "borrowed handle" is
impossible**. Unregistered own-handle => silent `return 0` at `0x46ee23`/`0x46ee3e` (no
error string — the shipped failure is observable only by ABSENCE of writes).

Faithfulness decision (WildKnight, endorsed here): the port KEEPS the round-trip.
Collapse-on-success differs exactly where the difference can be seen (the failure path),
and the re-resolve doubles as a runtime integrity check of the port's own handle
registry: if Create-side bookkeeping ever drifts, the one-step form silently clamps the
wrong object while the round-trip fails the way shipped does. Cost is one list walk per
native call — noise against the per-frame entity pump the engine already models.
Suite asserts both branches: identity landing + failed path writes nothing.

Clamp side-effects for the band: `0x4629c0` is an UPPER-bound-only clamp (`mbr_0x24` HP =
min(param, `mbr_0x20` max) — HP CAN go negative through this door; combat lanes must not
assume HP >= 0). Tail `meth_0x46d620(handle, value)` = packet-88 fan-out, named-deferred
with the server era; HP is a local write until then, stated in source and docs as a known
divergence.

## Adjacent finds, verified same session

- `GetWeaponCombo` (0x45a9f0) is a SHIPPED STUB: every path returns 0 (`0x45aa34
  xor eax,eax`), the SUCCESS path included — `local_4` is resolved at 0x45aa1a and
  discarded; the "unable to find" message prints and still returns 0. Port rule: keep it
  a stub; returning the object would be a behavior difference dressed as a fix.
- `GetSquadAttitude`/`GetTeamAttitude` (0x45aa90/0x45aaf0): self-attitude by identity
  `params[1]==params[2] -> return 1` (0x45aab8) before consulting anything; callee is the
  same WSI vb-idiom `&WSI[1] + *(WSI[1].mbr_0x0 + 4)` (0x45aacf/0x45ab2f), slots 34 (squad)
  / 33 (team). `GameEntity::mbr_0x28` (server relationship) is stamped at ctor `0x4679aa`
  with the same idiom — one model names both the SetHP subject and the attitude callee.
- `GetPerkRank` (0x45ab50) ships a copy-paste bug verbatim: its resolve-failure line is
  `"GetPosition() was unable to find the requested entity."` (push at `0x45ab88` — docs
  that cite `0x45ab8e` land on the displacement byte; EA±1 class, now 4 samples).
- IDA naming hazard: `GameActionEquip::` on `0x54be10/0x46ee10/0x4629c0/0x46d620` and the
  WSI singleton is `cls_0x616328` contamination — bodies correct, class labels wrong;
  family is server-side era. Do not propagate the label into docs (stale instance at
  `src/F3/Commands.hpp:81` routed to its owner, not edited here).
- Port-test law from B4's first run: a field re-declared by a sibling header
  (`GameItem.hpp` re-declares +0x1DC/+0x20/+0x24) reads ZERO through the wrong owner type —
  "read/write shared fields through the OWNER type" is now a rule; register family:
  sibling-redeclaration instances (3rd), alongside the shared-pad items 43/44.
  Related: repro prints that evaluate state before a call must be sequenced, not
  one-lined (printf argument-eval order masqueraded as `ptrmatch=0` resolve failure).

## Addendum (same session) — corroboration from B10/Protagonist, all bytes re-read

WildKnight's B10 batch + the Protagonist native independently confirm the band from three
new angles (his suite `b2c3931`-era, 69 asserts, 13/0):

- **T-043 handle delivered as a NATIVE RESULT.** `F3::Protagonist` (0x45a960): WSI-idiom
  base at 0x45a991 (`&g[2] + *(g[2].mbr_0x0 + 4)` — THIRD base-plus-delta sighting),
  vft slot 26 (+104), `Party -> GameParty` cast, leader read at `[party+5]` cells =
  **+0x14** (bytes `8b 40 14` @0x45a9c7), then `return *(leader + 476)` — 476 = **0x1DC**:
  the handle field this note's chain resolves through, now seen as a plain success return.
  Two shipped oddities kept verbatim by the port: both failure lines print
  `"Returning INVALID_ID."` yet return 0; the leader slot is a +0x14 dword, not a member name.
- **`cls_0x616328` contamination is bigger than a function-label artifact.** Protagonist
  reaches its callee through a SECOND global singleton of the same bogus class
  (`F3::global_cls_0x616328_0x70bd98`, alongside the WSI `0x401150`). The label family is
  server-side-era objects, not `GameActionEquip`; treat every `cls_0x616328` name (methods
  AND globals) as untrusted pending era work.
- **Door/lock band asymmetry, named.** `LockDoor` (0x45a1b0) / `UnlockDoor` (0x45a240) /
  `IsLocked` (0x45b4e0) are DOOR-ONLY: IsLocked casts to `GameDoor` only
  (`"IsLocked() - Entity is not a door"`) and returns the BYTE at `GameDoor+0xBD`
  (`v3[189]`, movzx — port asserts 0xAB -> 171 unsigned). `GetLockDC` (0x45b570) instead
  tries `GameContainer` then `GameDoor` and reads **+0xC0 off either cast result** —
  bytes at 0x45b5dd `8b 80 c0 00 00 00` (IDA's pseudocode printed this displacement as
  "+48", a PSEUDOCODE-PRINT hazard: offsets in decompilation vary with the rendered
  pointer type — adjudicate offsets with bytes, same discipline as the EA±1 class, now
  4+1 samples). A container therefore has a readable lock DC but cannot be locked or
  IsLocked-queried by script. One field, two doors — the DC pair lives at +0xC0 for both
  classes through their common base.

### Family adjudication (B11/B12 relay, all offsets by instruction text)

- GetWeaponRange chain bytes: inv ptr 0x45b6af `mov eax,[eax+168h]`; GameWeapon range-src
  0x45b729 `mov eax,[eax+408h]`; fallback 0x45b732 `mov eax,[edi+6f4h]`; +0x2C 0x45b740.
  IsItemEquipped shares 0x45b821 `mov eax,[edx+168h]` on the PRE-cast base — +0x168 is
  Entity-era, +0x6F4 GameCreature-era; both natives deliberately mix bases.
- Protagonist's handle-as-result is literal: 0x45a9e0 `mov eax,[eax+1dc]`; leader cell
  0x45a9c7 `mov eax,[eax+14]`. AddCNPCToParty: slot-26 call 0x45a343 `call [edx+68h]`,
  GameParty VFT slot 0 with entity pushed 0x45a370.
- RULE — native discriminator (routes batch-B rows by kind, immune to address-range
  conflation): every true script native calls the gate trio within ~60 bytes —
  0x59AF40 (arity+name), 0x59AEF0 (handle resolve), 0x59AD20 (error print).
  Command*Window (0x45ea50/0x45eac0) has none: __usercall interface plumbing -> Commands.cpp.
- RULE — offsets by instruction text, never by the decompiler's number (two unit bugs in
  one batch: 0xC0 rendered as 48, +0x14 family rendered as cells; a correct ×4 on the
  wrong sample proves nothing). Same family as the EA±1 register, extended to disp
  renderings: 5 samples now.
- cls_0x616328 is a FAMILY of at least two server-era singletons with different semantics:
  0x70bd98 = party-side (slot 26 -> Party; Protagonist 0x45a991, AddCNPC 0x45a32f),
  0x401150 = map-icon side (attitude pair, GameEntity ctor mbr_0x28). When the registry
  era names them, name them SEPARATELY.

## Batch-B gate census (2025-10-22) — call-shape router validated 16/16

Fixed-window scans BLEED (first pass gave false resolve/error hits into neighbors for 9
rows; extents from the function table are mandatory). Within true extents, arity gate
(0x59AF40) is present in 15/15 natives and absent in CommandInventoryWindow -> WildKnight's
signature-shape reclassification of the Command* rows is CONFIRMED by independent means
(__usercall did not lie). Max arity offset observed: 0x32 (EquipItem/UnequipItem fat
prologues with security cookie). Presence/absence of the other two gates is SEMANTIC, not
routing: resolve=0x59AEF0 iff the native takes an entity ID; error=0x59AD20 iff it prints.

| row (size)                     | arity | resolve | error | notes |
|--------------------------------|-------|---------|-------|-------|
| HideInterfaces 0x45a100 (0x34) | 0x11  | -       | -     | silent |
| SetMoveMode 0x45a140 (0x68)    | 0x14  | -       | -     | silent |
| StartDialogue 0x45a040 (0xc0)  | 0x16  | 0x2e    | 0x40  | full trio |
| AddCNPCToParty 0x45a2d0 (0xa9) | 0x14  | 0x2c    | 0x3e  | |
| OpenTask 0x45b900 (0x3e)       | 0x12  | -       | -     | silent |
| CloseTask 0x45b940 (0x3e)      | 0x12  | -       | -     | silent |
| AddEntry 0x45b980 (0x3e)       | 0x12  | -       | -     | silent |
| GetWeaponCombo 0x45a9f0 (0x4a) | 0x14  | -       | 0x3c  | own resolver sub_45B9F0; shipped stub |
| Protagonist 0x45a960 (0x88)    | 0x12  | -       | 0x5b  | NO entity arg - party via global 0x70bd98 |
| GetSquadAttitude 0x45aa90 (0x59)| 0x12 | -       | -     | silent, self-check return 1 |
| GetTeamAttitude 0x45aaf0 (0x59)| 0x12  | -       | -     | silent |
| EquipItem 0x45a580 (0x163)     | 0x32  | 0x4a    | 0x5c  | STRING-TAKER #2: sub_59AB20@0x45a5f7, arity 3 (entity, name, slot) |
| UnequipItem 0x45a6f0 (0x270)   | 0x32  | 0x4e    | 0x60  | full trio, fat prologue |
| GetPrimaryAttribute 0x45ac70   | 0x14  | 0x2c    | 0x3e  | GameCreature; returns *(GC+0x760 + idx-by-param) - item-69 era |
| GetDerivedAttribute 0x45ad00   | 0x14  | 0x2c    | 0x3e  | GameCreature; delegates GC::GetDerivedAttribute 0x409a60 - item-69 era |
| CommandInventoryWindow 0x45eac0 (0xa2) | NONE | -     | -     | __stdcall(), zero args, GUI plumbing via area_map_icons[0]->int_0x2c - NON-NATIVE CONFIRMED |

EquipItem internals (INVENTORY-era trunk data): resolve -> GameCreature cast -> inventory
ptr via IDA-cell-90 = +0x168 (same pre-cast accessor as GWR/IIE) -> Inventory->GameInventory
cast -> StringTree prep (meth_0x401d20) -> GameInventory vft[+24] find-by-name ->
vft[+56](item, slot, 1) equip. Verbatim quirk: sibling error strings differ by a space -
"Entity ID does not belong" (Primary) vs "EntityID does not belong" (Derived); keep both.
Also: CommandInventoryWindow uses the base-plus-delta codegen shape itself
(`v2 + *(v2->vft[1])`) - the vb-adjust idiom is program-wide, not a server-singleton quirk.

## Addendum 4 — derived HIT_POINTS: the off-by-one is a TRANSCRIPTION, not arithmetic
(2025-10-22, falsifiable claim resolved statically; supersedes the "x87 width" worry)

WildKnight's a63fffc port table shows derived HP off by one LOW at every exact divisor of
6000 (AP=20 -> 299). The chain (his structure, fully verified): HIT_POINTS case re-enters
ACTION_POINTS (= agility_0x774 + 20), stores AP to [esp+8] (89 44 24 08 @0x409b15),
0x409b19 `db 44 24 08` FILD, 0x409b1d `dc 0d 90 d5 64 00` FMUL qword [0x64D590],
0x409b24 FCOM [0x64D588]=0.0, zero-branch -> 1000 (mov esi,3e8h @0x409b45), else
0x409b30 FLD [0x64D580]=1.0 / 0x409b36 FDIVP / 0x409b38 CALL __ftol2 (0x616E24), then
FSTP pop - NO memory round-trip between FDIVP and ftol2 (so 80-bit if width mattered).

THE SIGN ERROR: stored double at 0x64D590 = 0x1.5d867c3ece2a5p-13, repr
0.00016666666666666666, exactly the correctly-rounded double of 1/6000 and -3.34e-17
RELATIVE *BELOW* the true value. AP*C lands BELOW AP/6000, the reciprocal lands ABOVE
6000/AP, truncation CANNOT drop the integer. Simulated both widths at the stored constant
(Fraction-based 64-bit-mantissa rounding + IEEE double): EVERY exact divisor of 6000 in
1..40 yields the EXACT value (20 -> 300, 30 -> 200, 1 -> 6000, 6 -> 1000, 12 -> 500).
Widths agree; the model's error direction was the bug.

THE DIAGNOSIS: IDA renders the constant in pseudocode as 0.0001666666666666667 (17 digits) -
which parses to C + 1 ulp, ABOVE 1/6000. 80-bit-precision arithmetic with THAT literal
reproduces WildKnight's full port table exactly: {1:5999, 6:999, 12:499, 20:299, 30:199,
7:857, 0:1000}. So the port is x87-faithful in shape but its constant was TRANSCRIBED FROM
THE DECIMAL PRINT, not taken from the image bits. The shipped exe should read 300 at AP=20;
if lockstep ever shows 299, the constant read here is the thing to re-check first (nothing
else in the chain can produce k-1: at C both widths are exact at divisors).

HAZARD CLASS FINALIZED - the print is not the value: pseudocode-print now has three shapes,
all same-session: (i) displacement units (0xC0 rendered as +48), (ii) offsets in wrong
units (258 vs 0x408), (iii) NEW and worst: CONSTANT DECIMAL RENDERING off by 1 ulp
(0.0001666666666666667 vs stored 0.00016666666666666666) - a transcription that survives
15/16 non-divisor probes and only fires at exact divisors. Rule: constants are adjudicated
against stored BITS (struct.unpack), never against any decimal rendering, and the divisor
lattice of any reciprocal-truncation formula is the natural detector for this class.
Also honest-flagged: my earlier "his fild/fmul cites are off by 1" was MY parse error -
0x409b19 and 0x409b1d are correct opcode starts; only 0x409b3c (ftol2 call, starts 0x409b38)
is mid-instruction. The ±1 rule applies to the person applying it too.

## Addendum 5 — spill question answered from bytes; the engine IS integer floor-division
(2025-10-22, after WildKnight's 9e11a57 retraction and its two counter-corrections)

CORRECTION TO ADDENDUM 4 (annotate, not rewrite): the pasted qword 0x3F15D867C3ECE2A5 was
MY transcription error - the image says 0x3F25D867C3ECE2A5 (bytes a5 e2 ec c3 67 d8 25 3f,
three independent reads agree). 0x3F15 = 1/12000; the hexfloat 0x1.5d867c3ece2a5p-13, the
BELOW/ABOVE analysis and every conclusion were unaffected. Hazard register gains costume
family #4 - TRANSCRIPTION COSTUMES: (his) reading VA without base/section mapping
(9.69e-312 garbage), (mine) hand-converting hexfloat to qword (one nibble). Same law as
EA±1 and pseudocode-print: regenerate digits from the artifact, never from memory.

SPILL, ANSWERED (objdump --32, file-offset 0x9b15 and 0x216e24):
Caller 0x409b15-0x409b3d: mov [esp+8],eax; fild [esp+8]; fmul [0x64d590]; fcom [0x64d588];
fnstsw; test ah,44; jnp->1000; fld [0x64d580]; fdiv st,st(1); call 0x616e24; fstp st(0);
mov esi,eax. NO real-value store between fmul and the call - only constant loads.
__ftol2 = classic MSVC register-direct: fld st(0) / fst DWORD[esp+18] (a FLOAT32 *probe*
for the correction chain - not the conversion path) / fistp QWORD[esp+10] (integer from
the REGISTER, round-nearest) / fild reload / fsubp (value - int, 80-bit) / sign+adc fixup
= truncation. The real never traverses the double grid. So the width dichotomy {300=spill,
299=80bit} is INVERTED: at the correct BELOW constant, no width can produce 299 (299 needs
r < 6000/AP, i.e. a constant ABOVE). Lockstep prior is not 50/50 - it is 300, and anything
else means a bytes read is wrong, not a width.

THEOREM (proven + empirically swept AP 1..4000, ZERO mismatches at either width):
shipped derived HP = floor(6000/AP) for AP>=1, and 1000 at AP=0 (zero-branch, the 0x3E8
immediate). Non-divisors sit >= 1/AP >= 1.7e-4 from an integer - beyond any ulp at any
width; divisors land a few ulps ABOVE the integer (BELOW constant) - truncation keeps them.
Consequence: the faithful port is INTEGER DIVISION plus the zero branch - no FP at all.
"6000/AP diverges at round numbers" was true only against the C+1ulp port; against the
exe, the obvious simplification IS the semantics. WildKnight's port producing 199 at AP=30
"with the correct bits" is impossible under either pure chain (both simulate 200) - that
pin is a THIRD (mixed) shape in the port, and dump-its-intermediates (sequenced printf of
t-bits and r-bits) will name it. If port pins 199 and lockstep exe shows 300, the pin is
codifying port!=exe; break it now, not in the run.

## Addendum 6 — B16 internals verified; the MIXED width is the third shape (corrects my absolutism)

WildKnight's ac88048 mechanism, verified bit-for-bit at relay time:
  k = 0x3F25D867C3ECE2A5 (compiler 1.0/6000.0 == image constant - input was RIGHT);
  s = fl_dbl(30*k) = 0x3F747AE147AE147B — BYTE-IDENTICAL to fl_dbl(0.005), whose own error
  lands ABOVE exact 1/200: the below-ness of the real product (~1.7e-19) is smaller than
  half-ulp (~2.2e-19) at that scale, so narrowing to double INVERTS the error direction.
  Then r = 1/s: at double -> exactly 200.0 (0x4069000000000000); consumed at 80-bit ->
  199.9999999999999958 -> truncate -> 199. Pure-double = 200, pure-extended = 200,
  MIXED (product narrowed, division wide) = 199. My addendum-4/5 claim "at the correct
  constant truncation can NEVER drop" was scoped to pure widths and stated as absolute -
  the mixed chain is precisely the "else" I said couldn't exist. Ownership: his measurement
  found it, my framing missed it. The EXE is pure-wide (no real store between FMUL and the
  register-direct __ftol2) so the lockstep pins stand: 6000//AP + 1000 at 0.
  Negative side pinned with opposite-direction care: AP=-7 -> -857.14... -> toward-zero
  int = -857; floor would give -858. C++ int conversion matches __ftol2's truncation; an
  integer port must use division semantics, not floor, on negatives.

B16 observables, all verified in the image:
  - sub_59AB20 (AMX string fetch) = !sub_5CE350 && !sub_5CE3F0 && LEN < BUFSIZE &&
    sub_5CE510(dst,src)==0: the `<` is STRICT (13-into-13 rejects) and short-circuit
    ordering guarantees the buffer is untouched on every failure. sub_5CE510 returns 0 on
    BOTH branches (0x5ce556, 0x5ce572) - no decode-error path exists to imitate.
  - GameInventory::meth_0x46a6c0 find-loop compares names with _stricmp at the exact cited
    0x46a777 - case-insensitive match is literal, not convention.
  - GameInventory::virt_meth_0x46a860 is the equippedness test: do-loop over cells +0x14
    through +0x2C with `while (v3 < 6)` at 0x46a8cc - SIX slots, membership, returning a
    char flag. (Attribution query: the "slot-10 find" he credits as making six "evidence
    not convention" is not in this session's lane output - the six is proven by v3<6
    regardless; whoever's number it is, cite the artifact.)
  Process rule hardened (second sample today): a bit-dump printing NOTHING with rc=53 is
  the loader dying before main (missing -static), not a program crash - check linkage
  before believing any runtime verdict (WildKnight hit it, JadeBear hit it on the d3d8 probe).

## Addendum 7 — B18 (UnequipItem) verified; unit-flip consequence; attribution fixed

WildKnight's B18 claims, all re-checked against the image at relay time:
- Arity copy-paste CONFIRMED: UnequipItem's gate registers the native under
  ArgList "EquipItem" at 0x45a72c (wrong-arity on Unequip complains about Equip).
  Copy-paste family now: this + shared "was unable to find the attacking entity."
  tail at 0x45a5dc/0x45a750 (function-name prefix differs, Attack vocabulary
  identical) + GetPerkRank/GetPosition.
- byte_65DC00 = 2A 00 ('*') read from .rdata, not inferred from the symbol name ✓.
- The wildcard lie CONFIRMED structurally: named-slot exact strcmp success ->
  payload call -> return 1 (0x45a920); wildcard strcmp success -> SAME payload call
  -> FALLS THROUGH to return 0 (0x45a93f region). Unequipping by "*" works and
  reports failure. Kept, not fixed.
- Two-comparison-rules pair CONFIRMED: named-slot path uses strcmp (0x45a8e7);
  slot-7 "everywhere" path routes through meth_0x46a6c0 whose matcher is _stricmp
  (0x46a777). "HUNTING RIFLE" reads equipped via IsItemEquipped but refuses a
  named-slot unequip. Neither path prefix-matches (both full-string compares).

UNIT-CONVENTION FLIP, with a sharper consequence than "adjacent lines differ":
EquipItem's payload call bytes @0x45a6ce are ff 52 38 = call [edx+38h] -> +56 BYTES,
slot 14 (the pseudocode's "+56" was truthful there). UnequipItem's pseudocode renders
its payload as "+15" — CELLS — real disp 0x3C = slot 15. The two natives use
DIFFERENT payload slots (14 vs 15), and a reader who took "+15" as bytes would probe
[edx+0Fh] (slot 3, mid-table nonsense) — pseudocode unit flip can silently MOVE THE
FUNCTION, not just misstate it. Instruction text settled both; census rule extended:
for vft-call citations, quote the bytes `ff /N disp`, never the pseudocode addend.

ATTRIBUTION FIXED (annotate): the six-slot equipped array (GameInventory vft entry 10
= 0x46a860, while (v3 < 6) over +0x14 cells) is WILDKNIGHT'S find; my earlier "your
slot-10" credit was his crossed wire, self-corrected on query. This lane's actual
outputs there: +0x168 accessor, EquipItem vft+24/+56 pair, [party+5] = +0x14 cell math.

EquipItem named-deferred, endorsed: payload GameInventory::Equip (0x46be40 via
[edx+38h]) bundles RB-tree bookkeeping, style-slot remap (cases 2/3/7 with
Get_mbr_0x98 reads), server-context-gated ApplyEquipEffects (0x46c1a1),
SetWeaponStance(0) reset, own double-equip log — an era, not a row. Unequip portable
alone because the siblings share only the preamble (visible in the decompiles).

SILENT-VERDICT FAMILY (third member, WildKnight's): accumulating message-capture that
stops at buffer-full with no error — 10 downstream misses reading as "natives went
quiet". Same shape as rc=53 loader death (this lane) and gate-kept-standard port-annot
(WildKnight, twice). Rule: a dead observer is indistinguishable from absent behavior;
observers must fail LOUD (overflow restarts / linkage checks / cited anchors).

## Addendum 8 — B19 verified; the relay-line hole (and the gate that closes it)

B19 byte verdicts (all WildKnight cites landed on EXACT opcode starts this time):
- Shipped-crash deviation CONFIRMED: 0x45b722 `e8 b9 ae 1b 00` -> call 0x6165e0
  (__RTDynamicCast) -> 0x45b727 `8b 80 08 04 00 00` mov eax,[eax+408h] — ZERO test
  between cast failure and dereference: null + 0x408 = AV. Port answers 0, declared.
- Unbounded slot walk CONFIRMED: 0x4693d4 `8b 4c 91 14` = mov ecx,[ecx+edx*4+14h] —
  first indexed read after entry; the null test (85 c9 74 15) comes AFTER the load.
  Shipped inspects whatever follows the six cells; port bounds it, declared.
- EntryByName: zero traces in src/ — retracted work left no corpse, verified.
- Declare-don't-assert (mixed bases indistinguishable in the port): endorsed as the
  POSITIVE twin of the dead-observer family — a test that cannot distinguish what it
  claims to prove is a silent prover. First declined assert of the session; correct call.

THE HOLE: relay-line commit hashes are unverified data. `git cat-file -t` says
f2b3a1c, 23418f1, 2521b31 AND f7f2d4c DO NOT EXIST — WildKnight has cited "suite
commits" that never existed across three batches (B17/B18/B19), because the suites are
INLINE in the batch parents (968343e, 5a4b0a3 each carry their +asserts in
ScriptNatives.cpp). Even his self-correction of the phantom re-entered the same phantom
class from memory. This lane's share: I twice relayed his lines without querying hashes
that `git cat-file -t` resolves locally in 20ms — acceptance is propagation. Costume #5
of the transcription family: hashes-from-memory; and its vector: un-queried relay.
NEW GATE (both lanes): every commit hash entering a relay line or README flip must be
re-emitted from `git log`, never from memory, and suites inline means cite the batch
parent only. Corrected relay line — his: b2693a8/cc8e54b/b44b459/e8e1a6a/b8d91b9/a63fffc/
9e11a57/ac88048/968343e/5a4b0a3; mine: 12a3a91/3031b1a/04087a6/e53eecb/544a1fa/f30a9fb/
c6e6023/4118cd4. Dead-observer family gains member four: an assert against a
block-local out of scope — lifetime failure reading as class failure; "observers must
fail loud" now in the suite DESIGN notes, not a war-story file (right vehicle).
