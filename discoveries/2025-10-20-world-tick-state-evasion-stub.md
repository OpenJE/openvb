# 2025-10-20 — World tick driver, state enum semantics, evasion stub, tree #1 = owned list (session 24)

Target: ORIGINAL_REFERENCE. Read-only. Four resolutions from the session-23 leftover queue.

## 1. `JE::World::virt_meth_0x544a90(dt)` = WORLD TICK DRIVER — and the +0x34 state enum

```
if (state[+0x34] == 4) return                       // 4 = updates disabled
delta = dt - last(+0x48) + carry(+0x44); last = dt
if (carryFlag[+0x58]) { carry -= +0x58; +0x58 = 0 } // one-shot skip
if (this->vft[+20] /*slot5*/(delta)) {             // pre-tick gate (F3 hook: 0x546B50
                                               //   neighbor — InitClock also slot5?? recheck)
    switch (state-1):
      0 → outer.vft[+56](delta)
      1 → outer.vft[+60](delta)
      2+→ outer.vft[+64](delta)                    // per-state update bodies
    +0x64 = carry; +0x68 += outer.vft[+12]()      // accumulators/frames
} else FatalError("Trying to update world when state is invalid")  // when state==0
```
⇒ **+0x34 is the world run-state {0=invalid, 1..3=active kinds, 4=updates-off}** —
PackState's `==3` check is "state must be normal(3)". The still-missing *writer* (reimpl
runtime watch, requested earlier) now has a name to log: state transitions ∈ {0..4}.
Fields: +0x44 carry, +0x48 last-dt, +0x58 skip-request, +0x64/+0x68 tick accumulators —
these are 3 of the 8 "meta ints" of PackState §1(session15). (Clock fields were session 23.)

## 2. Evasion = STUB in this build

GameCreature+0x7FC (the EVASION derived attr's storage) has exactly 3 accesses in the whole
binary: ctor-set 0 (0x40d2da, `ebx`=0), read by GetDerivedAttribute (0x409ab0), read once in
actionbar code (0x41886b). Nothing ever writes a nonzero value ⇒ **EVASION always 0**;
to-hit's "armor" term is therefore ONLY the called-shot part table in the shipped demo.
Reimpl: hardcode 0 (with a comment), don't build an evasion pipeline.

## 3. Called-shot part NAMES: not in the exe

No head/torso/limb strings near 0x64F858 (or in the .rdata window scanned); the difficulty
vector [10,40,40,40,40,60,120,60,(sentinel 0x7F7F7F7E)] is index-only. ⇒ part labels live in
interface resources (targeting-window MSG/text) — out of exe scope; order can be pinned
behaviorally (which index the targeting UI sends) or from VB2 data files. Do not guess.

## 4. World tree #1 (+0x2C/+0x30/+0x34-as-count… caution) = OWNED-OBJECT LIST

`World::virt_meth_0x5460d0` walks +0x30 circular list, `(**[node+8])(node,1)` (deleting
dtor call) per entry, then frees the list head — a shutdown/clear path. Combined with the
ctor (sentinel) and 0x5480e1 usage, tree #1 holds **world-owned objects**. Not the entity
registry (that's the global 0x70BE0C list — session 15) and not maps (+0xE4 — session 16).
Remaining readers to name it precisely: 0x544b32, 0x5480e1 (deferred).

## 6. BONUS (found chasing tree #1): `JE::World::UnpackState` @0x547e60 — the PackState mirror

The tree #1 reader turned out to be the whole world-state INBOUND path:
```
1. pre-call outer vft[+80] hook (vb-adjusted base)
2. read 8 world ints (stream via 0x45e500): → +0x3C,+0x40,+0x64,+0x68,+0x6C,+0x74,+0x78,+0x7C
   (PackState's first two ints are +0x44/+0x48 — field lists do NOT perfectly mirror, see §7)
3. entity loop: u32 id read as 24-BIT (3 bytes LE), 0-terminated;
     outer-vb vft[+24](id) → CREATE ENTITY BY ID (same create-thunk family 0x46fdc0→factory
     as the EME2 spawn path!) then Entity::UnpackState_0x54e690(newEntity, id, stream)
   ⇒ CONFIRMS the reimpl session-11 hypothesis at world level: create-then-unpack.
4. map loop (4-byte ids): outer vft[+28] get/create map; Map::meth_0x572f10(map, substream)
   per-map unpack; map registered by NAME into container at world+gap90[72]-ish
5. tail: writes this->+0x2C = 3  ★ same "normal" enum value as PackState's +0x34==3 check
```
Buffer helpers throw `net::underflow<net::buffer>` ("Buffer underflow.") — pack-side overflow's twin.

## 7. Geometry contradiction — RAW FACTS FINALIZED, one experiment left (my earlier
   "+0x24 base" guess was WRONG — corrected by raw bytes)

Ctor raw (0x548ae0 window): `lea ebx,[esi+0x2C]; mov ecx,ebx; call 0x56e0B0` then
`[ebx+4]=ret`, `[ebx+8]=0` ⇒ the 3-dword block at **+0x2C/0x30/0x34** (self-ref/head/count)
is exactly where session 14 said — a real circular-list structure, and `0x5460d0` shutdown
genuinely walks +0x30 calling deleting-dtors on `[node+8]`.

The paradox: tick gates on +0x34 ∈ {0→FatalError, 1|2|3→update-kind dispatch, 4→skip},
PackState requires +0x34==3, and UnpackState writes 3 to +0x2C.

**Leading unified model (unproven): +0x34 IS the list count and the engine overloads
count-as-state** — a healthy world owns exactly 3 subsystem objects (count 3 = "normal"),
0 = invalid, 1/2 = partial-startup kinds, ≥4 = don't-tick. UnpackState's +0x2C=3 is then
either an adjusted-view artifact (its body opens with a vb-adjust call ⇒ incoming `this` may
not be the ctor's base) or a true sentinel overwrite (benign if nothing re-walks the list).

**Closing experiment (delegated, definitive, ~5 min):** hardware write-breakpoints on
world+0x2C and world+0x34 through startup + one warp; log (value, return address, this).
Settles base-identity AND count-as-state in one run. Static evidence exhausted — NOT
resolved until then.

### BONUS from the same hunt: JE::Network config map + INI keys (0x5441f0 = config writer)

```
 +0x04                    → [Network] configuration
 +0x08 +0x0c +0x10 +0x14  → [Server] port / debug / rate / timeout
 +0x1c +0x20 +0x24 +0x28  → [Client] port / debug / rate / timeout
 (written via cls_0x56af40 — confirmed: it is THE settings/ini map, not "VFX_Parameter")
```
openje's Network should carry exactly these fields + key names; the reimpl's sys.ini parse
should see these sections when the original WRITES config.

## 8. Queue update

1. §7 write-breakpoint experiment → reimpl (value + return-address + this settles it).
2. Pack↔Unpack full field diff (both sides documented; +0x44/+0x48 read-path still unknown).
3. Standing: clip names, sys.ini key coverage vs §7-BONUS table, cls_0x56af40 →
   "Settings" rename sweep (affects earlier docs' vocabulary — noted, not retro-edited).
