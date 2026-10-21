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

---
## RESTORED SECTION (2026-10-20): findings that silently failed to append earlier
Process note: several `## Tools:`-anchored appends were no-ops after the anchor drifted; the
content below was "banked" in commits since but never actually written. Verified-by-code
counterparts exist; this restores the living-backlog record. (Also: GamePathManager.md append
used the same failure mode - re-appending its two updates here rather than re-editing.)

### Resource resolution + maps
- FindRht is NAME+FTYPE aware (ini=800 map=1300 crt=1400 veg=1500 int=1600 8=400 rle=1700
  itm=1800 dds=200 ...); duplicate basenames across ftypes exist in shipping data
  ("00_03_Tutorial_Junktown": map 1300 in Maps + blob 1200 in Scripts); last match per key
  wins (InsertResource_0x50BF30 insert-assign).
- Octree: Map+0x2C ".8" string (object; buffer alias +0x30), lazy GetOctree_0x56D960;
  LoadOctreeFromFile_0x5b0ad0 = fourcc stream 8TRE>HEAD{4xu32}>LGTD/LVLD/OSRF/OSUR/OTEP>
  TREE>NODE>INFO. OCTREE_ 0x100 bytes, primary vft 0x6A86F4, GetBounds_0x5adb10 ->
  octree+0x6C..0x80 {minXYZ,maxXYZ}. Default_StartMap: HEAD(2,14,2,3), AABB {-1,-1,-1} to
  {71,1,51}. CellToWorld_0x560710: x lerp col/cols(minX..maxX); z anchored on bounds[5]=maxZ
  (NOT the local_18[4] IDA alias showed); y=0. WorldToOctreeCell_0x560680 inverse; ProcessJobs
  seeds cells from world vectors when job->map set.
- .rle = walkability grid: header {u32 cols,u32 rows} + per-row runs {u32 count,u8 mask};
  Default_StartMap 144x104/306 runs/1538B exact; masks 0x2F floor / 0x02 blocked;
  Port: Map::LoadRleGrid (flattened mask array; RB-run-tree original documented).

### EMAP routing correction (superseded the "numeric 97B EMAP" claim)
- Dispatcher table (vft idx->EA): 15 EMAP 0x56C9D0 | 16 EMEP 0x573B00 | 17 EMTR 0x575050 |
  18 EMEN 0x573BF0 (obsolete spawner) | 19 EME2 0x573F30 (REAL entity records) | 20 2MWT
  0x575A40 | 21 ECAM 0x56CCF0 | 22 EMFG 0x56CD70 | 23 EMNO 0x5742B0 | 24 EMNP 0x575180 |
  25 EPTH 0x576940 | 26 EMSD 0x56C330? NO: 0x576C30 | 27 EMEF 0x578FB0.
- EMAP v5 = u32 flags + 4 {u16,chars} strings (.8/.rle/_AM.dds/empty) + version-gated byte
  extras incl. 3 float extras (250.0/500.0/{0,1}.0; ctor defaults 16384/524288/1). EMEP body
  97B numeric IS entry points {u8 id, 6x{x,y,z} ring, 6 tail}. EME2 record = {u16 entity
  resource name, pos3, rot3 rad->deg, u8 flag, ONE nested property chunk (EEOV seen)};
  original instantiates via world-server vb-table (+52 name lookup, +24 create) + pushes map
  entity RB at map+0x134 (count mbr_0x13c = "Map %s contains %d entities").

### ActionMoveTo (ported, then adopted into game flow)
- Field map (+0x11 mover, +0x1C job, +0x20 target, +0x24..2C snapshot, +0x33 requested,
  +0x34 arrive-dist, +0x3A flag, +0x3D lastMs, +0x44 lastReqMs) implemented as named struct.
- Bugs fixed en route: TraceSegmentClear goal-x FLOAT BIT PATTERN passed as int overflowed
  distance -> every long segment looked clear (fast path swallowed A*); DistanceDispatch
  param_3 must be node context (cell +16/+20, grid +80) - NULL deref crash; waypoint pop
  accounting (advance-to-head before pop).
