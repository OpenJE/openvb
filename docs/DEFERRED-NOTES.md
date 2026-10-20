# RE/Reimplementation — Deferred Notes

Things discovered this session that are intentionally NOT done yet, kept with evidence so they
don't need to be rediscovered. (Session: pathfinding RE, startup crash hunt, subsystem table,
command registry.)

## Pathfinding follow-ups
- `JE_PathSteerer_SmoothPath_0x561fa0` internals (string-pull funnel) — only its call site is known.
- `JE::Map` grid/octree fields used by pathing: `map+364` (grid), `map+492`, `map+820`
  (net-collision flag), `map+0x330` (blocking entity set via `JE_EntitySet_Contains_0x54aeb0`),
  `map+0x84` (octree cell dims). None modeled in `JE/Map.hpp` yet.
- `JE_PathJob` pool constants: 24-byte nodes × 10000 cells, element ctor `0x560490`; open heap
  `cls_0x562460` (ctor 0x562460, push 0x562550, pop 0x560e40, siftup 0x560aa0, siftdown 0x560890).
- Clearance threshold units {43,32,21,10} vs `GameCreature` size/radius — recheck when GCRE chunk
  RE lands (`van-buren-docs/Van-Buren-Chunk-Specification.md` has GCRE layout).
- The A* policy vft consumed by the steerer is `GamePathManager` slots [0..3]; steerer object is
  `World.mbr_0xf0` (cls_0x560c00). Its ctor (who installs the policy ptr) not traced yet.
- Door open semantics partial: `F3_GameDoor_IsOpen_0x40e5a0` uses `door+360` object vft[+20] with
  key anchor `door+196`, and CanTravel checks a door byte flag at `door[188]` (=GameDoor::ctor
  zeroes bytes 752/756 region — map the real names via GameDoor header next pass).

## Command registry / startup
- Red-black rebalance (`Meth_RbInsertAtBottom_0x59f200` fixup + rotations 0x583930/0x4C8E50)
  intentionally omitted in `cls_0x4d8d70.cpp`; add if any code path ever iterates by
  `_Tree::iterator` order or when adding erase-by-key (original erase: meth_0x59f0b0/0x59ece0).
- `JE::Entity::virt_meth_0x550750_550750` called from GameEntity ctor — unresolved side note:
  `*(&this->mbr_0x4 + *(this->mbr_0x4+4))` patterns = secondary-base vftptr fixups; layout of
  GameEntity's second base (0x1E8-byte `Entity` at gap9D+7?) still unmodeled.
- `ProcessMessagesAndUpdateTime` (0x56AF60): `PeekMessageA(..., 0, 0, 1, nRemoveMessage1)` —
  wRemoveMsg param modeled as uninitialized local in the original too; confirm PM_REMOVE
  semantics during lockstep.
- Console command execution path (`Meth_FindValue_0x5ace40` callers, message loop dispatch of
  typed commands) not yet RE'd.

## Subsystem slot table (docs/f3/StartupAndSubsystemTable.md)
- Slots 1/2/4/6 factory->class mapping VERIFIED by decompile: 0x45E110 Resources, 0x45E1C0
  Scripting, 0x45E170 Network, 0x45E2D0 GameAnimationCache.
- IMPORTANT: Network/Scripting objects are 4-byte {vftptr} engine classes patched to the
  F3::GameNetwork/GameScripting vftable by the factory (`mov [obj], vftable`) - game classes
  override dtor + a few slots; the generic loop's Tick slot is EMPTY (0x404B70 retn) for every
  subsystem EXCEPT JE::Network (0x543770 - the real per-frame netcode pump, must be reconstructed
  before multiplayer behavior works).
