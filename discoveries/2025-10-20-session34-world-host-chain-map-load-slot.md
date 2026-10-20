# Session 34 — SetFocusEntity adoption internals: host chain resolves to World+0x160 (352-adjust), slot bodies 0x546FD0 = LOAD-MAP (none.map fallback, VFX_Parameter["Mainmenu"]."Map Name"), 0x545490 = WorldMap factory, 0x5452B0 = Party factory; exact vft+84 target needs ONE debugger read (documented recipe)

Target: ORIGINAL_REFERENCE. Read-only. Completes (to static limits) the host vft+84/+40 item
the coordinator green-lit after 10f.

## 1. Host singleton identity — FIRM

`F3::global_cls_0x616328_0x70bd98` = a **JE::World** object: its ctor (`JE::World::ctor_0x548a80`,
0x548a80) throws `"World object already exists."` (JE::Common::Exception) if the global is
non-null, then stores `this` @0x548CCF into 0x70BD98. World primary vft = 0x691A1C (DB row
`.?AUWorld@@`, 20 entries).

## 2. The multi-inheritance host chain (used at 0x545820, 0x546140, and ~50 other sites)

Idiom: `host = World + *(vftB[1]) + 8` where `vftB = *(World+0x8)`.
- Ctor seeds `*(World+0x8) = &0x691A94` (param_1 mode) and `*(World+0x10) = &0x691A8C`;
  both blocks = {−4, N} MI-descriptor pairs: `0x691A94 = {-4, 352(0x160)}`,
  `0x691A8C = {-4, 344(0x158)}`.
- ⇒ canonical host base = **World+0x160** (the 352 branch; the one whose ctor store at
  0x548AD2 lands via `(&mbr_0x8 + *(mbr_0x8+4))` — i.e. `World+0x160` receives
  `World::\`vftable'` — the vft whose IDA-labeled slots are the +84/+40/+58 callees).
- `World+0x158` (344 branch) = `cls_0x569f10` 16-byte pool manager: 2000-entry free-list
  pool (`meth_0x569d50` ×2000 ctor loop, +0xC = 2000 count) — object id-pool, not an
  interface. (Consistent with ctor writes `*(this+344) = param_1` byte.)

## 3. What the host vft slots do (bodies verified; slot-ID mapping caveat below)

Reading the World `vftable-3` neighbourhood at 0x691A1C+0x38.. as the candidate slot set
(DB `offset` entries ×4; note IDA decompile displays show `+N` BYTES on the vft pointer):
- entry[14] = **`0x546FD0`** = **LOAD-MAP**: `World::ChangeState(3)`; default name `"none.map"`;
  map name resolved from **VFX_Parameter ini registry** section `"Mainmenu"`, key
  `"Map Name"` (`sub_56AF40` + `meth_0x4339b0` find, `sub_4DF070` {Map Name} lookup), falling
  back to `"none.map"`; if `ResourceSystem::meth_0x49abd0(name)` succeeds →
  `host2.vft[+0x58](name)` (`*(World+0x158)+4` chain — the 344 interface!) then
  `vft3[+4](&name)`. ⇒ THE map-load wire + the ini keys the save/warp protocol needs:
  `[Mainmenu] Map Name` — matches YoungStorm's <gv_NAME> ini finding from another direction.
- entry[12] = **`0x545490`** = `WorldMap` FACTORY: `new(0x80)` + `JE::WorldMap::ctor_0x599f90`,
  returns fresh object (no args) — map-session object creation per load.
- entry[8] = **`0x5452B0`** = `JE::Party` FACTORY: `new(0x14)` + `ctor_0x5a0760` — party
  singleton creation (session 13's Team registry is separate; Party = player group).
- entry[13] `0x545430`, entry[15] `0x5455A0`, entry[16] `0x545D60` (session-14 per-entity pump
  region!) — the World interface rows share bodies with the pumps documented in session 14.

## 4. The unresolved 4% — honest line + 1-read recipe

`SetFocusEntity` calls `host.vft+84(id)` / `controller.vft+40(id)` through the RUNTIME vft
stored at World+0x160 (ctor seeds one value; engine-attach may overwrite — I could not pin
which class's vft lands there purely statically; the {−4,352} descriptor says World+0x160 is
the *primary-ish* base of an engine interface built elsewhere, ctor line 0x548AC8 shows the
seed = same object as +0xC store). **Recipe:** break @0x5458AF after `World+0xD4=id` write,
`p/x *(X86_32*)World+0x160` → dump entries [+0x54]/[+0x28] (one read; settles both).
UltraBear or any debugger session can close it in a minute; semantics (§6 of session 33)
are already pinned regardless.

## 5. Cross-links

- Session 33 §6 (SetFocusEntity shape) unchanged; this file pins host=World+0x160 chain.
- Session 31 §1: `sub_41D050 = World+0xC` accessor — consistent: three interface bases
  (+0xC/+0x18/+0x24) + engine-host sub-objects at +0x158(pool)/+0x160(vft'd).
- Save/warp protocol (implementer docs): ini path `[Mainmenu]/Map Name` + `none.map`
  fallback + ResourceSystem gate + ChangeState(3) ordering = the exact map-load preamble to
  replicate; WorldMap 0x80 allocation per load.
- Tooling: this session used IDA get_bytes/decompile exclusively after the objdump flat-image
  drift discovery (session 33 §4) — 0x691A1C dump via offline python matched IDA here
  (.rdata at VA−0x400000 is fine; the drift bit only in high .text).
