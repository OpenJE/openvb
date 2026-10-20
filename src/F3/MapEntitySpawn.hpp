// Copyright 2024 OpenJE

#ifndef F3_MAPENTITYSPAWN_HPP_
#define F3_MAPENTITYSPAWN_HPP_

#include "JE/ActionMoveTo.hpp"   // JE::Mover layout excerpt

namespace JE { class Map; class PathManager; class ActionMoveTo; }

namespace F3 {
	// Map entity spawning from JE::Map EME2 records (the original chain: world-server vft+52
	// template-lookup-by-name -> vft+24 create -> Entity init/SetPosition/SetRotation -> RB push
	// at map+0x134, count mbr_0x13c; class chosen by F3::CreateGameEntityByClass_0x46f530).
	//
	// DEVIATION (entity layout reconstruction pending): entities are JE::Mover-layout-excerpt
	// objects (pos@+4, mapId@+0x1EC, yaw@+0x1F0, moving@+0x3A2) rather than the full-size
	// F3 classes (GameEntity 0x47C / GameCreature 0xBDC / ...); classId records which shipped
	// class the resource would map to.
	// Entity lifecycle states (Entity+0x1E0; discovery sessions 11/12 pump switch):
	// 0 INVALID(never pumps; original THROWS) 1 CREATED->4 2 DESTROY->8 3 CREATED-ALT->4
	// 4 ACTIVE 5 SPAWNED(ctor birth via 0x550750(entity,5) single caller) 6 DYING
	// (OnDeath; 6->7 flip server-only) 7 DEAD(pumps only while +0x1BC link) 8 REMOVED.
	enum EntityState {
		kStateInvalid = 0, kStateCreated = 1, kStateDestroy = 2, kStateCreatedAlt = 3,
		kStateActive = 4, kStateSpawned = 5, kStateDying = 6, kStateDead = 7,
		kStateRemoved = 8
	};

