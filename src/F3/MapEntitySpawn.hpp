// Copyright 2024 OpenJE

#ifndef F3_MAPENTITYSPAWN_HPP_
#define F3_MAPENTITYSPAWN_HPP_

#include "JE/ActionMoveTo.hpp"   // JE::Mover layout excerpt

namespace JE { class Map; class PathManager; }

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
		SpawnedEntity() : classId(0), yawRad(0.0f) { name[0] = 0; }
		int   classId;      // CreateGameEntityByClass_0x46f530 selector (0..10)
		char  name[0x40];   // resource name from the EME2 record
		float yawRad;       // rotation.y from the record (pitch) - unused by movement yet
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

	// Find a spawned entity by name prefix ("Default_PC") - test/game accessor.
	SpawnedEntity* FindSpawnedEntity( const char* prefix );

	// Startup map flow from sys.ini ([Mainmenu] "map name", [Server] "Start map" + "Start map
	// entry point"): load the maps through JE::Map and spawn their entities. Called at the tail
	// of GameWorld::Initialize once resources + parameter trees exist.
	void LoadStartupMaps();
} // namespace F3

#endif // F3_MAPENTITYSPAWN_HPP_
