# Session 13 — Team add/remove primitives (+Entity+0x368 TeamId), OnDeath detail fix, armor-DR investigation (negative result)

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.

## 1. Team registry — `AddToTeam_0x56a330` / `RemoveFromTeam_0x56a1a0` (GameServer methods)

Both operate on `this` = the GameServer interface (the same object `global_area_map_icons[0]->int_0x18`
casts to `JE::Server→F3::GameServer` — used in Reload/Ammo server syncs).

`AddToTeam(entityNetId, teamId)` @0x56a330:
1. `ent = lookup_by_id(entityNetId)` (`meth_0x54be10`) — else debug
   `"AddToTeam() - Invalid entity ID %d.\n"` → 0.
2. `cur = ent->TeamId` — field at **`Entity+0x368`** (raw `*(v5 + 872)` with v5 = Entity interface
   base −4 bias ⇒ Entity+0x368; IDA's DynamicObject gap list currently unnamed — name it TeamId).
   If `cur == teamId` → 0 (already member).
3. `team = server.findTeam(teamId)` (`meth_0x5691d0(this, out, &teamId)`) — else
   `"AddToTeam() - Invalid team ID %d.\n"` → 0.
4. If already on another team → `RemoveFromTeam(entityNetId)` first @0x56a3af.
5. Push netId into `team+1` member vector (`cls_0x401b00::meth_0x437e30` = vector push_back).
6. `ent->TeamId = teamId`; return 1.

`RemoveFromTeam(entityNetId)` @0x56a1a0: same lookup pair; removes netId from the team vector
(`meth_0x40e780` = erase); clears `ent->TeamId = 0`; the "Invalid entity ID/team ID" debug strings
mirror. TeamId 0 = "no team" (vector never holds 0 — teams are 1-based; *(inferred)*).

Team record layout: `cls_0x401b00 {…, +0x00 team header, +1 = {netId vector}}` — the vector at
`team+1` (struct-into-struct: cls_0x401b00* +1 = +4 → member-list object). `findTeam`
`meth_0x5691d0` scans the server's team map by id. *(Map vs vector semantics of the team table:
inferred from call shapes; pin exact container when porting.)*

## 2. OnDeath correction to session 12 §2

Session 12 read step 3 as "by-netId lookup vft[+0x84] then dereg". Raw vft slots of the
0x66856C interface at +0x80/+0x84 (idx32/33) are **no-op stubs** (`JE::GUI::Window::virt_0x5a9320`
@0x5a9320 = empty body; IDA owner-label wrong — same misattribution family seen before).
Corrected OnDeath sequence @0x5509b0:
1. `RunScript("OnDeath", ...)`
2. server-gate (throw outside server mode)
3. `x = worldServer(0x66856C) vft[+0x84](this.Entity+0x1DC netId)` → **returns void/0 — stub**;
   result object passed next is the RAW lookup value; since the stub does nothing, step 4's
   object is `eax` untouched (`mov ecx,eax; call 0x56a1a0` — ecx = returned 0?? At 0x550a2d:
   `mov ecx,eax` eax from the stub call = undefined — decompiler shows `meth_0x56a1a0(obj)`;
   because the stub is `ret` without value, in practice ecx = whatever the callee left in eax).
   **Practical reading: `RemoveFromTeam(<netId>)` — the dying entity leaves its team.**
4. `ChangeState(7)`.
So death = OnDeath script hook + team removal + DEAD state. (The vft+0x84 stub slot is probably a
*compiled-out* "lookup" that the shipped link merged to the window no-op — treat as no-op.)

Also: **`JE::Entity::Kill()` @0x551ca0 has exactly one caller** —
`JE::ActionDie::virt_meth_0x55fb30` (GameActionDie::Execute's base step, session 8 §3 chain).
The whole death entry = damage → GameActionDie queued → Execute → Kill(→6) → pump → OnDeath(→7).
No other code path reaches state 6 in F3.exe.

## 3. Armor DR application — investigation result: NOT in the shipped flow (negative)

Session 6 §8 item 5 asked where equipped armor feeds the creature per-part DR arrays. Findings:

- The two SIB-readers of `+0x7B0`/`+0x7C8` (damage formulas @0x40a076/@0x41341e) address
  **`base + vbdisp + disp32` where base is the TARGET entity and vbdisp = `*(target+4)`** —
  creature-relative own fields (in-range for GameCreature 0xBDC ✓; NOT for GameArmor 0x7A8 ✗).
  So DR always read from the **creature**, as the GCRE docs named:
  `+0x7B0` = current per-part DR (6 dwords), `+0x7C8` = base DR.
- Equip-UI DR formatter `GameActionEquip::meth_0x43c570` @0x43c5fc/0x43c610 also reads
  `creature + 4*part + {0x7C8 | 0x7B0}` (arg-mode selects base vs current;
  `sprintf "%d-%d" (DR/2, DR)`; an armor RTTI-cast is made at @0x43c5d7 but its result is
  **unused** except for validation) — confirms the display model, not an application path.
- GIAM (ammo) and GIWP (weapon) readers share an identical 8-dword own-block layout at
  +0x3B0..+0x3CC (ammo: caliber@+0x3B0, dmg-type@+0x3B8, dmg-amount@+0x3BC, DT@+0x3C4,
  material@+0x3C8 — evidence: `OnServerUpdate_0x407270` @0x4074bf/0x4074c1 reads ammo +0x3B8
  as DT and +0x3B4; `CalculateDamage_0x4133d5-0x413450` reads +0x3C0/+0x3C4/+0x3C8; ctor stores
  @0x4145f3-0x414618; GIWP loader stores @0x415699+). **GIAR armor reader
  (`ReadFromFileBody_0x408680` @0x408680)** is a simple loop: 6 dwords @+0x3B0..+0x3C4 + one
  @+0x3C8 with NO per-part DR array — armor's own block is 8 stats dwords only.
- **Conclusion:** in shipped F3.exe no code adds armor DR into the creature `+0x7B0` array
  (no writer exists: SIB-scan shows only 8b reads at +0x7B0/+0x7C8; plain [reg+7B0] stores would
  be global-data false-positives, all checked and attributed to ammo/weapon/item ctors).
  Armor DR application is either (a) dead code (demo has no armor equip effect at runtime), or
  (b) happens via a rep-stos/copy block not matching this scan — low probability given the
  targeted re-verification this session. **Reimpl: skip armor→DR application for demo parity;
  keep the two creature DR arrays fed solely from GCRE current/base values.**
  *(Inferred, evidence-backed negative.)*

## 4. Quick donations to the reimpl

- `Entity+0x368 = TeamId (u32, 0=none)` — add to your entity model now; party = GameServer
  team records {id, member-netId-vector}; death auto-removes.
- Death chain entry is exclusively GameActionDie (queued by damage Apply) — nothing else calls
  Kill(); your client-mode death sim can call the same 3-step OnDeath minus the state flip.
- Ammo field map refinement (session 6 §4 rows superseded): caliber@+0x3B0, +0x3B4(?),
  damageType@+0x3B8, damageAmount@+0x3BC, DT-resist@+0x3C4, material@+0x3C8 (all ammo+... own
  bytes; weapon mirrors the same offsets with its own meanings per §3).

## 5. Still open

- `findTeam` container shape (`meth_0x5691d0` map walk details).
- `GameEntity+0x18` — unchanged (save-only).
- Gregorian exact getter slot — debugger item.
