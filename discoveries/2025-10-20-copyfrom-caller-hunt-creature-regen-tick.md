# 2025-10-20 — Session 15: vft+0x28 (CopyFrom) caller hunt + creature +0x60 regen-tick decode

Target: ORIGINAL_REFERENCE. Read-only. NOTE: IDA MCP servers were DOWN this session (ports
8745/8746 dead — someone recycled the devenv processes); all analysis here used raw objdump +
byte scans. Items needing IDA re-verification are marked ⟲. Follows
[body-part-table](2025-10-20-body-part-table-item-copyfrom-plus18-verdict.md) §2 (whose "caller
scan open" this addresses) and closes creature slot +0x60.

## 1. `call dword ptr [reg+28h]` census (raw scan of .text; 186 memory-opcode hits, 84 calls)

Bucketing the interesting ones:

### 1.1 Server-interface calls (NOT entity CopyFrom)
`0x4683e6`, `0x552664`, `0x55ad3c` — all `ecx = [0x70bd98]+4` (GameWorld[0] iface) → their
vft[+0x28] = **factory "GetTemplateById" slot (dec 40)** used by CopyFrom-effect-chain, EEOV
chain, etc. (gameworld-interfaces doc slot table). Excluded.

### 1.2 The prime entity-slot caller: `JE::World/Mover` code @0x55b6be — **RETRACTED (same-day follow-up)**

**⚠ 0x55b6be is NOT the entity CopyFrom call.** Deeper walk (function body 0x55b4f0.., same
class region 0x55b100..0x55b700 — ~10 methods, all `mov esi,ecx` on one object shape) shows
`esi` is a **controller/transaction object**: +0x14 = Entity*, +0x18 = position-ish object,
+0x20 = id, +0x24..2C = copied position floats (from `[eax+4/8/C]` @0x55b540), +0x35 =
once-guard byte, +0x40/44 = pointers. `mov eax,[esi]; call [eax+28h]` @0x55b6b4 dispatches
**the controller's OWN vft slot 0x28**. The mapId (+0x1EC) and moving (+0x3A2) accesses are
real but go through the `[esi+0x14]` ENTITY — not `esi`; my "proven downstream" note was
wrong. The transaction itself is worth A's attention independently: JE::World entity-transfer/
teleport path (copies position, once-per-transfer guard, same-map check, helper 0x55b420).

### 1.2b CopyFrom caller: static analysis EXHAUSTED — runtime confirmation path only

- **Zero direct E8 callers** for all 11 CopyFrom stubs (GE 0x468710, Creature 0x40d460,
  Player 0x411c50, Usable 0x412630, Door 0x40f2f0, Container 0x409150, Projectile 0x411f10,
  Item 0x410520, Weapon 0x416a60, Armor 0x408770, Ammo 0x4082c0): the virtual is called ONLY
  through vft slot +0x28.
- The 84 `call [reg+28h]` sites cannot be disambiguated statically (every class vft has a
  slot +0x28; regfield filter already applied; the two .rdata immediates for 0x55b4f0 were
  misaligned byte-coincidences, re-checked).
⇒ **Canonical confirmation: breakpoint 0x468350 (GE CopyFrom body) during map-load/spawn in
lockstep — the return address gives the caller in one hit.** Mechanism (create → +0x28 apply)
remains supported by the three subclass chains, but the caller site is UNPROVEN. A: wire
template-apply only when the breakpoint fires.

### 1.3 Remaining engine hits (0x5458bc, 0x5461ee, 0x54682d, 0x54fbe6, 0x55ae96, 0x56385b)
Not yet bucketed — same method applies when IDA is back (check loaded vft family).

## 2. Creature slot +0x60 body `sub_409BE0` — regen-tick-like accumulator (ID-less honest decode)

Raw (objdump 0x409be0..0x409c9b; ecx = Entity* per stub `sub ecx,[ecx-4]` no-op for creature):
```
call JE::Entity::0x549090(Entity, arg)        ; engine side-effect first  @0x409bfa
if ([Entity+0x3A2])                          ; MOVER MOVING FLAG — yes, +0x3A2 = [vbdisp−0x45E]
     [Creature+0x28] = arg                   ; ⚠ slot dual-use? (GE+0x28 == net-id field!)
else:
     rate  = GetDerived(creature, 14)        ; @0x409c2a → sub_409A60 (session6-classid's
     delta = arg − [Creature+0x28]           ;   GetDerivedAttribute table — THEIRS, don't re-claim)
     cap   = GetDerived(creature, 0)
     field@Entity+0x174 += delta / 14?  (idiv [esp+10h] = param-derived divisor)
     clamp [Entity+0x174] to [0..cap]
     [Creature+0x28] = arg                   ; store last-tick param
```
Reading: accumulates into **Entity+0x174** (the session-5 "+0x174 dword, 6 usages, name
pending" field) at a rate derived from SPECIAL-derived-stat #14, capped by derived #0, only
while NOT moving, with `Creature+0x28` holding the previous timestamp/param. **Classic
in-combat/out-of-combat REGEN tick (HP or AP)** — but note the ⚠: the stored slot reads as
Creature+0x28 = GameEntity+0x28 = the NET-ID field, which the ctor fills from
WorldServer-vft[8]. Either (a) this virtual is NEVER called on net-spawned entities (their
+0x28 is a real net id — garbage as timestamp), or (b) my base is off by one slot (+0x24?)
because the caller passes a different handle. ⟲ needs one IDA/lockstep check; semantics
(regen-like accumulation while stationary) are robust, the slot identity is not.
- `JE::Entity::0x549090` pre-call: engine tick hook (slot +0x60 region of engine defaults).
- Also in this TU (raw, adjacent accessors): 0x409ca0 `get [creature+0x264]`; 0x409cb0 SSO
  string accessor {cap@+0x24C, buf@+0x238}; 0x409cd0 `get [creature+0x26C]` etc. — small
  getters over the +0x238..0x274 block that GCRE-v0 loop3/loop4 populate ⇒ those lists have
  live readers ✓ (partial answer to "list@0x25C/tree@0x260 semantics": exposed via accessors).

## 3. Updated creature Entity-vft body ledger (from the gcre-session13 doc table)

### 3a. +0xB4 body `sub_40d840` — bbox/yaw WORLD-VOLUME record factory (decoded raw)
`this` = entity handle (entry ecx = entity*, stub `sub ecx,[ecx-4]` no-op verified).
Reads `[entity−0x7f0..−0x7e4]` = **Entity bbox min/max floats (0x10..0x24)** and
`[entity−0x610]` = **yaw (Entity+0x1F0 ✓ reimpl excerpt)**; clamps each extent: `<0 → 0`,
abs, `<4.0f (0x64d7d4 = 4.000000) → 4.0` (minimum volume size), `_fabs` (0x616e24) combos,
`operator new(0x58)` (0x6054b0) then `sub_40c9b0(creature, record, {sx,sy,sz, minx…, yaw,…})`
with creature base = entity−0x804. ⇒ per-creature **collision/visual volume construction from
bbox+yaw**, min-extent floor 4.0 — this is the engine slot (default 0x550780) that creatures
specialize; directly relevant to A's spatial-grid objects (the record is 0x58 bytes). Full
field map of the record (`sub_40c9b0`) pending — likely an octree/OBB insert primitive.

| slot | body | status |
|------|------|--------|
| +0x28 | 0x40acc0 | CopyFrom ✓ (session 13) |
| +0x34 | 0x40d6b0 | skill-list builder ✓ (session 13) |
| +0x60 | 0x409be0 | **regen-tick-like, this doc §2** ⟲ slot verify |
| +0xa0 | 0x40bb80 | GCRE save writer ✓ (session 13) |
| +0xb4 | 0x40d840 | **bbox+yaw volume-record factory, §3a** (0x58-byte record via 0x40c9b0) |
| +0xe0 | 0x40ae60 | body-part registration ✓ (session 14) |
⇒ 5/6 closed this pair of sessions; +0x34's callers not yet traced. Only the ledger-table
"open" row remaining is trivial: none — all six have semantics; record-field map of 0x40c9b0 is
the new sub-item.

## 4. Questions / handoffs
- **A**: (1) breakpoint plan §1.2 — settle the CopyFrom caller; (2) if your live pump has an
  entity sitting still + a timer, §2's accumulator (+174 clamped by derived#0) is worth a
  lockstep value comparison to name it HP-vs-AP definitively. (3) IDA servers were down —
  when they're back, re-run xref on 0x55b6be's owning function for the interface identity.
- Everyone: `GetDerived` (0x409a60) cases 0..14 = derived-stat formulas table (STR+1)*25 etc. —
  already credited to session6-classid; §2 just consumes it.

## 5. Evidence index
Scan recipe: FF modrm sweep with modrm-regfield==2 filter (call). Prime caller window
0x55b69b..0x55b6e9; regen body 0x409be0 (moving-check @0x409c0f, GetDerived calls 0x409c2a/
0x409c42, accumulate @0x409c64..0x409c8c, store @0x409c90); accessors 0x409ca0/0x409cb0/0x409cd0.
All raw-objdump; imagebase 0x400000, .text raw@0x1000.
