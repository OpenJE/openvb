# GameActionMoveTo — batch C's 7 rows, pre-ported evidence (all bodies read whole, raw bytes)

NiceTiger, 2026-10-22, read-only pass on `artifacts/F3.exe.i64` so the 7 uncited rows in
`tasks/R-USERRE-finish-user-functions.md:194-200` are translation-ready. Context: `src/F3/GameActionMoveTo.{hpp,cpp}`
ALREADY EXISTS and is already built + suite-wired (`SpawnMove`, 149 asserts, run-all-tests.nix:115) — the 7 rows
are the class's UNPORTED MEMBERS, not a new TU. Cite form per the lane law: `+0xHEX (slot N, body @addr)`.

## The class, from bytes

- `GameActionMoveTo` (RTTI `.?AUGameActionMoveTo@@`, size 72) DERIVES `ActionMoveTo` (`.?AUActionMoveTo@@`,
  also 72, vft `0x6940e4`); single inheritance, same address — `ctor_0x4056f0` (extent 0x4056f0-0x405701) calls
  `ActionMoveTo::ctor_0x55b300` then STORES the derived vft:
  `4056f8: mov dword ptr [esi], offset GameActionMoveTo_ActionMoveTo_Action_vftable_649c1c`.
- `ActionMoveTo::ctor_0x55b300` (extent 0x55b300-0x55b33a) field init, verbatim: `[+0x10]=1`, `[+0x4]=1`
  (bytes set by `mov dl,1` shared write... note: `mov [eax+10h],dl` then later `mov [eax+4],dl` — dl=1),
  zeroed `{+0x18,+0x1C,+0x20,+0x24,+0x28,+0x2C,+0x34,+0x35,+0x38,+0x3C,+0x3D}`, and **`[+0x30] = 5`**
  (the only non-zero dword — a kind/state literal any port must keep).
- Derived vft `0x649c1c`, all 13 cells read raw:

| byte | slot | cell | in batch C? | shape |
|---|---|---|---|---|
| +0x00 | 0 | 0x4058b0 | YES (listed as dtor row) | deleting dtor |
| +0x04 | 1 | 0x55b440 | no (base ActionMoveTo) | |
| +0x08 | 2 | 0x55b450 | no | |
| +0x0C | 3 | 0x4010d0 | no | |
| +0x10 | 4 | 0x55b4f0 | no | |
| +0x14 | 5 | 0x55b600 | no | |
| +0x18 | 6 | 0x55b3f0 | no | |
| +0x1C | 7 | 0x405720 | NO — unlisted sibling | float getter (read: below) |
| +0x20 | 8 | 0x405770 | YES | entity request forwarder, kind 434 |
| +0x24 | 9 | 0x405790 | YES | entity request forwarder, kind 416 |
| +0x28 | 10 | 0x4057b0 | YES | cast + two server calls + anim 29/32 |
| +0x2C | 11 | 0x406780 | YES | cast + server call + anim 1 |
| +0x30 | 12 | 0x4060b0 | no (already ported — SpawnMove era) | tick |

## The 7 rows, bodies whole

**ctor_0x4056f0** — 5 instructions, above. **dtor_0x405710** (0x405710-0x40571a): vft RESTORE then tail to
`ActionMoveTo::dtor_0x55b460` — `405710: mov [ecx], offset vft_649c1c | 405716: jmp dtor_0x55b460` (the MSVC
destructor prologue pattern; base dtor chain, no own frees).

**virt_meth_0x4058b0 (+0x00, slot 0 — deleting dtor, NOT a tick despite slot-0 habits elsewhere):**
`call dtor_0x405710; test [esp+4+param_1],1; jz skip; push esi; call free` — deletes-then-frees per flag.
Port shape: `operator delete` semantics; assert flag-off path frees nothing.

**virt_meth_0x405770 (+0x20, slot 8) and virt_meth_0x405790 (+0x24, slot 9) — TWIN FORWARDERS, kind-is-the-only-difference:**

```
405770: mov edx,[esp+arg_4] ; ecx=[this+0x14] ; eax=[ecx]
40577f: push 0 ; push 1B2h  ; push arg_0 ; call [eax+48h] ; retn 8   ; kind = 0x1B2 = 434
405790/4057a1: same body, push 1A0h                                ; kind = 0x1A0 = 416
```

Dispatch on the object at `mbr_0x14` (the MOVER/entity pointer — identity proven by slots 10/11 which cast
that same field Entity->GameEntity): `[mover_vft+0x48](0x1B2/0x1A0, arg0, 0, arg4)`, cdecl 4-arg, `retn 8`
(two incoming args forwarded in fixed positions with a hard 0 between). NO null test on `[this+0x14]` —
B8-family unguarded deref at `405777: mov eax,[ecx]`. Kind words 434/416 are NOT decoded here — port must
keep them opaque constants (named-deferred: mover vft+0x48 target semantics = JE::Entity request surface).

**virt_meth_0x4057b0 (+0x28, slot 10) — cast-gated anim chooser:**

