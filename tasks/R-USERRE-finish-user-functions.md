---
status: in-progress
assignee: SwiftOwl `01a16198-6cbb` (R-USERRE-A dispatched by QuickNova 2026-10-22 ~04:2x post-BrightYak-seal: "the board's biggest vacant seat"; resume = their ledger tail, first census delivered same-sitting as slice-4e) — predecessor: BrightYak `01a16198-7b63-74d2-a895-a5fc6a051707` (R-USERRE-A sealed by coordinator QuickNova/LoudDragon 2026-10-22 03:2x; fresh claim, no inherited identity)
type: implementation (product code; batch A = 112 console-command ports, per-row recipe: port + EA-cite (item 57) + gate green)
sequencing: A first (this claim); B/C/D follow per PARK lines R-USERRE-A/B
---

# R-USERRE — the user-era named functions, ported to completion

Inventory: every `F3::`-named function in the live IDB (names anchored by EA per convention) whose address is NOT cited anywhere in src/ or modules/openje/src. These are the user's hand-mapped functions left unported; **completion = implement + cite (item 57) + gate green**. Verification pass per row before porting: the name is a CLAIM, the bytes are the contract (item 50's law applies to human-era names too). False-negative note: suffix-EAs in names may be uppercase — re-check cited-ness case-insensitively at write time.

## Batch A-console — A-console(0x588-0x58e/0x45e)
- `F3::CommandAdapterInfo` @ 0x588bc0
- `F3::CommandAttackModeCycle` @ 0x45ee60
- `F3::CommandCameraAzimuth` @ 0x58b450
- `F3::CommandCameraAzimuthDec` @ 0x5883a0
- `F3::CommandCameraAzimuthInc` @ 0x588360
- `F3::CommandCameraAzimuthSpeed` @ 0x58b4a0
- `F3::CommandCameraDistance` @ 0x58b5b0
- `F3::CommandCameraDistanceSpeed` @ 0x58b600
- `F3::CommandCameraDown` @ 0x588420
- `F3::CommandCameraElevation` @ 0x58b500
- `F3::CommandCameraElevationSpeed` @ 0x58b550
- `F3::CommandCameraFOV` @ 0x58b660
- `F3::CommandCameraMouseRotate_Off` @ 0x5885d0
- `F3::CommandCameraMouseRotate_On` @ 0x5885c0
- `F3::CommandCameraReset` @ 0x58a7f0
- `F3::CommandCameraRotateLeft` @ 0x588540
- `F3::CommandCameraRotateRight` @ 0x588560
- `F3::CommandCameraScrollDown_Off` @ 0x5884b0
- `F3::CommandCameraScrollDown_On` @ 0x588470
- `F3::CommandCameraScrollLeft_Off` @ 0x588480
- `F3::CommandCameraScrollLeft_On` @ 0x588440
- `F3::CommandCameraScrollRight_Off` @ 0x588490
- `F3::CommandCameraScrollRight_On` @ 0x588450
- `F3::CommandCameraScrollSpeed` @ 0x58b7d0
- `F3::CommandCameraScrollUp_Off` @ 0x5884a0
- `F3::CommandCameraScrollUp_On` @ 0x588460
- `F3::CommandCameraSetTarget` @ 0x58b6e0
- `F3::CommandCameraTiltDown` @ 0x5885a0
- `F3::CommandCameraTiltUp` @ 0x588580
- `F3::CommandCameraToggleConstraints` @ 0x5885e0
- `F3::CommandCameraTop` @ 0x588400
- `F3::CommandCameraTopDown` @ 0x5883e0
- `F3::CommandCameraZoomIn_Off` @ 0x5884e0
- `F3::CommandCameraZoomIn_On` @ 0x5884c0
- `F3::CommandCameraZoomOut_Off` @ 0x588520
- `F3::CommandCameraZoomOut_On` @ 0x588500
- `F3::CommandConnect` @ 0x58dc50
- `F3::CommandConsoleWindow` @ 0x588630
- `F3::CommandCreateEntity` @ 0x58dda0
- `F3::CommandDebugLight` @ 0x588320
- `F3::CommandDebugOctreeLight` @ 0x588330
- `F3::CommandDisableScripts` @ 0x58e4a0
- `F3::CommandDisconnect` @ 0x588290
- `F3::CommandEditToken` @ 0x58bb30
- `F3::CommandEnableScripts` @ 0x58e5b0
- `F3::CommandEndDialogue` @ 0x58a7e0
- `F3::CommandEscapeMenu` @ 0x45ec00
- `F3::CommandFadeIn` @ 0x58ba10
- `F3::CommandFadeOut` @ 0x58baa0
- `F3::CommandFeedbackWindow` @ 0x45ee00
- `F3::CommandFontTest` @ 0x5887b0
- `F3::CommandForceEntityStep_0x460720` @ 0x460720
- `F3::CommandGameOptions` @ 0x45edb0
- `F3::CommandGetGlobal` @ 0x58dfd0
- `F3::CommandGuiOptimizationLevel` @ 0x58b8b0
- `F3::CommandHotKeyOptions` @ 0x45edc0
- `F3::CommandKill` @ 0x58a7b0
- `F3::CommandListTextures` @ 0x588b10
- `F3::CommandLoadGameOptions` @ 0x45edd0
- `F3::CommandMemoryPoolTest` @ 0x589580
- `F3::CommandMovieTest` @ 0x588d00
- `F3::CommandMultiplayerOptions` @ 0x45ede0
- `F3::CommandOneScriptPass` @ 0x58e6c0
- `F3::CommandOptionsWindow` @ 0x45eb70
- `F3::CommandPipBoy` @ 0x45ec70
- `F3::CommandRequestNewGame` @ 0x58dbe0
- `F3::CommandScreenCap` @ 0x588620
- `F3::CommandSetDisplaymode` @ 0x58b8e0
- `F3::CommandSetGlobal` @ 0x58e1b0
- `F3::CommandSetScript` @ 0x58df10
- `F3::CommandShowCursor` @ 0x58b830
- `F3::CommandStartDialogue` @ 0x58a7d0
- `F3::CommandStartMap` @ 0x58db80
- `F3::CommandTestAnim_0x460920` @ 0x460920
- `F3::CommandToggleBilinearFiltering` @ 0x5882e0
- `F3::CommandToggleBuilding` @ 0x58bb90
- `F3::CommandToggleCameraFog` @ 0x5882f0
- `F3::CommandToggleDebugTools` @ 0x588750
- `F3::CommandToggleDebugWindow` @ 0x588690
- `F3::CommandToggleEntities` @ 0x588840
- `F3::CommandToggleFOWAutomap` @ 0x58b370
- `F3::CommandToggleFOWLOS` @ 0x58e340
- `F3::CommandToggleFogOfWar` @ 0x58b290
- `F3::CommandToggleFullscreen` @ 0x5882b0
- `F3::CommandToggleGUI` @ 0x588860
- `F3::CommandToggleHeightFog` @ 0x588340
- `F3::CommandToggleLighting` @ 0x5882c0
- `F3::CommandToggleMipMapping` @ 0x588610
- `F3::CommandToggleOctreeDrawing` @ 0x588600
- `F3::CommandToggleOctreeNodeDebug` @ 0x588350
- `F3::CommandToggleRain_0x4606c0` @ 0x4606c0
- `F3::CommandToggleSampling` @ 0x588b30
- `F3::CommandToggleScene` @ 0x588820
- `F3::CommandToggleScripts` @ 0x588880
- `F3::CommandToggleShadow` @ 0x5882d0
- `F3::CommandToggleSnow` @ 0x4606d0
- `F3::CommandToggleStackWindow` @ 0x5886f0
- `F3::CommandToggleStartupMovies` @ 0x58b1b0
- `F3::CommandToggleStatistics` @ 0x5882a0
- `F3::CommandToggleTransparency` @ 0x588310
- `F3::CommandToggleVisualEffects` @ 0x58e380
- `F3::CommandToggleWater` @ 0x58e7e0
- `F3::CommandToggleWind_0x4606e0` @ 0x4606e0
- `F3::CommandToggleWireframe` @ 0x588300
- `F3::CommandUseSkill` @ 0x45ee30
- `F3::CommandVRAM` @ 0x5888e0
- `F3::CommandVSync` @ 0x588b60
- `F3::CommandVideoOptions` @ 0x45edf0
- `F3::CommandWarp` @ 0x58b880
- `F3::CommandWeaponToggle` @ 0x45eed0

## Batch B-script-natives — B-script-natives(0x45a-0x45c)

> **CLAIMED 2026-10-22 ~10:00 by WildKnight `01a1639b`** (PARK line R-USERRE-B read "any idle hand
> post-A"; A is sealed at 5ccd0b5/97bc9de, so B is open). Recipe inherited verbatim from A's slices:
> read the shipped body RAW (item 50 — the name is a claim, the bytes are the contract), port, cite
> every def by EA (item 57), gate green (run-all-tests + msvc-build + port-annot + cppcheck).
> Read-only IDB discipline: no renames unless a row escalates to a naming ruling (single-writer law;
> JadeBear `01a163e9` is the other live seat and may want the IDB writer role — coordination sent).
> New TU expected (`src/F3/ScriptNatives.cpp`) — BUILD-AFFECTING: cmake.toml + CMakeLists.txt are
> hand-synced and the cmkr regen guard is INACTIVE (item 60), so both get edited and verified.

- `F3::AddCNPCToParty` @ 0x45a2d0
- `F3::AddEntry` @ 0x45b980
- `F3::AddHP` @ 0x45b390
- `F3::CloseTask` @ 0x45b940
- `F3::CommandActionbarWindow` @ 0x45ea50
- `F3::CommandInventoryWindow` @ 0x45eac0
- `F3::EquipItem` @ 0x45a580
- `F3::GetBaseSkill` @ 0x45ad90
- `F3::GetCurrentHP` @ 0x45b1b0
- `F3::GetDerivedAttribute_0x45ad00` @ 0x45ad00
- `F3::GetEthnicity` @ 0x45afd0
- `F3::GetGender` @ 0x45af40
- `F3::GetItemType` @ 0x45b240
- `F3::GetLevel` @ 0x45abe0
- `F3::GetLockDC` @ 0x45b570
- `F3::GetMaxHP` @ 0x45b110
- `F3::GetPerkRank` @ 0x45ab50
- `F3::GetPrimaryAttribute` @ 0x45ac70
- `F3::GetRace` @ 0x45aeb0
- `F3::GetSquadAttitude` @ 0x45aa90
- `F3::GetSubRace` @ 0x45b070
- `F3::GetTeamAttitude` @ 0x45aaf0
- `F3::GetTotalSkill_0x45ae20` @ 0x45ae20
- `F3::GetUsableObjectType` @ 0x45b440
- `F3::GetWeaponCombo` @ 0x45a9f0
- `F3::GetWeaponRange` @ 0x45b620
- `F3::HideInterfaces` @ 0x45a100
- `F3::IsItemEquipped` @ 0x45b750
- `F3::IsLocked` @ 0x45b4e0
- `F3::LockDoor` @ 0x45a1b0
- `F3::OpenTask` @ 0x45b900
- `F3::PlayMusic` @ 0x45b9c0
- `F3::Protagonist` @ 0x45a960
- `F3::SetHP` @ 0x45b2e0
- `F3::SetMoveMode` @ 0x45a140
- `F3::SetWeaponCombo` @ 0x45aa40
- `F3::StartDialogue` @ 0x45a040
- `F3::UnequipItem` @ 0x45a6f0
- `F3::UnlockDoor` @ 0x45a240

## Batch C-combat/weapon/read — C-combat/weapon/read(0x401-0x415)
- `F3::GameActionAttack::CalculateAndQueueDamage_0x404060` @ 0x404060
- `F3::GameActionAttack::CancelAttack_virt_0x402080` @ 0x402080
- `F3::GameActionAttack::FinishAttack_virt_0x4021b0` @ 0x4021b0
- `F3::GameActionAttack::Get_mbr_0x58_0x403d50` @ 0x403d50
- `F3::GameActionAttack::OnCreate_0x4011d0` @ 0x4011d0
- `F3::GameActionAttack::PrepareAttack_0x402680` @ 0x402680
- `F3::GameActionAttack::dtor_0x403c70` @ 0x403c70
- `F3::GameActionAttack::meth_0x4012c0` @ 0x4012c0
- `F3::GameActionAttack::meth_0x4012e0` @ 0x4012e0
- `F3::GameActionAttack::meth_0x401410` @ 0x401410
- `F3::GameActionAttack::meth_0x401790` @ 0x401790
- `F3::GameActionAttack::meth_0x401890` @ 0x401890
- `F3::GameActionAttack::meth_0x401a10` @ 0x401a10
- `F3::GameActionAttack::meth_0x4023c0` @ 0x4023c0
- `F3::GameActionAttack::meth_0x402e00` @ 0x402e00
- `F3::GameActionAttack::meth_0x403300` @ 0x403300
- `F3::GameActionAttack::meth_0x404270_404270` @ 0x404270
- `F3::GameActionAttack::virt_meth_0x403f80` @ 0x403f80
- `F3::GameActionMoveTo::ctor_0x4056f0` @ 0x4056f0
- `F3::GameActionMoveTo::dtor_0x405710` @ 0x405710
- `F3::GameActionMoveTo::virt_meth_0x405770` @ 0x405770
- `F3::GameActionMoveTo::virt_meth_0x405790` @ 0x405790
- `F3::GameActionMoveTo::virt_meth_0x4057b0` @ 0x4057b0
- `F3::GameActionMoveTo::virt_meth_0x4058b0` @ 0x4058b0
- `F3::GameActionMoveTo::virt_meth_0x406780` @ 0x406780
- `F3::GameCharacter::ReadData_0x4117a0` @ 0x4117a0
- `F3::GameCreature::ReadData_0x40e360` @ 0x40e360
- `F3::GameCreature::ReadVersion2_0x40d9e0` @ 0x40d9e0
- `F3::GameItem::ReadFromFile_0x411260` @ 0x411260
- `F3::GameProjectile::DropPayload_0x412090` @ 0x412090
- `F3::GameWeapon::DeductPrimaryAmmo_0x412b10` @ 0x412b10
- `F3::GameWeapon::DeductSecondaryAmmo_0x412b90` @ 0x412b90
- `F3::GameWeapon::ReadAttackModeData_0x40dba0` @ 0x40dba0
- `F3::GameWeapon::ReadAttackModeData_0x4148a0` @ 0x4148a0
- `F3::GameWeapon::ReadData_0x415190` @ 0x415190
- `F3::GameWeapon::ReloadPrimaryAmmo_413A90` @ 0x413a90
- `F3::GameWeapon::ReloadSecondaryAmmo_413CA0` @ 0x413ca0
- `F3::GameWeapon::ResolveTargets` @ 0x415740
- `F3::GameWeapon::UnloadPrimaryAmmo_0x4135c0` @ 0x4135c0
- `F3::GameWeapon::UnloadSecondaryAmmo_0x413810` @ 0x413810

## Batch D-stragglers — D-stragglers
- `F3::Attack` @ 0x459eb0
- `F3::Debug` @ 0x5c32f0
- `F3::GameActionMoveTo::meth_0x55b350_55b350` @ 0x55b350
- `F3::GetTypeFromFilename` @ 0x544c80
- `F3::Gfx_InitAnimationSystem` @ 0x5b3070
- `F3::JustAttack` @ 0x459f70
- `F3::ServerUpdate` @ 0x549130
- `F3::StartAddress` @ 0x5909a0
- `F3::TransformGrannyFileToLHS` @ 0x4a1e20
- `F3::crt_run_static_ctors` @ 0x61898d


## Slice-1 LEDGER (BrightYak, 2026-10-22) - CAMERA SET-FAMILY + REGISTRATION MACHINERY
Landed (openje c2a2409 + parent, gates: msvc rc0 / run-all 10/10 app=124 / annot PASS 0-uncited / cppcheck 0):
- Commands.cpp/hpp (new, src/F3): CommandCameraAzimuth 0x58b450, AzimuthInc 0x588360, AzimuthDec 0x5883a0,
  AzimuthSpeed 0x58b4a0, Elevation 0x58b500, ElevationSpeed 0x58b550, Top 0x588400, TopDown 0x5883e0,
  TiltUp 0x588580 (xor word cam+0xC2 bit 0x100 - NAME LIED, bytes said flag-toggle), TiltDown 0x5885a0 (bit 0x200).
- JE/Client.cpp (new, openje): ctor_0x58e900 field-zero map (stores cited 0x58e92f-0x58ea36), tick stamp
  0x58ea39, the 10 registrations LIVE + the FULL 84-command call-site EA table preserved as the slice TODO map.
- JE/CameraControl.cpp (new, openje): setters 0x585ea0 (azimuth anim/instant), 0x586010 (elev clamp->anim),
  0x585fd0 (elev instant, shipped arg-oddity ported VERBATIM); field rows 0xa4..0xf8 cited (export GAP, raw law).
- Seam + accessors: Gfx_CamCtrl::set_pose_0x424670 (named-absence), Network::GetGameNetwork (0x5438fa mirror),
  Network+0x2C RE-LABELED game-client (RTTI 0x43978a), Client::GetCamera (+0x84).
- DEFECT-FIX x2: net/packet.hpp:36 + net/connection.hpp:81 typeless j-corrupted export decls -> well-formed
  neutral names (cited); ~48 pattern sites REPORTED -> R-HEADERSAN minted (decb94a) after slice 2.
- GATE REPAIR (coordinator ruling (a)): cmake.toml drift fixed - +JE/Gregorian.cpp, +JE/SpatialGrid.cpp
  (openje); +src/F3/MapEntitySpawn.cpp, +src/F3/CombatManager.cpp, +src/F3/GameActionAttack.cpp,
  +src/F3/CombatMath.cpp (F3); iterated on the real LNK2019 census (12 -> 2 -> 4 -> 0). cmkr bootstrap
  never regenerates once build/CMakeCache.txt exists (cmkr.cmake:121 era) - regen via
  build/_cmkr_v0.2.46/build/cmkr gen, NOT just re-running msvc-build. 2026 lesson for the harness.
Slice 2 (next): camera tail (Distance 0x58b5b0/DistanceSpeed 0x58b600 setter 0x586140 READ, fields 0xac/0xe8..0xf8;
ScrollSpeed 0x58b7d0 field 0x110 needs one re-read; scroll/zoom On/Off 0x588440-0x588520; Rotate/Tilt-era
0x588540/0x588560; MouseRotate 0x5885c0/0x5885d0; ToggleConstraints 0x5885e0; Reset 0x58a7f0/SetTarget 0x58b6e0
need byte_70BE0C entity-list walk + 0x54be10 resolver) + CameraDown 0x588420 + the EXECUTION dispatch trace.
Slice 3 (device era, G1 warm): CameraFOV 0x58b660, Toggle* fullscreen/lighting/shadow..., AdapterInfo, VSync,
SetDisplaymode - now testable against real device behavior.


## Slice-2 LEDGER (BrightYak, 2026-10-22) - CAMERA TAIL + FIELD-KEY CORRECTION
Landed (gates: msvc rc0 / run-all 10/10 app=124 / annot PASS 0/386 / cppcheck 0):
- 18 new handlers, all raw-read: Distance 0x58b5b0 (setter 0x586140 ported), DistanceSpeed 0x58b600,
  scroll x8 on client+0x112 DWORD (0x588440-0x5884b8, On=OR/Off=AND-NOT), zoom/rotate x6 on
  cam+0xC2 DWORD (0x5884c0-0x58856e; zoom OR/AND, rotate XOR hold-toggles), CameraDown 0x588420
  (sub_586310 -> animated elev 0.0). 14 registrations added (total 24 live of 84).
- FIELD-KEY CORRECTION (the slice's real find): slice-1's setter mapping came from DECOMPILER
  PROSE that swapped anim target vs direction. Re-derived from store-opcode DISPLACEMENTS:
  azimuth {target 0xcc fstps @0x585fae, dir +-1.0 @0xd0 0x585f8f/a7, start @0xd4};
  elevation {target @0xe4 0x5860fd, dir @0xe8, NO start field - current 0xa8 IS start};
  distance {target @0xf8 0x58621e, dir @0xfc, bounds 0xec/0xf0}. cpp + hpp corrected together.
- SHIPPED SLOT COLLISIONS (ported VERBATIM, unified by nothing):
  (a) cam+0xF8 = distance anim TARGET == DistanceSpeed command slot (0x58b633 writes what
      0x586140's else-branch treats as target);
  (b) cam+0x110 = Gfx_Engine VIEW POINTER (ctor stamp 0x586fd9) == ScrollSpeed command's float
      slot (0x58b803) - setting scroll speed erases the view ptr. CameraScrollSpeed + FOV
      DEFERRED to slice 3 until the anim-tick/FOV consumers read these slots and the collision
      is EXPLAINED (this is why named-absence-with-EA beats a plausible port).
- Deferred-with-reasons: MouseRotate pair 0x5885c0/d0 -> sub_587870(client,bool) input-mgr era;
  ToggleConstraints 0x5885e0 -> meth_0x586590(cam, [cam+0x114]==0) (quick read pending);
  Reset/SetTarget -> entity-list byte_70BE0C walk + meth_0x54be10 resolver (spawn era).
Slice 3 next: the two collisions' consumer reads (anim tick walk = CameraControl's vft/update
era), ToggleConstraints, then the toggle-family census (Toggle* x20, mostly flag bytes on
engine/display objects - device era now LIVE per G1 so these get REAL validation).


## Slice-3a LEDGER (BrightYak, 2026-10-22) - TICK CONSUMER + FIELD KEY v3 + SELF-CORRECTION
THE CONSUMER READ (planned first, delivered first): CameraControl::meth_0x5869c0 = the animation
tick (33ms dt-gate, per-bit walk legs + mode-register legs). It re-decoded the ENTIRE field key:
- MODE REGISTER = dword at 0xC2 (not a tilt-word!): scroll bits 1/2/4/8 (MoveTarget legs using
  scroll speed 0x110), zoom bits 0x10/0x20 (FOV ramps using speed 0xC4), rotate 0x40/0x80
  (orbit yaw @0xC8 speed), tilt 0x100/0x200 (orbit pitch @0xD8 speed). The 32-bit toggle writes
  OVERLAP the zoom-speed half by construction - shipped, our port matches width, explained.
- SELF-CORRECTION REGISTER (gen-5 class, my own claim): the slice-2 'cam+0x110 view-pointer
  collision' was MY misread - I pattern-matched the tick's scroll-leg dereference to the ctor's
  stamp instead of checking the ctor's STORE DISPLACEMENT, which is 0x118(%esi) (raw fstps
  @0x586fd9) / 0x11c. 0x110 is scroll speed, pure. ScrollSpeed + FOV commands UNBLOCKED the
  same sitting they were deferred for: CommandCameraScrollSpeed 0x58b7d0 LIVE, CommandCameraFOV
  0x58b660 LIVE (clamp 104/108 + enable 114 ported; the view-stamp trio 0x58681f-0x586828 stays
  NAMED-UNPORTED behind the Gfx_Engine seam - real boundary, no fake pointer). Corrections
  relayed to the coordinator the same cycle the finding died.
- CLAMP KEY v3 (all displacement-verified): elevation bounds 0xdc/0xe0 (v2 said d8/dc - 0xd8 is
  the elevation SPEED the tick multiplies); distance bounds 0xf0 LOWER (fcomps 0xf0 @0x58615c)
  / 0xf4 UPPER (@0x586179); 0xec = distance speed (setter leg + tick @0x586b8a both read it);
  0xf8 = distance target AND the DistanceSpeed command slot - UNIFIED meaning (the 'speed'
  command sets where the camera MOVES TO), collision explained instead of feared.
- anim_tick_0x5869c0 PORTED (azimuth leg live: walk 0xd4 toward target 0xcc at 0xc8*dir 0xd0,
  flag clear 0xfffe, orbit set_pose @0x586aa7 cite; elevation/distance/mode legs banked to
  slice 4 - they need MoveTarget 0x586410 + orbit-nudge 0x4C62B0 seam ports).
Registrations now 26 of 84 LIVE. Gates: msvc rc0 / run-all 10/10 app=124 / annot 0/390 / cppcheck 0.
Remaining camera deferrals (cited): MouseRotate pair -> sub_587870(client,bool) input era;
ToggleConstraints -> meth_0x586590 (read pending); Reset/SetTarget -> entity walk 0x70BE0C +
resolver 0x54be10; CameraReset needs client+0xBC/0xE0 keys (v3 note: those are CLIENT fields,
world+0x1DC-handle era - rides P-CMBT/T-043 inventory, honest paper exists).
Next: toggle-family census (Toggle* x20 - flag flips on engine/display objects; device era LIVE,
G1-validated reads possible), then the EXECUTION dispatch trace.


## Slice-3b RESEARCH (BrightYak, 2026-10-22) - dispatch chain + toggle census (memory obs #130)
EXECUTION CHAIN (the open question since beat 1, ANSWERED for the script path):
JE::Entity::meth_0x54ca40 (Entity+0x22C gate, +0x218 script text) -> wrapper 0x585a50 -> core
sub_5857F0 -> lower-bound 0x545710 on the tree at EXECUTOR+0x10, executor = singleton [0x70C528]
(all 38 AMX/script sites load exactly that global before the call). The handler at node+0x28 is
invoked with the argv cell-chain convention. OPEN QUESTION (named, not guessed): does the
console registry tree 0x70C668 == executor's +0x10 tree? Raw census: 0x70C668 has ZERO .text
readers outside init/RegisterCommand/ GLOBAL__I / atexit - so either the trees are the same
object (one ctor read of 0x70C528 settles it) or console text flows through the script executor.
NEXT READ (queued): who STORES 0x70C528.
TOGGLE FAMILY (13 uniform thunks): every CommandToggle* 0x5882a0-0x588610 = 'mov eax,[0x70bd90];
mov ecx,[eax+2c] (+ optional mov ecx,[ecx+74] -> CLIENT+0x74 IS A CAMERA/VIEW PTR, layout
finding); jmp setter' where setter flips ONE bool [obj+disp] and pushes it to the Gfx_Engine
[0x707CE4] vft (Statistics: flip [Client+0x4a] -> vft+0x1BC @0x581654; Lighting: flip
[view+0x54] -> call 0x581600). Heavy non-thunk toggles: FogOfWar 0x58b290, Water 0x58e7e0,
Building 0x58bb90, VisualEffects 0x58e380, StartupMovies 0x58b1b0, FOWAutomap 0x58b370,
FOWLOS 0x58e340. Port posture: thunks port 1:1 ONLY with per-member engine vft numbers READ
(no guessing) + Gfx_Engine seam = slice 4, with G1 device validation available per keeper.
REGISTRY ORDER: full 84-command ctor decompile re-verified verbatim against my slice map this
pass (zero divergence); shipped RegisterCommand total = 109 sites (84+24+quit @0x5ace0f).


## Slice-3c RESEARCH — dispatch question CLOSED in shape, registry CONSUMER open
Store census for [0x70C528] (corrected 3d): 0x585b8a sits in JE::ModelCache::dtor_0x585b10
(the g_model_cache CLEAR, +0x76 into the 0xee-byte dtor); the CONSTRUCTION store is
g_model_cache=this @0x585cce in the ctor (port already mirrors both: ModelCache.cpp lines 18/40).
[0x70C528] == JE::ModelCache singleton, CONFIRMED both directions. => the script-execution tree
belongs to the MODEL CACHE: ModelCache holds TWO name-trees (tree_a +0x04/+0x08/+0x0C VFX-Param
sibling destroy 0x45d1d0; tree_b +0x10/+0x14/+0x18 StringTree destroy 0x585640, node+0x28 holds
CACHED MODEL objects freed via 0x567f50). So 'executor+0x10 lookup, call [node+0x28](argv)'
means: command name -> per-command arg string -> CACHED OBJECT -> object's slot-0 called.
The console registry tree 0x70C668 has, by raw census, EXACTLY TWO consumer-callers of its
lookup meth_0x5ace40: cls_0x4c5ac0::meth_0x56bd50 (@0x56bdb5) and EngineStringSystem::meth_
_0x56be30 (@0x56c117, the '<token>' fall-through - ESS token substitution). Working shape
(NAMED as hypothesis, consumer read queued): in this demo the command console IS the string
system's token path - script text flowing through ESS::Format resolves command-name tokens by
LOOKING UP the registry; the F3::RegisterCommand tree is the name->handler map consumed by the
TOKEN path, and the 'call' leg is the remaining evidence gap. NEXT READ: cls_0x4c5ac0::
meth_0x56bd50 (0xd8 bytes, the non-ESS consumer). Until it reads, no port claim about HOW the
handler gets invoked — only HOW IT'S FOUND (registry, proven) and WHERE lookups happen (2 sites,
census-closed).


