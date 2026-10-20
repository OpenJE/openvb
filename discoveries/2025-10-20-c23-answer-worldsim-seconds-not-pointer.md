# 2025-10-20 — Session 34: ANSWER C-23 — IWorldServer slot 0 = WORLD-SIM SECONDS (scalar), not a pointer

Target: ORIGINAL_REFERENCE (IDA). Read-only. Direct answer to implementer-A chat C-23
(combat turn-cadence depends on this). Verdict: option **(a') IDA misnamed a trivial getter** —
the slot returns a PLAIN SECONDS COUNTER; `1000 * (*vft0)()` is real, not a pointer multiply.

## 1. Chain, fully resolved

* `F3::GetWorldServerInstance_0x401150` → throws unless server-flag; returns
  `global(0x70BD98)+4` = **IWorldServer interface at GameWorld+4** (vft 0x66856C ✓ roster
  doc's interface). ⚠ Interface `this` is GameWorld+4 ⇒ field offsets = interface+0x3C ⇒
  **GameWorld+0x40** — reconcile with the roster doc's "World+0x3C" phrasing (their scan used
  interface-relative offsets; free site 0x545642 presumably via the same slot-0 value — see §4).
* Slot 0 `0x436240` = `return [this+0x3C]` — a dword scalar (getter only).
* Neighbours (0x66856C dump): slot+4 `0x46f250` = **`return (double)[this+0x70]`** —
  IDB-verified **world-sim SECONDS getter**; slot+8 `0x46f260` = seconds SETTER; slot+0xC
  `0x46f270` small; +0x14/+0x18 engine Gregorian accessors (0x544a60/0x544a70 = calendar
  get/set pair — the YearPassed-native's "World-vb +116" object).
* Consumer `cls_0x5a9330::meth_0x5a94a0` @0x5a95aa: `mbr_0x10 = 1000 * seconds() + mbr_0x10`
  and later `@0x5a95ce`: `entity+668 = mbr_0x10 / 1000` — ms accumulator ↔ entity seconds
  deadline (Entity+0x29C = the turn deadline field, per A's C-24 + scripting doc). The ×1000
  and ÷1000 are one coherent unit conversion ⇒ **slot 0 returns seconds** (or sub-ms — see §2).

## 2.1 UPDATE (same-day, coordinator cross-check): slot-0 field = SECONDS-PER-ROUND CONFIG — **PROVEN via INI key, T-010d CLOSED**
Follow-up hunt (T-010d, requested by NiceFalcon) found the initializer:
`JE::World::virt_meth_0x546b50` (world config-load, called at world init):
```
this->mbr_0x40 = 6                                  ; @0x546b5b — the census default ✓
params = sub_56AF40(); sec = params->find("World")  ; INI/params section "World"
val  = section->find("Seconds Per Round")           ; @0x546c35 literal key
if (found) this->mbr_0x40 = atol(val)               ; @0x546c8f
… then Year/Month/Day/Hour/Minute/Second → mbr_0x14..0x28 ; (the roster's epoch block ✓)
```
⇒ GW+0x40 is **INI `[World] Seconds Per Round`, default 6, parsed once**; writers census =
{this init, net world-unpack 0x547e9e} only; **no clock-object writeback channel exists**
(the object's vft+88 has no path to +0x40 — usage-list-proven). The config reading is
FINAL; T-011 arms per-round quantum ×1000 with confidence. Side observation (flagged, not
resolved, outside T-010d scope): net-unpack @0x547e93 also writes a stream u32 into the
**pointer slot GW+0x3C** (clock object) — restore protocol or shipped quirk; if the client
ever net-rebinds the clock object, verify before trusting snapshots.
JoinCombat caller-set (3 attack-action xrefs @0x401229/0x4033d4/0x40359b, NiceFalcon) proved
join-on-attack — my "not evidenced" line withdrawn. Deeper tick pass (`JE::World::virt_meth_0x545d60`,
0x545ee0..0x545fbc): NON-combat branch advances ms via the **clock object @GW+0x3C** (vft+88
AddSeconds, ms-carry `%1000` on [GW+0x44]) and NEVER writes GW+0x40; the combat branch READS
`1000*[GW+0x40]` only. Writer census: no in-exe writer for GW+0x40 (2 scan hits were false
positives: 0x5a2996 = fresh-object store; 0x46af-region = stack). ⇒ GW+0x40 reads as CONFIG
(default 6 = seconds/round — A's "SecondsPerRound" label stands; the pending channel named
there was CLOSED above by the INI-key proof — census re-run via the typed usage list showed
only {config-init, net-unpack} as writers). T-011: arm deadlines
as per-round quantum ×1000ms, re-verify if the +0x3C object's AddSeconds is found to sync
+0x40). §2's earlier "seconds accumulator" lean and §2's "mbr_0x70 sibling" phrasing are
SUPERSEDED by this row (siblings are iface-view +0x70 = separate double get/set pair).

## 2. RESOLVED field identity (off-by-four untangled — verified at the dtor)
`JE::World`'s dtor frees `[esi+0x3C]` RAW (@0x545642→0x6162e0) with `esi` = **GameWorld base**
(the same dtor frees +0xF0/0xF4/0xF8/0xFC/0x100 exactly per the roster's subsystem table).
IWorldServer slot 0's `this` = GameWorld+4 ⇒ `[this+0x3C]` = **GameWorld+0x40 — a DIFFERENT
slot from the freed object**.
⇒ roster's "raw-malloc'd time object @+0x3C" = GW+0x3C ✓ stands (and its getter is some
other slot); **slot-0 returns GW+0x40, a plain scalar**, sibling slots +0x44/+0x64 (0x46f250
/0x46f270) are likewise scalar seconds-domain getters, +0x70 double-seconds set/get.
(a)/(b) collapses to: slot 0 = world-sim SECONDS counter (ms math at 0x5a95aa/ce confirms
units); the pointer object is one slot lower and unrelated to the ×1000.

## 3. `cls_0x5a9330` turn-queue semantics (bonus — the function A is porting; full body read)
State: {+0 active?, +4 per-turn flag, +8 pass-guard, +0xC cursor, +0x10 ms-accum, +0x14
mode-queue?, +0x1C ring(head)}.
Pass-1 (once, +8 guard): every queued entry with `state@+480==5` gets its `vft[+96]`
(TurnPrep-ish) called; every OTHER entry is **unlinked and raw-freed** (list node `free`,
count+0x20--). Cursor reset to head.
Pass-2 (per tick, param dt?):
  if +0x20 count: if +4 (per-turn mode): take current node's entity(+8); if
    `entity+793` (byte @Entity+0x319 ✓ A's turn fields): clear Entity+0x29C deadline,
    advance cursor; if +0x14 → recurse `meth_0x5a9330(0)` (next-entry step); else continue
    with the next node's entity.
  else (not per-turn): `deadline(Entity+0x29C) = accum/1000`, and pump
    `F3::ServerUpdate(entity, dt + accum)` — the accumulated-ms injection (matches A's
    accMs wiring); condition: `!entity+484 || entity+788` (both A-modeled).
  time source for the accum: `1000*seconds()+old` (§1).
⇒ the DISPATCH A withheld is now specified: deadline-gated round-robin over the ring,
per-turn mode short-circuits via the recursive step; entity fields +0x319/+0x314-ish (793/
788 dec) are the gate bytes; slot0×1000 legitimately feeds the shared ms clock.

## 4. RESOLVED for the roster doc (was: flagging; now: both readings coexist)
The dtor check (§2) proves the roster's +0x3C pointer claim AND my slot-0-scalar claim are
both right — they name different fields separated by the interface rebase. No port action
needed beyond: remember **interface offsets = GameWorld − 4** on this vft (0x66856C @GW+4),
and prefer citing absolute GW offsets from now on to kill this whole bug class.

## 5. Evidence
0x401150 body (flag check 0x401160, return GW+4 0x40118a); 0x66856C slots 0..+0x14 (0x436240,
0x46f250 double-get, 0x46f260 set, 0x46f270, 0x544a60/70); 0x5a94a0 (pass-1 0x5a94a3..0x5a9511,
accum 0x5a95aa, deadline store 0x5a95ce, accum-as-dt 0x5a95de); field consts: 480=0x1E0 state,
668=0x29C, 788=0x314, 793=0x319 (A's C-24 fields ✓).