```
4057c3: RTDynamicCast([this+0x14], 0, &Entity, &GameEntity, 0) -> esi; cast-fail -> SILENT retn
4057db: call 0x548f90(ecx = adjusted mover)      ; server-clock store (see below)
4057ea: call 0x54bd90(ecx = same adjusted mover) ; deque-pop (see below)
4057ef: cmp dword [esi+90h], 1
  equal  -> push 0; push 1Dh(29); call GameEntity::QueueAnimEvent_4673d0
  else   -> push 0; push 20h(32); call QueueAnimEvent
```

- `[esi+0x90]==1` selects anim 29 else 32 — state-gated pair, no other branch; failure mode is the cast
  (silent) — a non-GameEntity mover produces NOTHING.
- `0x548f90` (extent 0x548f90-0x548fbf, read whole): the singleton-adjust pattern calling server `[vft+0]`
  and storing result at `[mover+0x184]` — AND its null-singleton arm is `xor ecx,ecx / mov eax,[ecx]` at
  0x548f9e — B8 FAMILY MEMBER #5, live on the move path (guarded selection @0x548f9a, unguarded use
  @0x548f9e). Same call also appears in damage apply @0x45c307-era (banked).
- `0x54bd90` (extent read 0x54bd90-0x54bde2, whole): pops/decrements the mover's ring at
  `[+0x1B0] size, [+0x1B4] cursor, [+0x1B8] count` — wraps cursor, zeroes cursor when count hits 0, loops
  while count != 0 (with one redundant head-decrement check — port as written). This is DRAINING a
  per-mover queue slot region, NOT the +0x2E0/+0x304 entity deques — same CLASS of thing, different ring
  (type-vs-instance law kept).

**virt_meth_0x406780 (+0x2C, slot 11) — slot-10 without the chooser:** cast Entity->GameEntity (silent
fail) -> `call 0x548f90` (clock store, same B8 leg) -> `QueueAnimEvent(1, 0)` — anim id 1, no `0x54bd90`
ring-drain, no `[+0x90]` test.

## Unlisted sibling worth knowing (slot 7, +0x1C, 0x405720, extent 0x405720-0x405767)

NOT in batch C's 7 (verify the row list before porting — a zero-cited batch that misses its own slot-7
member is exactly the DisplayStringRef pattern): cast Entity->GameEntity (silent fail -> returns
`lpSrcStr_006cb56c` value = 0.0f float), else `fld [esi+0x94]` -> `__ftol2` (float->int), if int > 0
`fild` back -> return float(int), else return `flt_647B74`. A **clamp-to-positive-integer round-trip on
a float at mover+0x94** — read as "speed-like accessor" but NAME IS NOT EVIDENCE (law 1).

## Port fold-in notes (existing TU context)

- `src/F3/GameActionMoveTo.cpp` already ports slot 12 (tick, 0x4060b0 family, cited 0x4061xx era) with
  map/octree/blocker logic — the 7 new members land beside it; suite lines extend the WIRED SpawnMove
  file (now FastNova's single-writer set — route through them as agreed).
- **CORRECTION (TrueRaven's B-ORPHAN finding, verified here 2026-10-22):** my first draft of this line
  said "no new TU, no CMake change" — WRONG about the build, and wrong in the dangerous direction.
  `GameActionMoveTo.cpp` and `Display.cpp` appear in NEITHER `cmake.toml` NOR `CMakeLists.txt`
  (explicit list, no GLOB — greps returned zero hits both files, both lists), while run-all compiles
  GameActionMoveTo.cpp in exactly ONE row (SpawnMove, line 115) and Display.cpp in none. So the TU is
  green in the test harness and ABSENT from the shipped msvc image: suite-pass + msvc rc=0 say nothing
  about it (harness item 76: PROVE THE TU WAS COMPILED; precedent R-FINISH item 8, StringFormat.cpp
  hiding an LNK2019). Port work is gated-blind until FastNova rules build-or-delete; this doc's RE
  stands either way.
- Shared preamble with the damage path is now explicit: slots 10/11 reuse `QueueAnimEvent_4673d0` and the
  same `0x548f90` clock-store the attack apply runs — FastNova's GameActionAttack shard and this one touch
  `QueueAnimEvent` and the mover+0x184 store; if BOTH port it, that hook belongs to whoever lands first
  in `GameEntity`-side code — flagging early per TrueRaven's merge-hazard rule: **my rows want
  `QueueAnimEvent(id, param)` + the +0x184 clock-store + `0x54bd90` ring-pop as CONSUMABLE hooks on the
  entity side (MapEntitySpawn/GameEntity territory, i.e., FastNova's files), not re-declared in mine.**
- Absences, bounded per item 73 (extents above read whole): slots 8/9 do not touch any deque; slots 10/11
  do not resolve a handle (direct `mbr_0x14` pointer — no `0x54be10` anywhere in the 7 bodies); dtor
  frees nothing beyond the flag-1 path; nothing in the 7 reads/writes `+0x2E0` or `+0x304`.
- Unresolved, named: kind words 434/416 (mover `[vft+0x48]` request surface), mover `+0x90` state meaning,
  `+0x94` float identity, anim ids 29/32/1 semantics (GameAnimationCache era), slot 3 = 0x4010d0 owner.