## Slice-3d LEDGER notes (BrightYak, 2026-10-22) - two self-corrections on the board
(1) 'console UNREACHABLE / registry zero readers' RETRACTED as overstated: the registry tree
0x70C668 is consumed via register-passed pointers (lea [obj+0x10] shape - an imm32 scan is
structurally blind to it); the EXECUTOR INVOKE MECHANISM is fully found: sub_5857F0 tokenizes
whitespace-separated text -> builds the argv cell-chain -> lower-bound on the passed tree ->
invokes node+0x28 handler. OPEN now only: which tree instance the console's registered names
land in (console registry vs ModelCache tree_b). (2) 0x70C528 store census corrected above
(ctor 0x585cce = g_model_cache=this, dtor clear 0x585b8a; port already models both).
NEXT (slice 3e): read the ModelCache ctor's tree_b (+0x10) init + who INSERTS command names into
it (one insert-site census); if tree_b IS populated from the same RegisterCommand path, the two
trees unify and the port needs an Execute(name,args) on F3:: ported from sub_5857F0 verbatim;
if not, the console registry stays a find-map and the invoke seam is the ModelCache path alone.


## Slice-4 opener (BrightYak, 2026-10-22) - tree-identity census, RAW-VERIFIED
ModelCache::ctor_0x585c00 (decompile + IDB ledger note agree): tree at +0x08/+0x0C built via
0x45c940 (0x38-byte nodes, VFX_Parameter sibling destroy) ; tree_b at +0x10/+0x14/+0x18 built
via StringTree::buynode_0x59ec40 = 0x30-byte-node StringTree; trailing loop probes DECIMAL
string keys '1','2',... case-insensitively (0x4339b0 + _stricmp) - cache-by-index-name era.
Executor call chain RAW-PINNED: 0x54ca40-0x54caa6: mov ecx,[0x70c528] (=g_model_cache, ctor
store 0x585cce / dtor clear 0x585b8a) -> wrapper 0x585a50 -> core sub_5857F0 with ecx = the
ModelCache, core lower-bounds the tree at [ecx+0x10] = tree_b, INVOKES *(node+0x28) (the value
slot in BOTH tree shapes: console registry AND cache tree).
READ RESULT: script commands execute against the ModelCache tree_b; console registrations go
to the F3 registry (0x59f5d0 -> InsertOrGet @0x59f6a8 - whether THAT function also/instead
feeds tree_b was NOT yet read: the insert-target census inside 0x59f5d0's callees is the
FINAL open query). Tree-identity answer status: CONFIRMED two trees exist; the invoker uses
tree_b; console registry consumption = still open, ONE read (0x59f5d0's insert target + 0x38-
vs 0x30-byte node-shape discriminator, which is decisive since the two builders differ) away.
No claim beyond that. Slice-4 body work (Gfx seam per ruling) continues next.


## FINAL read this session (BrightYak, 2026-10-22) - tree-identity CLOSED as TWO distinct consumers
(1) RegisterCommand @0x59f63a/0x59f6b5 (decompile): lower_bound + insert-or-get BOTH on
&F3::global_cls_0x4d8d70_0x70c668 (lea imm32 in the .data era) - console registrations land in
THE F3 REGISTRY TREE, full stop. (2) Executor core sub_5857F0 @0x58589c RAW: 'lea 0x10(edi),esi;
mov esi,ecx; call 0x545710' - looks up the tree at EXECUTOR-OBJECT+0x10; script path's ecx =
g_model_cache (0x54caa0) so tree_b (+0x10, StringTree-buynode class) is the SCRIPT command tree.
=> ARCHITECTURE ANSWERED: name->handler registry (console) and ModelCache script-command tree
are DISTINCT consumers of the same StringTree machinery (value slot +0x28 in both shapes). The
remaining wiring question - who fills ModelCache tree_b with command names (its insert callers:
StringTree::meth_0x59f540-class or the decimal-probe era) - is the NEXT lane's first census,
named here as OPEN, not answered. Port consequence unchanged: F3::RegisterCommand mirrors
shipped (registry tree, proven); an Execute() seam on the registry is port-side value-add and
must be marked PORT-SIDE (no shipped twin) when slice 4/5 builds it.
SESSION LEDGER: slice1 9e5698e / slice2 e7731d0 / 3a b69fca0 / 3b bedff55 / 3c 55a76af / 3d 5cf7d2f
(+ intermediate openje 9e/c2/f2/37a commits, all pushed) - 26/84 registered, gates green at every
pointer, two self-corrections landed same-sitting. Resume: ledger tail above -> tree_b inserters
census -> Gfx_Engine seam (toggle thunks + FOV view trio). -end-of-transmission (compaction imminent)


## Slice-4e CENSUS CLOSED (SwiftOwl 01a16198, 2026-10-22) - tree_b inserters found, the
"executor" attribution SUPERSEDED, and the REAL executor is FULLY read. Corrections relay-
forward per register law; BrightYak's text above stands as the record of what was believed when.

### 1. ModelCache tree_b is a MODEL CACHE. Not a command tree. (census, raw-verified this seat)
- The tree at ModelCache+0x10/+0x14/+0x18 has EXACTLY ONE inserter in the image: sub_5857F0
  @0x58598e via JE::StringTree::meth_0x585700 (insert-with-value; meth_0x585700's xref list is
  that ONE site - IDB xrefs_to, complete, no truncation).
- sub_5857F0 (0x228) READ VERBATIM: copy key -> sub_62B180 (lowercase) -> StringTree::
  meth_0x545710 lower_bound + sub_433930 key-equal -> HIT: `mov edi,[eax+28h]` (VALUE LOAD -
  no call of any kind) / MISS: operator new(0x34) -> cls_0x5638e0-family ctor sub_567010(name,
  global dword byte_6FF0D8, 1, char) -> insert. THIS is get-or-create-by-lowercased-name, the
  dtor (0x585b10) confirms the semantics by destruction: per node, dtor meth_0x567f50 + free
  of the object at node+0x28 - "the cached models" (IDB's own dtor annotation agrees).
- Wrapper sub_585A50: null-name guard -> core -> `mov [obj+28h], flagbyte` -> `call
  sub_565760(obj, param)` - a fixed member call on the CACHED OBJECT, not a handler dispatch.
- Sole callers: Entity::meth_0x54ca40 (-> wrapper) <- Entity::meth_0x54dc50. Model request path.
- Ctor's decimal loop (0x585cf5-0x585dcd: _itoa "1","2",... -> meth_0x4339b0 lower_bound on the
  +0x04 object, 0x38-node tree) probes the OTHER tree (VFX_Parameter-shaped) and STOPS at the
  first miss - an entry COUNT scan; it inserts nothing anywhere.
- CONSEQUENCE: slice-3d's "EXECUTOR INVOKE MECHANISM fully found: ... invokes *(node+0x28)
  handler" and the FINAL read's "script commands execute against the ModelCache tree_b" are
  REFUTED by the bytes above; `*(node+0x28)` never appears as a call in this chain (value slot
  holds a 0x34-byte model object whose dtor the cache calls at teardown). No port code was
  built on the attribution yet - zero blast radius. tree_b stays UNPORTED-by-this-row (it
  belongs to the ModelCache era, not the console era).

