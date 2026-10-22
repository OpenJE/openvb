---
status: in-progress
assignee: BrightYak `01a16198-7b63-74d2-a895-a5fc6a051707` (R-USERRE-A sealed by coordinator QuickNova/LoudDragon 2026-10-22 03:2x; fresh claim, no inherited identity)
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