	struct SpawnedEntity : public JE::Mover {
		SpawnedEntity() : classId(0), yawRad(0.0f), netId(0), regCol(-1), regRow(-1),
		              statMax(1), statCur(0), classIdTag(0),
		              animState(0), weaponStance(7), unk34(7),
		              entityState(kStateSpawned), pumpEnabled(1), worldLinked(1),
		              doorByte0xBC(0), doorByte0xBD(0), doorLock0xC0(0), moveSpeed0x94(8.0f), animCountdown94(0), animLastMs(0) {
		name[0] = 0;
		for ( int i = 0; i < 4; ++i ) { actions[i].kind = 0; actions[i].action = 0; }
		gentF[0]=gentF[1]=gentF[2]=gentF[3]=gentF[5]=0;
		gentF[4]=4;   // GameEntity mbr_0x18 ctor default (discovery session 1 §2)
	}
		int   classId;      // CreateGameEntityByClass_0x46f530 selector (0..10)
		char  name[0x40];   // resource name from the EME2 record
		float yawRad;       // rotation.y from the record (pitch) - unused by movement yet
		// Door fields (GameDoor own block; GDOR chunk reader 0x40e6e0, session 9 §2):
		float   moveSpeed0x94;   // GameEntity+0x94 (ctor 8.0f): the mover speed read by
		                         // GameActionMoveTo slot 7 (0x405720: <=0 => 10.0 fallback)
		// Engine anim window (JE::Entity-vb +0x94 INTEGER countdown - NOT the GE-own +0x94
		// float; name-collision warning per discoveries ge94 §2). Door open/close: OperateDoor
		// issues -durMs; the per-entity pump counts elapsed up to 0 and flips doorByte0xBC
		// there (engine anim 514 + completion, ge94 §2).
		int   animCountdown94;      // negative while an engine anim is in flight
		int   animLastMs;           // pump bookkeeping for the countdown
		unsigned char doorByte0xBC;  // OnServerUpdate skip when != 0; isDoorOpen_0x40e5a0
		                             // returns (byte == 0) - open/closed polarity STILL AMBIGUOUS
		                             // (see C-14 to discovery); default 0 from shipped GDOR blobs.
		unsigned char doorByte0xBD;
		int           doorLock0xC0;  // v>=1 u32 + optional signed jitter (0x40e6ca)
		int   entityState;      // Entity+0x1E0 semantics (pump dispatch, session-11 table)
		unsigned char pumpEnabled;  // Entity+0x3A0 (pump master gate, read at 0x549157 region)
		unsigned char worldLinked;  // Entity+0x1BC (world-link; gates DEAD-state pumping)
		int   regCol;         // occupancy-grid registration cell (-1 = unregistered)
		int   regRow;
		int   netId;        // GameEntity mbr_0x28: world-server-assigned serial (vft[8]);
		                    // 0 in client/editor contexts per ctor 0x4678B3 evidence
		// GENT block (discovery session 3 §1): 6 u32 f1..f6 -> GameEntity+0x08..0x1C, then
		// (version>=1) max/cur -> +0x20/+0x24 with cur>max clamped to max. Flat storage:
		int   gentF[6];       // GameEntity+0x08..+0x1C (ctor defaults 0,0,0,0,4,0)
		// Session 5 §2.1: GameEntity+0x2C = AnimationState (ctor 0; activate sets 5),
		// +0x30 = WeaponStance (ctor 7, valid 1..7), +0x34 unnamed (ctor 7). Flat fields:
		int   animState;      // GE+0x2C
		int   weaponStance;   // GE+0x30
		int   unk34;          // GE+0x34
		int   statMax;        // +0x20 (vft[0] getter 0x4080c0 returns this slot)
		int   statCur;        // +0x24, clamped <= statMax
		int   classIdTag;     // Entity+0x15C runtime tag (== factory case for 1..9; player
		                      // keeps 1 - discovery full-geometry §3/§5). Door/Container are
		                      // UsableObject SIBLINGS, not a chain.
		// Entity action list (original push = JE::Entity::meth_0x550720, iterated by the
		// server pump; blocker paths push Wait(1000ms)+fresh MoveTo, see GameActionMoveTo
		// header). Caller-owned objects; slots auto-clear on completion/fetch.
		struct ActionEntry { int kind; void* action; };   // kind 0 empty, 1 move, 2 wait
		ActionEntry actions[4];
	};

	enum {
		kClassGameEntity = 0, kClassGameCreature = 1, kClassGameUsableObject = 2,
		kClassGameDoor = 3, kClassGameContainer = 4, kClassGameItem = 5,
		kClassGameWeapon = 6, kClassGameArmor = 7, kClassGameProjectile = 8,
		kClassGameAmmo = 9, kClassGamePlayer = 10
	};

	// Resource extension -> class selector heuristic (fallback only; kept for maps whose
	// template blob can't be demanded). See ClassIdFromTemplateChunks for the ground truth.
	int ClassIdForResourceName( const char* name );

	// 0x46FB20 with the player flag: .CRT -> (flag ? 10 : 1). The template chunk signature
	// can therefore only ever yield 1 for a player - promotion happens HERE, matching the
	// original's resolver exactly (discoveries ext-classid-resolver-player-flag.md).
	int ClassIdForResourceNameEx( const char* name, int playerFlag );

	// Ground-truth class discriminator from the template resource's chunk signature (VERIFIED
	// against all six ftype families in the shipping rht and the van-buren chunk spec's entity
	// order table): every template is EEN2, GENT, then type-specific chunks:
	//   GCRE[,(GCHR)]                 -> GameCreature  (1)
	//   GITM                           -> GameItem     (5)
	//   GITM,GIWP / GITM,GIAR / GITM,GIAM -> Weapon(6) / Armor(7) / Ammo(9)
	//   GOBJ                           -> GameUsableObject (2)
	//   GOBJ,GDOR / GOBJ,GCON          -> GameDoor(3) / GameContainer(4)
	// data/len are the demanded template bytes. Returns kClassGameEntity if no marker found.
	int ClassIdFromTemplateChunks( const unsigned char* data, unsigned len );