- Game layer: F3::SpawnMapEntities (records->SpawnedEntity, 29-bit net ids
  (x&0x1FFFFFFF, IWorldServer 0x66856C +8 read), GENT stats via ParseGentBlock (shipped
  templates ver=1; writer emits 3; >=1 gate covers both; cur<=max clamp 0x463910),
  classIdTag ~ Entity+0x15C, ext resolver 0x46FB20 adopted (player=creature+flag, .pro=8);
  chunk-signature resolver authoritative (GCRE/GITM[+GIWP|GIAR|GIAM]/GOBJ[+GDOR|GCON]).
- MapCreate registry (1-based ids into s_loadedMaps[17]); entities carry registry mapId
  (entity+0x1EC semantics) + push into Map flat list (map+0x134 port). Managed Demand cache
  (1,000,000B cap, stamp eviction, demandCount bump on hit; linear-table deviation noted).

### Pumps and schedulers
- JE::World::ServerUpdate_0x545d60: GetPathManager seam -> ProcessJobs (0x545E18) +
  Host::TickWorldEntities (entity loop 0x545E71). NO static caller exists in original .text
  (exhaustive disp8/32 vft-call scans); subsystem Ticks all empty stub 0x404B70 except
  Network::Tick_0x543770 (net server/client vft[6]/[7] + housekeeping). Session pump belongs
  to the unmapped session layer.
- Entity action list (kind move/wait), PushActionOntoEntity (0x550720), finished-latch slot
  drop; F3::GameActionMoveTo::OnServerUpdate_0x4060b0 skeleton (faithful NULL-grid early
  return at 0x4060D9) + full blocker-response semantics in its header (classTag switch:
  moving creature -> Wait(1000)+fresh MoveTo via world-server factory vft+20 (2=MoveTo,
  5=Wait); door -> segment-plane test -> OperateDoor+Wait+MoveTo).
- Anim-event roadmap (discoveries session 8): QueueAnimEvent ids (1 Idle, 32 Walk, 7 ToCombat,
  9 ToDefault, 21/22 Equip/Unequip, 29 Limp, 43..52 locking, 53..104 attack/skill);
  Queue_Idle = (animState x weaponStance) -> clip index; scheduler sub_5505F0 returns
  duration-ms; queue at Entity+0x1A8, next-link dword_6F3960, GE+0x9C busy-lock.

### Spatial grid thread (IN PROGRESS - this is where work resumes)
- CreateGrid_0x57a8a0 (map+0x88, 0x18B): per-row 0x14 RB nodes {sentinel,count,x=cols};
  rows from .rle header; per-cell run nodes from LoadRleGrid_0x57AA00 keyed on mask byte
  (sub_56E390 insert). Consumers: NodeContext/job+80 grid pointer; Entity+0x16C per-entity
  occupancy link; QueryEntities_0x405F80(grid, results, cellX, cellY, ...) blocker set.
- NEXT: decompile QueryEntities_0x405F80 + caller of CreateGrid (confirm map+0x88 == grid);
  port grid build into LoadRleGrid; entity insert/remove hooks; then OnServerUpdate blocker
  bodies + real TraceSegmentClear cell-walk (per-cell callback = entity vft+38 per EME2
  recon notes).

### Coordination (live)
- implementer-B on tasks/chat board (claims-by-commit; lanes: B=tooling/tests/GUI recon).
- Discovery agent delivers to discoveries/ (we commit on their behalf); responses file shared.

### Grid milestone update (verified append)
- JE::SpatialGrid ported + wired: Map owns (LoadRleGrid builds), RegisterOccupancy per entity
  at spawn + pump tick; F3::GamePathManager::TraceSegmentClear now samples occupancy at the
  shipped 1.666 stride (verified semantics: runs are ABSOLUTE columns - the original's
  p5+run.x-p3 with equal anchors is identity; 1-wide footprints CAN fall between samples,
  faithful approximate LOS; A* chain took over when a blocker landed on a sample (3 wp)).
- Active-map seam: F3::SetActiveMap/GetActiveMap (single-map session slice; original resolves
  per-mover via +492).
- NEXT: OnServerUpdate_0x4060b0 blocker-response bodies (classTag rules incl. door open-byte
  when GameDoor lands), DistanceDispatch penalty path, and per-entity multi-cell footprints
  from model bbox (ModelInstance::meth_0x563a40 chain).


### Waypoint ORDER correction (verified this round, supersedes "[goal, start]" notes)
- sub_560900 is a FRONT-insert: the fast path pushes goal then start -> LIST = [start, goal];
  consumption pops head, completion is count==0 => GOAL MUST BE LAST. First port had
  append-push order, making movers reverse mid-walk; fixed in ProcessJobs (chain emitted
  reversed, fallback [start, goal]) + all order asserts updated ("goal-last").
- FIFO action pump: ONE active action per entity (head slot), matching original where a
  Wait gates the MoveTo behind it.
- Step-aside verified: mover cell shared with MOVING creature -> Wait(1000)+fresh MoveTo
  (job handover, speed copied), mover still reaches goal (test: 119.5,300 vs goal 120,300).

### Entity lifecycle state machine adopted (sessions 11/12)
- SpawnedEntity: entityState (Entity+0x1E0 semantics; birth=5 via the single-caller
  0x550750(entity,5) ctor idiom), pumpEnabled (+0x3A0 gate), worldLinked (+0x1BC DEAD gate).
- TickWorldEntities now runs the session-11 pump switch: INVALID logs+skips (original throws -
  deviation), CREATED/CREATED-ALT advance to ACTIVE without also pumping, DYING/REMOVED are
  no-ops pending OnDeath (server-only 6->7 flip, session 12 §2) and the 0x550ad0 tail, DEAD
  pumps only while linked. Birth state 5 pumps normally => all suites unchanged + green.

### Door engine-anim countdown (ge94 §2 adopted)
- OperateDoor now ISSUES the anim window (E+0x94-style integer countdown, -1000ms fallback
  until GetDuration_4a26d0 models exist) and completes; the DOOR's pump flips doorByte0xBC
  when the countdown reaches 0 (GE-side completion). Test pumps door + mover independently.
- Countdown guard (animCountdown94==0) prevents double-issue while animating.

### Script layer plumbing (this round)
- Entity script region mapped (0x54AEF0/0x54AFE0): name SSO @E+0x2A4 area, cache +
  sticky-fail latch; loader = IWorldServer vft+36 -> JE::World::0x544b80 -> Script_Small
  (0xE0, vft 0x6A539C). Runner 0x5A5A90 exposes the compiled-VM API behind mbr_0x26:
  sub_5CCCC0/5CCD00 ('MPS?' fourcc state), 5CCA80 (fn lookup), 5CE310 (invoke),
  5CCE50 (interrupt; 0/12 = success codes), 'OnUpdate'->'MEVE' special case + 580770.
- Ported as name-keyed NATIVE registry (VM deliberately unported; behaviors get native
  re-impl): RegisterNativeScript/LoadScriptByName/EntityGetScript/EntityRunScript with
  original flow (server gate logs vs throws - deviation; sticky fail latch; unknown => 0).
  Host::IsServerContext() added. New Scripting test suite green (suite total now 7 dirs).

### Frame pump chain landed (session 14 parity)
- World::Update_0x544a90 (pause skip on mbr_0x2c==4, ms accumulate mbr_0x44 w/ pause
  credit mbr_0x54, vft3[+0x14] gate seam WorldUpdateGate, dispatch mbr_0x34: 0 log(deviate
  vs FatalError), 1 -> ServerUpdate_0x545d60(dt) per-entity pump, 2/3 stage virtuals;
  mbr_0x64 elapsed + mbr_0x68 frame counter) and World::Tick_0x544b30 (same accumulate +
  PlayersPump_0x545ff0 virtual seam) ported; World zero-ctor added (calendar defaults 6s).
- Pump ordering fix per session-14 §2: entities born SPAWNED(5) are NOT pumped;
  SpawnMapEntities flips to ACTIVE(4) at add-to-world (RegisterOccupancy site).
- App wiring: F3::GameNetwork::Tick -> world->Update_0x544a90(TickMilliseconds()).
  Parity note: original reaches Update through the LOCAL Server object's pump
  (JE::Server::0x552b60 socket frame); no server-connection object modeled yet, so
  the network subsystem tick stands in (documented). GameWorld::Initialize sets
  mbr_0x34=1 (server update path live).
- Gregorian clock PORTED (0x544950 ctor shape, 0x69166C vft ladder): AddSeconds/Minutes/Hours/
  Days exact-op-order incl. the shipped boundary quirk (nested unit before room-add => a
  boundary tick advances +0x34 by whole-unit spans, 23:59:59+1s = +90061). DaysInMonth table
  @0x6FF4F3 1-based; leap = (y&3)==0&&(y%100||!(y%400)) + Feb+1 in AddDays caller. World pump
  step-6 no-players carry wired (WorldClockSpeed virtual, mbr_0x44 %= 1000); players-online
  branch waits for player collection; "Done" idle heartbeat (sub_5567F0) still future.
  Open: semantics consumers of Gregorian idx3 Get_mbr_0x34 (0x41D1D0) - filed to discovery.

### World clock consumers (this round, ported as fields only)
- World+0x70 = double worldSimSeconds (ctor seed 1.0; IWorldServer slot0 getter 0x46F250,
  slot2 setter 0x46F260). Combat manager cls_0x5a9330 (World+0x8C) RE'd structurally:
  initiative turn queue; acc_ms += 1000*worldSimSeconds; entity turn deadline +0x29C;
  per-frame latch +0x319; pruning prunes state-5 entries via vft+96; ServerUpdate(dt+acc)
  per turn entity. CombatManager port NOT started (queue item; discovery offered field
  semantics help - see C-22).
- JE::World::ctor_0x548a80 identity fixed (was mislabeled cls_0x4d8d70); global 0x70BD98 =
  the JE::World object ("world server instance" naming clarified).

### Sim-seconds thread: closed with corrections (this round)
- The combat time source (IWorldServer slot 0, 0x436240) = Get_mbr_0x3c -> the GREGORIAN
  CLOCK POINTER, not a seconds getter. Combat's `1000*slot0()+acc` therefore reads the clock
  OBJECT address as int?? - IDA decompiler artifact: the real call likely goes through the
  Gregorian vft seconds-getter; REQUIRES runtime/lockstep confirmation before I model
  turn cadence from it. FLAGGED to discovery (C-23).
- IWorldServer+0x6C (SetWorldSimSeconds caller) scan: only hit was a false positive inside
  the YearsPassed SCRIPT NATIVE (0x5C4A3B: registration probe sub_59AF40, 'myself' fetch via
  sub_5CCCC0 fourcc 0x466C6C79? ('myself' family), error strings verbatim, entity+0x29C
  (turn deadline) read, calendar v9 from World-vb idx29 (+116) + vft+108/12/88 - this is
  GOLD for the ending/script system; entity+668 confirmed = turn-deadline read by scripts.
- Sim-seconds setter callers: no static direct or near-WSI call sites found (87 [reg+0x68]
  sites are cross-vft noise). Likely advanced through GameWorld-level pacing not yet mapped;
  parked. My pump's carry remains the authoritative world-time advance for the reimpl.

### Pump-gate semantics (this round, shipped-faithful)
- record.flag -> Entity+0x3A0 pumpEnabled wired (session 22). Demo data: ALL shipped EME2
  flags are 1 (mainmenu.map pc/armstrong/boxcar) -> everyone pumps; the "NPCs frozen
  offline" hypothesis is FALSE for this slice (C-25). TickWorldEntities gate reads
  pumpEnabled; tests that hand-drive TickWorldForEntity bypass it (as in original where
  scripts call entity updates directly).
- YearsPassed (0x5C4A3B) = PAWN NATIVE, not clock driver: 'myself' fourcc fetch, entity
  +0x29C turn-deadline read, calendar via World-vb+116 + object vft+12/+88/+108. Second
  confirmed native after the script-abi session; native-registry stand-in unchanged.

### Tooling traps hit (document so nobody re-hits)
- MSVC 2002 decorates struct vs class (U/V) in names: forward decls MUST match keyword
  (class PathJob; vs struct PathJob => LNK2019 with only-truncated names in console).
- grep -c prints 0 but EXITS 1: never `&&` anything behind it.
- LNK lines are not 'error C...': build greps must include LNK/MSB patterns.
- cmake objs live in obj/ at repo ROOT (build/ is exe output).
- /tmp/F3*.exe copies are private & must be rm'd before cp (stale copy masqueraded as pass).
- **Include-order-dependent headers are invisible to the mingw host suites (T-024, implementer-B,
  2026-10-20).** An inline accessor in `src/F3/GameWorld.hpp` referencing `JE::Gregorian` — which
  `World.hpp` only forward-declares — **compiled clean under mingw and failed MSVC with C2027**
  ("illegal use of non-existent structure or class"). Each mingw TU happened to include the real
  definition first, so the header's own contract was satisfied by luck of include order. Consequence:
  **`msvc-build` is not optional polish for header changes** — a green host gate says nothing about
  whether a header compiles standalone. Check: if you edited a `.hpp`, run `msvc-build`, not just
  `run-all-tests`.
- **A self-checking gate cannot detect its own staleness (T-024, 2026-10-20).** The stale-table guard
  lives *inside* `run-all-tests`, so the running payload — baked into `/nix/store` at shell build time
  — contained **zero occurrences of the guard** the tree file has. It then reported `CombatMath`
  BUILD-FAIL (undefined reference to `JE::LogDebugString`) purely because ITS rows predated the tree's,
  i.e. a stale payload produced a false RED as well as hiding false GREENs. Fix: `tools/gate-integrity.sh`
  checks from outside (one grep, no execution): table identical + stamp block present, exit 2/3/4. Any
  quoted gate number must come from a run preceded by that check returning 0, and must carry the
  `GATE STAMP` lines (HEAD, openje, payload path, table hash). A run that prints no stamp is by
  definition not the tree's gate.


## Shipped-defect ledger (candidates found by statics; reimpl mirrors, never "fixes")

Three findings where the shipped binary is wrong, incomplete, or unreachable-by-data. The rule for
all of them: **mirror the shipped behaviour and cite the site.** A reimpl that silently improves the
original is a different game, and the divergence reads as our bug.

1. **Null-page addend, unarmed damage.** `GameCreature::CalculateUnarmedDamage_0x409f90`
   @0x40a045 = `03 35 24 00 00 00` → `add esi, ds:0x24`. `MEMORY[0x24]` is literal, VA 0x24 is the
   null page, so the branch **faults on execution** whenever its guard holds
   (`param_5 != null AND mode+0x18 ∈ {1,3} AND weapon+0x3B0 ∈ {9,10}`). Demo data never satisfies
   the guard. Permanently unported; do not "recover" a base register. (canonical §11; C-40.)
2. **Armor in requirement slots 2..5 contributes nothing to DR.** Coverage is aggregated over the
   pair table `sub_469430`, which collapses req{2..5}→phys2 while only phys0/phys1 are summed
   (`GameCreature::meth_0x40A670`, node class byte ∈ {0,1}; values >1 silently ignored). Either a
   deliberate two-zone body+head model or a silent bug; indistinguishable statically, and the reimpl
   must carry **two** DT rows, not six. (FastArrow B7/B8.)
   **Dominated by data now (IronStorm session36):** shipped GITM taxonomy is type 0=tools /
   2=armor / 4=weapons and eqslot 0=body / 1=head / 2=weapon-or-tool, and **nothing uses 3..5** — so
   the 2..5 blind spot is unreachable by shipped content. Downgraded from "probably a bug" to
   "harmless-by-data, still mirrored as two rows". The same sweep also **KILLED T-010b**: there are
   zero stores to `weapon+0x3B0`, the weapon ctor initialises only +0x3E0..+0x428, all 51
   `_WEA.grp` records show 9/10 at eight *different* GIWP offsets rather than one class field, and
   the demo ships no limb-type weapon at all. The `{9,10}×{1,3}` guard is permanently-dead code
   sitting behind the already-ledgered fault at 0x40a045 — mirror, and do not invent the field.
3. **Four console commands register to return-stubs.** `AttackModeSelect`, `CharacterRecordWindow`,
   `SneakToggle`, `WeaponLinkToggle` — stealth/mode UI shipped unimplemented. Parity is
   register-and-stub; omitting them diverges from the shipped console surface. (HappyWolf B38.)
4. **Skin apply divides by a record dimension with no zero test.**
   `Window::virt_meth_0x4b06c0` @0x4b0775..@0x4b07c4 computes a tile/repeat count by dividing the
   texture's own extent by the **record's** width or height — `xor edx,edx / div %ebp` (height, the
   `flags&1` branch @0x4b079d..@0x4b07a1) or `div %ebx` (width, @0x4b07bd..@0x4b07bf) — and **neither
   divisor is tested for 0 anywhere on the path**. A 0 dimension is reachable from the shipped
   parser: `Skin::LoadDefault_0x523490` defaults `width`/`height` to **0, not 8**, when the section
   names a texture, omits the key, and has no parent (@0x523912, @0x523a08 — the `8` is only the
   no-texture default @0x523926). So `[g.slot]` with `texture=` and no `width=`/`height=` faults the
   demo at the moment a widget's skin is applied. **Not reachable by the five shipped skin inis** —
   every textured record carries dimensions, which is why the gate's group sweep over all of them
   never trips it — so this is "reachable by authored data", not a claim that the demo is broken.
   Mirror: the port neither faults nor invents a clamp. It leaves the fragment's tile field at its
   authored value and counts the case (`SkinApplyReport::tilesDivideByZero`, asserted in
   `tests/je/GUILoading`); a lane that ever "fixes" it by defaulting the divisor to 1 is silently
   changing what a malformed skin does. (NiceHawk T-007 §8d-5, `docs/f3/GUILoadingRecon.md`.)
4. **A loud consistency check, not a silent defect: `FatalError("A weapon can't be both consumed
   and thrown.")`.** Fired by `GameWeapon::ReadAttackModeData` when the v5 flag pair is both set
   (FastArrow B9). It is the first ledger entry the shipped binary *diagnoses itself*, and the
   mirroring rule changes shape: parity here means **reproduce the fatal error**, not route around
   it. Do not "handle" it gracefully — a reimpl that tolerates the combination has silently
   diverged from a shipped behaviour the original considers fatal, and any shipped data that trips
   it should crash identically. Whether any shipping record actually trips it is an open data
   question, and the answer only matters for whether we can ever observe it.

5. **Read of an uninitialised stack slot in `GameEffectDamage::Apply` — `var_B1`, the wrap/sector
   byte — on the MAJORITY of reachable paths.** It has **no zero initialiser**: the only stores are
   `mov byte ptr ...,1` at 0x45C2D2 and 0x45C2DB, and the apparent `mov [esp+0Fh], bl` at 0x45C1B6
   targets a *different* slot (`var_A5`) because the `sub esp,0Ch` at 0x45C22C shifts the window by
   0xC. An esp-walk of every store in 0x45C140..0x45C650, **including four-byte store windows**,
   finds nothing that initialises it. So the read is not confined to the two early exits at
   0x45C1BA / 0x45C1CB: the whole `+0x10 == 0` / `meth_0x54be10 -> 0` pair jumps straight to
   0x45C2E4 past the entire compare chain and consumes the residue too. Reachable and
   behaviour-affecting, since reaction anims depend on it. The reimpl mirrors this as an explicit
   residue parameter rather than choosing a quiet default — an intentional observable difference from
   any "clean" implementation. *(Wording broadened from an earlier, narrower claim of "in-band angles
   and the two early exits" on UltraBear's correction; the esp-walk is their evidence.)*

6. **`FatalError("A weapon can't be both consumed and thrown.")` is the one self-diagnosed entry**
   — see item 4 above; parity means **crash parity**, not tolerance. Whether any shipped v5 record can
   trip it is open as T-019.

Also-standing-but-mundane: the `0x6FF650` socket name table is a model-attachment index space, not
the called-shot index space — a live trap for anyone re-deriving part naming, and the reason 10c's
`socketIdx = id+1` bridge was retracted.

## RESOLVED by statics (was: candidate defect surfaced by the T-026 digest) — it is SHIPPED behaviour, mirror it

**World clock starts ~8 hours old.** `JE::World::Update_0x544a90(ms)` (
`modules/openje/src/JE/World.cpp:119`) computes `delta = ms - mbr_0x48`; `mbr_0x48` initialises to
**0** while `ms` is `Host::TickMilliseconds()` = `timeGetTime()` = **machine uptime**. The first pump
therefore folds the whole uptime into the accumulator and the step-6 clock carry
(`World.cpp:99-106`) adds it as world seconds in a single frame — observed as
`frame=0 tickMs=29242857 clock=29242` in an app digest run. Later frames only add deltas, so the
value sticks and the sim runs on an 8-hour-old clock.

**Shipped-side statics, checked before calling it ours** (`docs/f3/SimDigest.md` cites the EAs): the
original's `JE::World::ctor_0x548a80` zeroes the contiguous dword range that covers `+0x44`/`+0x48`
(stores at 0x548b1d..0x548b43), and `World::PackState_545930` **persists `+0x48`** (usage at 0x545958).
So the shipped engine has the same zero-initialised stamp, and it recovers the stamp on load — meaning
the first-`Update` jump is structurally available in the original too, and would be *worse* on a long-
uptime machine. NOT fixed here for that reason plus the mirror rule (never silently improve).

**SETTLED, no debugger spent** (`docs/f3/WorldTickPump.md`). Two shipped sites answer it:
1. `JE::Server::virt_meth_0x552b60` calls World **vftable slot +24** with the value of
   `JE_TickMilliseconds_0x56af50()` (0x552bad loads it, 0x552bb5 calls `*(vft+24)`) — so `ms` is
   **absolute machine uptime**, exactly what our `Host::TickMilliseconds()` supplies.
2. `JE::World::virt_meth_0x546690` (the world-entry path that reads `globals.ini`) stores **0 into
   `+0x48`** at 0x5466b5, while seeding `+0x4C` (0x5466bd) and `+0x54` (0x5466c8) with the tick.

So the original computes the same `uptime - 0` on its first `Update` and takes the same leap. **Our
behaviour is faithful here, and the mirror rule binds: do NOT 'initialise the stamp to the first tick' —
that would silently improve the original.** Consequence for T-031: `clock` IS a valid comparison field,
because both sides start from the same leap, so a `clock` divergence means elapsed sim time, not an
epoch mismatch.

Open remainder — a question with a location, not a defect: why is only `+0x48` left unseeded when `+0x4C`
and `+0x54` get the tick, and what do those two seeded stamps mean for the consumers that read them
(`+0x54` is the field our `Update` subtracts as pause credit).
See `docs/f3/SimDigest.md` and `docs/f3/WorldTickPump.md`.

## Port deviations (deliberate, registered — T-027)

The mirror rule says do not silently improve the original. These are the places where the reimpl is
structurally different from the shipped code **on purpose**, so the next reader does not have to
discover them by diffing.

**D-1 — the random-draw seam.** (`docs/f3/CombatRngSeam.md` §2/§4 is the mechanism; `src/F3/CombatMath.{hpp,cpp}`.)

1. **`RandFn` parameters do not exist in the original.** Shipped `sub_48CD00` takes two ints and reads
   one global dword; all 18 consumers share that single stream. Our `CombatDice`/`CalculateDamage`/
   `CalculateToHit*` take an optional third argument so a test can pin the stream. It is
   behaviour-neutral when `NULL` — which is what every app path passes.
2. **`SeedShippedRand(unsigned)` is a reseed the shipped binary cannot do.** `F3::start_time` has exactly
   one writer, `F3::SetStartupTime` (0x48CCA0), reached once from `sub_48CCC0` with `time32(NULL)`.
   Nothing in `src/F3/*` other than tests calls `SeedShippedRand`; if production code ever does, that is
   a fidelity break and must be reverted, not documented.
3. **This row *removed* a deviation rather than adding one.** Before T-027 the un-injected fallback was
   `std::rand()` — same 0..32767 window, different stream, so the app path drew numbers the original
   could not have produced. It now draws `sub_48CCC0` verbatim. Recorded here because "it used to be
   wrong" is the information a reader needs when comparing old captures against new output.
4. **Known-unmirrored bias, kept.** `Dice(0,100)` maps 32768 values onto 101 buckets with the low 8 bits
   discarded (32768 % 101 = 104), so the shipped to-hit roll is not uniform. We mirror the modulo
   exactly. Do not "fix" it — a uniform draw is a different game's combat.

**D-2 — the entity action queue is one block, not the deque.** (`docs/f3/EntityActionQueueSeam.md`,
`src/F3/MapEntitySpawn.cpp`.) Shipped `JE::Entity::meth_0x550720` pushes onto a block-growing deque at
Entity+0x304 (`cls_0x54e070::meth_0x54ea00`: 4 pointers per 0x10-byte node, slot = count & 3, block =
count >> 2, grows unboundedly). Our `SpawnedEntity::actions[4]` is that container's **first block**, so a
fifth concurrent action is refused (return 0) rather than appended. Refusal is the honest failure mode;
silently dropping an order is not, and the SpawnMove test asserts the refusal.

Two further deviations inside the same seam, both measured rather than assumed:
1. Shipped writes the owner at action+0x14 with ONE type-free store (`0x550737`). Our port cannot, because
   the action models are not uniformly shipped-layout: `GameActionAttack::actor` sits at offset **0** (its
   `+0x14` comment is the shipped offset), `JE::ActionMoveTo::mover` is the shipped **+0x11** and is set by
   the registration path, and the pump-side `GameActionWait`/`GameActionDie`/`GameActionOperateDoor`
   structs have **no owner field at all**. A blind +0x14 write corrupted `JE::ActionMoveTo` and failed to
   link the attack — both observed — so the seam links kind 4, verifies kind 1, and refuses the rest with
   a single log line. Adding owner members to make the store uniform would invent unverified layout.
2. The third shipped step, `JE::Entity::meth_0x54ef30` (per-entity latch + global list at `byte_70BE24` +
   `F3::global_gui_interface_0x70be20` poke + `byte_70BDE8`), is deferred GUI/net refresh and is **not
   ported and not stubbed**. An absent call with the EA cited is honest; a flag nobody reads is a false
   model of a subsystem we have not reconstructed.

**Clock-seed finding (T-038) — NOT a deviation, and deliberately NOT given a D-number.**
(Renumbered from "D-3" on 2026-10-21: this file already carried a second D-3 for T-037's ref-key gap,
and a D-number here would have implied the port differs from shipped. It does not — both sides seed the
date and both leave `totalSeconds` at 0. The only port-side difference is the literal fallback, which is
a bug and is tracked as **T-042**, not a deviation.)
(`docs/f3/WorldYearSeedConsumer.md`, `modules/openje/src/JE/World.cpp:22`, `src/F3/GameWorld.cpp:89-92`.)
`JE::World::virt_meth_0x546b50` (called as the FIRST statement of `Initialize`, `0x5466b0` -> vft slot 5)
reads `[World] Year/Month/Day/Hour/Minute/Second` + `Seconds Per Round` out of the engine ini tree into
`World+0x14..+0x28` / `+0x40`, and `JE::Gregorian::ctor_0x544950` (`0x54497d`) copies those 24 bytes into
the clock's running fields. `Gregorian+0x34 totalSeconds` — the quantity the digest prints as `clock` —
is set to **0** at `0x5a375a`, and every catch-up branch in `virt_meth_0x5a3740` is dead at construction
because seed == running. `PackState_0x545930` persists `+0x44/+0x48` (the tick pair) but **not** the
calendar block, so the date is re-read from disk every run. Therefore: `clock`/`clockDelta` stay
compared, no `year` field belongs in the hashed set. **T-045 later overturned the other half of what this
row asserted:** the frame-0 `clock=29242` is NOT a shared shipped leap — `Update` is entered with
`this = &World[4]` (proven twice: its `+0x34` state read is `ChangeState`'s `+0x38`, and `Tick_0x544b30`
carries an explicit `mov edx,[eax-0Ch]`), so the field it subtracts is primary `+0x4C`, which
`0x5466bd` **seeds**; and `JE_TickMilliseconds_0x56af50` returns an accumulated session delta
(`0x56af50 mov eax, [0x70c2b8]`, written only by `F3::ProcessMessagesAndUpdateTime`, with
`0x70c2bc` seeded in `WinMain` at `0x56b8aa`) — not `timeGetTime()`. The leap is a port artifact of
`src/F3.cpp:786` returning raw `timeGetTime()` and of our `Initialize` never stamping the field. The year
conclusion above is unaffected. Residual inference: `param_1 == &World+0x14` is proved by
shape only (six consecutive ints, identical `0,1,1,0,0,0` ctor defaults, and zero direct readers of
`World+0x14`), not by resolving the thunk's `-0x230 / -0x154` arithmetic.

**Filed as a BUG, not a deviation:** `src/F3/GameWorld.cpp:90-92` falls back to the literals
`?: 2253 / ?: 10 / ?: 18`. Shipped's fallback for a missing key is the World ctor's
`0, 1, 1, 0, 0, 0` (`0x548af6..0x548b08`) — 2253-10-18 comes from `sys.ini`, never from code. Both sides
agree while the key ships; they disagree the moment it does not. The port's `JE::World` defaults are
also all-zero (`modules/openje/src/JE/World.hpp:13`) where shipped uses `0,1,1` for year/month/day.
Fix = delete the literals and match the ctor defaults; it changes no digest value. **Tracked as T-042
(NiceHawk).** No D-number: a bug we intend to fix is not a deviation we accept.

**Standing rule from this row:** no test may assert a damage value (or any drawn value) while the draw
comes from the live global stream — pin the seed or inject the generator, assert the exact number, and
keep a sensitivity control (a different seed must give a different answer). Range-only assertions on an
unseeded stream are how two lanes signed off on a test that asserted nothing.


## Port deviations registered from T-037 (WildYak, `0160a9c`/`c925bf6`)

**D-3 — target is carried as a reference KEY with re-resolution; our port carries neither.** Shipped
`meth_0x403fa0` stores `record+0x38 = *(target + 0x1DC)` and **every** consumer re-resolves it through `0x54BE10`.
`StartDialogue` inverts the shape: it stores the **pointer** at `+0x1C`, converts to the key in its `OnCreate`
(`0x404B99`, stored at `+0x20`), and `OnUpdate` re-resolves **every tick**, setting `+0x18=1` if the key died. Two
doors, one invariant (a target can vanish underneath a queued action), different sites. Our port has no key and no
re-resolution, so **"target despawned mid-attack" is currently unreachable in the sim** — a representation gap, not a
missing branch.

**D-4 — `StartDialogue` flushes the source's entire action deque before allocating.** `0x54c440` walks `Entity+0x304`,
calls `vft+8`, deletes. A scripted dialogue **preempts pending orders**; nothing in the driver can express that, and it
is part of the door rather than an extra.

**Ledger entry 7 (shipped defect, documented so nobody "restores parity" by copying it):** `ReloadWeapon` enqueues
**NULL on allocation failure** (`0x45a3fb`), and the funnel then executes `mov [ecx+14h], esi` with `ecx = 0` — an AV in
the shipped code. Our seam refuses a fifth insert instead of dropping it (D-2, RedXenon), so the failure modes differ
deliberately; this entry exists so the difference is chosen, not discovered.

**Also noted:** `JustAttack` is literally `Attack` minus two ctor default bytes (`+0x6C` turn-to-face gate, `+0x6F`
approach gate), and the port has no fields for either — so **`JustAttack` is currently inexpressible in our model**,
which is a stronger statement than "we invented two fields". And the ctor stores a **never-initialised stack slot** into
`record+0xBC` (`0x403f27`→`0x403f3e`); our port zeroes it, so a byte-level digest would compare shipped garbage against
a deliberate 0.
