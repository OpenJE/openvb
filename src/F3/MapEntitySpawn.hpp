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
	struct SpawnedEntity : public JE::Mover {
		SpawnedEntity() : classId(0), yawRad(0.0f), netId(0),
		              statMax(1), statCur(0), classIdTag(0),
		              animState(0), weaponStance(7), unk34(7) {
		name[0] = 0;
		for ( int i = 0; i < 4; ++i ) { actions[i].kind = 0; actions[i].action = 0; }
		gentF[0]=gentF[1]=gentF[2]=gentF[3]=gentF[5]=0;
		gentF[4]=4;   // GameEntity mbr_0x18 ctor default (discovery session 1 §2)
	}
		int   classId;      // CreateGameEntityByClass_0x46f530 selector (0..10)
		char  name[0x40];   // resource name from the EME2 record
		float yawRad;       // rotation.y from the record (pitch) - unused by movement yet
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
	// by-name template lookup (+52) / create (+24) pair our spawn chain mirrors lives on the
	// GameWorld interface vftable 0x668494 (discovery session 5).
	// MapCreate registers each loaded map and returns its id; unknown id -> NULL.
	int   MapCreate( const char* resourceName );          // 0x57B8A0 order port
	JE::Map* FindLoadedMap( int id );
	int   LoadedMapCount();

	// Push an action onto an entity's list (JE::Entity::meth_0x550720 port; returns 0 when
	// the 4-slot list is full - the original list is unbounded, deviation documented).
	int PushActionOntoEntity( SpawnedEntity* e, int kind, void* action );

	// Game-side per-entity server tick driven by JE::World::ServerUpdate_0x545d60 (host seam):
	// iterates each entity's action list (move: JE Update + OnServerUpdate skeleton; wait:
	// expiry), clearing finished slots.
	void TickWorldEntities( int ms );

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