- Per-subsystem Initialize/Terminate/Tick slot targets recorded in JE/Subsystem.hpp. JE::Network
  now has a faithful skeleton (ctor field defaults; Initialize/Terminate/Tick wired and observed
  running at the correct cadence in the trace log). PENDING bodies: sub_5A2A10 (WSA init/refcount
  0x70C6C0), vft[6] 0x543AC0, vft[7] 0x5441F0, meth_0x543930, Tick tail sub_559B30/sub_5A34A0/
  sub_5A2B60, GameNetwork slots [4]0x45E040/[5]0x45DFD0; F3::GameScripting [4]0x46CD60; the
  MemorySystem pair Initialize(0x5B3DB0)/Terminate(0x5B3870); and World
  (JE::World::Initialize_0x546690/Terminate_0x5455b0; server tick is vft[16] 0x545D60 - verify
  which caller drives it, GameWorld vft[3] is empty!).
- ModelCache: original vft[1]/[2] are thunks into member subsystems (0x59c230/0x5b1510), NOT
  plain Subsystem overrides — revisit when ModelCache init is reconstructed (UNRESOLVED loop in
  ctor comment: decimal key "1","2","3"... probes into tree-A).
- `F3::SetupConfigFile` (0x56B220) and `F3::SetupLogFile`/`OpenLogFile` (0x56B100/0x498000) are
  commented out in WinMain and original main.cpp region; original DOES call them — reconstruct
  when display/config RE starts. Also `Display::CreateGameWindow` reads `.ini` parameters via
  `global_vfx_parameter_0x70c2cc` (VFX_Parameter tree) — big stub remains commented-out block.
- ShutdownGlobalLogStream (0x497B70) + `_GLOBAL__I_` static-init walker semantics partially noted
  in F3.cpp comments.

## Strings (SS) — DONE this session
- StringSystem reconstructed + TESTED against the real English.stf (3281 strings; ordinals; male
  fallback order active->male->extras; boundary cases). Files: modules/openje/src/JE/SS/
  StringSystem.{hpp,cpp}, tests/je/StringSystem.
- STF format note: directory entry = {u32 offset, u32 length, u32, u32}; ids are ORDINALS.
  van-buren-docs §2.1 said stringOffset at entry+0 len at +4 'unknown[8]' — consistent; the
  third/fourth dwords are still unknown (candidate: per-string flags / sound-bank refs).
