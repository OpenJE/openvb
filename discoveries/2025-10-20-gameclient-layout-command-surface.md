# 2025-10-20 — Session 38: GameClient layout + THE FULL CONSOLE COMMAND SURFACE (~130 commands, two ctor tiers)

Target: ORIGINAL_REFERENCE (IDA). Read-only. GameClient thread (coordinator-assigned). The
headline is not the layout — it's what the ctors turned out to contain.

## 1. ⭐ The console surface IS the client constructors

`F3::GameClient::ctor_0x460b30` (23 registrations) and its base
`JE::Client::ctor_0x58e900` (~110 registrations) each call
`F3::RegisterCommand(name, handler)` — together they are the shipped command registry.

**F3 tier (23):** EscapeMenu, AttackModeCycle, **AttackModeSelect→RET-STUB**,
**CharacterRecordWindow→RET-STUB**, FeedbackWindow, GameOptions, HotKeyOptions,
InventoryWindow, LoadGameOptions, MultiplayerOptions, PipBoy, **SneakToggle→RET-STUB**,
UseSkill, VideoOptions, **WeaponLinkToggle→RET-STUB**, WeaponToggle, ActionbarWindow,
ForceEntityStep (0x460720), OptionsWindow, TestAnim (0x460920), ToggleRain/Snow/Wind
(0x4606c0/CommandToggleSnow/0x4606e0 — the fact-sheet weather trio 110/111/112 ↔ handlers
named here for the first time at console level).
⇒ **4 named-but-dead commands** (stubs = 0x404b70 ret): AttackModeSelect,
CharacterRecordWindow, SneakToggle, WeaponLinkToggle — UI/stealth features registered in
data, unimplemented at F3 layer. Parity: register+stub, don't omit.

**Engine tier (~110, all live-looking):** session/net: StartMap, RequestNewGame, NewGame
(≡RequestNewGame), Connect, Disconnect, Kill, StartDialogue, EndDialogue/EndDialog,
CreateEntity, SetScript, ToggleStatistics; rendering toggles: ToggleFullscreen/Lighting/
Shadow/BilinearFiltering/StartupMovies/CameraFog/Wireframe/Transparency/OctreeDrawing/
MipMapping/HeightFog/OctreeNodeDebug/FogOfWar/FOWAutomap/FOWLOS/VisualEffects/Water/
Building/Scene/Entities/GUI/Scripts(+Disable/Enable/OneScriptPass), DebugLight,
DebugOctreeLight; camera suite: CameraReset/SetTarget/Azimuth(+Inc/Dec/Speed)/Elevation
(+Speed)/Distance(+Speed)/FOV/TopDown/Top/Down/ScrollSpeed, Scroll×{L,R,U,D}×{On,Off},
Zoom×{In,Out}×{On,Off}, Rotate{Left,Right}, Tilt{Up,Down}, MouseRotate_{On,Off},
ToggleConstraints; debug/misc: ScreenCap, ShowCursor, Warp, GuiOptimizationLevel,
ConsoleWindow, DebugWindow, StackWindow, DebugTools, FontTest, Get/SetGlobal, VRAM,
ListTextures, ToggleSampling, VSync, AdapterInfo, SetDisplaymode, MemoryPoolTest, FadeIn,
FadeOut, MovieTest, EditToken. Full EAs: each `RegisterCommand` call line in
0x58ea4e..0x58eff1 / 0x460b6f..0x460c53. (Names are the grep-able keys for per-command
follow-ups; handler names follow `Command<Name>` convention.)

## 2. Verified layout rows (client ptr = `[0x70BD90]+0x2C`, sess-27/37)
| off | content | evidence |
|-----|---------|----------|
| +0x00 | vft **0x664804** (the 56-slot UI-factory table; slots 43/47/48/50 = Dialogue/CSMenu factories, sess-28) | 0x460b69 |
| +0x0C | **client id (u32)** — `printf("[client %u] …", client+0xC)` | 0x59c52a; explains the 22-site mbr_0xc census (per-command send wrappers, NOT iface refs) |
| +0x50 | sender host/address string (`*(this+20)` dwords) | 0x59c524/0x59c533 region |
| +0x54 | **net::connection\*** (send: `meth_0x5ac450(conn, packet, flag)`) | 0x59c533..0x59c540 |
| +0xE0 | actionbar-represented-entity slot — read by GameInterfaces vft+0x14, passed to Actionbar refresh 0x41c910 | sess-27/37 |
| +0xAC.. | `0x587a60/70` (engine Client band) | unexplored |
| +0x28 | object w/ iface-ptrs at +0x0C/+0x48 — shown by Actionbar `sub_416B20`; **install site NOT found** (net::client ctor 0x59c2e0 zeroes gap28[0x28/2C] — the layout-ambiguity family, see §3) | sess-37 + this |
| ~+0x170 | `int_0x188` byte — "startup movies?" unverified | older sess-15 sighting |

## 3. Method honesty: the `gap28` offset soup
`net::client::ctor_0x59c2e0` writes `gap28[0x28/0x2C]=0` and `gap28[0x30]=net_address vft` —
whose absolute landing depends on IDA's nested-member arithmetic (critical_section size
assumed 0x18, ctor sets +0x1C..0x24 as "gap28[24..40]" ⇒ effective 0x40..0x58 in the DB view
— self-inconsistent by 0x18). Conclusion drawn here uses DIRECT-ADDRESS evidence only
(vft 0x6a1934 at client+0x30 ✓ matches the DB mbr_0x30 row, so the table lands right there);
anything about client+0x28 beyond "some session-object, installed before map load" is
explicitly NOT claimed. Rule reminder: in this family, a decompiled nested offset is only
evidence when a second path (vft constant, direct disp) agrees — matches the coordinator's
harness-notes theme.

## 4. Send-path skeleton (all ~22 "mbr_0xc" users are this pattern)
packet ctor (net::packet 0x45f300, ~0x210B stack locals) → `packet+0x30 = GAME_ID_*` (e.g.
72 @0x45f545) → field appends via `meth_0x45f130` → `meth_0x59c4f0(client, packet, 1)`
= trace-printf + connection-send. The per-command wrappers enumerate the client→server
surface mechanically by ID constant — pair with fact-sheet §1 registry (IDs already named
there; the wrapper EAs are now the SENDER side of those rows).

## 5. Open
- client+0x28 identity (needs net-class ground truth; T-016-read-2 adjacent "World+0x160
  live vft" style — same debug-session economics; noted for whoever runs T-016).
- Which tier each RegisterCommand name's handler actually reaches (bodies unexplored —
  camera suite probably engine-side state, the F3 UI commands reach the GameInterfaces
  open/dispatch path from sess-31).
- `+0xAC` engine-band pair; `mbr_0xec/f0/f4` (0x46172a region = the CommandStartMap-adjacent
  code, per prior sess-31 rows).

## 6. Evidence
Ctors 0x460b30 (F3 tier, stub rows at 0x460b8d/0x460b9c/0x460c17/0x460c44) and 0x58e900
(engine tier, 0x58ea4e..0x58eff1); send skeleton 0x45f4f0/0x59c4f0 (+0x54 conn deref
0x59c540); weather handler EAs as registered. net::client ctor 0x59c2e0 §3.
