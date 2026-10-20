# Session 14 — the per-frame pump chain: client frame → World::Tick → per-entity ServerUpdate

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Motivation: the reimpl agent is building the per-frame live pump; this is the original's
complete frame-driven call graph, raw-verified. (IDA MCP went dark mid-session; the
offline DB + raw `.text`/`.rdata` scans + objdump `-b binary -m i8086 -M i386` carried the
remaining analysis — tool recipe in §5.)

## 1. Interfaces recap (all on the GameWorld singleton, global `0x70BD98`)

| C++ expr | slot table | role |
|---|---|---|
| `global+4` | **0x66856C** (IWorldServer) | server-side ops: +0x18 Update(0x544a90), +0x28 effect factory(0x46fa40), +0x2C PackState(0x545930), +0x44 entity-counter… |
| `global+12` | **0x668548** (IWorld client iface) | +0x0 Tick(0x544b30), +0x4(0x546140), +0x8(0x545820), +0x10 unpack-load(0x547e60), +0x14 VFX frame(0x548840), +0x18(0x545aa0) |
| JE::World vb (this+8, 0x390 entry) | **0x6685a4** (GameWorld override of World vft) | **idx16 +0x40 = 0x545d60 per-entity pump**; idx17 +0x44 = **0x545ff0** per-player pump; idx1 +0x4 = 0x546690 Initialize; idx13 +0x34=0x545430, idx14 +0x38=0x546fd0 |
| `global+0x154` | entity registry obj (cls_0x5a4340-ish) | vft+0x44 → 0x545ff0 route |

## 2. Frame chains (both entry points)

**Client render/loop frame** — `JE::Client::virt_meth_0x58c660` (frame fn; F3 tail via
`GameClient::virt_meth_0x45f000` @0x45f046/0x45f054 → `JE::Client frame @0x587ee0`):
message pump (`vft[+8]` packet loop while type≠2… standard net pump) → camera/Graphics/
renderer updates → **only when client-state `esi+0x58 == 3` (IN-GAME)**:
```
worldIf = global+12
worldIf->vft[+0x00](JE::TickMilliseconds())   ; 0x544b30 World::Tick — client path
worldIf->vft[+0x14]()                          ; 0x548840 per-frame VFX (octree hit queries)
if (!esi[264] && !esi[265]) worldIf->vft[+0x18]() ; 0x545aa0 renderer batch update
```
`World::Tick @0x544b30`: skip iff world-state mbr_0x2c==4; ms-delta accumulate into
`mbr_0x44` (minus pause credit mbr_0x54); then `(*(World+0x154)+0x44)` →
**`JE::World::virt_meth_0x545ff0(dtMs)` = hosted per-PLAYER pump** (NOT the seconds-carry):
global-fps meter `cls_0x581190` refresh → per loaded-map `sub_574F10(map, dtMs)` → per-player
`meth_0x548d60(entity+0x377)` + ModelInstance anim advance (`0x563dd0/0x563dd0→0x564280(1)`) →
player `vft[+8](dtMs)` each.

**Server/world update frame** — `JE::Server::virt_meth_0x552b60` (@0x552b60):
`net_server::virt_0x5a1bd0` (socket pump) + server-flag gate +
`global+4 vft[+0x18]` → **`World::Update @0x544a90`(TickMilliseconds)**: skip if state==4;
same ms accumulate (mbr_0x44/48/58, pause credit); gate `vft3[+0x14]`;
then dispatch on world-state mbr_0x34:
```
state 1 → vb(+this−4) vft+56   → 0x545d60  PER-ENTITY PUMP (§2 below)
state 2 → vft+60
state 3 → vft+64
state 0 → FatalError "Trying to update world when state is invalid"
then: mbr_0x64 = elapsed; mbr_0x68 += vft[3]()   (frames counter)
```

**Per-entity pump `JE::World::virt_meth_0x545d60(dt)`** — the one feeding
`F3::ServerUpdate` (session 12). Sequence:
1. Players list `byte_70BE0C`: drop entries whose `entity.state(+0x1E0)==8`
   (`GameServer::meth_0x5555a0` removal), call `cls_0x54aa20::meth` (per-player frame prep)
   for entries with flag +0x171 set.
2. `this->mbr_0x154 && *mbr_0x154` (has-players local_5).
3. World-vb `call [vft+8]` time-getter → `cls_0x5a9330::meth_0x5a94a0(this->mbr_0x8c, t)`
   (combat manager frame tick, session 8 §3 family).
4. `JE_PathSteerer_ProcessJobs(this->mbr_0xf0)` @0x545e18 → PathManager per-frame jobs.
5. If combat idle (`!(mbr_0x8c[8])`): per loaded-map (`mbr_0xe8` list) `Map::meth_0x56f0a0(map, time)`
   (map ambient tick), then **entity loop over `byte_70BE24` (all registered entities)**:
   ```
   for e in list:
     if e.state(+0x1E0)==5: continue            ; freshly constructed, not activated — skip
     dt = worldvb vft[+8]()
     e->vft[+0x60](dt)                          ; Entity::virt_0x549090: e+0x300 = dt; e+0x319 = 0
     dt = worldvb vft[+8]()
     F3::ServerUpdate(e, dt)                    ; session 12 state dispatch (+0xB8/C0/C4/C8/BC/D0)
   ```