- ftype map from shipping RHT (for the docs' §1.4 table): 100=mdl(3916) 200=bmp/tga(1653)
  1100=sound(113) 1200=(83) 1300=map(38) 1400=crt(108) 1500=veg(119) 1600=int(25)
  1700=rle(39) 1800=itm(5) 1900=(19) 2000=gls(3) 2100=sco/psf? FO3(1) 2200=F3 tokens(1)
  2300=MA1_PS2(1) 300/400/600/700/800=? (800 includes 'sys' ini configs; 700=448 — likely art).
- WIRING PENDING: who calls SS::EngineStringSystem::Init/ctor with which root dir — ctor is
  referenced from a static-init stub at 0x643565 (GLOBAL__I for the ESS global); Init call site
  not yet traced. EngineStringSystem::Format_0x56be30 + Skin LoadDefault decompile FAILED ->
  needs disasm pass (or larger decompile stack frame).
- JE::Strings subsystem (4-byte vft holder) vft[1..5] all empty thunks/retn — the real work is
  in the EngineStringSystem global, NOT the subsystem slot; subsystem exists for lifecycle order.

## Assets / formats (later)
- `dependencies/granny2/` (include/, lib/, gstate/, tutorials/) — needed when implementing .gr2
  model playback (`modules/patterns` has gr2.pat stub; B3D spec fully documented in
  van-buren-docs/File-Format-Specifications.md §4).
- Resource formats inventory in `van-buren-docs/Van-Buren-Chunk-Specification.md`:
  EEN2/GENT/GCRE/GITM/GOBJ (USE: DOR=1/CON=2 USEType), ftype table 100..3600 in
  File-Format-Specifications.md — use these against `JE::ResourceSystem` when loading real data.

## Resource layer (started — data-loaded milestone)
- `JE::ResourceSystem::Init` now parses the REAL shipping `resource.rht` (7044 entries verified
  by tests/je/ResourceSystem + live app run from staged data via `f3-reimpl-run-data`).
- DONE since: FindRht (linear, case-insensitive interim) + ReadResource end-to-end lump reads
  verified against the shipping data (Int_Cave1_Rock_Brown -> Tiles.grp lump 0, 49196 bytes,
  header {2,1,2979} — van-buren-docs GRP/RHT 'vMinor always 0' claims CORRECTED in place from
  the binary: both are 1); extension-stripped name resolution ("sys.ini" -> RHT "sys" in
  Engine.grp, cf. ResolveResourceName_0x499f10); JE::VFX_Parameter ini behavior layer
  (ParseIniText faithful to sub_4E0540 incl. destructive-trim quirks; GetInt=atol per
  sub_4DF770; case-insensitive sections+keys); Resources::ctor now loads+PARSES the real
  sys.ini at boot (trace-verified: ReadResource(sys.ini) -> ParseIniText(1088));
  F3::g_vfxParameterTree global + JE::Host::GetParameterTree wired.
- DONE since (latest): JE::Res (0x30) + MemStream (0x10) layouts CONFIRMED from machine code
  (Demand writes 0x49b959-6b: size+0x1C key+0x20 managed+0x24 count+0x28 data+0x2C; dtor/Release
  reads match; name SSO string +0x04/14/18; MemStreamOpen new(0x10) {res,cursor,end,start});
  Demand_0x49b750 / Release_0x498eb0 / MemStreamOpen_0x49beb0 / MemStreamRead_0x498170 /
  MemStreamStartMinusCursor_0x4981d0 (literal start-cursor) / MemStreamClose_0x49b1d0 all
  implemented and TESTED end-to-end on real data (KitsFirst_Aid_Kit -> _ITM.grp -> EEN2 tag via
  bounded stream reads; overrun guard verified). Common::Chunk::Read now matches the original
  1-arg signature (meth_0x4a1cc0).
- PENDING: hash-table sub-object (cls_0x50dbb0 @rs+0x108, meth_0x50cd40/meth_0x50bf30/
  meth_0x50c9e0) so lookups go through the engine structure instead of the file-static array;
  InsertResource_0x50bf30 + ScanGroupFolders_0x49bc60 + cache (cls_0x50caa0::meth_0x50cb10,
  cache_size 1000000); FileStreamOpen_0x49b9f0/Seek_0x498800/Read_0x499220 (+ .grp lump reads
  per van-buren-docs File-Format-Specifications §1); F3::LoadIniResource_0x4e0960 (sys.ini ->
  VFX_Parameter tree 0x70C2CC + parser sub_4E0540); JE::World::virt_meth_0x548840 (the function
  spanning 0x548930) expects g_resources + global_resource_system wired — verify per-map init.
- f3-reimpl-run-data.nix becomes a devenv script after the next `devenv up`/shell reload; until
  then stage manually (symlink demo root, run wine from it) — see script for the 4-line recipe.

## World clock + vft topology corrections (this round)
- IMPLEMENTED: JE::World::ApplyWorldClockParams (0x546B50) in modules/openje/src/JE/World.cpp,
  called from GameWorld::Initialize after the ini parses. Test asserts the canonical start date
  (2253-10-18 13:13:13, Seconds Per Round=6) read from the REAL sys.ini. World fields
  +0x14 Year +0x18 Month +0x1C Day +0x20 Hour +0x24 Minute +0x28 Second +0x40 SecPerRound.
- VFT TOPOLOGY CORRECTION: 0x691A20 is the World *secondary/mixin* lifecycle vft with its own
  numbering: [0]Initialize_546690 [1]Terminate_5455b0 [3]InitInterfaces_5460d0 [4]WorldClock_546b50
  [6..12] small accessors (0x545250..0x545490, ~60B each) [13]0x546FD0 [15]server-tick_545d60.
  GameWorld's table 0x6685A4 is offset by one (dtor [0]) and overrides [7..12] with the create-and-store
  bodies (PathManager/Party/Journal/Encyclopedia/StoreManager/WorldMap). Earlier doc slot numbers
  ([7]..[12]) refer to the GAMEWORLD table; engine-side callers index the mixin table - keep both.
- vft[5] at Initialize start (0x5466B0) is a no-op in engine (0x404b70); area-map branch -> +0x8C.
- IMPLEMENTED: ResourceExists_0x49abd0 (probe via merged index interim); World::ChangeState
  state machine (0x5454F0: {1,2}->3->4<->3, pause timing +0x58/5C/60, verbatim error strings);
  LoadStartupMap analysis done (base slot[13] 0x546FD0 = mixin GameWorld slot[14] via dtor-offset
  skew): ChangeState(3); map name = [Mainmenu]."map name" default "none.map"; ResourceExists ->
  subobject vft[12] call with name (cls_0x5a42b0 0x14-byte map-load REQUEST record at GameWorld
  slot[13], stored world+0x104 - request manager). Wire when map request flow implemented.
  GameWorld vft = 20 slots confirmed; cascade results: +0xF0..+0x104 incl cls_0x5a42b0 at +0x104.
- TODO: identify who calls LoadStartupMap (grep vft[14] callers / 0x546FD0 xrefs) - likely F3::Main
  first iteration or interface state machine
  sub_546870 (0x2D9 - called by? trace xrefs), small accessors 0x545250..0x545490 (name them once
  consumers show up).

## Entity factory + proto chain (fresh, not yet implemented)
- IMPLEMENTED SINCE: F3::GameWorld::Initialize vft[1] subset live in the reimpl - parses the
  REAL globals.ini (19638 B) + locals.ini from Scripts.grp into the world parameter tree during
  boot (trace-verified, zero faults; test asserts [Globals] Greeting=0, SKILLDIFFTRIVIAL=10,
  atol-with-trailing-comment behavior). Full vft cascade [4..13] + calendar + area-map still below.
- F3::CreateGameEntityByClass_0x46f530 = the entity class factory (GameWorld SECONDARY vft at
  0x6684ac... slot TBD). Class enum -> size/ctor table (all captured here):
    0 GameEntity 0x47C/4678A0   1 GameCreature 0xBDC/40D010   2 GameUsableObject 0x498/412420
    3 GameDoor 0x4C8/40E7F0     4 GameContainer 0x4D8/408C20   5 GameItem 0x78C/410180
    6 GameWeapon 0x808/414510   7 GameArmor 0x7A8/408580       8 GameProjectile 0x494/411C60
    9 GameAmmo 0x7B0/407FB0    10 GamePlayer 0xBFC/411820      >10 -> JE::World::CreateEntity
  Factory return = (obj + *(obj[1]+4) + 4) -> secondary-base adjustment thunk.
- This class enum is the same numbering as the CheckEntityClearance (EntityGate) mover-type at
  GameEntity+0x18 {<=3 big props/doors, 4 container, 5 item, >=6 weapon..player} - units TBD.
- JE::World::Initialize_0x546690 sits at GameWorld vft[1] (0x6685A8) AND base JE::World vft
  0x691A20[1] - the generic subsystem Initialize for slot 7 IS the world init; its body is the
  map/proto load driver (NOT yet decompiled) and is the next thread: find World::Initialize body,
  the CreateEntity vft callers, and the prototype table.
- GameAmmo::ReadFromFileBody_0x408680 pattern: 6x MemStreamRead(dword, this+0x3B0) - template for
  all ReadFromFile field readers once the chunk driver exists.

## Map chunk verification (this round)
- JE::Map (Map.hpp/Map.cpp) implemented: LoadFromFile loop (0x574F60) + dispatcher (0x56C770) + ReadEMAP (v5: flag + 6x3 u32 + 6 u32 = 97 of 98B body; semantics unknown).
- Framing VERIFIED on Default_StartMap (Engine.grp lump17 263B): EMAP(110) EMEP(109) ECAM(28) EMNP(16); size INCLUDES 12B header; SkipToEnd = seek(consumedAtChunkStart + size). mbr_0x1c captured BEFORE header reads.
- van-buren-docs EMAPc C# table describes a different writer (version slot = il flag; strings layout) - NOT authoritative for v5 binary; engine reader is.
- GameWorld::Initialize cascade live: pathManager/party/journal/encyclopedia/storeManager + NULL mapRequestMgr (+0x104, cls_0x5a42b0 unmodeled). Fixed pre-existing openje header syntax bugs (Encyclopedia virt_meth_0x58jee0 missing type).
- Next map work: EMEP/EMEN readers etc; EMEN_0x575A40 is the entity-instantiation driver (huge flat field reader) - biggest fish.
- CHUNK STRING PRIMITIVE: JE::ResourceSystem::StreamReadString_0x49C8D0 = {u16 length, bytes}
  NUL-terminated assign into the SSO string (chunk-spec doc's "1 or 4 byte length" claim is wrong
  for this path - it is u16 here).
- EMEN record field order extracted (per sub-record): u32,u32,u32, 4x u8 flags, then ~35 u32
  fields, plus name/filename string pair via StreamReadString, with "Water%d" fallback naming and
  tile records {u32,u32} pair + 4 floats (+-0.00390625 = 1/256 unit inset) inserted into
  container this+0x64-ish (cls_0x574e20::meth_0x5725d0). Water-tile mesh branch confirmed.
- EMEP body is numeric (first u16 read as string length = garbage 46081) - fixed-size records of
  floats (coords); decode when entry-point consumers are implemented. Default_StartMap contains
  EMEP+ECAM+EMNP with 2 float-ish dwords each trailing; full semantic decode queued.

## Tooling & conventions
- Naming convention (per user): IDB and implementation use the same MSVC-2002-style identifiers:
  `Namespace::Class::Name_0xADDR` in the IDB (hex suffix always), `Name` matching in code with the
  hex address in a `// 0xADDR` comment above each definition. Renamed ~100 symbols this way
  (pathfinding, steerer/heap/nodepool, registry tree, subsystem vfts, F3::Startup/
  GameStateLoop/Shutdown/RegisterCommand, JE::String assign/copy/reserve, thunks 0x59C230/
  0x5B1510, empty virt 0x404B70). Keep NEW renames consistent with this scheme.
- Remaining IDB data renames not applied (names differ there): MemorySystem/Resources vftables
  (already fully documented in Subsystem.hpp comments).
- Can add packages to `devenv` (user confirmed): candidates when needed — radiff2/r2 for
  function diffing original vs reimpl, xdelta3, capstone for scripted disasm passes.
- `dependencies/granny2/` available (include/, lib/, gstate/, tutorials/) for .gr2 work.
- `mcp__ida_original_f3__set_comments` is broken in this pi harness (arrays arrive stringified);
  persist comments via renames + docs, or fix the MCP bridge when convenient.
- Unit tests convention: `modules/openje/tests/je/<Class>/<Class>.cpp`, compile
  `i686-w64-mingw32-g++ -std=c++98 -static`, run with wine +
  `.devenv/state/f3-reimpl-wineprefix` prefix (wine at
  /nix/store/b0d6l5vh4fdawmvq5j504a47pcxfsk8i-wine-11.0/bin/wine).

## Verification status at time of writing
- msvc-build + msvc-build-tracing: clean; f3-reimpl-run(±trace) runs until timeout without faults
- cppcheck-openvb/openje: clean for touched files
- tests: PathManager OK, cls_0x4d8d70 registry OK (all under wine)
