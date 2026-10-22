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