### 2. THE REAL EXECUTOR: cls_0x580ba0::meth_0x580ba0 - the console's shipped Execute twin.
- Read verbatim (decompile this seat, 0x580ba0, 0x161 bytes): walks a queue of 28-byte command
  strings (members mbr_0x8 begin / mbr_0xc end - the object is 16B, embedded at GameClient+124
  (byte-indexed [124] in the pump's decompile)); per command: strcpy to a 256B local ->
  sub_580B00(v17, &v13) TOKENIZER -> `v5 = sub_59F030(*Block)` FIRST-TOKEN LOOKUP ->
  `if (v5) v5(&v13)` **HANDLER CALLED with the argv vector** -> miss branch logs
  "Unknown command %s\n" (0x580c65) -> queue advance erases consumed strings (meth_0x401b20
  shift loop + meth_0x4014b0 dtor steps).
- Tokenizer sub_580B00: strtok (sub_61A7D1) against byte_671C60 = `db 20h` - a SINGLE SPACE,
  no tabs - into a std::vector<char*> (sub_5457B0 push_back when room, sub_5807D0 realloc-grow
  else); *Block = argv[0] = command name; &v13 (the vector head) is the handler's sole argument.
- Lookup chain CLOSED: sub_59F030 copies the token -> sub_59EC80 = lower_bound (meth_0x545710)
  + key-equal (sub_433930) on &F3::global_cls_0x4d8d70_0x70c668 -> returns node+0x28 VALUE
  (0x59ecd6 `return v3[10]` = +0x28) or 0. The console registry's node+0x28 holds the HANDLER -
  the +0x28 value slot does mean "callable" THERE, which is exactly where slice-3d's shape
  belonged (ModelCache's node+0x28 holds a MODEL instead - same offset, different class of
  value; the discriminator is which tree).   
- Registry 0x70C668 consumer census COMPLETE (xrefs_to, all): RegisterCommand (2 data refs,
  insert), sub_59EC80 (2 data refs, lookup), static-init 0x643f60 (ctor), atexit thunk 0x646b90
  (dtor). Nothing else touches it. Driver call site: GameClient pump
  virt_meth_0x45f000(+tail) @0x587fd5, per frame.

### 3. PORT-LAW CONSEQUENCE for the Execute() decision (the dispatch's second task)
- Execute() is NOT port-side value-add: the shipped twin EXISTS and is now fully read
  (queue+drainer cls_0x580ba0 @GameClient+124, tokenizer sub_580B00 [SPACE-delimited, argv
  vector, strtok semantics], dispatch sub_59F030->0x59EC80 [lower_bound + equal, value=+0x28],
  miss-log "Unknown command %s\n"). The port must MIRROR it, cited, item-57 style - handler
  contract: __cdecl(void* argvVector), argv[0]=lower?? NOTE: lookup is _stricmp-keyed (the
  tree's compare policy) so handler matching is case-INSENSITIVE; the tokenizer does NOT
  lowercase - the registry's case-insensitive keys do the work.
- OPEN NEXT (named, not answered): the ENQUEUE side - who pushes command strings into the
  GameClient+124 queue (console widget Enter path, era-guess CDebugInterface/GUI console;
  the pushers define the queue element's 28-byte layout {string, +8, +C?}). Until read, the
  port's queue is modeled as a vector<string> with the 28B stride NAMED-UNKNOWN.
- Then: Execute() port lands in Commands.* with the drain-per-frame wiring (GameClient::Update
  era from slice-1/2/3a) + the 58 remaining registrations proceed per the task's batches.

GATES at this census commit: paper-only (task file); no src touched. Census method was
xrefs_to/decompile on ida_original_f3 + F3.json class tables; every claim above carries its
EA and can be re-dumped cold.


### Slice-4e FOLLOW-UP (SwiftOwl, same seat, same night) — queue identity found; enqueue census
RE-SHAPED, not closed. Plus BrightYak's independent re-verification relayed (bytes both ways).
- The drained object is NOT a plain queue: GameClient+0x7C holds a POINTER to a 0x18-byte
  GameHotKeys object (vft GameHotKeys_HotKeys_vftable_0x6647f4; base JE::HotKeys vft 0x69fd34).
  Factory = GameClient::virt_meth_0x45e880 (new(0x18) @0x45e89c -> HotKeys::ctor_0x599810 @0x45e8be
  -> vft-patch @0x45e8c3 -> object returned in eax for the Initialize-chain caller to store).
  Construction READS +0x78/+0x7C BEFORE any store -> begin/end seed 0 = EMPTY RANGE (matches the
  drainer's empty-check @0x580be5).
- HotKeys::ctor stores param_2->mbr_0x8, param_1->mbr_0x4, LOBYTE(param_1)->mbr_0xc
  (0x599820-0x599831) - the drainer's range cursor IS the HotKeys base fields; 28-byte stride
  comes from the drain-advance shift/dtor loops (0x580c70-0x580cb0) - elements are 0x1C-part
  SSO strings. GameHotKeys ALSO owns a StringTree at +0x10 (buynode @0x599833, 0x30B nodes) =
  the KEY-BINDING table. ONE object is both hotkey map and command FIFO: the console era and
  the hotkey era share an owner class - port consequence: Commands' owner mirrors the 0x18
  layout, cited to 0x599810/0x45e880.
- STILL OPEN, SHAPED: who APPENDS into [GC+7C]+mbr_0x8..mbr_0xc. Candidates for the next
  block (enumerated, none assumed): key-event handlers via the two vfts (census
  0x6647f4/0x69fd30 slots), GUI console Enter path (CDebugInterface 0x558740 era), script
  command emitters. Port models the FIFO as vector<string> with pusher-sites NAMED-UNKNOWN.
- BrightYak re-verified every pillar on the md5-pinned exe BEFORE accepting (0x5858e4 byte
  alignment 8b 78 28 = mov, zero calls; sub_585700 caller census [0x58598e]; meth_0x580ba0
  caller census [0x587fd5]; sub_59EC80 prologue 'b9 68 c6 70 00' = mov ecx,0x70c668 in-register;
  invoke 0x580c4b call *%eax; miss-string push 0x69ad98 @0x580c60) and added one context fact:
  the drain sits BETWEEN 'call *0xd0(%eax)' @0x587fcc and 'mov ecx,0x84(%esi)' @0x587fda -
  queue-drain is pump-ADJACENT to the CameraControl(+0x84) tick, so registered camera commands
  take effect one instruction before the camera animates. Their records: obs #130 rewritten with
  supersession banner, #131 resume-recipe item (2) marked SATISFIED, next-read narrowed to the
  enqueue write-census above (converges with my shaping independently).
- Method notes for the register (both directions, self included): (a) my first +0x7C reading
  came from BYTE-INDEXED decompiler output on an untyped this - the raw 'mov ecx,[esi+7Ch]'
  @0x587fd2 settled it; (b) F3.json member-usage lists MISS write-sites (no [GC+7C] store is
  listed anywhere) - a FLOOR, never a census; every 'complete' census above is an xrefs_to one;
  (c) BrightYak's own generalization, adopted: 'no shipped twin' is itself a negative claim that
  requires a census wide enough to see register-passed objects - an imm32 scanner literally
  cannot find this executor, and that must stop any lane calling a seam twin-less on scan-absence.
- Follow-up to the follow-up (same night, bytes from the blessed vft census): the queue's
  IDENTITY just moved. HotKeys vft (0x69fd34) = {0x599D50, 0x5998C0, 0x599A90}; slot[1]
  0x5998C0 READ: the [Hotkeys] INI SECTION LOADER - walks the ENGINE parameter tree
  (sub_56AF30 = GetIniParameterTree, the 0x70C2C0 era) [Hotkeys] entries, parses 'Ctrl+'
  (mask 1) / 'Shift+' (mask 2) in a TWO-PASS loop (order-pinned 0x5999c8 before 0x5999e2),
  resolves the key name via mbr_0x4's vft+16 (mbr_0x4 = [GameClient+0x78] = the KEYMAP object,
  the twin pointer HotKeys::ctor took), and binds: GameHotKeys::meth_0x599860(keyId u16,
  repeatFlag char, modMask int, command string).
- meth_0x599860 READ (0x599860-0x5998b2): PACKS param_3 = keyId | ((repeat&0xF | 16*(mods&0xF))
  << 16) into ONE dword key; inserts into the StringTree at &this->mbr_0xc (note: tree rooted AT
  +0x0C in GameHotKeys - the field HotKeys::ctor had seeded as LOBYTE(param_1); StringTree-
  object layout {+0 head-cursor?, +4 header-node ptr @buynode, +8 count} overlapping the
  range-cursor fields - the SAME 16 bytes are the FIFO cursor at HotKeys-base level and the
  tree-object base one subclass down; the object is genuinely BOTH structures, not an adjacency
  accident); assign-stores the COMMAND STRING into the node VALUE slot (meth_0x52bc90 ->
  meth_0x401bd0). KEY->COMMAND BINDING TABLE CONFIRMED; NOT a FIFO push (zero vector writes).
- RECORDED TENSION (self-caught, not smoothed): GameHotKeys+0x0C is BOTH the HotKeys-base FIFO
  end-cursor (drain range +8..+C) and, per the binder's &mbr_0xc, the head of a StringTree
  object spanning +C..+14. Ranges and a tree header cannot share a dword without collision -
  either the drain cursor fields resolve differently than the ctor-store reading suggests, or
  the object swaps role per era (binds before any push, or vice versa), or one of today's two
  structural reads is wrong at one dword. The FIRE-SITE read settles it; named here rather than
  papered over, and NO port layout decision is taken until then.

### THE TENSION, SETTLED SAME NIGHT (and the resolution indicts MY OWN byte-indexed decompiler
### reading, not the machinery)
The pump raws at 0x587f95/0x587fd2/0x587fa5 load THREE DISTINCT GameClient slots: `mov
ecx,[esi+80h]` -> fire 0x599720, `mov ecx,[esi+7Ch]` -> the DRAIN, `mov ecx,[esi+78h]` ->
keymap twin (its vft+16 resolves key NAMES to ids @0x599a01). The decompiler's '[124]'/'[128]'
were BYTE-INDEXED GUESSWORK on an untyped this, and the '+0x7C queue-object with cursor fields'
claim was built on them. Raws, resolved:
- [GC+0x7C] = the COMMAND FIFO itself: a std::vector of 28-byte SSO strings (begin/end/cap
  triple) - the drainer's 'mbr_0x8/mbr_0xc' are THAT VECTOR's begin/end, read as its own
  this+fields. cls_0x580ba0 is the vector struct's label, not a 16-byte queue-owner.
- [GC+0x80] = the GameHotKeys object: {+0 vft-patched 0x6647f4, +4 = keymap ptr, +8 = FIFO ptr,
  +0xC..+0x14 = the binding StringTree} (0x18 bytes - fits exactly). HotKeys::ctor's
  param_2-store at +8 is a POINTER to the vector (the fire's push reads it back: 0x59978f
  `sub_580D10(this: *(this+8), value)`; 0x580D10 = temp-string then meth_0x4c60a0(esi+4) =
  push_back-at-end). The factory virt_meth_0x45e880 receives the FIFO+keymap by READING
  [GC+7C]/[GC+78] - so the Initialize chain allocates vector+keymap FIRST, GHH last; the
  'pre-store empty-range' reading dies with the misread (no contradiction remains: ctor never
  seeded cursors, it stored POINTERS). One loose end stays honest: the ctor's
  LOBYTE(param_1)->+0xC store lands inside the tree's base word - unexplained byte-write
  quirk of the era, NO semantic claimed, and none needed.
- Register entry (second one tonight, self-inflicted, named as it happens): BYTE-INDEXED
  DECOMPILED FIELD PATHS ARE NOT OFFSETS. Every slot claim above was re-pinned against the
  'mov ecx,[esi+XX]' raws before it's worth a word; the F3.json usage-list floors caught the
  same class from the other side (missed stores).

FULL CHAIN, ALL SLOTS RAW-PINNED (the era's spine, port-ready):
  (a) init: Initialize chain fills GC+78 keymap, GC+7C command FIFO (vector of 28B strings),
      GC+80 GameHotKeys (factory virt_meth_0x45e880, built FROM the first two - ctor stores at
      its +4/+8, buynode tree at its +0xC..+0x14);
  (b) bind: HotKeys::virt_meth_0x5998c0 [Hotkeys] ini walk (ENGINE tree sub_56AF30), 'Ctrl+'(1)
      'Shift+'(2) two-pass @0x5999c8/0x5999e2, keymap vft+16 name->id, pack
      keyId|(repeat|mods<<16, GameHotKeys::meth_0x599860 inserts key->COMMAND STRING in the tree;
  (c) fire: per pump frame each dequeued input event -> sub_599720(this=[GC+80]): pressed-guard
      0x599729, IDENTICAL pack @0x59974f, tree find @0x599760, HIT -> push the value string into
      the FIFO through *(this+8) @0x59978f;
  (d) drain+execute: cls_0x580ba0(this=[GC+7C]) SAME FRAME: space-tokenize -> registry
      (0x70C668) -> handler(&argv) or 'Unknown command %s\n';
  (e) camera tick immediately after (0x587fda, BrightYak's adjacency pin: fire in frame N,
      bound commands act BEFORE the camera animates in the same frame).
- Console input path: a SEPARATE feeder into the same FIFO (named candidate: CDebugInterface
  0x558740 era) - no read claimed; the port's Execute() mirror needs (c)+(d) shapes verbatim
  and (b)'s pack format; (a)'s slot map is port-internal until digest parity cares.

### FEEDER CENSUS CLOSE (SwiftOwl, same sitting; BrightYak's 17-site census corrected us both -
### 'the console Enter path is the one unnamed feeder' was WRONG SHARED FRAMING: nine feeder
### FUNCTIONS, console is one. Register convergence #4, both benches complicit, corrected by
### the bench that ran the census.)
- Two of BrightYak's weighted candidates READ this seat (cheap-pass verdicts, bytes only):
  (i) sub_5B6BC0 (0xB82, 3 push sites): NOT console - a dialog/menu handler pushing LITERAL
      commands: button-label strings "Disable Scripts"/"Enable Scripts" adjacent to the literal
      "disablescripts" pushed via [icons+2Ch]->+7C FIFO @0x5b6d54. Script-toggle UI fires
      commands through the same FIFO.
  (ii) sub_41CA00 (0x5D3, 2 sites): GUI event dispatcher, also LITERALs - "InventoryWindow"
      @0x41cb89, "Pipboy" - window-open commands from widget events, with an RTTI cast to
      GameClient + AreaMapIcon::meth_0x436ad0(0x10) sibling branch. Menu buttons ARE command
      feeds: an era architecture fact, portable when those windows land.
- THE CONSOLE ENTER FEEDER: FOUND, NAMED, ONE HOP DEEPER THAN BOTH BENCHED GUESSES.
  sub_59E8E0 = the runtime-LINE pusher (formats a caller-supplied text via sub_59E8A0 @0x59ea07,
  pushes the SSO result into the same FIFO ([icons+2Ch]->+7C) @0x59ea29) - the shape both benches
  predicted (reads a line buffer, not a literal). Its EXTERNAL caller census = exactly one:
  JE::CConsoleInterface::virt_meth_0x59ec10 (8-byte tail-feeder; class .?AVCConsoleInterface@@,
  size 8, 2 methods, 1 vftable) - the debug console widget's command-submit slot. Console era
  is CConsoleInterface, NOT the wave-4-candidate CDebugInterface (0x558740) - the named
  candidate dies, the named class lives. The vft that dispatches INTO 0x59ec10 (Enter-key ->
  submit) is GUI-widget detail, not load-bearing for any port row; not traced, not guessed.
- REMAINING UNREAD PUSH SITES (six, each a named function, all FIFO consumers, none blocking
  Execute()): sub_42FE50 (x2), JE::GamePlayerAvatarManager::meth_0x429900, meth_0x42a190,
  sub_433EE0, sub_440CE0 (x3), sub_59E8E0 DONE above, JE::Client::SetFocus_virt_0x58d9a0 -
  BrightYak's flag noted: a FIFO push inside SetFocus is either scripted-focus-command firing
  or a shape trap; UNREAD, flagged, not adjudicated. Optional era-census row, not a gap.
- Execute() UNBLOCKING STATE FINAL: bind->fire->push->drain->dispatch->camera-tick all
  raw-pinned; console feeder named; every other pusher is a CLIENT of the FIFO, not part of
  the executor chain. The port mirror is complete to write: drain loop + tokenizer + registry
  dispatch + 'Unknown command %s\n' + a port-side PushCommand entry (its shipped twin per
  feeder-era: literals from UI events, line from console - all the same one-word call).
- Law family adopted from BrightYak's proposal: the three blindnesses tonight (imm32 scan
  blindness, usage-list floors, byte-indexed decompiler paths) enter the register as one
  family under their name: 'THE INSTRUMENT IS NEVER THE VERDICT.'
- Consequence for the still-OPEN enqueue question (re-shaped AGAIN, honestly): the FIFO gets
  fed by KEY-EVENT FIRES that look up this binding tree and push the found command string -
  the fire site is the next read (candidates: the keymap vft+16 twin's consumer chain, or
  GameClient window/keypress handlers calling INTO [GC+7C]; the drain is pump-side, so the
  fire-and-drain pair straddles frames exactly like the camera-adjacency note predicts).
  Port stays vector<string>-with-named-pushers until the fire is read. NOT ported ahead of
  evidence; the loader era ([Hotkeys] ini section) can land WITHOUT the fire read, and is
  itself a candidate port row once the 58 registrations resume.
- GameClient vft note from the same table read: vft 0x6647f4 carries '??_R4GameClient@@6B@' at
  +0xC - it is GameClient's OWN vft region; the [esi+7Ch]-stored object's vft-patch 0x45e8c3
  set it to that table's GameHotKeys range; RTTI adjacency noted, no claim built on it.
- Gates: paper-only again (this file, only); vft tables read via ida_original_f3 disasm of
  0x69fd34/0x6647f4, methods decompiled whole; every EA re-dumpable cold.


## Slice-5: Execute() LANDS (SwiftOwl, 2026-10-22 ~05:2x) - mirror + handler ABI correction

- src/F3/Commands.{cpp,hpp}: PushCommand (sub_580D10 counterpart) + ExecutePendingCommands
  (cls_0x580ba0::meth_0x580ba0 counterpart) - full chain cited in-file (256B copy local
  @0x580c20, strtok vs ' ' @byte_671C60, FindValue=0x5ACE40 for the 0x59F030->0x59EC80 lookup,
  dispatch shape 0x580c46-0x580c4d, miss 'Unknown command %s\n' @0x580c65, drain-then-camera
  ORDER law @0x587fd5/0x587fda). Pump CALL POSITION named-absent (GameClient pump has no port
  TU yet) - the successor who ports virt_meth_0x45f000 calls the drain at that position.
- ABI CORRECTION (drove every __stdcall->__cdecl retag: 29+29 Commands, 38 Client.cpp, F3.hpp,
  F3.cpp): shipped dispatch is push/call/add esp,4 - CALLER cleanup, handlers bare retn
  (CommandCameraDown @0x588435). With __stdcall, any executor pushing argv at a void() handler
  mis-cleans by 4 bytes per call - caught BEFORE the first dispatch existed, zero blast radius.
- Two documented deviations, both on UB paths (slice-1 guard precedent): >255 lines truncate
  (shipped strcpy smashes), all-space lines skip (shipped derefs empty vector).
- Gates: msvc TRUE_RC=0 (openje.lib + F3.exe relinked), run-all 11/0 app=124, port-annot GATE
  PASS stock 0/392 (the two new defs cited in-file).
- RIG DEFECT FOUND IN-PASS (filed to keeper+coordinator, NOT silently patched): the item-60
  cmkr-regen block (ad6c494, 2026-10-22 00:08) has never executed successfully: its cached
  binary build/_cmkr_v*/build/cmkr is a LINUX ELF (mtime Jul 15), the container wine is i386-
  only, and upstream cmkr releases ship PE32+ only - wine rejects both ('File not found' /
  'Bad EXE format'). All post-midnight msvc-build greens ran via the pre-activation installed
  PATH (T-049's own note: 'activation = coordinator re-up call, not performed'); my print-dev-
  env re-eval activated the new script tonight and the block failed on first-ever real run.
  Mitigation applied: cache dir moved to /tmp/disabled-cmkr-cache-20261022 (restore or fix per
  keeper), restoring the known-good pre-item-60 flow - item 60's GUARD IS NOW INACTIVE: until
  it's fixed right, cmake.toml edits silently no-op AGAIN. Every cmake.toml touch between now
  and the fix must be manually verified against CMakeLists.txt. The regen design needs a host-
  native cmkr leg (ELF in, wine out) - named for rework, not guessed at a fix.

## Slice-5b: behavior proof RUNS - and catches the registry's latent insertion defect
- New Commands row in the run-all table (10th suite): modules/openje/tests/je/Commands/
  Commands.cpp drives register -> push -> drain -> dispatch end-to-end under mingw+wine:
  cell texts/cell math (read DURING dispatch - the shipped cells die with the drain; my own
  post-drain read was dangling and the suite caught its own author first), PROBE case-
  insensitivity with typed-case tokens, strtok run-collapsing, FIFO order (alpha immediately
  before beta), void()-handler ABI cleanliness (probe survives it in-queue), miss-continue,
  drain-empties-queue, empty/space-line skip, >255 truncate deviation - 20 asserts, floor 16.
- DEFECT FOUND BY THE SUITE (openje 1f46073): cls_0x4d8d70::InsertOrGet's lower-bound-node
  LEFT-slot placement overwrote an OCCUPIED slot, orphaning the subtree below it. The app's
  26 registrations never hit it (first key CameraAzimuth is the alphabetical floor of the
  family - every descent attached right-of-root or into empty left slots); 'alpha' after
  'beta' is the first key pair in the port's history that made it reachable - silently
  registered-but-unfindable, count incremented anyway. Fixed by mirroring the shipped
  _Insert_hint descent (0x59f3f0-0x59f443): walk to the NIL slot, attach there, bound
  pointers update only when the parent WAS the bound. RB deviation untouched.
- Registry TU split (src/F3/CommandRegistry.cpp): global + RegisterCommand moved VERBATIM
  from F3.cpp so the console subsystem links standalone (test rows pull defs, not apps).
  Build-affecting: cmake.toml + CMakeLists.txt updated; CMakeLists regenerated HOST-NATIVELY
  with the parked ELF cmkr (~/cmkr-verify/cmkr-elf gen) - the ONLY diff was my line,
  proving the committed list was cmkr-canonical and my hand-edit matches what regeneration
  will produce when the guard is revived. THE KEEPER'S REWORK IS NOW DEMONSTRATED, not
  suggested: the ELF cmkr runs on the host directly (wine was never needed for cmkr itself).
- Gates at this row: run-all 12/0 (with app), msvc TRUE rc0, annot PASS; cls_0x4d8d70 suite
  re-run green post-fix (no regression in the shared tree code).

## Slice-6: SCRIPT-CONTROL FAMILY LIVE (26->30 of 84) - SwiftOwl, 2026-10-22 ~06:2x
- CommandToggleScripts @0x588880, CommandDisableScripts @0x58e4a0, CommandEnableScripts
  @0x58e5b0, CommandOneScriptPass @0x58e6c0 ported + registered (0x58ef1c/0x58ef2b/0x58ef3a/
  0x58ef49). Shipped shape, all raw-read this slice: gate = [0x70BD90->0x04] & 1 (local-client
  authority bit, read through the mislabeled AreaMapIcon getter @0x58e4d2 era); effect = the
  conn byte trio [net->mbr_0x18] + 0x70E4 toggle / +0x70E5 disabled / +0x70E6 one-pass;
  ToggleScripts is gate+flip with NO echo in either branch (@0x58888d-0x5888a2 - the only
  branchless-shape sibling); OneScriptPass goes through sub_5567F0(conn,1) whose param-1 arm IS
  the unconditional store @0x5568a6 (the clear/"Done" arm belongs to the script-pump caller
  era - named, not ported, not needed by this command).
- ECHO DEVIATION (named-absent, EchoClient precedent): the shipped echoes route through
  JE::CDebugInterface::meth_0x5583e0 on [client+0x8C] ([[0x70BD90]->0x2C]+140 @0x58e4c3); the
  port's CDebugInterface is a vft stub, so EchoConsole logs via tracing and skips the real
  console write. Shipped color-arg quirk DOCUMENTED in-file (Disable/remote (0x80,0), Enable
  (0,128), OneScriptPass (0x80,128)) - lands fully when the console TU ports.
- Boot-safety PROVEN by the suite (new asserts 21-22): with the Network singleton absent the
  four commands dispatch, return at the port guard, and the queue keeps flowing with argv
  intact - the guard-deviation is now a TESTED contract, not a comment. Suite 22 asserts.
- Family census note for the batch: script-control trio flags found at their shipped offsets
  (+0x70E4-E6 in the conn object) - first members of the connection object EVER located;
  the object itself (0x7100+ bytes) remains unported, named per-field as located.
- Gates: run-all 12/0 Commands [22>=16] app=124, msvc TRUE rc0, annot GATE PASS stock 0/398,
  cppcheck clean. Registration counter: 30 of 84 ctor-era live.


## Wall-1 ENQUEUE FEEDER CENSUS — CLOSED (BrightYak, 2026-10-22; coordinator ruling 9328c0-off walls)
Question this lane named (#131): who pushes command strings into the FIFO at [GameClient+0x7C].
Census: ALL 17 sub_580D10 push-sites load the SAME object — chain `mov [0x70BD90]; mov +0x2C;
mov +0x7C` verified byte-pattern at every site (two register permutations + the hotkey self-link
variant [esi+8]). The '9 unnamed feeders' from #131 are now named — they are GUI INTERFACE vft
handlers, i.e. the game's own windows enqueue COMMANDS BY NAME through the console FIFO:
  sub_41CA00 = CActionbarInterface era (virt 0x41cfe0): pushes 'InventoryWindow' (0x650b18)
    @0x41cb89, 'OptionsWindow' (0x650b00) @0x41cc05 — actionbar buttons are command aliases.
  GamePlayerAvatarManager meth_0x429900 @0x429a18 + meth_0x42a190 @0x42a1eb: 'RequestNewGame'
    (0x652008) x2 — char-gen completion starts the game VIA the command bus.
  sub_42FE50 = CCharacterGenerationInterface (virt 0x430750): 'OptionsWindow' @0x42ffe6 +
    RUNTIME-CONSTRUCTED string @0x4306ba (stack buffer built pre-push).
  sub_433EE0 = CEscapeMenuInterface (virt 0x434110): 'LoadGameOptions' (0x653e98) @0x4340b2.
  sub_440CE0 = CMainmenuInterface (virt 0x440fb0): 'RequestNewGame' @0x440d92, 'LoadMap'
    (0x656720) @0x440e8e, runtime string @0x440f62 — the main menu's New Game/Load Map are
    console commands on the same bus.
  JE::Client::SetFocus_virt_0x58d9a0 @0x58dac0: runtime string via 0x616d8c (string-transform),
    pushed on FOCUS events — the anomaly stands (scripted on-focus command firing, shape
    consistent with window .int metadata era).
  sub_59E8E0 = JE::CConsoleInterface::virt_meth_0x59ec10 THUNK (e9 @0x59ec13) -> the CONSOLE
    ENTER FEEDER, FOUND. Pushes runtime strings built from console input @0x59ea29 —
    closes the 'separate feeder into the same FIFO' open candidate from SwiftOwl's chain.
  sub_5B6BC0 = CDebugToolsInterface (virt 0x5b7750): 'disablescripts' @0x5b6d54, 'enablescripts'
    @0x5b6dde, 'OneScriptPass' @0x5b6f38 (+ 'ToggleBPsBtn' 0x6ac574 site @0x5b6dde-adjacent
    branch) — debug tools panel buttons enqueue script-control commands.
ARCHITECTURE SUMMARY (all cited): the console FIFO at [client+0x7C] is the GAME'S CENTRAL
COMMAND BUS — windows enqueue command NAMES (static) and command LINES (runtime, console
input + SetFocus strings); the same-frame drain (0x587fd5) + cdecl dispatch (0x580c4b) executes
whatever the registry holds. ShowCursor-era note: 'ToggleBPsBtn' (0x6ac574) joins the static
payload list (wall-2 neighbor intel, banked for whoever ports it).
STRIDE-28 RECORD: fields observed at sites: cell+0x00..0x04 header/next shape + string SSO
{buf@+4(16B), size@+0x14, cap@+0x18} = 0x1C — matches drain-side 28B stepping; the two
'named-unknown' fields from the coordinator's brief are the SSO size/cap pair, now NAMED.
METHOD AUTOPSY (law family fifth member): my E8-scan AND my absolute-pointer scan AND my
jmp-rel32 FORMULA all missed the console thunk; IDA's xrefs caught it — RESOLVED same-sitting,
and two of my own sentences here were wrong: (a) there IS no 'direct call' at 0x59ec11 — the
bytes are 'sub ecx,[ecx-4]; jmp 0x59e8e0' starting AT 0x59ec10 (MSVC SEH4 SPLIT PROLOGUE: the
vft holds the 8-byte thunk as the real ENTRY; 0x59e8e0 is a MID-FUNCTION body address no caller
references, and grep shows exactly that: 1 def line + 1 jmp, zero calls); (b) therefore 'it IS
there (581390 region)' was FALSE — my original E8 census was CORRECT, as were the abs-ptr and
jmp-formula scans; the error was mine in CALLING 0x59e8e0 'the function' at all. Law member #6
of the instrument family: SEH4 split prologues make ENTRY vs BODY distinct addressable things —
census the ENTRY (vft holds the thunk), and a zero-xref 'function' beginning 'sub ecx,[ecx-4]/
jmp' is a body, not an entry. This unifies the corpus's standing 'decompile fails (SEH prologue)'
notes (GUILoadingRecon G4 era) under one mechanism: those bodies are entry-detached the same
way. LESSON STANDS SHARPENED: three scanners agreed because they were all right; the FALSE
negative lived in my TARGET CHOICE, not in any instrument — the fifth and sixth blindnesses are
one family: verify the address IS an entry before censusing it.
NEXT: this census makes Execute() mirror fully proven (console Enter feeder named), and hands
wall-2/wall-3 their command names ('ShowCursor' registration site unchanged; 'ToggleBPsBtn'
is a bus payload, not a registered command — census note for the cursor lane).

## Slice-7a: GAMECLIENT TU OPENS + the four shipped NO-OP registrations (30->34 of 84+)
- NEW: src/F3/GameClient.cpp (ctor_0x460b30, both forms per Client.cpp convention) +
  modules/openje/src/JE/DialogueManager.cpp. Read whole this slice: the shipped ctor = base
  JE::Client::ctor_0x58e900(port) @0x460b52 + vft store @0x460b69 + 23 RegisterCommand sites
  0x460b6f-0x460cbf. SURPRISE FIND: FOUR of the 23 (AttackModeSelect @0x460b8d,
  CharacterRecordWindow @0x460b9c, SneakToggle @0x460c17, WeaponLinkToggle @0x460c44) are
  registered straight at JE::DialogueManager::virt_meth_0x404b70 - whose body is ONE `retn`
  (@0x404b70; shared empty override, 77 sites per ModelCache.hpp census). SHIPPED SHIPS
  DO-NOTHING COMMANDS: they resolve, dispatch, and do nothing. The byte-faithful port is the
  no-op itself; inventing behavior would be the defect. All four land LIVE this slice.
- STATIC MODELING (documented in-file): the port's DialogueManager is non-polymorphic and the
  shipped calls never pass an object ('this' unused, bare retn) - the method is modeled
  static so the address semantics match usage exactly. First MSVC compile caught the
  thiscall->cdecl reinterpret impossibility (error C2440 x4); fixed same sitting, mingw +
  MSVC both green after.
- The remaining 19 land as gated comments IN CTOR ORDER with BOTH EAs (registration site +
  handler body from the batch map): window/GUI era (EscapeMenu 0x45ec00...OptionsWindow
  0x45eb70...ActionbarWindow 0x45ea50), weather trio (ToggleRain/Snow/Wind 0x4606c0-e0 -
  GameClient-member flag era, likely near-portable next), entity/skill era (UseSkill,
  WeaponToggle, ForceEntityStep), TestAnim, options-file era. PipBoy/InventoryWindow notes
  CROSS-REFERENCE the menu-literal feeder (sub_41CA00 pushes exactly those strings - the
  registrations are WHY the buttons work; when they land, menu buttons come alive for free).
- Suite: +section 13 (23 asserts) - the four stubs registered + driven through the real
  executor (dispatch, zero cost, queue survives). Row now links src/F3/GameClient.cpp +
  $JE/DialogueManager.cpp (ctor-era link proof rides every future run).
- BUILD-AFFECTING (coordinator-approved era): f3 sources += src/F3/GameClient.cpp, openje
  sources += JE/DialogueManager.cpp - cmake.toml edited, CMakeLists.txt REGENERATED by the
  host-native cmkr (diff = exactly the two lines; hand-verify law satisfied BY TOOL).
- Gates: msvc TRUE rc0 (F3.exe linked), run-all 12/0 Commands [23>=16] app=124, gate-
  integrity table OK (payload holds new row), annot GATE PASS stock 0/401, cppcheck clean.
- Slice-7 order ahead: pump virt_meth_0x45f000 (installs the ExecutePendingCommands position),
  weather trio handlers, then whatever the pump unlocks first.

## Slice-7b PRE-SCAN (SwiftOwl, same sitting): pump mapped whole - and it is NOT next.
- virt_meth_0x45f000 (0x45f000 + extended tail 0x587ee0-0x588097) read COMPLETE and mapped
  call-by-call. Order with dependency names (raw slots re-pinned per 4e law):
  head ACTION-CONTAINER check ([client+0x28]->+0x44/+0x48 walk, entity+0x40 write) ->
  net::client::virt_meth_0x59d2d0 NET PUMP -> INPUT EVENT QUEUE [client+0x78] vft+8 poll loop
  (per event: state-copy to client+0xC4.. fields on type==2; +0x74 object vft+16 consume;
  +0xA4 vft+28 + vft[0]+200 guards; HOTKEYS FIRE sub_599720([client+0x80]) @0x587fa0 ->
  GUI-ALIVE query JE::GUI::Manager::meth_0x4b2c10(&0x707E48) -> vft+0xD0 dispatch ->
  CONSOLE DRAIN cls_0x580ba0([client+0x7C]) @0x587fd5 -> CAMERA TICK 0x5869c0([client+0x84])
  -> +0xA4/+0xA8 vft+16 ticks -> cls_0x5b4f50::meth_0x5b5ae0(+0xB4) -> +0x74 vft+4 ->
  vft+0xD4(0) -> TIMER TRIO @0x70BD98 era (state==3: JE_TickMilliseconds -> mbr->vft[0](ms),
  vft+20, vft+24 gated by +0x108/+0x109) -> +0x74 vft+8 -> sub_590860 tail.
- VERDICT: the pump is a WIRE, not a subsystem - every segment names an object the port does
  not yet construct (action container, net::client pump, event queue +0x78, audio/timer trio
  0x70BD98, +0xA4/A8/B4 objects, vft+0xD0/D4 overrides, sub_590860). Porting it now would
  fabricate a hollow skeleton with 12 named-absent guards - the register's rule beats the
  schedule: the CONSOLE CHAIN's remaining two wires (client CONSTRUCTION + feeder body path
  [[net]+0x2C]+0x7C) both gate on the NET-BOOT era: F3::GameClient is never instantiated in
  port (its factory virt_meth_0x45dfd0 and the +0x2C install sites are the named gaps).
- ORDER PROPOSED: slice-7c = NET-BOOT era (GameNetwork::Tick's local-server path already
  ports; open client creation + install + FIFO/GHH construction) -> THEN the pump lands as a
  real wire over real objects -> weather trio + Connect/Disconnect + pump-wired Execute all
  inherit from it. Until 7c, the console era stays exactly as honest as 7a left it: 34
  registrations live, executor fully driven by the suite, pump position cited and waiting.
- ERRATUM LANDED IN-CODE (GameClient.cpp gated-comments): weather trio was labeled 'member
  flag era (GameClient member)' from shape-reading - the bodies read this pre-scan are
  net::packet broadcasts (type 110 @0x460242 via client send 0x59c4f0). Label corrected to
  PROVEN with EAs. Third instance of the register's own law biting this seat: shape words
  ('weather flag') replaced by body bytes, caught before any port code used the guess.

## Slice-7c: THE CONFIG PIPELINE - read whole, ported live, instantiation named-gated (fence held)
- Network::virt_meth_0x543ac0 CONFIG READ ported COMPLETE: nine defaults @0x543ad9-0x543aff
  (config 3; [Server] port 21498/debug 17416248/rate 33/timeout 600000; [Client]
  22501/17416248/0/600000), then the same nine via the engine ini tree [Network]configuration,
  [Server]port|debug|rate|timeout, [Client]... (atol-with-keep-default == the port GetInt
  def-contract, sub_4DF770 semantics, EAs per key @0x543c2f...0x5441dd).
- Network::meth_0x543930 -> ApplyNetConfig(int) ported COMPLETE (the old stub's comment had
  mbr_0x4 data flow BACKWARDS - it is WRITTEN @0x543a1c/0x543a38, corrected in the hpp with a
  supersession note): param&3 validation + FatalError('Invalid network configuration
  requested.\r\n') @0x54394a; SERVER branch (vft[4] factory @0x543962-0x543965, conn init/
  log-sink/address/rate/timeout/START sequence named per step 0x54396c-0x54399f incl
  net::server::meth_0x5a19a0(1)); CLIENT branch (vft[5] factory @0x5439e4, the SAME named-
  per-step init sequence @0x5439ee-0x543a19); teardown else-arms (world-server stop chain
  @0x5439bd, deleting-dtor vft[0](conn,1) @0x5439cb/0x543a2f, slots null @0x5439cd/0x543a31).
- FACTORIES READ WHOLE and CITED, INSTANTIATION NAMED-GATED per the coordinator fence:
  vft[5] 0x45dfd0 = new(0x118) + GameClient::ctor_0x460b30(this, net->mbr_0x1c) @0x45dfee/
  0x45e00f (engine default 0x543820 same shape onto JE::Client); vft[4] 0x45e040 = new(0x7108)
  + GameServer::ctor_0x46ee70(this, net->mbr_0x8) @0x45e05e/0x45e07f. Constructing either in
  port TODAY emits a vft referencing ~35 declared-undefined virtuals (LNK2001 both toolchains)
  - the P2 surface lands them; the port-guard path on factory-0 is the named deviation (the
  shipped factories never return 0, guard unreachable shipped-side).
- THE CLIENT INSTALL HUB MAPPED: conn vft[1] = Client::virt_meth_0x58f010 (read whole) = the
  site that constructs EVERY pump-mapped inner object through client vft slots +132..+196
  (graphics pair, four CLoadMapInterface/CDebugInterface windows with their exact geometries
  420x110/300x420/1024x768 @0x58f0f0-0x58f14d, the +0x78/+0x7C/+0x80 event-queue/FIFO/
  GameHotKeys trio @0x58f1ab-0x58f239 - the pump's exact slots!), the veg selector store
  'FX_Selection_Critter_Medium_01.veg' @0x58f255, and the LOCALHOST self-connect @0x58f28d
  gated on the server bit. Enumerated in ApplyNetConfig's comment so no successor re-derives.
- GameServer OWNERSHIP: the slice-6 script-flag trio +0x70E4-E6 lives inside the 0x7108-byte
  GameServer ([net->mbr_0x18]); RequestNewGame @0x58dbe0 adds +0x70A8 (map-name string,
  meth_0x401bd0 store) and +0x70C4 (flag clear) to the layout; the authority gate [net]+0x04&1
  is PROVEN to be the config SERVER bit (only-a-server controls scripts). Comments updated.
- Test section 14 (suite now 32 asserts): constructs a REAL JE::Network, installs the
  singleton, drives config READ (defaults asserted per-EA) and APPLY(2) (config write +
  gated-factory slots + boot-safety), then dispatches the script commands through the LIVE
  config gate (2&1==0 -> remote-echo branch, queue survives) - the config pipeline is tested
  end to end, not comment-tested.
- Gates: msvc TRUE rc0 (F3.exe), run-all 12/0 Commands [32>=16] app=124, annot GATE PASS
  stock 0/401, cppcheck clean on all four touched files. One MSVC catch: reinterpret_cast
  integer->integer rejected (C2440 x2) - static_cast, mingw had accepted it (toolchain
  strictness asymmetry, named for the register: MSVC2002 polices casts the modern cross
  compiler lets slide).

## Law-#6 triage DONE (same sitting, pre-wave-4): GetGlobal/SetGlobal = REAL SEH4 BODIES
- both decompile-fails de-thunked: in-function SEH4 heads (5-push), NOT split thunks;
  bodies read COMPLETE by disasm (168+137 insns) and filed verbatim:
  discoveries/2025-10-22-getglobal-setglobal-seh-triage.md (per-step EA tables).
- Shared shape: argc guard -> 'Usage:' echo via [client+0x88] sink -> GAME-object
  (0x70BD98, cls_0x616328) slot-27 (+0x6C) name->handle -> sub_5A9D50 is-registered.
  Get: miss echo 'Global variable <%s> not registered.'; hit meth_0x5A9E60 toInt +
  '<%s> = %d'. Set: WARNING-ONLY on unregistered (FALLS THROUGH - creates globals!)
  + atol(argv[2]) + GameClient::meth_0x58A9A0 set-sink (body unread, named; message
  tail past 'It' = data-region read, open, not guessed).
- Wave-4 manifest fed: 0x70bd98 role CONFIRMED dual (pump timer + globals owner -
  re-cut with shared-adjacency check before any mint), toInt/toBool sibling pair,
  slot-27 census row, GameServer +0x70A8/+0x70C4 layout rows. Port posture of both
  commands: GATED (game object + echo object unconstructed), bodies read = comments-
  to-code when the era lands - same as the 7c factories.


## SESSION FINAL (SwiftOwl 01a16198, ~03:3x) — five seats, all closed-to-zero or closed-with-evidence
R-ANNOT (97->0) / R-FINISH (9/9) / R-HEADERSAN (57/57 + tool) / R-USERRE-A (34 live regs, console
mapped keypress->camera, Execute+config pipeline LIVE and suite-driven, 0x58f010 hub enumerated) /
WAVE-4 (refreshed table, salvage, toBool write+save, R4 blocked-with-defect -> LD ruled and wrote
isGlobalRegistered_0x5a9d50 himself, R5 holstered with regen receipt). Item 65 (py_eval+set_cmt
channel law) born from this seat's honest stop. Everything durable: this file, the task/PARK rows,
discoveries/, git tips (parent 3475adb-side, openje tip, i64 saved 03:02:41). A warm successor
resumes at: the P2 virtual-surface era (unblocks create+hub+pump+weather+Connect in one), then the
remaining 50 registrations ride the existing recipe. -end-of-transmission

## Slice-B1 (WildKnight 01a1639b, 2026-10-22 ~10:1x) — the GameCreature getter family READ WHOLE; three shared native helpers found; ONE typing trap caught

Family read from the bytes, five natives, all the same shape (`__cdecl (int amx, int params)`):

| native | EA | field read (bytes) | shipped error strings (verbatim) |
|---|---|---|---|
| F3::GetLevel | 0x45abe0 | `mov eax,[eax+0BCh]` | "GetLevel() - Unable to resolve the requested entity." / "GetLevel() - Entity ID does not belong to a game creature." |
| F3::GetRace | 0x45aeb0 | `[eax+0C8h]` | "GetRace() - Unable to resolve requested entity." / "GetRace() - EntityID does not belong to a game creature." |
| F3::GetGender | 0x45af40 | `[eax+0D0h]` | "GetGender() - Unable to resolve requested entity." / "GetGender() - EntityID does not belong to a game creature." |
| F3::GetEthnicity | 0x45afd0 | `[eax+0D4h]` | "GetEthnicity() - Unable to resolve requested entity." / "GetEthnicity() - EntityID does not belong to a game creature." |
| F3::GetSubRace | 0x45b070 | `[eax+0D8h]` | "GetSubRace() - Unable to resolve requested entity." / "GetSubRace() - Entity ID does not belong to a game creature." |

Shared body (identical control flow, cited per-EA where it diverges): `sub_59AF40(amx, params, expectedCells, name)`
arity gate → `sub_59AEF0(amx, params[1], &out, 1)` entity resolve → `___RTDynamicCast(out, 0, &Entity, &GameCreature, 0)`
→ return the dword, `xor eax,eax` on every failure path (0 for all three failures — no distinguishing return value).

THE TYPING TRAP (item 50 applied to the DECOMPILER, not just names): the pseudocode rendered these as
`*(v3 + 52)` on a pointer it LABELS `char *`. Those are not byte offsets — 52 is a **cell index** (0xD0/4),
and the same for 47→0xBC, 50→0xC8, 53→0xD4, 54→0xD8. Taken at face value the family reads as five adjacent
BYTES in the GameEntity prologue; the instruction text says four-byte dwords at 0xBC..0xD8. Bytes win: the
port's own `src/F3/GameCreature.hpp:53-59` already carries `mbr_0xbc…mbr_0xd4` as dwords in exactly that band,
so the getters NAME five existing anonymous fields — no layout collision, no new offsets invented:

  mbr_0xbc=level  mbr_0xc8=race  mbr_0xd0=gender  mbr_0xd4=ethnicity  mbr_0xd8=subrace
  (mbr_0xc0/0xc4/0xcc stay anonymous — no native reads them; do NOT guess names for them)

Three shared helpers, all UNPORTED (grep of src/ + modules/ for their EAs returns nothing) — these are the
actual batch-B prerequisite, since all 39 rows call them:
- `sub_59AF40` @0x59af40 — arity check: `cells = params[0] >> 2`, compare to expected; on mismatch reports
  TWO strings in order: "Invalid number of paramters passed to function %s()." then
  "Expected %d parameters but recieved %d." (the misspellings `paramters`/`recieved` are SHIPPED — add to
  docs/notes/shipped-literals.md; T-022 byte-identity applies).
- `sub_59AEF0` @0x59aef0 — entity resolve: `out = F3::GameActionEquip::meth_0x54be10(handle)` (the T-043
  reference-handle resolver, already known at src/F3/GameActionEquip.hpp:274 but not ported), 0 → fail; the
  4th arg is a "check active" flag: when set and `*(entity+928 /*0x3A0*/])==0` it reports
  "Attempt to use inactive entity in script." and STILL returns 1 (warns, does not block — worth a test,
  it reads like the guard everyone would assume blocks).
- `sub_59AD20` @0x59ad20 — the script error reporter: resolves the script name and the running script's file
  through `sub_5CCCC0(amx, tag, &out)` (tags 1179407181 / 1279415366), `_splitpath`s the file, then
  sprintf("!Run-time error in script %s, file %s%s, line %d: ") + vsprintf into a 1028-byte buffer and hands
  it to `(*(**byte_70C65C + 16))(*byte_70C65C, buffer)` — a logging-sink virtual, NOT FatalError: the shipped
  natives REPORT and CONTINUE. Note the width: 254+253 path buffers + 1028 message, SEH cookie present
  (stack canary at [ebp-4]) — the port has no cookie, documented deviation like every other ported body.

NEXT (slice B2, code): port the trio (error reporter via the port's existing tracing/logging channel,
sink-object deferral NAMED not faked per the EchoConsole precedent), port these five getters into a new TU
`src/F3/ScriptNatives.cpp` (BUILD-AFFECTING: hand-edit cmake.toml AND CMakeLists.txt, cmkr guard INACTIVE
per item 60/62 — verify generation), register them, and add a suite asserting arity-mismatch, unresolvable
entity, non-creature cast, the five field reads, and the INACTIVE-entity warns-but-returns-1 behavior.

## Slice-B2 (WildKnight 01a1639b, 2026-10-22 ~10:5x) — the trio + five getters LIVE, and the suite caught a layout lie that a green build would have shipped

Landed: `src/F3/ScriptNatives.{hpp,cpp}` + `modules/openje/tests/je/ScriptNatives/ScriptNatives.cpp`
(26 asserts) + the `ScriptNatives` row in run-all-tests + cmake.toml AND CMakeLists.txt (dual edit,
**generation verified by running cmkr 0.2.46 host-native: `diff` of generated vs hand-edited CMakeLists =
0 lines**, so the item-60/62 hand-sync rule was satisfied by proof rather than by care).

Shipped bytes → port, per body: arity `0x59af40` (params[0]>>2 cells; BOTH misspelled messages
"paramters"/"recieved" verbatim), resolve `0x59aef0` (store-then-test ordering, the `arg_C` **byte**
active flag, and 0x59af2e `mov al,1`: warns but returns 1), reporter `0x59ad20` (`!Run-time error in
script %s, file %s%s, line %d: ` with the line read at amx+0x2C, into JE::LogDebugString because
byte_70C65C's sink object is unported — the two sub_5CCCC0 name/file lookups are NAMED-DEFERRED, so the
shipped INITIAL values "<Unknown>"/"<unknown>" are what print, which is what the engine itself shows when
those lookups miss). The natives REPORT AND CONTINUE; no FatalError anywhere in this slice.

THE FINDING THAT MATTERS (harness item 69, filed same commit): the first draft read the fields by
shipped displacement — `(char*)creature + 0xBC` — exactly as B1's byte-verified table says, and the
MSVC-equivalent compile said nothing. **10 of 26 asserts went red.** Reason: the generated `src/F3/*.hpp`
structs are name-SORTED excerpts, so the hex in a member NAME cites the ORIGINAL layout and is NOT a
position in the port. Measured on this toolchain: `offsetof(GameCreature, mbr_0xbc) = 1212 (0x4B4)` and
`sizeof(GameCreature) = 1252`, while the shipped object is 3036 bytes with that field at 0xBC. So
`obj->mbr_0xbc` and `(char*)obj + 0xBC` name two DIFFERENT fields, and any port code that does raw
displacement arithmetic against these structs is silently wrong — the class is pre-existing, not new.
Cure in this slice: the five bodies use NAMED members and cite the shipped instruction per line
(`// shipped: mov eax,[eax+0xBCh] at 0x45ac5f`); a comment block in the file states the trap so the next
hand doesn't rediscover it.

Suite coverage that would otherwise be folklore: unresolvable handle => 0; arity 0/2/3 cells => 0 while
1 cell reads the field; handle is params[1] ALONE (decoy in params[2]); non-creature => 0; **inactive
entity (mbr_0x3a0 == 0) still resolves and still returns the field** — the anti-obvious law asserted, so a
future "fix" that makes it block goes red on purpose; five DISTINCT full-width sentinels
(0x11111111..0x55555555) across 0xBC/C8/D0/D4/D8 proving dword width AND per-field mapping, with the
byte-trap refutation asserted directly (`!= 0x5A`); failed resolve STORES 0 into the out param before
returning (0x59af03 ordering); re-publish under a new handle; unregister => unresolvable; table shape
(5 entries, EA order, expected cells, and the table's fn pointer being the SAME body).

Registration stays NAMED-ABSENT on purpose: the shipped `{name,fn}` arrays are consumed by the amx_Register
era (0x5CCDE0, per-entry 0x5CCD60, miss err 19 @0x5CCE2E) and the port has no amx module table, so this
slice exposes the table (`ScriptNativeGameCreatureTable`) and does NOT fake a VM hookup — wiring into a VM
that does not exist yet would be inventing the thing being reversed. Same for the entity list: the byte_70BE0C
walk runs against a documented 64-entry port-side stand-in (unbounded shipped list vs fixed table = named
deviation), which MapEntitySpawn can adopt when the real container lands.

GATES: run-all-tests **13 pass / 0 fail** (new `ScriptNatives=OK [26>=26]`, app-boot 124, port-annot GATE
PASS after the four port-side helpers got their citations — stock 0/413), `msvc-build` rc=0 with
`Building CXX object .../src/F3/ScriptNatives.cpp.obj` in the log and `[100%] Built target F3` (MSVC 2002
is the product toolchain; mingw-green alone would not have proved the TU), cppcheck-openvb clean,
cppcheck-openje nothing-changed-under-JE (test lives under openje/tests).

NEXT (B3): the remaining getter families — health trio (GetCurrentHP 0x45b1b0 / GetMaxHP 0x45b110 /
AddHP 0x45b390 / SetHP 0x45b2e0: same shape, offsets unknown until read), the skill pair (GetBaseSkill
0x45ad90 / GetTotalSkill 0x45ae20 / GetPerkRank 0x45ab50 — expect INDEXED reads, so params[2] matters and
the arity gate's expected cells will differ), then the action verbs (EquipItem/UnequipItem/IsItemEquipped)
which are the first rows that WRITE.

## Slice-B3 (WildKnight 01a1639b, 2026-10-22 ~11:2x) — HP pair LIVE with an IS-A class ladder; four families deferred WITH their mechanisms named

Landed: `ScriptNative_GetCurrentHP` (0x45b1b0) and `ScriptNative_GetMaxHP` (0x45b110), the registry's
creature flag generalised into a **class ladder** (`SNC_NONE < SNC_GAME_ENTITY < SNC_GAME_CREATURE`, compared
with `>=`) because the two families cast to DIFFERENT targets (B1 five getters: Entity->GameCreature at
0x45af9f/0x45af0f/0x45b037/0x45b0d7/0x45ac3f; HP pair: Entity->GameEntity at 0x45b217/0x45b177), and the table
grew to 7 entries. Suite now **35 asserts**, floor raised 26->35 in the run-all-tests row. Gates: run-all-tests
**13 pass / 0 fail**, `ScriptNatives=OK [35>=35]`, app-boot 124, port-annot GATE PASS; msvc-build rc=0.

Two RE facts worth the board's time:

1. **GetMaxHP is not a field read.** 0x45b197 `mov edx,[eax]; mov ecx,eax; call dword ptr [edx]` - vftable
   SLOT 0 of GameEntity (0x666130[0] = virt_meth_0x4080c0), which is itself `mov eax,[ecx+20h]; retn`. So max
   HP is a virtual today's implementation answers from +0x20, and a derived class could override it. The port
   funnels it through ONE seam (GameEntityMaxHp) instead of inline reads, with the missing vftable named as the
   deviation. GetCurrentHP, by contrast, is the plain `[eax+24h]` at 0x45b237. Doc: docs/f3/CreatureHpAccessors.md.
2. **Two int arrays inside GameCreature, discovered via the natives that index them**: perk ranks at
   `+0x114` (0x45abd2 `mov eax,[eax+ecx*4+114h]`) and base skills at `+0xE0` (callee 0x409824, a 3-instruction
   `thiscall (int skill)` with `retn 4`, which the IDB labels `cls_0x409820::meth_0x409820` - another
   owner-mis-attribution of the GameActionEquip kind: the caller RTTI-casts to GameCreature and passes it
   unchanged). No bounds test in either native. Doc: docs/f3/CreatureSkillAndPerkArrays.md.

DEFERRALS, each with the blocker named rather than the row quietly skipped (this is the part that keeps the
batch honest):

- **SetHP (0x45b2e0) / AddHP (0x45b390)**: arity 2, and neither writes a field - both fetch a subject through
  `mov edx,[eax+4]; mov edx,[edx+4]; mov eax,[edx+eax+1E0h]` then thiscall `meth_0x46ee10` whose `this` comes
  from `*(F3::global_area_map_icons+0x18)`, and 0x46ee10 itself runs the HANDLE RESOLVER on that `this`
  (0x46ee19 calls meth_0x54be10) before casting to GameEntity and calling `meth_0x4629c0(entity, value)` AND
  `meth_0x46d620(fetched, value)` - two writes. The `+4/+4` chain's meaning is unresolved, so WHAT the setter
  writes to is not yet known. AddHP's value expression is `currentHP + params[2]`, i.e. shipped AddHP is a
  set-to-computed, not an increment primitive. B4 reads 0x4629c0/0x46d620/0x46ee10 whole, then ports the pair.
- **GetPerkRank / GetBaseSkill / GetTotalSkill**: need REAL array members. The port header carries the first
  dword of each band (`mbr_0x114`, `mbr_0xe0`) and item 69 means `(&obj->mbr_0x114)[i]` would index the wrong
  neighbours while compiling clean - so this needs a layout decision on the header, not a private edit in a
  natives TU. Filed as B4's prerequisite with the candidate shapes.
- **GetTotalSkill's formulas** (callee 0x409830, the 1x/2x/3x attribute weights + the flat Firearms/Melee arm +
  the `UNKNOWN SKILL: %d` default) are recorded as a LEAD with numbers marked unverified - switch lowering and
  constant scaling are precisely where pseudocode is quietly wrong, so B5 verifies against bytes with a named
  claim list rather than porting the pretty version.

Shipped-telemetry finding banked in docs/notes/shipped-literals.md: **GetPerkRank reports
`"GetPosition() was unable to find the requested entity."` (0x45ab8e) when its own resolve fails** - a shipped
copy-paste bug. The port reproduces wrong strings verbatim (T-022); a parity diff that launders an engine bug
into clean text is worse than the bug. Also logged: `EntityID` vs `Entity ID` and `the requested entity` vs
`requested entity` split across the SAME family, per-native, measured at each push site.

Self-note, caught by my own suite again (item 69's discipline working, not a shame entry): the HP asserts were
first placed AFTER the section that unregisters the test entity, so 2 asserts failed for harness ordering
reasons, not product ones. Diagnosis was `offsetof`+standalone repro before touching the code - which is how
the B2 layout finding got found in the first place.

## Slice-B4 (WildKnight 01a1639b, 2026-10-22 ~11:5x) — the HP WRITERS read WHOLE, and stay unported on ONE named question

No new natives landed. What landed is the full read of the deferred chain, in docs/f3/CreatureHpWriters.md,
with the port blocked on a single identified uncertainty rather than on vagueness:

- **SetHP/AddHP are arity 2 and write nothing themselves** - both go through `meth_0x46ee10` (0x46ee10), whose
  IDB owner label (`GameActionEquip`) is wrong the now-customary way. Its legs: resolve the FETCHED HANDLE
  (0x46ee19 -> 0x54be10, argument passed on the STACK - the same shape 0x59aef0 uses, which is why IDA prints
  it as `this`), RTTI Entity->GameEntity, then `meth_0x4629c0(entity,value)` + `meth_0x46d620(server,handle,value)`,
  return 1; every failure path returns 0 and, unlike the natives, **silently** (no error string at all).
- **`meth_0x4629c0` (0x4629c0) is a clamp with a hole**: `mbr_0x24 = value; if (value > mbr_0x20) mbr_0x24 = mbr_0x20;
  return mbr_0x20`. Upper cap at max HP, **NO lower clamp** - HP goes negative through this door. The combat
  lane must not assume HP >= 0 on the strength of this setter.
- **AddHP is `set to current + N`** (0x45b3de passes `[entity+0x24] + params[2]`), not an increment primitive.
- **HP is server-authoritative and broadcast**: `meth_0x46d620` builds a `net::packet` (0x2048-byte frame,
  cookie), sets id **88**, writes handle then value through `packet::meth_0x45f130` (0x46d67d/0x46d68e), sends
  via `GameServer::meth_0x5a0a00(pkt,1)` - a fan-out loop `i < *(server+92)` calling `meth_0x5a09d0` per peer -
  and logs `"Server - Sending GAME_ID_ENTITY_SETCURRENTHP"`. A script cannot set HP locally without a packet.
- **THE BLOCKING QUESTION, named**: the clamp is applied to the entity resolved from
  `*(entity + vbdisp + 0x1E0)`, not from `params[1]`. It is the same object iff that slot holds the entity's own
  net handle. The evidence says yes (MSVC virtual-base displacement chain, +0x1E0 sits right before the handle
  band T-041 pinned at +0x1DC/+0x1E4, and the same value is the packet's target field) - but `artifacts/F3.json`
  records NO member at +0x1E0 (f3_member_index: zero rows), so the frozen export cannot confirm the writer.
  Porting the two-step identity as a one-step one would differ exactly in the borrowed-handle case, if it
  exists, so the rows wait.
- Close cost, one sitting: find the writer of +0x1E0 in the create/pack path (`Entity::Create` 0x55190c-era /
  `PackState`, both already known to touch the handle band). If it stores the entity's own id, port SetHP/AddHP
  as resolve -> clamp(upper only) -> packet-88 (NAMED-deferred until the GameServer era) -> return 1, and assert
  the negative pass-through.

Batch-B running scorecard: 7 natives LIVE (B1 five + B3 HP pair), 2 read-whole-deferred (SetHP/AddHP), 3 read-
whole-blocked-on-layout (the skill/perk arrays - item 69's header decision), and the remaining batch-B rows still
unread. The deferrals are all "mechanism known, one link open", not "we did not look".


### B4 follow-up (same sitting) - the +0x1E0 probe made the deferral STRONGER, and caught me in a lookup error

Two things, filed so nobody re-derives them (full text in docs/f3/CreatureHpWriters.md "FOLLOW-UP"):

1. **My own error, corrected**: I wrote "the frozen export records NO member at +0x1E0" after
   `f3_member_index(class=GameEntity, 0x1e0)` returned zero rows. The member EXISTS - it is
   **JE::Entity+0x1E0** (`mbr_0x1e0`, size 4, base-inherited; 9 usages incl. `Entity::ctor_0x5513a0` @0x551475
   and `Entity::virt_meth_0x54ab00` @0x54ab0c/0x54ab53). Querying the DERIVED class and concluding the evidence
   was absent is the item-66 trap pointed at my own tool call: absence in one query is not absence in the data.
2. **The link is now sharper, not closed**: that virtual compares `*(this+0x1E0)` against **1, 2, 3, 6** - a
   small-integer enumeration, i.e. a TYPE/MODE-looking field, while `SetHP`'s chain feeds the value fetched at
   that offset into `meth_0x54be10`, whose entire purpose is matching a cell against `entity+0x1DC`. So either
   the `+4/+4` chain puts a DIFFERENT object in the base register (and my offset reading is about another
   object), or the setter's subject is found through the resolver's map fallthrough (0x54be41) and has nothing
   to do with the script's handle. Both keep the deferral; neither is guessable safely, because a clamp applied
   to the WRONG entity is precisely the bug a test cannot catch - the test would assert the same wrong entity.
   Next probe is one window, not a chase: disassemble `Entity::ctor_0x5513a0` at 0x551475 and see whether the
   stored value is a small constant (enum) or an id-like value (handle). That single read decides it.

Net: batch B stays 7 natives LIVE; SetHP/AddHP remain read-to-the-last-interpretable-instruction with a named,
cheap, decisive next probe. I stopped rather than ported an ambiguous operand - the row's own law (bytes are the
contract) cuts both ways, and an ambiguous subject IS a bytes-not-settled state.


## Slice-B4b (same sitting) - the +0x1E0 probe ran: the field is an ENUM (1,2,3,6,7), so one hypothesis died and the blocker got honest

Measured: `virt_meth_0x549030` (11 bytes) returns `*(this+0x1E0) == 7`; `virt_meth_0x54ab50` compares it against
1/2/3/6 before falling through to `[vft+0x64]`, `+0x2F0`, `+0x314`; `Entity::ctor_0x5513a0` @0x551475 zero-inits
it in a run with +0x1C4/+0x1DC/+0x1E4/+0x1E8/+0x1EC; and `Entity::UnpackState_54e690` writes it at 0x54e6ac and
0x54e733. A field equality-tested against one-digit constants, deserialized from state, zero-initialised next to
the handle band - that is a TYPE/MODE enum. The handle proper is +0x1DC (T-043), a DIFFERENT adjacent offset.

So hypothesis (b) - "SetHP hands an enum to the handle resolver" - is dead, and with it my own inference that the
`[edx+eax+1E0h]` fetch reads the same Entity subobject these readers share. The `+4/+4` chain is still
uninterpreted: a vbtable displacement would land on the enum, which the readers just excluded, so the base is a
different object or the table's slot 4 is not what I assumed.

Net for the batch: SetHP/AddHP stay unported, and the question is now sharper than the one I filed an hour ago -
not "is the handle self-referential" (it isn't a handle) but **"which object owns the fetched operand"** - i.e.
whose HP gets clamped. Guessing it would put a wrong-entity write into product code with a green test asserting
the wrong entity, which is the failure mode this board's laws exist for. Next probe named: `virt_meth_0x54dea0`
(uses +0x1E0 at 0x54df63) plus the two UnpackState sites; if +0x1E0 arrives from a byte stream it is a serialized
type code and the `+4/+4` chain gets its own short doc rather than another paragraph here.

The sitting still banked real facts regardless: clamp is upper-only (no floor - HP goes negative), AddHP is
set-to-current+N, HP writes broadcast as packet id 88 through GameServer::meth_0x5a0a00's per-peer fan-out, and
the setter chain fails SILENTLY (no string), unlike the natives.


### B4c - the two blockers turned out to be ONE: batch B's remaining rows all wait on a shipped-ordered entity layout

`artifacts/F3.json` records `JE::Entity` embedding **`JE::DynamicObject` at +0x0, size 984 (0x3D8)** - so
+0x1DC and +0x1E0 are both INSIDE the DynamicObject span, and SetHP's `[entity+4] -> [+4] -> [+entity+1E0h]` is
a **virtual-base displacement**, not a plain field read. It is interpretable only with the real subobject
geometry (which table is at +4, what its slot 4 holds, what the displacement is), and the coincidence "enum slot
== handle slot" requires the displacement to be 0 - which is exactly the assumption that would ship a
wrong-entity HP write.

The port cannot express it: per item 69 the generated headers are name-sorted excerpts with no subobject spans
and no vbtable at all. So the three array natives (perk +0x114, base-skill +0xE0) and the two HP writers are
parked on **one prerequisite** - a shipped-ordered GameEntity/GameCreature layout with sized array members - not
on five separate mysteries. Filed as the batch's dependency (header-owner lane, blast radius beyond batch B)
rather than taken inside a natives TU; the minimum viable version that unblocks the arrays without pretending
to model virtual bases is written up in docs/f3/CreatureHpWriters.md (arrays as real `int[]`, lengths still
UNKNOWN and not to be guessed), leaving SetHP/AddHP honestly waiting.

Batch B final shape for this sitting: **7 natives LIVE** (+ the three shared helpers every later row needs),
**5 rows blocked on the layout decision with a named minimum fix**, and the HP setters documented down to the
instruction where the ambiguity actually lives. Everything deferred is deferred on a *question*, with the cheap
answer-path named - not on effort.


### B5 - the named probe ran, the enum is confirmed a fourth way, and two "ints" in the pseudocode are floats

`Entity::virt_meth_0x54dea0` (0x54dea0) tests `Entity+0x1E0` against **5** (0x54df73) and picks between two
`sub_563D30` overlay calls whose trailing literals the decompiler prints as integers: **1029785518 = 0x3D6147AE
= 0.055f** and **1028443341 = 0x3D4CCCCD = 0.05f**, with **0xFFFF0000** / **0xFF00FFFF** colour-shaped partners.
So +0x1E0 is a presentation/state MODE (now seen tested against 1,2,3,5,6,7 across four sites, serialized by
UnpackState_54e690, zero-init in ctor_0x5513a0) - which kills the last comforting reading of SetHP's operand:
whatever `[edx+eax+1E0h]` fetches, it is not this entity's handle, and the `+4/+4` chain lands on a DIFFERENT
object. SetHP/AddHP stay unported; the blocker is now "identify the object the +4/+4 chain produces", with the
float claim flagged unverified-in-sub_563D30 rather than asserted.

Bonus for the map lane, found under a probe aimed elsewhere and written down so it isn't lost: the same body
resolves a Map via `sub_401130()` -> `[mbr_0x0+84](*(entity+0x22C))` -> `JE::Map::GetOctree_56d960` ->
`Entity::meth_0x568140(octree)`, guarded by a one-shot byte latch at entity+0x164 and fed by an SSO-shaped string
at +0x21C (capacity test `< 0x10` at 0x54dedd, size slot +0x230). Cited path from the string band into octree
attachment, which the spawn/map lane may want.

Batch B stands at: 7 natives LIVE, 5 rows blocked (3 on the item-69 array/layout decision, 2 on the +4/+4 subject
identity), all with named next probes. Nothing in this commit is a port - docs and ledger only.

## Slice-B6 (WildKnight 01a1639b, 2026-10-22 ~12:3x) — type/DC family LIVE, class model becomes a BITMASK, and the suite caught item 69 biting inside MY OWN code

Three more natives: `GetItemType` (0x45b240, `+0xA0` at 0x45b2c7), `GetUsableObjectType` (0x45b440, SAME `+0xA0`
at 0x45b4c7, different class), `GetLockDC` (0x45b570 — the dual-cast native: `GameContainer` at 0x45b5d3, else
`GameDoor` at 0x45b5fc, one slot `+0xC0` read at 0x45b5dd/0x45b5e8). Suite 35->**43 asserts**, table 7->10
entries, gates re-run: run-all-tests 13 pass / 0 fail with `ScriptNatives=OK [43>=43]`, msvc-build rc=0.

Two model changes the bytes forced:

1. **The class ladder is now a bitmask.** `SNC_GAME_ENTITY/CREATURE/ITEM/USABLE/CONTAINER/DOOR` are bits and a
   registration records the whole accepted set (a container = `CONTAINER|USABLE|ENTITY`), because the families are
   SIBLINGS, not rungs - `GetItemType` must reject a `GameUsableObject` and vice versa (asserted both ways), while
   `GetLockDC` must accept TWO of them. The linear `>=` ladder I introduced in B3 was right for exactly one pair
   of classes and wrong everywhere else; better to have caught it at 10 rows than at 39.
2. **Read a shared field through its OWNER type.** First draft read the DC slot via `GameContainer*`; the Door
   assert went red. Cause: `GameUsableObject.hpp:15` declares `mbr_0xc0` AND `GameContainer.hpp:16` re-declares
   the same-named member, and per item 69 each generated header is its own name-sorted excerpt - so the same
   *shipped* slot sits at two different *port* offsets depending on which class you name it through. Shipped has
   one address; the port needed the owner type to keep that true. Filed as an extension to item 69 with the third
   consequence spelled out: a test that registers class A and writes the field through class B's name is silently
   comparing two addresses, so a green dual-cast test proves nothing unless the write goes through the owner.

Shipped telemetry banked (docs/notes/shipped-literals.md next time it's touched; recorded here so it is not lost):
`GetItemType() - Entity is not an Item` has NO trailing period; `GetUsableObjectType() - Entity is not an usable
ubject` carries a shipped TYPO ("ubject") plus an "a/an" slip; `GetLockDC` and friends in the "action" tier use the
OTHER resolve wording — `was unable to find the entity.` rather than `Unable to resolve requested entity.` — so the
batch has at least three distinct resolve-failure phrasings, all preserved verbatim in the port.

Still open in batch B after this: the 2 HP writers (`+4/+4` subject identity, B4b/B5), the 3 array natives
(item-69 layout decision), and the rest of the unread rows — plus `IsItemEquipped` (0x45b750) now read: it is the
first STRING-argument native (uses `sub_59AB20` with a 0x80 buffer at 0x45b7cc), reaches the creature's inventory
through `*(creature+360)`, casts `Inventory->GameInventory`, and probes via a `StringTree` lookup
(`JE::StringTree::meth_0x401d20` + `GameInventory::meth_0x46a6c0`) with five distinct failure lines; it needs the
AMX string-fetch helper and the inventory container, so it is queued rather than squeezed in.

## Slice-B7 (WildKnight 01a1639b, 2026-10-22 ~13:0x) — research tranche: one shipped STUB, one self-answer shortcut, and the idiom that reframes the SetHP paradox

Three rows read, none ported (the first is portable in principle; it is not worth a commit until the helper it
calls is understood, and it returns nothing either way).

**1. `F3::GetWeaponCombo` (0x45a9f0) is a shipped stub that always returns 0.** Byte-verified, not pseudocode:
after the arity gate (0x45aa04) it calls `sub_45B9F0(amx, params[1], &local_4)` (0x45aa1a), reports
`"GetWeaponCombo() was unable to find the requested entity."` when that fails (0x45aa2c), and every path lands on
`loc_45AA34` = `xor eax,eax; …; retn`. `local_4` — the resolved answer — is **never read**. So scripts calling
this native get 0 no matter what, and any script logic that trusts a weapon-combo return value has been reading a
constant for the whole life of the build. Porting it is trivial and must stay trivial: return 0, keep the error
line, and do NOT "helpfully" return the resolved object.

**2. `GetSquadAttitude` (0x45aa90) / `GetTeamAttitude` (0x45aaf0) answer the easy case before asking anyone.**
Arity 2, no entity resolve at all, and the FIRST thing they do is
`if (params[1] == params[2]) return 1;` (0x45aab2-0x45aaba, same at 0x45ab12-0x45ab1a) — an entity's attitude to
itself is "same" by identity, not by table lookup. Otherwise:
`GetWorldServerInstance_0x401150()` → the indirection below → vft slot **34** (136/4) for squad, **33** (132/4)
for team → then `GameActionEquip::meth_0x5692e0` / `meth_0x56a240` take the two handles. Deferred: needs the
world-server relationship object, those two virtuals, and the port's own server-side stance.

**3. The idiom — and why it matters to B4b.** Both attitude bodies compute their callee object as

    v4 = &WSI[1] + *( WSI[1].mbr_0x0 + 4 )        ; 0x45aacf / 0x45ab2f

i.e. *take a value loaded from a slot at +4 of something, and add it to a base*. That is structurally the same
shape SetHP uses three times in a row (`mov edx,[eax+4] ; mov edx,[edx+4] ; mov eax,[edx+eax+1E0h]`,
0x45b367-0x45b36d). Two unrelated natives in the same batch doing it means it is **a deliberate engine indiom —
a base-plus-delta addressing of an associated object — not a compiler artifact and not a vbtable quirk.** Which
supports JadeBear's census conclusion from the other side: SetHP's `+0x1E0` is read off a DIFFERENT object than
the entity, so the "is it self-referential" framing was always the wrong question, and my reading of the base
register was the mistake. Their Entity+0x1E0 lifecycle-state finding (birth 5, `==7` at 0x549030, five "Invalid
state change" strings in `meth_0x5507f0`) and this idiom are the same fact seen from a dump and from the code.

**Also ruled out, cheaply and by name:** Entity's own ctor stores exactly ONE vftable (`Entity_DynamicObject_vftable_692084`
at `[esi+0]`, 0x5513c8) and writes nothing to +4, so `[entity+4]` is not a second vftable installed by Entity —
which was the leading static hypothesis for the displacement chain. Settling it needs one live read
(`[entity+4]`, then `*([entity+4]+4)`), already requested from the session holding the lockstep tooling.

Batch B tally: 10 LIVE, 3 read-whole-deferred (attitude pair: relationship object; weapon-combo: nothing to
port), 5 blocked (3 arrays on the item-69 layout ask, 2 HP writers on the now-identified delta-idiom subject),
and one shipped stub documented so nobody "fixes" it into returning something.

## Slice-B9 (WildKnight 01a1639b, 2026-10-22 ~13:5x) — SetHP/AddHP UNBLOCKED and LIVE: the +0x1E0 paradox was a convention shift, and the round-trip is kept because its failure is observable

JadeBear closed the blocker statically (a vbtable entry is a compile-time constant, so no live read was ever
needed): `GameEntity::ctor_0x4678a0` stores `[esi+4] = 0x666134` at 0x4678d4, and the SAME ctor proves the
arithmetic at 0x4678f5/0x4678fd - it installs the secondary vftable at `this + delta + 4`, which IS
`Entity-base`. So SetHP's `[edx+eax+1E0h]` = Entity-base + 0x1DC = **T-043's handle field**, the `0x1E0` being
`0x1DC + 4` with the vbptr position folded into the disp32. My branch (ii) ("code-address garbage") is dead, and
so was the "borrowed handle" worry: `Create` 0x55190c stamps +0x1DC once and `UnpackState` restores the pair, so
handle -> registry -> entity is the identity. Both readings were correct about their own halves - JadeBear's
Entity+0x1E0 = ChangeState state, my base-plus-delta idiom - and the resolution is that the instruction's
displacement was never Entity-relative.

Landed: `ScriptNative_SetHP` (0x45b2e0) and `ScriptNative_AddHP` (0x45b390), suite 46 -> **58 asserts**, table
13 natives, floor raised in the run-all row. Gates after the change: `run-all-tests` 13 pass / 0 fail with
`ScriptNatives=OK [58 >= 58]`, `app-boot` 124, and `msvc-build` rc=0 with `ScriptNatives.cpp` compiled.

Deliberate faithfulness calls, all asserted rather than commented:
- **the round-trip is KEPT, not collapsed.** A one-step port would be equivalent on the success path but would
  lose the shipped SILENT failure (0x46ee23/0x46ee3e return 0 with no error string) - so the native re-reads the
  entity's own `mbr_0x1dc` and re-resolves it, and the suite proves both that it lands on the same object and
  that an unregistered own-handle writes NOTHING.
- the clamp's HOLE is now test-enforced in both writers: cap at `+0x20` only, negative values store as-is.
- `AddHP` is `set to current + N`, clamping the **sum**, and a second call adds to the NEW current.
- the packet-88 broadcast (`meth_0x46d620` -> `GameServer::meth_0x5a0a00` per-peer fan-out) is **named-deferred**
  with the server era. Until that lands, HP is a LOCAL write - a real divergence for any multiplayer parity work,
  written in the source and here so nobody rediscovers it from a diff.

Third instance of item 69's sibling-redeclaration trap, caught by this very suite: `GameItem.hpp` re-declares
`+0x1DC`, `+0x20` and `+0x24`, so the test's first writes went to different port addresses than the
owner-typed native reads (`handle_slot=0`). Owner-typed member access is now a rule in these tests, not a
preference - and note the debug detour that found it: my first repro printed `ptrmatch=0` as if resolve were
broken, and the real cause was printf's unspecified argument evaluation order reading `pe` before the call.
Measure the sequence, not the one-liner.

Batch B after B9: **12 natives LIVE** (B1 five, HP pair, type/DC three, PlayMusic stub, SetHP/AddHP), 3 array
rows still blocked on the item-69 layout decision, the attitude pair waiting on the world-server relationship
object (JadeBear located it: GameEntity's own ctor stamps `mbr_0x28` with the same idiom at 0x4679aa), and two
documented shipped stubs. B4's two rows went from "subject unknown" to shipped in one sitting because the
question was answered with a byte sequence instead of a guess.

## Slice-B10 (WildKnight 01a1639b, 2026-10-22 ~14:2x) — the door-lock trio LIVE, and one byte reveals a shipped asymmetry

Landed `LockDoor` (0x45a1b0), `UnlockDoor` (0x45a240), `IsLocked` (0x45b4e0): all arity 1, all cast
Entity->GameDoor ONLY (0x45a20f / 0x45a29f / 0x45b53f), and the entire lock state is the byte at
`GameDoor+0xBD` (store 1 / store 0 / `movzx` read at 0x45b55f). Suite 58 -> **69 asserts**, table 16 natives.

Four facts the bytes gave up:

1. **The asymmetry against `GetLockDC` is real and now asserted**: a `GameContainer` has a lock DC readable by
   `GetLockDC` (0x45b570 accepts CONTAINER|DOOR through their common base) but CANNOT be locked, unlocked, or
   `IsLocked`-ed, because those three are door-only. So "container lock DC" is data the script layer can read but
   not act on through this family.
2. **`IsLocked` zero-extends** (`movzx` at 0x45b55f), so a byte of 0xAB returns **171**, not -85 - asserted,
   because the port's `byte` type plus a signed read would have been an easy silent divergence.
3. **`IsLocked` cannot distinguish "unlocked" from "not a door" by value** - both are 0; only the report differs.
   Recorded because a script that treats 0 as "unlocked, proceed" is also proceeding on a wrong entity class.
4. **Locking a door is LOCAL state** - no packet, no broadcast, in pointed contrast to the HP writers two slices
   ago, which broadcast id 88 per peer. Same "mutate entity" shape, opposite network contract; that difference is
   a design fact about the shipped engine, not an oversight to normalize.

Read and deliberately NOT ported (each with what it waits on):
- `HideInterfaces` (0x45a100) - arity 0, calls `sub_46EDF0`, returns 1. Porting it now would be an empty shell with
  a success return; waits on the GUI-hide routine.
- `SetMoveMode` (0x45a140) - arity 3, uses `sub_59AA50` (an AMX object fetch) plus the `global_area_map_icons[0]
  ->int_0x18` server-context guard and `meth_0x46ec70`; waits on the AMX object-reference helper.
- `Protagonist` (0x45a960) - arity 0, and a third sighting of the same base-plus-delta idiom on the world-server
  singleton (0x45a991) before vft slot 26 (104/4) and a `Party -> GameParty` cast. Its payoff is the best single
  corroboration of B9 in this batch: on success it returns `*(entity + 476)` = **+0x1DC** - T-043's handle field,
  delivered as a native's RESULT. Two shipped oddities banked with it: both failure paths say
  `"Protagonist() ... Returning INVALID_ID."` while returning **0**, not an INVALID_ID sentinel; and the
  GameParty member is read at `[party+5]` cells (= +0x14) with a null test before it.

Gates after the change: `run-all-tests` (floor raised 58->69) and `msvc-build`, both re-run; `ScriptNatives.cpp`
is compiled by both toolchains, not just mingw.

## Slice-B11 (WildKnight 01a1639b, 2026-10-22 ~14:5x) — four rows read; none ported; one method correction adopted, and a categorization finding that moves two rows out of this TU

Read: `GetWeaponRange` (0x45b620), `AddCNPCToParty` (0x45a2d0), `StartDialogue` (0x45a040),
`CommandActionbarWindow` (0x45ea50). Nothing landed, and that is the correct outcome for this tranche -
each is blocked on something specific, named below, rather than on effort.

**1. Method correction, from JadeBear's receipt against my own claim.** My `GetLockDC` `+0xC0` came from
multiplying the pseudocode's `*(v3 + 48)` by 4. The bytes at 0x45b5dd (`8b 80 c0 00 00 00`) say the same thing,
so the value was right and the derivation was not: the decompiler's offset rendering depends on a pointer type it
*guessed*, and it happened to be a 4-byte type here. The rule now written into the source comment is that
**offsets are adjudicated against instruction text, never against the decompiler's number** - a sibling of the
cell-index trap, and the reason this batch's every field citation carries a `mov` line.

**2. `GetWeaponRange` (0x45b620) is a four-hop object chain, not a field read.** Arity 1, cast to GameCreature
(0x45b68d), then `*(creature+360)` = +0x168 for the INVENTORY (the same slot `IsItemEquipped` 0x45b821 uses -
two natives, one accessor, consistent), RTTI `Inventory -> GameInventory`, `Get_mbr_0x98` (0x463b60) for the
equipped-weapon slot, `GameInventory::meth_0x4693d0` to find that item, RTTI `GameItem -> GameWeapon`, and
`+258`-cells (+0x408, needs byte confirmation) - **else** falls back to the creature's own `mbr_0x6f4`; whichever
value survives is then dereferenced `+ 44` for the actual range, and a zero at either step returns 0. Blocked on
the inventory/weapon era (GameInventory + GameWeapon unported); note for later: the creature's `+0x6F4` is a
POINTER that gets dereferenced, so a range of "0" and "no weapon pointer" are again indistinguishable by value.

**3. `AddCNPCToParty` (0x45a2d0) - fourth sighting of the world-server base-plus-delta idiom** (0x45a32f, slot
26 via `+104`, RTTI `Party -> GameParty`), and its payload is `(**v5)(v5, entity)` - **GameParty vft slot 0**
called with the resolved entity. So "add NPC to party" is a virtual add on the party object, and the whole native
is a lookup + dispatch. Blocked on the party class and the singleton's slot-26 accessor. Its failure line names
the party, not the entity: `"AddCNPCToParty() was unable to find the party."`

**4. `StartDialogue` (0x45a040) is the closest to portable and one seam away.** Arity 2; resolves source and
target with DISTINCT messages (`"...unable to find the source entity."` 0x45a080 vs `"...target entity."`
0x45a0ae); then `JE::Entity::meth_0x54c440(source)` which is NOT a getter - read whole, it walks the entity's
action deque at `gap2F4`/+0x304, calls each action's `vft+8`, destroys them, and finishes with
`if (!vft[27](this)) meth_0x54bf00(this)` - i.e. **starting a dialogue FLUSHES the source's pending action
queue** before enqueuing. Then `operator new(0x24)` + `ctor_0x404b40`, stores the resolved TARGET ENTITY
(dword) at `+0x19`, clears `HIBYTE(mbr_0x14)`, and pushes through the T-030 funnel 0x550720, returning 1.
Dependency is precise, not vague: the port has the push seam (`MapEntitySpawn.hpp`'s 0x550720 model, kinds
4/1 only) and the action struct (`src/F3/GameActionInitiateDialogue.hpp`) but has **no action-queue CLEAR** and
no dialogue kind wired into the seam's owner-linking. Porting it therefore means extending the seam, which is
map/spawn-lane work - named rather than done under a natives commit.

**5. Categorization finding - two batch-B rows are not script natives at all.** `CommandActionbarWindow`
(0x45ea50) is a `__usercall (a1@<ebp>)` with NO amx/params pair, reaching GUI state through
`global_area_map_icons[0]->int_0x2c + 160/+224` and toggling via `CLoadMapInterface::meth_0x495220(0|1)` +
`CActionbarInterface::meth_0x41c910` / `sub_416B20`. It (and `CommandInventoryWindow` 0x45eac0) belongs to the
**console-command family in `src/F3/Commands.cpp`**, not to `ScriptNatives.cpp` - the user-era inventory that
minted this batch mixed both families by address range. Recorded so the remaining rows get routed by kind
instead of by address, and so nobody ports a console command with an arity gate it does not have.

## Slice-B12 (WildKnight 01a1639b) — StartDialogue sized: the seam already has what it needs EXCEPT three things, and the action's own ctor reveals unaligned fields the port header approximates

Recon for the port showed the queue side is NOT the obstacle: `MapEntitySpawn.cpp` already has
`ClearActionQueue` (line 342) - so the flush `meth_0x54c440` performs is expressible today - and
`EnqueueEntityAction(entity, action, kind)` with per-kind owner policy (line 238-276). What actually stands
between batch B and `StartDialogue`:

1. **The action's real field layout.** `GameActionInitiateDialogue::ctor_0x404b40` writes:
   `gap5[8]=1` (+0xD), vftable `GameActionInitiateDialogue_Action_vftable_64815c`,
   `HIBYTE(mbr_0x14)=1` (+0x17), `BYTE2(mbr_0x14)=0` (+0x16), `*gap19 = 0` - an **unaligned DWORD at +0x19** -
   and `*(&mbr_0x1c + 2) = 0` - a second **unaligned dword at +0x1E**. The port header's `byte mbr_0x19` +
   `dword mbr_0x1c` are an approximation that cannot hold those two stores without aliasing them wrongly, and
   `StartDialogue` itself writes the target ENTITY pointer into the +0x19 dword (0x45a0e4) and then clears
   `HIBYTE(mbr_0x14)` (0x45a0e7) - i.e. the native touches precisely the fields the header misrepresents. Needs
   an explicit byte-array-plus-offset model (or a port-side named struct with the same extents), which is the
   action-TU lane's call, not a natives TU's.
2. **A dialogue kind in the seam**, with an owner-link policy decided (who is `this` for the pump: source entity
   at +0x14? the shipped uniform store 0x550737 says the enqueuer).
3. **The consumer.** The action's vft handler eventually opens the dialogue interface, and
   `GameDialogueInterface` is unported - so even a correct enqueue is a no-op end to end today.

Verdict: `StartDialogue` is blocked on an action-layout decision plus an era, not on a missing seam primitive -
worth writing down because my first guess (needs a queue flush) was WRONG, and `ClearActionQueue` was already
there at line 342. Filed rather than forced.

**Leverage note for choosing the next target, from this tranche's blockers.** The INVENTORY era is the cheapest
multi-row unblock in batch B: `GetWeaponRange` (0x45b620) needs `creature+0x168` -> `GameInventory`
+ `GameWeapon`; `IsItemEquipped` (0x45b750) needs the same inventory plus one AMX string helper (`sub_59AB20`,
0x45b7cc); and `EquipItem` (0x45a580) / `UnequipItem` (0x45a6f0) sit behind the same door. Four rows on one
container plus a string fetch, versus `StartDialogue` needing an action-layout decision AND the dialogue
interface, and `AddCNPCToParty` needing the party class. Recommendation: take the inventory era next, starting
with `sub_59AB20` because it is shared by every string-taking native in the batch, not just these two.

## Slice-B13 (WildKnight 01a1639b, 2026-10-22 ~15:2x) — the SPECIAL band lands, derived HP's off-by-one is shipped behavior, and two bug classes I caused myself

JadeBear's census routed `GetPrimaryAttribute` (0x45ac70) and `GetDerivedAttribute` (0x45ad00) to this lane as
"item-69 era" work. They turned out to need no layout reconstruction at all: the IDB already names the band, and
the bytes show it is a real contiguous array. Both are LIVE; suite 69 -> **92 asserts**, table 18 natives.

What shipped gave up:

- **The primary band is `int[]` at `GameCreature+0x760`, stride 4, post-cast, UNBOUNDED**
  (`mov ecx,[esi+8]` / `mov eax,[eax+ecx*4+760h]`, 0x45acef/0x45acf2). Index 99 is shipped behavior, not a
  crash guard - asserted only in range, with the missing guard documented rather than "fixed".
- **Charisma (`+0x76c`) is in the band but read by NO derived case** - a hole in the switch. The temptation to
  invent a charisma formula is exactly the invention the census note tells me to avoid.
- **EVASION reads `+0x7FC`, not the band** (`maybe_agility_0x7fc`), so the derived switch straddles two
  regions. IDA's hedged name kept verbatim; it is the only evidence of what the field is.
- **`HIT_POINTS` is not a field at all** - it re-enters for ACTION_POINTS, scales by a stored double
  `1/6000`, compares to zero, reciprocals, and converts via `__ftol2` (0x409b3c, truncation). Because
  `1/6000` is inexact, the product lands above `AP/6000`, the reciprocal lands below `6000/AP`, and every
  AP that divides 6000 evenly comes out **one LOW**: AP=20 -> 299, AP=30 -> 199, AP=12 -> 499, AP=1 -> 5999.
  Zero AP yields **1000**, the largest value in the range and off the curve entirely. My first assert said
  200 for AP=30 and FAILED - the suite caught me writing the formula instead of the engine.
  Consequence recorded in `docs/f3/CreatureSkillAndPerkArrays.md`: a `6000 / AP` implementation diverges at
  precisely the round numbers a test would choose, and the artifact is a falsifiable lockstep prediction
  (AP 20 shows 299) rather than a claim.

**Two self-inflicted bug classes, both found by the suite and worth the record:**

1. `(int)(memset( g_params, 0, 12 ), g_params[0]=8, g_params[1]=h, g_params[2]=i)` - I expected a params
   pointer and got **0**, because the comma operator yields the LAST assignment's value. That is a NULL params
   pointer into a `__cdecl` native and a page fault on read of `00000000`. Fixed by a named `makeParams2()`
   helper; the lesson is that a clever one-liner in a test is still a one-liner whose value you must name.
2. A test-helper insert that silently no-op'd twice because I anchored on text I remembered instead of text I
   checked (`//: Methods` vs the actual `// ---- attribute readers`), then a helper inserted *inside* a function
   body. Both caught by grepping the anchor count before trusting the edit - the same
   "assert-the-count-before-write" discipline that has been saving this lane all session.

Also added, as a named seam rather than behavior: `ScriptNativeGetLastError()`. The shipped sink is
`JE::LogDebugString` and a test linking `JE.cpp` cannot override a definition it links against, so byte-exact
assertions on shipped sentences go through a copy of the last formatted message. That is what makes the
**sibling-space quirk** enforceable: `GetPrimaryAttribute` prints `"Entity ID does not belong to a game
creature."` with a space and `GetDerivedAttribute` prints the same sentence as `"EntityID..."` without one -
JadeBear spotted it, T-022 eats it, and both are now asserted character-for-character instead of being
"unified" by the next person who finds them ugly.

Note the default case's OTHER behavior for the record: unknown derived index (>14) returns 0 AND logs
`"GameCreature::GetDerivedAttribute() - Unknown attribute requested: %d"` via `JE::LogDebugString_0x497120`.
That sink is unported, so the port keeps the value (0) and does NOT emit the line - named-deferred like the
packet-88 broadcast, and deliberately NOT routed through `ScriptNativeReportError`, because shipped does not
write the script error string here and conflating the two sinks would be an invention.

## Slice-B14 (WildKnight 01a1639b, 2026-10-22 ~15:5x) — the array trio is UNBLOCKED (the blocker was a framing error), 21 natives live, and my headline claim from B13 turned out to be an artifact of my own transcription

Two things happened in this slice, and the second is the more important.

**The trio landed.** `GetPerkRank` (0x45ab50), `GetBaseSkill` (0x45ad90), `GetTotalSkill` (0x45ae20) are LIVE -
suite 92 -> **119 asserts**, table **21 natives**. The B4-era note said these needed a layout decision from the
header owner because turning `mbr_0xe0`/`mbr_0x114` into arrays would shift later port offsets. True only for a
MID-LIST edit; B13's append-at-the-end shape sidesteps the whole question. Both band sizes came out of the code,
each confirmed twice: skills = 13 (`(0x114-0xE0)/4` and `cmp edx,0Ch` in the switch = cases 0..12), perks = 13
(the ctor's `memset(<mbr_0x114>, 0, 0x34)` at 0x40d277). Four facts bought along the way:

- **The bands are ADJACENT, which makes out-of-range skill indexes an ALIAS, not a fault.** `GetTotalSkill`
  calls the base-skill accessor BEFORE its switch (0x409837), so skill 13 reads perk rank 0 and returns it
  unchanged; skill 14 reads perk rank 1. Modelled as ONE 26-int array (`statBands_0xE0`, skills [0..12], perks
  [13..25]) precisely so that behavior stays defined and assertable instead of undefined and untestable - the
  two separate arrays I first wrote would have made a real shipped behavior invisible.
- **`GetPerkRank`'s resolve-failure line is a shipped COPY-PASTE**: it prints
  `"GetPosition() was unable to find the requested entity."` (0x45ab8e) while its own arity gate names it
  `GetPerkRank`. Preserved verbatim under T-022 and asserted, so nobody "fixes" it into a divergence.
- **The `Entity ID` / `EntityID` space splits the creature-native family 2-with / 3-without**: with space =
  `GetPrimaryAttribute`, `GetPerkRank`; without = `GetDerivedAttribute`, `GetBaseSkill`, `GetTotalSkill`.
  Five natives, one sentence, two spellings.
- **Charisma is used** - by skills (3x for barter/persuasion, 2x in deception), not by the derived switch. B13's
  "a hole in the switch" is a hole in ONE consumer, not an unused field. And claim 4 from the old parked list is
  settled: the firearms/melee arm is `mov esi,32h`, a REAL +50 immediate with no attribute load at all.

**Then my headline finding was disproved, by the method I had just written down.** B13 claimed derived HP comes
out one LOW at every exact divisor of 6000 (AP=30 -> 199), stated as a shipped law in a commit message, a doc
section, and a ping. JadeBear read the constant's BYTES from the image: the stored double at `0x64D590` is
`3F25D867C3ECE2A5` - the nearest double to `1/6000`, which sits BELOW it - while IDA's pseudocode prints
`0.0001666666666666667`, which parses one ulp ABOVE. I had typed the print. My table is reproduced exactly by
simulating the engine with IDA's printed decimal, which is why it looked like a discovered law: it was a
measurement of my own transcription, and the error's sign was inverted. I confirmed the bits myself, straight
from `$OPENVB_ORIG_EXE` (.rdata at file offset for VA 0x64D590), independent of both our IDA readings - and in
doing so hit the mapping trap myself: section headers carry RVAs, so `0x64D590` must be reduced by the image
base `0x400000`, and my first attempt "found" a section that returned `9.69e-312`.

Third pseudocode hazard in this file, and the nastest: **it survives testing**. 15 of 16 AP values agree between
the two constants; only the exact divisors of 6000 differ. A suite that probes interesting round numbers is
exactly the instrument that cannot see it. Constants are adjudicated against stored BITS, like offsets and
addresses - now written in the source comment, not just the doc.

**And the residual is genuinely unknown, which is the honest end-state.** With the right bits, the reciprocal is
`>= 6000/AP`, so the answer is `6000/AP` or one HIGHER - never one lower, which is what my retracted claim
asserted. But the LAST bit at exact divisors depends on intermediate width, and I measured that rather than
arguing it: **inside one binary, the shipped-shaped path returns 199 for AP=30 while `1.0/(30.0*k)` returns 200.**
Same compiler, same constant, different expression shape. So the suite now asserts the bound, this build's pin,
and the ulp relation - and hands the last bit to lockstep with a concrete experiment (agility 0 -> AP 20 -> read
HIT_POINTS: 300 means double-rounded intermediates, 299 means 80-bit kept, anything else means the constant was
read wrong after all). `docs/f3/CreatureSkillAndPerkArrays.md` carries the retraction in full, including the
chain of error, because a corrected claim without its path is only half the value.

Batch B after B14/B15: **21 natives LIVE**, 3 array rows retired from "blocked on layout", remaining blockers
concentrated in the inventory/party/dialogue eras plus the AMX string fetch trunk (`b8d91b9`).

## Slice-B16 (WildKnight 01a1639b) — the string seam and the first inventory native: 22 LIVE, 134 -> 133 asserts net, and the era built as a declared stand-in

`ScriptNativeFetchString` + `ScriptNativeRegisterStringParam` replace the three-leg AMX fetch with a registered C
string, preserving exactly the observables the legs produce: unknown cell fails (leg 1's error-5 path, 0x5ce36b),
`strlen >= bufSize` fails because 0x59ab40's comparison is **strict** - so a 13-character name is REJECTED by a
13-byte buffer, asserted both sides - and the destination is untouched on failure. Deliberately modelled as
having NO third failure mode: `amx_GetString` returns 0 on every path, so a "decode error" would be inventing
behavior shipped cannot exhibit.

`F3::ScriptInventory` (new header, model not reversal) carries what the matcher and the virtual prove: name
matching is case-insensitive (0x46a777 `_stricmp`), and equippedness is **membership in SIX slots at
`GameInventory+0x14`** - vft slot 10 at 0x46a860 walks `*(&this->mbr_0x14 + v3) == a2` under `while (v3 < 6)`
(0x46a8cc), which is where the number 6 comes from rather than from convention. Slot 6 stores nowhere, so an
over-cap equip is invisible exactly as it would be in the engine. Its `bIsGameInventory` flag exists solely to
keep the fifth shipped message ("... - Game Inventory not Found", the RTTI-attempt failure) reachable and
testable instead of dead code.

`IsItemEquipped` (0x45b750) is the first consumer: arity 2; five shipped messages verbatim; and the check ORDER
is asserted rather than assumed - resolve (0x45b79e), THEN fetch (0x45b7cc), THEN the creature cast (0x45b7fe) -
proven by feeding a non-creature an unfetchable name and watching it report the NAME, not the class.

Gate note, because the gate was right and I was not: `port-annot` failed my first run with one uncited
definition (`ScriptNativeRegisterStringParam@746`) - a stand-in seam still needs the shipped EA it stands in
for, and I had commented the semantics without the address. Fixed by citing 0x59ab20/0x5ce350/0x5ce3f0/0x5ce510;
`--gate` now PASS. Suite 134 -> 133 asserts net (three bound-style asserts collapsed into equality ones below),
`ScriptNatives=OK [133 >= 133]`, table 22 natives.

## Slice-B17 (WildKnight 01a1639b) — the last open question closed: derived HP is `floor(6000/AP)`, and the port's residual 199 was a MIXED precision chain

The B15 retraction left one thing open: the last bit at exact divisors. It is closed from bytes and a sweep, and
the answer makes the whole FP shape unnecessary.

Engine chain (0x409b15-0x409b3d, decoded): integer store of AP, `fild`, `fmul` by the stored constant, `fcom`
against zero, `jnp` to the 1000 branch, `fld 1.0`, `fdiv`, `call __ftol2`, `fstp`. **No store of a real between
FMUL and the call**, and `__ftol2` reads the register directly (`fistp qword`; its internal `fst DWORD` is a
float32 probe used by the correction chain, not a spill). With the constant's true bits BELOW 1/6000, the
product is exact and slightly under `AP/6000`, the reciprocal lands slightly above `6000/AP`, truncation yields
`floor(6000/AP)`; non-divisors are safe because their distance to an integer is at least `1/AP`. Swept AP 1..4000
against both a double-rounded and an extended-precision chain: **zero disagreements** (my first 80-bit emulator
reported 4000 mismatches - that was my emulator's significand handling being wrong, and it is recorded as mine,
not as evidence).

So the port divides in ints plus the zero branch, and lockstep grades a certain value now.

**The instructive part is the port's own defect.** Mirroring the formula in FP gave 199 at AP=30 while the engine
gives 200. Measured bit-for-bit in my build, sequenced as the standing rule requires: `k` = `3f25d867c3ece2a5`
(compiler's `1.0/6000.0` equals the image's constant - the input was right), the product stored as a double =
`3f747ae147ae147b`, which is ABOVE 0.005 - **the store inverted the below-constant direction** - and the
reciprocal rounded to double = `4069000000000000` = exactly 200.0. So 199 needed a MIXED chain: product rounded
to double (above the true value) then reciprocal consumed at extended precision (199.9999999999999958) and
truncated down. Neither pure width does it. My earlier dichotomy - "double-rounded means 300, 80-bit means 299" -
had both horns wrong, which is exactly why measuring beat choosing.

Why not `long double`: on MSVC `long double` IS `double`, so keeping it wide is not portable across the two
toolchains this file must build in, and the answer would depend on each compiler's spill decisions. Integer
division is deterministic in both, and because C++ truncates toward zero it also matches `__ftol2` for negative
AP - pinned by a new assert: `agility = -27` (AP = -7) gives **-857**, where a floor-based implementation would
give -858, an off-by-one in the OPPOSITE direction from the claim I retracted.

Tooling footnote worth keeping: my first attempt to run the bit-dump returned rc=53 with NO output at all, and it
looked like a crash. It was a missing `-static` - the loader failed before `main`. A silent nonzero exit with no
stdout is a linkage fact, not a behavior; run a hello-world in the same prefix before believing a fault story.

Arc of this one finding, for the register: asserted a law from a formula -> suite rejected it; asserted the
inverse law from a transcribed constant -> peer corrected the constant by reading bits; engine bytes then showed
the whole computation is an integer division; and the port's remaining error was a precision mixture that no
amount of formula-matching could expose. Every step was a source-of-truth question - the print, the bits, the
instruction sequence, the arithmetic.

## Slice-B18 (WildKnight 01a1639b, 2026-10-22 ~16:4x) — UnequipItem LIVE (23 natives, 151 asserts): one slot array, two case rules, a wildcard that lies, and an attribution correction I owe JadeBear

Ported `UnequipItem` (0x45a6f0) on top of the B16 seam and model. `EquipItem` (0x45a580) is **not** ported, and
the reason is named in the header and in `docs/f3/InventoryEquipPaths.md`: its payload `GameInventory::Equip`
(0x46be40, reached by `[edx+38h]` at 0x45a6ce) is an era - RB-tree bookkeeping, a style-slot remap over cases
2/3/7 with `Get_mbr_0x98` reads, `ApplyEquipEffects` gated on the server-context bit (0x46c1a1), a
`SetWeaponStance(0)` reset, and its own "already equipped trying to equip itself again" log. Unequip was
portable alone because the two share only the preamble.

What the native gave up, each preserved and asserted rather than corrected:

1. **Two comparison rules, one native.** The named-slot path uses `strcmp` (0x45a8e7); the slot-7 "everywhere"
   path goes through the `_stricmp` matcher (0x46a777). So `"HUNTING RIFLE"` can be reported equipped by
   `IsItemEquipped` and REFUSE to unequip from slot 0 - asserted as a pair so unifying them is a visible
   change, not a cleanup. And neither is prefix matching: my first draft queried `hat` against items named
   `Hat A`/`Hat B` and matched nothing - the suite caught an assumption I had not checked.
2. **`7` is a sentinel, not a slot.** 0x45a81d `if (v3 != 7)`; slot 7 clears every case-insensitive match. The
   equipped array is still SIX cells (0x46a8cc `while (i < 6)`), so slot 9 and slot -1 store nowhere and
   answer 0 - both asserted.
3. **The wildcard lies.** `"*"` is `byte_65DC00` - read from the original image's `.rdata` as `2A 00`, not
   inferred from a name - compared against the requested name at 0x45a932. Matching it clears the slot and
   falls through to `return 0`, while the exact-name branch returns 1. Unequipping by wildcard **succeeds and
   reports failure**; a port that returns 1 there is a divergence dressed as a fix.
4. **Three shipped copy-pastes**, now a documented family: `UnequipItem`'s arity gate is registered under
   `"EquipItem"` (0x45a72c, so wrong arity on Unequip complains about Equip); both item natives say
   `"... unable to find the attacking entity."` - Attack's vocabulary in inventory code; and `GetPerkRank`
   prints `GetPosition`'s message. All kept byte-identical under T-022.

Two slot numbers were adjudicated rather than trusted: the pseudocode rendered EquipItem's finder as
`+24` and Unequip's slot-15 call as `+15` in the same family of functions - one in bytes, one in cells.
Instruction text settles it: `[edx+18h]` = slot 6 and `[edx+3Ch]` = slot 15. Guessing from either rendering
would have mislabeled the table.

Suite work, because two of my own instruments were the bugs: the message capture went from last-only to
accumulating (the arity gate legitimately reports two messages in order, and last-only made the first
unassertable) - and the first version of that change **silently stopped capturing once the 1028-byte buffer
filled**, producing 10 downstream failures that looked like the natives had stopped reporting. It now restarts
on overflow: a capture that goes dead is indistinguishable from absent behavior. Separately, one leg reused a
handle registered against a block-local object that had already left scope - reading destroyed storage and
calling it "the class check" is noise; that leg now registers a fresh static item. And B18's first run had 8
reds traceable to exactly one missing line: 6i detaches the creature's inventory at its end, so 6j silently
tested "no inventory" everywhere - a lesson in test-block coupling worth keeping.

**Attribution correction I owe JadeBear:** their question is right and my message was wrong. The six-slot
equipped array is MY find from this sitting - I queried the `GameInventory` vftable, took entry 10 = `0x46a860`,
and decompiled it to read `while (v3 < 6)`. Calling it "your slot-10 find" was a crossed wire; their actual
outputs in that area were the `+0x168` accessor, the `vft+24`/`+56` Equip pair, and the `[party+5]` cell math,
and their `c6e6023` receipt confirms the six on the bytes regardless. The number stands on its own evidence,
which is the point of citing it at all.

## Slice-B19 (WildKnight 01a1639b) — GetWeaponRange LIVE on the same model (24 natives, 162 asserts), plus two named deviations and a dead method deleted

Ported `GetWeaponRange` (0x45b620) without needing a new era, because B16's model already carried the inventory.
Its chain: `mov eax,[esi+168h]` inventory (pre-cast, shared with `IsItemEquipped`), the one-instruction getter
`0x463b60` for `creature+0x98` as the SLOT, `0x4693d0` for the item there (RTTI'd to GameItem),
`mov eax,[eax+408h]` - a POINTER - else the creature's `+0x6F4` fallback (post-cast base), and only then
`mov eax,[eax+2Ch]` for the value. Three consequences encoded:

1. **The range is two dereferences deep**, so the suite puts 42 at index 11 and 999 at index 5 of the same
   record: reading the wrong offset is a different, wrong number rather than a lucky one.
2. **Two bases in one expression, by design** - `+0x168` is Entity-era (pre-cast), `+0x6F4` is GameCreature's
   own (post-cast). In the port both share an address, so this is STATED rather than asserted: a test that
   cannot distinguish the thing it claims to prove is worse than no test.
3. **"Range 0" and "no record" are the same answer**, like `IsLocked`'s unlocked-vs-wrong-class - asserted on
   both legs so nobody invents a distinguishing sentinel.

**Two deviations, named instead of smoothed over.** (a) When the slot holds a non-weapon, shipped dereferences
the FAILED RTTI result at `+0x408` with no null test (0x45b727) - a crash. The port answers 0. Reproducing the
crash is not worth copying; pretending the two agree would be a lie, so the case is explicit in the code.
(b) `meth_0x4693d0` reads `*(this+0x14 + slot*4)` with NO bound check (0x4693d4) even though the equippedness
scan is strictly six cells - shipped inspects whatever follows the array for slots 6/7/9. The port bounds it.
Chosen because an unbounded read of port-side neighbors is a less reproducible wrong answer than a declared one.

**JadeBear's unit-flip sharpening, verified at the bytes and adopted as a census rule.** This family's two
payload calls are DIFFERENT virtual slots rendered in DIFFERENT units: EquipItem's `[edx+38h]` (0x45a6ce,
`ff 52 38`) = slot 14, UnequipItem's `[edx+3Ch]` (0x45a918) = slot 15, while the pseudocode printed the first
as `+56` (bytes, truthful) and the second as `+15` (cells). Taking `+15` as bytes probes `[edx+0Fh]` = slot 3 -
mid-table nonsense that reads like a corrupted vftable. **Unit flips can move the function, not just misstate
it**, so vft citations quote call bytes and never the printed addend.

Housekeeping: the model gained `AddWeapon`/`pRangeRecord`/`bWeapon` and `ItemAtSlot`, and I deleted the
`EntryByName` helper I had added speculatively in the same sitting - it had no callers, and dead code in a
product header is a liability that compounds quietly. Removed before the gates ran so the evidence covers the
tree that ships. Suite 151 -> 162 asserts, 24 natives, floor raised; run-all 13/0 with port-annot OK,
`msvc-build` rc=0, cppcheck rc=0.


## Citation correction (same sitting) — my suite hashes were REAL and my peer's verdict was right for the wrong repository

JadeBear ran `git cat-file` on my relay line, found `f2b3a1c` a phantom (it is - typed from memory, self-caught),
and then also found `23418f1`, `2521b31`, and `f7f2d4c` "missing", concluding that batches are single commits
with the suite inline. The tree says otherwise, and I checked before conceding:

    f2b3a1c   parent=MISSING  openje=MISSING      <- true phantom, mine
    23418f1   parent=MISSING  openje=commit       <- real suite commit (B18)
    2521b31   parent=MISSING  openje=commit       <- real suite commit (B19)
    $ git show --stat 5a4b0a3 | grep openje
      modules/openje | 2 +-                        <- the parent bumps the gitlink

`modules/openje` is a gitlink (`160000 commit <sha>`), so the two databases resolve different hashes: a suite
commit is invisible to `git cat-file` in the parent and reads exactly like a fabrication. The correction is not
"drop the suite hashes" - that would erase real identity - it is **qualify the repository** (`openje:2521b31`)
and verify with `git -C modules/openje cat-file -t`. Filed as `docs/notes/commit-citation-protocol.md`, including
the part that is easy to get wrong under pressure: when a peer reports a phantom, check the second database
before amending, because accepting a wrong verdict turns a correct record into a false one.

The law is the one already on the books for offsets, constants, and slot numbers - re-emit from the artifact,
never from recall - and my own fifth violation of it today was a hash, which is a number someone else's tool
printed kindly.

## Slice-B20 (WildKnight 01a1639b) — EquipItem LIVE: 25 natives, 181 asserts, the case asymmetry completes as a three-way split, and the era was ported by declaring what it skips

`EquipItem` (0x45a580) landed on B16's model with the observable contract of `GameInventory::Equip` (0x46be40)
kept and its era named rather than imitated. The interesting part is what completing the family revealed:

**Three name comparisons, two rules, one gameplay consequence.** `IsItemEquipped` matches with `_stricmp`
(0x46a777); `EquipItem`'s finder - `JE::Inventory` vft slot 6 at 0x55fda0 - also walks names with `_stricmp`;
`UnequipItem`'s named-slot path uses plain `strcmp` (0x45a8e7). So a script can `EquipItem("SNIPER RIFLE", 1)`
successfully and then `UnequipItem("SNIPER RIFLE", 1)` and be REFUSED, leaving the item on the creature. Asserted
as a pair in one block, because "let's make these consistent" is exactly the edit that would silently change
how the game plays.

**The vftable had to be read from the image.** The IDB's `GameInventory` listing returns 10 of 16 entries and
omits precisely the slots this family dispatches through (6, 7, 12). Reading the dwords at `.rdata:0x666754`
gives all sixteen, confirms 14=0x46be40 / 15=0x46b350 / 10=0x46a860 independently of my byte adjudication
earlier today, and shows slots **8 and 9 are the same function** (0x469370) - an alias, not near-duplicates,
which matters before anyone "fixes" one half.

**Two shipped silences, now asserted AS silence.** A failed name lookup in all three natives prints nothing;
and re-equipping an already-equipped item fails at 0x46be8b with a `LogDebugString` line (0x46be97) on an
unported sink - so the port keeps the 0 and emits nothing, and the suite checks the error capture is EMPTY
after the refusal, not merely that the return is 0. A test that only checks the value cannot tell "silent" from
"reported something else".

**Three declared deviations** (all in the source comment with EAs, all in `docs/f3/InventoryEquipPaths.md`):
the port does not fold slots 2/3/7 through the creature's weapon style (0x46bff4-0x46c051), it overwrites a
taken cell instead of relocating the displaced item, and it performs none of `ApplyEquipEffects` (server-context
gated, 0x46c1a1), `SetWeaponStance(0)` (0x46c1e9) or `meth_0x469cf0` accounting (0x46c1bc). Equipping changes
slot state and nothing else. The rule I'm holding to: refuse with the same VALUE the engine refuses with, stay
silent where the engine is silent to the script, and never imply the skipped side effects are equivalent.

Suite work: the RTTI-failure model instances were hoisted to function scope after this being the second time a
block-scoped declaration broke a later block - and static storage duration matters here for a second reason,
since a registered entity must outlive the leg that registered it. `port-annot` was run BEFORE the gates this
time instead of failing them, which is the right ordering for a standard I keep meeting late.

Batch B after B20: **25 LIVE**; remaining rows need the party, dialogue, GUI-hide, AMX-object-reference and
server eras, or are documented stubs; the item/inventory family is now complete and internally consistent.


### Post-hoc correction to B20's vftable claim (same day, before it hardened)

B20 wrote that the IDB `GameInventory` listing "returns 10 of 16 entries and omits precisely the slots this
family dispatches through (6, 7, 12)". Re-querying and counting its keys programmatically: **11 entries**
(0..5, 10, 11, 13, 14, 15), `"length": 16`, so the missing set is **{6, 7, 8, 9, 12}**. Two distinct errors in
that one sentence, both worth naming because they are different classes:

1. **Arithmetic on my own enumeration** - I listed the keys correctly in the same paragraph and counted them
   wrong. JadeBear caught it; their framing is the keeper: "counting a list by hand that you just typed
   correctly."
2. **A claim that the truncation was aligned to my task** - "exactly the slots this family uses" implied the
   excerpt was missing 6/7/12 because I needed 6. The family uses 6, 10, 14, 15 and only 6 is absent; 8/9/12 are
   missing for reasons unrelated to this work. The listing does not know or care which slot I am looking for,
   and dressing a generic truncation as a targeted one makes the next reader distrust the wrong thing.

Upgraded detection rule, from their addendum and my re-check: the tool **self-reports** the truncation - the
JSON carries `"length": 16` beside an 11-key map - so compare entries-present against the declared length
before concluding an entry does not exist. An excerpt that knows its own size confesses in the payload; the
16 dwords at image `0x666754` remain the census when the answer matters.

Bonus from the same image read, now in the doc: it shows **two different failure modes in one table** -
slots 8/9 are the identical dword `0x469370` (alias), while 12/13 are adjacent-but-distinct `0x469400` /
`0x469410` (near-duplicate). The alias punishes solo edits; the near-duplicate punishes assumed equality.
Neither risk is visible in the IDB listing.

## Slice-B21 (WildKnight 01a1639b) — the party door: Protagonist + AddCNPCToParty LIVE, 27 natives, 196 asserts, and the one field's third era

Both rows landed on a single new stand-in (`src/F3/ScriptParty.hpp`, header-only, same shape as
`ScriptInventory`), and the shared access path is now byte-pinned in `docs/f3/PartyAccessors.md`: the
`0x70bd98` singleton → `call [edx+68h]` (vft slot 26, 0x45a9a5/0x45a343) → `Party -> GameParty` RTTI → leader at
`+0x14` (0x45a9c7) → **handle at `+0x1DC`** (0x45a9e0, bytes `8b 80 dc 01 00 00`). That last load closes the
loop on the band field: `Create` stamps it, `SetHP` resolves through it, `Protagonist` returns it - three
subsystems, no shared code.

Two findings that only exist because the pair was read together:

- **The message lies about its return.** Both failure lines end `"Returning INVALID_ID."` (0x45a9bb/0x45a9d4)
  and both return **0**; no sentinel ever reaches the script. Kept exactly and asserted as value AND byte-identical
  message tail, because either "fix" - return a real INVALID_ID, or reword the line - diverges in one direction.
- **A missing singleton is a crash, not a message.** Both sites set the adjustment to 0 when the global is null
  and dereference it unconditionally at the slot-26 call, so the party-failure string describes only the cast
  failure. The port answers that string for both states and says so - declared collapse #1. Collapse #2 is `Add`'s
  `if (!pLeader) pLeader = pEntity`, a usability convenience these two call sites do nothing to prove.

`AddCNPCToParty` also breaks the copy-paste streak: its resolve line says **"the entity"** (0x45a30e), not the
"attacking entity" its inventory siblings use - so the copy-paste family is real but not universal, and the
difference is asserted rather than assumed. Its payload is `GameParty` **vft slot 0** with the entity pushed
(0x45a370: `mov edx,[eax]; push entity; mov ecx,eax; call [edx]`) and the native returns that call's value, so
the model's add policy is directly script-visible: asserted as add → 1, duplicate → 0.

Test-shape errors of mine, both caught by the suite before commit and worth the record because they are the
quiet-observer family again: (1) I set `party.pLeader` on a local that was never made the ACTIVE party, so the
native answered "no party" and the assert was measuring my wiring instead of the code; (2) I fed an arity-1
native a TWO-cell block, so the gate fired first and the party message I was asserting could never have been
produced. A missing precondition and a wrong-shaped input both present as "the code disagrees with me", which is
exactly when a test is most convincing and least reliable.

Process, applied this time instead of learned again: `port-annot` was run BEFORE queueing the gates (it passed),
and both commit hashes below are re-emitted from git rather than typed from memory.


## Status correction for the whole batch (WildKnight 01a1639b) — the natives are reversed and tested, but unreachable from the running port

While scoping `StartDialogue` I found that nothing in the tree can call the 27 natives: `ScriptNativeGameCreatureTable`
(`src/F3/ScriptNatives.cpp:1172`) has no callers outside the suite; `JE::RegisterNativeScript` is called only by
a test (`modules/openje/tests/je/Scripting/Scripting.cpp:39`) despite its own header comment promising "F3 game
script installation"; `JE::Script::Run` has no product callers; and the port's native typedef is
`int(*)(Mover*, a2, a3, a4)` (`Scripting.hpp:18`) while shipped natives are `__cdecl(int amx, int params)`.
On top of that the port has two unrelated entity representations - `JE::Entity : DynamicObject` (what my
registry stores) and `JE::Mover`, from which the live `SpawnedEntity` descends and whose only lookup is by NAME
PREFIX (`MapEntitySpawn.hpp:397`), with no handle bridge.

Consequences written up in `docs/f3/NativeWiringGaps.md`, including the distinction I should have drawn earlier:
batch B's remaining rows are either **era-blocked** (behavior not fully read) or **wiring-blocked** (behavior
fully read, port cannot act on it - `StartDialogue`, `Attack`, `JustAttack`, `ReloadWeapon`, all of which touch
the action queue). And the honest status line for the committed work: verified reversal with enforced behavior,
**not runtime parity** - no commit in this batch changes what the running port does. That was already implied by
the stand-in-registry deviation, but implying is not stating, and my own summaries could have been read as
parity claims.

Deliberately not done here: a handle→`SpawnedEntity` bridge or an ABI adapter invented inside the natives TU.
It would make the split look solved while hiding which world is authoritative - the same error as silently
mapping shipped's null-deref to a friendly message, except with no declared collapse to point at. The installer
plus an executable script path is the cheapest step that turns this batch from a test consumer into observable
behavior, and it belongs to the openje/spawn lanes as a decision.


### Gap-5 refinement, after a peer proposed the bridge was already there

Claim checked: "`SpawnedEntity.netId` is the same dword the census closed on (`+0x1DC`), so the resolver is a
dozen lines." It is not established, and the shipped stamping sites say the port labels the field to a
*different slot*: `MapEntitySpawn.hpp:115` documents `netId` as **GameEntity mbr_0x28**, and 0x28 is written in
`GameEntity::ctor_0x4678a0` from **WSI vb SLOT 2** (`call [eax+8]` @0x4679b0 -> `mov [esi+28h],eax` @0x4679b3),
gated on the area-map-icon flag. `Entity+0x1DC` is written in the Create-era site 0x55190c from **SLOT 9**
(`call [eax+36]`), and mirrored into the `+0x168` object at `+4` (0x55192a) and map-icons`+228` (0x551a60).
`GetWorldServerInstance_0x401150` returns `global_0x70bd98 + 1` (0x4118a->0x40118a), so both come from ONE
server object - and the census of that object is now three slots: **2** (-> `+0x28`, what the port calls netId),
**9** (-> `+0x1DC`, the script handle: resolver compares 476, SetHP resolves through it, Protagonist returns it),
**26** (-> the `Party`, Protagonist/AddCNPC).

So a `netId`-keyed resolver keys on the slot-2 value, which nothing here proves equals the slot-9 handle. Two
possibilities, named rather than picked: (a) they are the same number at runtime - settled by one sample of an
entity's `+0x28` vs `+0x1DC`, or by reading both slot bodies (the DB has no vftable for the singleton's class:
`f3_vftable_index cls_0x616328` and `f3_address_lookup 0x70bd98` both return empty, so it is a runtime or
xref-walk question, not a listing question); or (b) they differ, in which case the port lacks the script-handle
field entirely and must populate it at spawn the way `Create` does before a natives-side lookup can be honest.

The tempting version of this finding was "the key is already there." It is the tempting version that gets
checked.


### The fork resolved to (b) without a run - and the correction landed in our own header

JadeBear settled `+0x28` vs `+0x1DC` statically; I verified it before editing: `f3_member_index GameEntity
mbr_0x28` returns exactly two usages, **0x4679b3 and 0x4679b8, both stores** (`mov [esi+28h],eax` on the
map-icon-flag branch, `mov [esi+28h],ebx` on the else branch), zero readers. A shipped field nobody reads is
not the script handle by consumer definition. `Entity+0x1DC` is read by the resolver, SetHP, and Protagonist,
with two mirrors. So `src/F3/MapEntitySpawn.hpp:115` now carries that correction, plus a fix to its own
`(vft[8])` note - byte 8 is **slot 2**, and that cells-vs-bytes ambiguity was living in OUR comment, not just in
the decompiler's output.

Precision applied outward for once: their "nine consumers" figure doesn't reproduce as one list - the DB gives 7
usages on `JE::Entity`'s own member and attributes the native/resolver sites to other class keys, so the count
depends on class attribution. What the decision rests on is the asymmetry (reads vs writes-only), which is
verified. Checking the claim that supports a conclusion you already agree with is the same discipline, one step
earlier.

Also credited to them, from inside the same negative result: the singleton's real class is **unattributed** in
the DB (`0x667dd4`/`0x6927bc` slot-26 candidates refute themselves against the observed Party behavior), and
`0x46dd70` turns out to be a packet-dispatch id-map containing the **server-side twin of StartDialogue** (case
0x49 constructs `GameActionInitiateDialogue` and funnels through 0x550720), weapon/reload packet cases 0x64/0x65
as siblings of SetHP's id 88, and weather-VFX cases 0x6A-0x70. Header comment only; no behavior change;
annot gate PASS; the header parses standalone.

---

# PAUSED 2026-10-22 (~17:3x) — resume point for this lane

Snapshot taken at the user's stop. Everything below is re-emitted from the tree, not from memory; verify any
hash or count with the listed command before acting on it.

## Where the lane stands

| item | value | how to confirm |
|---|---|---|
| batch B natives LIVE | **27**, table `s_gameCreatureNatives[27]` | `grep -n "s_gameCreatureNatives\[" src/F3/ScriptNatives.cpp` |
| suite | **196 asserts, 0 failures** | `run-all-tests` → `ScriptNatives=OK [196 >= 196]` |
| gates at pause | run-all **13 pass / 0 fail** (incl. port-annot), `msvc-build` rc=0, cppcheck rc=0 | rerun; no dirty source behind them |
| branch | `automated-reversing`, **ahead 60, UNPUSHED** (no push without a lane decision) | `git rev-list --count origin/automated-reversing..HEAD` |
| dirty files | only `tools/t052-drive.py` (another owner's - never stage) | `git status --short` |
| last commits | parent `d9d23db` (netId correction), `4db02ca` (two-slot census), `0432d9f` (wiring gaps), `0475e8f` → `openje:62b17c7` | `git log --oneline -6`; `git ls-tree HEAD modules/openje` |

## Status semantics - the line that must travel with this batch

**Verified reversal with enforced behavior, NOT runtime parity.** No batch-B commit changes what the running
port does: the native table has no installer, the script registry installer is test-only, `JE::Script::Run` has
no product callers, and the port's `NativeScriptFn` ABI differs from shipped `__cdecl(amx, params)`. All five
gaps, with line citations, are in `docs/f3/NativeWiringGaps.md`. "27 LIVE" means 27 reversed, ported, and tested
against a declared stand-in registry - read it that way, and say so in any relay.

## Resume candidates, in the order that yields most per unit of work

1. **Task trio** - `OpenTask` 0x45b900, `CloseTask` 0x45b940, `AddEntry` 0x45b980. Arity 2, delegate to unported
   `GameServer::meth_0x46ecd0/0x46ed30/0x46ed90`, and **return 1 unconditionally** even when the singleton is null
   (B8 finding). Three rows, no other-lane decision needed; journal write named-deferred.
2. **The handle field the port is missing** - `Entity+0x1DC` (slot 9 of the adjusted `global_0x70bd98` singleton)
   has no port-side counterpart; `SpawnedEntity.netId` models `GameEntity+0x28`, which is **write-only** as shipped
   (stores at 0x4679b3/0x4679b8, zero readers). This is a spawn/openje lane decision, not a natives edit - the
   correction is already in `src/F3/MapEntitySpawn.hpp:115` so nobody builds a resolver on the wrong field.
3. **Installer + ABI adapter + executable script path** - the four-part gap above; converting batch B from
   test-consumer to observable behavior. Cross-lane by construction. **No longer an invention problem:**
   `discoveries/2025-10-22-server-packet-dispatch-idmap.md` (parent:b0998fc, JadeBear) maps case `0x51` (81) as
   shipped's own script-by-name execution path - it resolves the entity's script with `GetScript_54aef0` and
   invokes a NAMED entrypoint through two vft slots, a predicate `[script+16](script,"GetInfo")` then a call
   `[script+24](script,"GetInfo",&out)` (string @0x667f58). So shipped already treats a script as a
   named-entrypoint object, predicate-then-invoke: whoever takes this decision has a reference implementation to
   mirror instead of a shape to devise, and the `NativeScriptFn` ABI question can be answered against that call
   form rather than against taste.
4. Era-blocked remainder: dialogue (`StartDialogue` needs the unaligned `+0x19/+0x1E` action dwords, a seam kind,
   and `GameDialogueInterface`), world server (attitude pair, task trio's journal), GUI hide (`HideInterfaces`
   0x45a100 → `sub_46EDF0`), AMX object-ref (`SetMoveMode` 0x45a140 → `sub_59AA50`), `EquipItem`'s skipped
   effects/stance era, two documented stubs (`PlayMusic`, `GetWeaponCombo`).

## Durable artifacts this lane produced (paths, not prose)

- `docs/f3/ScriptNatives`-family notes: `CreatureHpAccessors.md`, `CreatureHpWriters.md`,
  `CreatureSkillAndPerkArrays.md`, `AmxStringParamFetch.md`, `InventoryEquipPaths.md`, `PartyAccessors.md`,
  `GfxEntityOverlayApply.md`, `NativeWiringGaps.md`
- `docs/notes/commit-citation-protocol.md` - two-database citation rules + the 247-token sweep (7 non-citations,
  `f2b3a1c` deliberately left unresolved as evidence)
- `src/F3/ScriptNatives.{hpp,cpp}`, `src/F3/ScriptInventory.hpp`, `src/F3/ScriptParty.hpp`,
  `src/F3/GameCreature.hpp` (appended bands + inventory + weapon fields), suite in
  `modules/openje/tests/je/ScriptNatives/`
- `docs/notes/harness-items.md` items 67-69; shipped literals ledger entries for the space quirk,
  "attacking entity", the `GetPosition`/`EquipItem` copy-pastes, and `INVALID_ID`

## Peer lane

JadeBear (`01a163e9`) holds the docs/discovery lane and owes nothing here; they were about to write the
`0x46dd70` packet id-map as a discoveries file at my request. Cross-lane claims this session were all verified
against the artifact before landing in either direction - keep that habit, including for claims that support a
conclusion you already agree with.

## Standing rules this lane learned the hard way (each cost a real red or a wrong claim)

1. Re-emit from the artifact: offsets by instruction text, constants by stored bits, vft slots by call bytes,
   hashes by `git`, counts programmatically, sets by recomputation - never from memory or prose.
2. Owner-typed member access, always, in this tree's generated headers.
3. Observers must fail loud; declare what a test cannot distinguish instead of covering it.
4. Keep shipped defects verbatim (spaces, copy-pastes, lying messages, wildcard-that-returns-0) and assert them.
5. Declared collapse beats silent mapping; a port does not earn credit for reproducing a crash.
6. Check linkage before believing a runtime verdict - silence is not a sample.
7. A database listing is an excerpt: empty listing is not proof of absence, and `length` fields confess truncation.
