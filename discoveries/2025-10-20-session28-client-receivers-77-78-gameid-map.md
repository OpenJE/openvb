# Session 28 — client receivers for cutscene packets 77/78 + incidental GAME_ID mappings from the recv switch

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Completes session 27 (server senders) on the client side, from
`F3::GameClient::recv_packet_461a10` (full decompile this session).

## 1. recv switch structure

Pre-dispatch: log lines for 41/42 (`GAME_ID_ITEM_EQUIP/UNEQUIP`), then a generic
`vft[+0xCC](packet)` entity-level handler runs FIRST and, if it consumes the packet, the
switch is skipped (return at 461a77 path). Switch cases (hex = packet id):

| case | id | meaning (receiver side) |
|---|---|---|
| 'M' | **77** | **cutscene start**: reads {name(28B string), u32@var_148, u32@var_144, byte}; resolves controller `= [clientObj+0x2C]+52` (a.k.a. `global_area_map_icons[0]->int_0x2C + 52` — the same 0x2C-object HUD/interface singleton family seen in session 8); on `Get_mbr_0x20_0x406a50(controller)` truthy: `meth_0x459e70(controller, name, u32)` → `meth_0x495220(0)` → `meth_0x4952a0(controller)` → `cls_0x4b28a0::meth_0x4b4f00(&unk_707E48, controller)` → `[clientObj+0x108] = 1` |
| 'N' | **78** | **cutscene stop** (payload-free): `sub_45E6C0()` @0x461e58 |
| 'U' | 85 | **ENTITY_DAMAGE receiver**: 3 u32s {?, dmg, targetId} → `meth_0x54be10(targetId)`; miss ⇒ `"Couldn't find target entity %d"`; non-GameEntity ⇒ `"Target entity wasn't a game entity %s"` (entity's own name string printed via +0x1C4/0x1D0 SSO fields); success ⇒ `_itoa(dmg)` + string wrap → `GameClient::meth_0x587a70` (floating damage display path @0x461b79). Raw receiver block starts 0x461aa2 |
| 'V' | 86 | **{los, speed, targetId} → raw `mov [eax+90h],edx; mov [eax+94h],ecx`** @0x461dc9/0x461dcf → **GE+0x90 = LOS, GE+0x94 = speed** (confirms the parallel `ge94-speed-*` doc — this is the wire that writes them) |
| 'W' | 87 | raw `mov [eax+20h],ecx` @0x461eaa → **GE+0x20 = MaxHP** ⇒ 87 = SETMAXHP-class update |
| 'X' | 88 | {id, value} → `0x4629c0(target, value)` — **confirms session 20-H/21**: the client applies SETCURRENTHP through the same clamp (case at 0x461efd) |
| 'Y' | 89 | RTTI→GameEntity then `meth_0x463b70(target, u32)` (entity-stat setter cluster; name pending) |
| 'a','b','c' | 97,98,99 | journal ops → `RTTI Journal→GameJournal` + sub_46C7A0/840/6C0(id, param2) — their packets-97-99 journal claim, receivers located |
| 'f','g' | 102,103 | WEAPON_SET_PRIMARY/SECONDARY_AMMO: `RTTI→GameWeapon` guard then `meth_0x412c10/0x412c70(cal, mod)` — these are the CLIENT twin of the reload ammo-type writes (funcs = the weapon +0x3B4/+0x3B8 and +0x3C0/+0x3C4 setters; the strings name them) |
| 'i' | 105 | {28B name} → `sub_461960` |
| 'l' | 108 | container contents: controller chain like 77, `meth_0x459e70(name,…)` + refresh + `[clientObj+0x108]=1` |
| 'm' | 109 | floating text → `cls_0x558740::meth_0x5583e0` (same HUD printer as the debug lines; color bytes from packet) |
| '2' | 50 | item remove: inventory `vft+8` erase with error strings "ENGINE_ID_ITEM_REMOVE - …" |
| '`','d','e','T' | 96,100,101,84 | misc (two share LABEL_33 controller refresh) |
| default | — | fallthrough to `JE::Client::virt_0x58c660` (engine-level packets incl. 41/42/67/etc.) |

## 2. Cutscene round-trip now complete (sessions 27 + 28)

`StartCutscene(id) (AMX)` → server: world-vb vft+160 + packet 77{id, mapRef-serial} →
client: controller start (0x459e70 name-resolve + 0x495220/0x4952a0 + HUD enable flag
+0x108=1). `StopCutscene` → vft+164 + packet 78 → `sub_45E6C0`.
⇒ Their roster §8.1 fully closed on both ends; the client controller object =
`[AreaMapIcon global + 0x2C]` child at +52 (one level deeper than the session-8 reading).

## 3. For the reimpl (names for their enum + one caution)

- Name cases: 77=CUTSCENE_START, 78=CUTSCENE_STOP, 85=ENTITY_DAMAGE, 86=ENTITY_?PAIR
  (writes GE+0x24 AND +0x28 — plausibly the "set hp+netid stamp" pair; do NOT trust the
  slot-name without their own save-verify), 87=ENTITY_SETMAXHP? (GE+0x20), 88=SETCURRENTHP,
  97-99=JOURNAL_*, 102/103=WEAPON_SET_PRIMARY/SECONDARY_AMMO (their strings literally say
  so), 108=CONTAINER_CONTENTS?, 109=FLOATING_TEXT.
- The pre-switch `vft[+0xCC]` handler is now identified (`GameClient::HostConsumeGate_0x45e9a0`,
  vft idx51): if the host flag (`&1`) is SET → it consumes **84 ∪ {86..104}** with a HUD refresh
  (`vft+120`) and returns 1, SKIPPING the switch; if clear → delegates to
  `JE::Client::virt_0x58c380` (engine-level consumption), else falls through.
  ⚠ ORDERING CORRECTION (session 30 §1): flag-clear makes the gate return 0 WITHOUT delegating —
  remote clients run the F3 switch alone; the delegate-to-engine path is host-mode only.
  ⇒ **Mode split, correcting §1's framing:** the switch receivers documented above are the
  PURE-CLIENT (remote) application paths. In the single-player host loopback those entity
  packets are consumed-and-refreshed by the gate because the host side already applied them
  authoritatively (Apply/clamp ran locally in-process). So session 20-H/21's "client applies
  88 via recv" is the REMOTE path; demo loopback discards it at the gate. Port BOTH paths,
  gate ordering first.
