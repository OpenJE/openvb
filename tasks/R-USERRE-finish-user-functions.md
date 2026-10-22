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