	extern SpawnedEntity* g_spawnedEntities[128];
	extern int            g_spawnedEntityCount;

	// Instantiate every record in a loaded map; returns the number spawned (logs the
	// original's "Map %s contains %d entities,\n" line).
	int SpawnMapEntities( JE::Map* map, const char* mapName );

	// Full usable map load: LoadFromFile + RLE collision grid (from the EMAP ".rle" name) +
	// octree prefetch so CellToWorld/ProcessJobs work on the returned map.
	JE::Map* LoadCompleteMap( const char* resourceName );

	// Loaded-map registry (stands in for the world-server map list used by GamePathManager/
	// entity map-id comparisons; original assigns mover+492 / entity+0x1EC from it). The
	// by-name (+52) / create (+24) pair our spawn chain mirrors is on the JE::World
	// virtual-base table 0x668494 inside GameWorld (NOT the IWorldServer interface - that is
	// 0x66856C, whose slot +8 = 0x46f280 hands out the next-entity-id feeding our NextNetId;
	// correction per discoveries gameworld-interfaces-effect-pipeline-closed §2/§6).
	// MapCreate registers each loaded map and returns its id; unknown id -> NULL.
	int   MapCreate( const char* resourceName );          // 0x57B8A0 order port
	JE::Map* FindLoadedMap( int id );
	int   LoadedMapCount();

	// The map the path/AI layer treats as "active" (original resolves per-mover via
	// mover+492 -> world registry). Single-map session slice: set at startup.
	void SetActiveMap( JE::Map* map );
	JE::Map* GetActiveMap();

	// Push an action onto an entity's list (JE::Entity::meth_0x550720 port; returns 0 when
	// the 4-slot list is full - the original list is unbounded, deviation documented).
	int PushActionOntoEntity( SpawnedEntity* e, int kind, void* action );

	// OperateDoor action (kind 3 in the list): the 0x406485 shape minus script/anim (toggles
	// the door byte, completes next tick). Fields: door ptr + start ms.
	struct GameActionOperateDoor {
		SpawnedEntity* door;
		unsigned startMs;
		GameActionOperateDoor() : door(0), startMs(0) {}
	};

	// Game-side per-entity server tick driven by JE::World::ServerUpdate_0x545d60 (host seam):
	// iterates each entity's action list (move: JE Update + OnServerUpdate skeleton; wait:
	// expiry), clearing finished slots.
	void TickWorldEntities( int ms );

	// Single-entity variant of the pump (tests; identical lifecycle gate + FIFO head).
	void TickWorldForEntity( struct SpawnedEntity* e, int ms );

	// GENT chunk payload parse (0x466510 semantics): fills f[8] = {f1..f6, max, cur} with the
	// cur<=max clamp; version from the chunk header gates the max/cur pair. Returns 1 when a
	// GENT chunk is present.
	int ParseGentBlock( const unsigned char* data, unsigned len, int f[8] );

	// Place an entity at one ring position of an EMEP entry point (party-slot semantics:
	// the 6 points are one per squad member). Returns 0 when the entry point id is absent.
	int PlaceEntityAtEntryPoint( SpawnedEntity* e, JE::Map* map, unsigned entryId, unsigned ringSlot );

	// Find a spawned entity by name prefix ("Default_PC") - test/game accessor.
	SpawnedEntity* FindSpawnedEntity( const char* prefix );

	// Startup map flow from sys.ini ([Mainmenu] "map name", [Server] "Start map" + "Start map
	// entry point"): load the maps through JE::Map and spawn their entities. Called at the tail
	// of GameWorld::Initialize once resources + parameter trees exist.
	void LoadStartupMaps();
} // namespace F3

#endif // F3_MAPENTITYSPAWN_HPP_
