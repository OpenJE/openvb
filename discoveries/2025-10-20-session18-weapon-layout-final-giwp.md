# Session 18 — GameWeapon layout FINAL: GIWP rebase SOLVED (reader base == weapon base), definitive offset table

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Closes session 17 §10's "rebase unresolved" flag and SUPERSEDES parts of session 6 §9.
Method: dispatch raw `lea ecx, -0x430(%esi)` (0x41555f, `esi` = incoming Entity interface) +
the vbtable-verified **GameWeapon Entity-vb offset = +0x430**
(`2025-10-20-full-class-geometry-vbtables.md` §2 table: `0x650024 [-4,+0x42C] | this+0x430`).
⇒ interface − 0x430 = **weapon base**; every `this+N` in `ReadData_0x415190`/`sub_412CD0`
(char*-typed, raw-confirmed at 0x412cde-0x412d20) is a TRUE ABSOLUTE weapon byte offset.
(For the record: the earlier "Weapon−0x430" spelling in the fourcc-matrix doc means exactly
this — the adjustment cancels to base; no negative-block mystery after all.)

## 1. GIWP wire order → absolute GameWeapon offsets

`sub_412CD0` (v0 core, 14 reads) then `ReadData_0x415190` (v1-gated pair + GWAM list):

| wire# | abs | evidence / role | confidence |
|---|---|---|---|
| w0 | **+0x3B0** | f0 dword — ctor default-store @0x414601 (`xor` reg), aligned consumer scan (0x4135c0-0x413e00) shows NO primary-path reads of [w+0x3B0]; only Unload's `mov 0x3b0(%edi)` = **ammo**+0x3B0. Role: likely "weapon ammo-class/flags" or legacy — still unnamed | loaded |
| w1 | **+0x3B4** | **PRIMARY caliber** — `cmp [ammo+0x3B0],[weapon+0x3B4]` @0x413ad4 raw | locked |
| v≥1 a | **+0x3B8** | **primary ammo MODIFIER** — `mov [weapon+0x3B8],[ammo+0x3B4]` @0x413b8e (reload tracks loaded type); also v≥1 wire slot | locked |
| w2 | **+0x3BC** | **PRIMARY RELOAD-DELAY dword, ctor-default 2** (@0x4145ef `mov eax,2; mov [w+0x3BC],eax` — plain value, NOT a pointer: my earlier "deref" reading was wrong). `GameActionReload::OnServerUpdate` reads [weapon+0x3BC] (primary, @0x4074f2) or [weapon+0x3C8] (secondary, @0x4074fd) — chosen by action byte +0x2C — as the action duration seed | locked |
| w3 | **+0x3C0** | **secondary caliber** (decomp idx240×4; cmp w/ ammo+0x3B0 in `sub_413810` @0x413697-adj raw `0x3c0`) | locked |
| w4 | **+0x3C4** | **secondary modifier** (idx241×4, `sub_413810`) | locked |
| w5 | **+0x3C8** | **SECONDARY RELOAD-DELAY dword, ctor-default 2** (@0x4145fa, same reg as w2's 2) | locked |
| str | **+0x3CC .. +0x3E7** | **std::string** (JE::String 28B: buf[20]@+0x3CC, size@**+0x3E0**, cap@**+0x3E4** — per the EEN2-doc struct) — `sub_49C8D0` read @972 | name-ish, role open |
| w6 | **+0x3E8** | dword (ctor @0x4146da) | open |
| w7 | **+0x3EC** | **primary magazine CURRENT** — `add [w+0x3EC],ebp` @0x413b82; space = `ebp(=[0x3F0] cap) − [w+0x3EC]` @0x413b10; `cmp desc[7], [w+0x3EC]` gate @0x412f79 (shooting needs cur ≥ cost-per-shot); ALSO used as the **"primary loaded" boolean** in both reload paths (@0x413b04, @0x413d14 — nonzero ⇒ willReplace=1) | locked |
| w8 | **+0x3F0** | **primary CAPACITY** — `mov ebp,[w+0x3F0]`; 0 ⇒ `"doesn't even use ammo."` @0x413aae; used as the cap base in both reload paths (0x413b10 space calc, 0x413d20 sec path) | locked |
| w9 | **+0x3F4** | **secondary CURRENT** — UnloadSecondary zeroes it after transferring to stacks; @0x413d20 subtracted from cap | locked |
| w10 | **+0x3F8** | **secondary CAPACITY** — raw-locked @0x413cbe `mov [w+0x3F8],%ebp` (0 ⇒ "doesn't even use ammo" secondary string @0x413cc8), space = `ebp − [w+0x3F4]` @0x413d20 | locked |
| w11 | +0x3FC | dword (ctor @0x4146f5) | open |
| w12 | +0x400 | dword | open |
| w13 | **+0x404** | **GWAM attack-mode RECORD COUNT** — `if(!*(this+1028)) return 1; while(v6<count){Read GWAM; sub_4148A0}` @0x41520f-0x41526d (1028 = 0x404) | locked |
| runtime | **+0x408** | mode-record POINTER (`*(this+258)` dword-idx form = 0x408) — NOT serialized (v13 f14 lands at 0x404 max) | locked |
| runtime | **+0x424 / +0x428** | primary/secondary ammo-prototype CACHE ptrs (raw `mov [esi+424h]` @0x412f8c; miss-fill @0x412fae via `sub_408110(w+0x3B4, w+0x3B8)`; unload zeroes) | locked |
| runtime | **+0x40C / +0x418** | two 12-byte sub-containers initialized by the ctor (`lea` @0x4145a4/0x4145c2 — RbTree-header shape, likely per-weapon effect/attachment lists; also +0x410/+0x414 in their range) | unnamed |

Wire count summary: 5 dwords + string(28B) + 8 dwords (v0 = 13 reads incl. string halves),
v1 adds the +0x3B8 re-read + one more — consistent with `sub_412CD0`'s listed offsets
{944,948,956,960,968,972→1000,1000..1028}.

Ammo (GameAmmo) own absolutes (reader `Ammo−0x3D8`, ammo vb = +0x3D8 ⇒ **base too**):
caliber +0x3B0, modifier +0x3B4, dmg-type +0x3B8, dmg-amount +0x3BC, to-hit bonus **+0x3C4**
(raw `add [ammo+0x3C4]` @0x413088), range bonus +0x3C8, material +0x3CC?? — see session 9
registry + session 17; unchanged.

## 2. Corrections to prior docs

- **NOTE (RESOLVED same round): the "secondary anomaly" was a misread.** Full aligned dump of
  ReloadSecondaryAmmo (0x413c90-0x413d30) shows the path is EXACTLY symmetric with the
  primary: capacity from **[w+0x3F8]** @0x413cbe (zero ⇒ "doesn't even use ammo" secondary),
  space = cap − [w+0x3F4] @0x413d20, cal gate `cmp ammo[0x3B0], [w+0x3C0]` @0x413ce4, and
  the [w+0x3EC] loads (@0x413d14) are only the "primary currently loaded" boolean (mirrored in
  the primary path @0x413b04 where it checks primMod mismatch). No original bug; no caveat
  needed — dual-magazine weapons are safe to port from this table.
- Ctor classId write raw-confirmed @0x4145e4: `movl $6,[eax+esi+160h]` (disp 0x160 → Entity+0x15C
  ✓ the −4 rule in action) and vb-entity init `lea ecx,[esi+430h]; call Entity::ctor_0x5513a0`
  ✓ +0x430.
- Weapon ctor init list (aligned @0x4145f4-0x41465b) writes EXACTLY:
  {+0x3B0,+0x3B4,+0x3B8,+0x3BC(0),+0x3C0,+0x3C4,+0x3C8, +0x3CC-str(buf0@0x3D0,size@0x3E0,
  cap=15@0x3E4), +0x3E8..+0x404 (all 0), +0x408(modeRec)=0, +0x424,+0x428} + two 12-byte
  sub-container init loops at **+0x40C and +0x418** (lea @0x4145a4/0x4145c2 — likely RbTree
  headers: attachment/effect lists — name later). NOTE the ctor does NOT zero +0x410/+0x414
  inline (loop covers them).
- **session 6 §9 table**: offsets f2..f15 were written on the same base and are ALMOST right;
  errors: f3 listed `+0x3B8 capacity` (it's the modifier; capacity = f9 @+0x3F0), f4 `+0x3BC
  modifier` (→ secondary fields shifted), f13 `+0x400 count` (→ **+0x404**), f5@+0x3C0 mislabel
  as "damage bonus" (→ secondary caliber). The §9 caveat note about OnServerUpdate idx242/243
  bytes +0x3C8/+0x3CC stands.
- **session 17 §10**: the "reader base = weapon−0x40/−0x30" confusion is VOID — base == weapon.
  The raw absolute fields it locked (+0x3B4 cal, +0x3F0 cap, +0x3EC cur, caches +0x424/+0x428)
  are all re-confirmed here; its §5 to-hit gate offsets are right as stated.
- **session 15 §9** weapon-cache rows "+0x424/+0x428" ✓ still correct.

## 3. Reimpl action

- Weapon load can now be implemented 1:1: read GIWP wire → the absolute table above;
  magazine **state is serialized** (w7/w9 carry saved round counts; w8/w10 capacities);
  GWAM list at count-field +0x404 records; caches +0x424/+0x428 start NULL and are filled by
  `sub_408110(primCal, primMod)` / `(secCal, secMod)` when nonzero (session 9 flow) —
  note: the prototype fills happen ONLY when the (cal,mod) pair is nonzero (@0x4155b4-ish gates).
- Reload parity checklist: space = cap−cur; move = min(space, stack@Entity+0x3BC);
  weapon.cur += move; weapon.primMod := ammo.mod (the "loaded type" tracking); stack update or
  destroy; server syncs keyed by netId; secondary mirror via +0x3F4/+0x3F8/+0x3C4.

## 4. ADDENDUM — World subsystem members, factory chain fully verified (aligned raw)

From `JE::World::Initialize_0x546690` tail @0x5467ee-0x54684e (instruction-aligned): the
clock comes from the **vb vftable 0x668494 idx4 (+0x10)**; the other subsystems dispatch
through the **primary vftable 0x6685a4** (GameWorld overrides). Every row below is a
(store-site, call-site, vft-entry) triple:

| member | call site | vft.slot → factory | object |
|---|---|---|---|
| +0x3C  | 0x5467fc | 0x668494·+0x10 → 0x46fda0 → 0x544d90 | **JE::Gregorian** clock (0x38) |
| +0xF0  | 0x546806 | 0x6685a4·+0x1C (idx7) → 0x46f9e0 | **GamePathManager** (0x10, wraps the engine PathManager) |
| +0xF4  | 0x546813 | idx8 +0x20 → 0x46f820 | **GameParty** (0x18 over JE::Party 0x14) |
| +0xF8  | 0x546820 | idx9 +0x24 → 0x46f880 | **GameJournal** (0x1C over JE::Journal 0x10) |
| +0xFC  | 0x54682d | idx10 +0x28 → 0x46f8e0 | **JE::Encyclopedia** (0x10) |
| +0x100 | 0x54683a | idx11 +0x2C → 0x46f930 | **JE::StoreManager** |
| +0x154 | 0x546847 | idx13 +0x34 → 0x545430 (base, not overridden) | players list |

`Shutdown_0x5455b0` frees exactly these (dtor names: StoreManager 0x5a2120, Encyclopedia
0x58f450, Party 0x5a06e0, PathManager 0x560bb0, +0xF8 generic-vft delete, +0x3C raw free,
+0x154 sub_404B70+free) — the destructor identities independently corroborate the mapping.
Decompiler spellings `v13[123]`/`this[123]` in other docs = byte +0x1EC relative to the
Entity-adjusted base seen at those sites; where the base is the World object, +0x154 is the
players-list member. `0x46fb10` (misattributed to this chain in a same-day draft) is
IWorldServer 0x66856C idx18 (+0x48) — a server-interface slot, not a subsystem factory.