6. Post-loop clock branch: players-online → `call [player-collectn+0x74] vft+8` + seconds
   carry `(*(World+0x3C) vft+88)(seconds)`; players-none → ms-scaled carry `(vft+4 speed ×
   Δms)/1000` into the same sink, `World.mbr_0x44 %= 1000`. **`World+0x3C` is the
   `JE::Gregorian` clock singleton** — confirmed this session: `World::Initialize_0x546690`
   @0x5467fc `call [vb-vft+0x10]` → GameWorld override `0x46fda0` → `sub_544D90` =
   `new JE::Gregorian(0x38)` ctor `0x544950(this, World−340)`; Shutdown `0x5455b0` frees it
   (as does the vb−0x12C getter of session 12 §5 — one and the same object reached through two
   tables).
7. `cls_0x5a4340::meth(this->mbr_0x154)` (players container post-tick), then
   `sub_5567F0(server, 0)` — **idle heartbeat**: prints `"Done"` through the
   cls_0x558740 message logger when the flag `entity+28902` is clear (the client-visible
   "pump is alive" tell).

Entity per-frame contract (step 5): `Entity+0x300 = dt ms` (frame delta for animations),
`Entity+0x319` cleared per frame (re-set by anim machinery = per-frame latch), then
ServerUpdate state dispatch. F3 game logic per state via entity vft (+0xB8 activate→state4,
+0xBC death 6→7, +0xC0 destroy→8, +0xC4 load→4, +0xC8 normal tick via 0x54c570, +0xD0
removed-deep 0x550ad0) — see session 12 doc for the table.

## 3. What the reimpl pump should look like (parity checklist)

- Drive: client frame (state==3 IN-GAME) → `World::Tick(ms)` → hosted path `0x545ff0`;
  server path → `World::Update(ms)` @0x544a90 → state==1 → per-entity pump `0x545d60`.
- World clock: `mbr_0x44` ms accumulator, subtract-pause credit (mbr_0x54/0x58),
  `mbr_0x48` = last tick stamp, seconds carry → world-events sink (vft+88 of +0x3c object).
- Entity pump order matters: state!=5 gate FIRST, then `e+0x300=dt` write, then
  state-dispatch. Frame delta is re-read per iteration from the world clock (monotonic).
- `sub_5567F0` "Done" heartbeat → observable in demo logs; useful as the reimpl's
  lockstep parity marker each idle frame.
- Maps tick (`Map::meth_0x56f0a0`) ONLY while combat manager idle (mbr_0x8c[8]==0).
- Player list maintenance (state==8 → unlink) happens at pump head — entities leave the
  world exactly one frame after reaching state 8.

## 4. Still open (frame-pump details)

- ~~The `this+0x3c` object~~ → **CLOSED: `JE::Gregorian(0x38)`** — traced end-to-end:
  World::Initialize @0x5467fc `call [vb+0x10]` → vb-vft idx4 → GameWorld override **0x46fda0**
  → `sub_544D90` → `new JE::Gregorian(0x38)` ctor 0x544950(this, World−340). Shutdown
  @0x5455b0 frees World+0x3C. Members **proven by Shutdown's destructors**: `+0x3C` Gregorian,
  `+0xF0` PathManager (`JE_PathManager_dtor_0x560bb0`), `+0xF4` **JE::Party** (`0x5a06e0`),
  `+0xFC` **Encyclopedia** (`0x58f450`), `+0x100` **StoreManager** (`0x5a2120`), `+0x154`
  players list, `+0x8C` Combat (cls_0x5a9330). `+0xF8` is likewise an Initialize-time factory
  child (dtor-with-flag in Shutdown; name pending — do not trust intermediate factory-index
  pairings across the base/GameWorld vb tables, they override at different slots).
  Note: `docs/f3/GamePathManager.md` already exists (14KB) — the +0xF0 shell created here is
  the F3-derived `GamePathManager(0x10)` (factory 0x46f9e0) wrapping the engine PathManager.
- `cls_0x581190` fps meter + `sub_4B81C0` display path (client HUD tie).
- `JE::Client::virt_0x58c660` head: the packet-type==2 early-exit + per-interface loop —
  cross-check with the client-packet docs rather than re-decode.

## 5. Tooling note (for the next dark-MCP stretch)

When the `ida_original_f3` MCP set is replaced/unavailable, `mcp_tools_enable` exact names
FAIL if the server re-registered under new ids; check `ps aux | grep idalib-mcp` and
`/dev/tcp/127.0.0.1/8745` first (both servers were restarted ~03:3x). Offline fallbacks that
worked this session: F3.json native tools + `objdump -b binary -m i8086 -M i386 -D
--adjust-vma=0x400000 --start-address=… /tmp/copy-of-F3.exe`. File-offset mapping (from the
PE section table): .text VA−0x400000, .rdata VA−0x400000, .data VA−0x400000, but
**.data1 differs**: rva 0x311000 ↔ raw 0x307E00 → fileoff = VA − 0x409200. Read the section
headers before trusting any offset.
