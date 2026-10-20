// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <new>
#include <stdlib.h>
#include <string.h>
#include "F3/MapEntitySpawn.hpp"
#include "JE/Map.hpp"
#include "JE/PathManager.hpp"
#include "JE/HostHooks.hpp"
#include "JE/Resources.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE.hpp"

namespace F3 {
	// class selector per CreateGameEntityByClass_0x46f530 (switch on template class id):
	//   0 GameEntity 0x47C | 1 GameCreature 0xBDC | 2 GameUsableObject 0x498 | 3 GameDoor 0x4C8
	//   4 GameContainer 0x4D8 | 5 GameItem 0x78C | 6 GameWeapon 0x808 | 7 GameArmor 0x7A8
	//   8 GameProjectile 0x494 | 9 GameAmmo 0x7B0 | 10 GamePlayer 0xBFC
	// The ORIGINAL takes the id from the parsed template resource (the EME2 name resolves to a
	// .CRT/.USE/... blob whose GCRE-style chunk carries the class); until template parsing is
	// reconstructed we map by resource extension, which matches every shipped EME2 name.
	int ClassIdForResourceName( const char* name ) {
		if ( !name ) {
			return kClassGameEntity;
		}
		const char* dot = strrchr( name, '.' );
		if ( !dot ) {
			return kClassGameEntity;
		}
		++dot;
		if ( _stricmp( dot, "CRT" ) == 0 ) {
			// Default_PC.CRT is the player in shipped maps; the template's class field is the
			// real selector (template parse TODO). Creature is the faithful-size match for the
			// demo records we can observe (all critters).
			return kClassGameCreature;
		}
		if ( _stricmp( dot, "USE" ) == 0 ) return kClassGameUsableObject;
		if ( _stricmp( dot, "ITM" ) == 0 ) return kClassGameItem;
		if ( _stricmp( dot, "DOR" ) == 0 ) return kClassGameDoor;
		if ( _stricmp( dot, "CON" ) == 0 ) return kClassGameContainer;
		if ( _stricmp( dot, "WEA" ) == 0 ) return kClassGameWeapon;
		if ( _stricmp( dot, "AMR" ) == 0 ) return kClassGameArmor;
		if ( _stricmp( dot, "AMO" ) == 0 ) return kClassGameAmmo;
		return kClassGameEntity;
	}

	// Walk the top-level {fourcc,ver,size} chunk stream and inspect the marker chunk set.
	int ClassIdFromTemplateChunks( const unsigned char* data, unsigned len ) {
		bool seenCRE=0, seenITM=0, seenWP=0, seenAR=0, seenAM=0, seenOBJ=0, seenDOR=0, seenCON=0;
		unsigned pos = 0;
		while ( pos + 12 <= len ) {
			dword fourcc, sz;
			memcpy( &fourcc, data + pos, 4 );
			memcpy( &sz, data + pos + 8, 4 );   // {tag,ver,size}; size includes 12B header
			if ( sz < 12 || pos + sz > len ) {
				break;
			}
			switch ( fourcc ) {
				case 0x45524347u /*GCRE*/: seenCRE = 1; break;
				case 0x4D544947u /*GITM*/: seenITM = 1; break;
				case 0x50574947u /*GIWP*/: seenWP  = 1; break;
				case 0x52414947u /*GIAR*/: seenAR  = 1; break;
				case 0x4D414947u /*GIAM*/: seenAM  = 1; break;
				case 0x4A424F47u /*GOBJ*/: seenOBJ = 1; break;
				case 0x524F4447u /*GDOR*/: seenDOR = 1; break;
				case 0x4E4F4347u /*GCON*/: seenCON = 1; break;
				default: break;   // EEN2, GENT, GCHR, ...
			}
			pos += sz;
		}
		if ( seenCRE ) return kClassGameCreature;
		if ( seenITM ) {
			if ( seenWP ) return kClassGameWeapon;
			if ( seenAR ) return kClassGameArmor;
			if ( seenAM ) return kClassGameAmmo;
			return kClassGameItem;
		}
		if ( seenOBJ ) {
			if ( seenDOR ) return kClassGameDoor;
			if ( seenCON ) return kClassGameContainer;
			return kClassGameUsableObject;
		}
		return kClassGameEntity;
	}

	SpawnedEntity* g_spawnedEntities[128] = { 0 };
	int            g_spawnedEntityCount = 0;

	// Records -> entities. Each: operator new(SpawnedEntity) + pos/rot/mapId/name from the
	// record (0x551880-equivalent init + SetPosition_0x54A8A0 + SetRotation_0x549FE0). The
	// entity's map id mirrors the loaded map (original: template/creation via world-server
	// subobject; here the caller supplies mapId through the map's context).
	int SpawnMapEntities( JE::Map* map, const char* mapName ) {
		tracing::instrument( tracing::LOCATION, "map=%s", mapName ? mapName : "?" );

		if ( !map ) {
			return 0;
		}
		int spawned = 0;
		for ( unsigned i = 0; i < map->m_recordCount; ++i ) {
			const JE::Map::EntityRecord& r = map->m_records[i];
			// Must run the constructor (not bare operator new): SpawnedEntity is polymorphic
			// (JE::Mover vft) and the movement pipeline calls its virtuals.
			SpawnedEntity* e = new SpawnedEntity();
			e->pos[0] = r.pos[0];
			e->pos[1] = r.pos[1];
			e->pos[2] = r.pos[2];
			e->yaw   = r.rotDeg[0];
			e->yawRad = 0.0f;
			// Ground-truth class from the template's chunk signature (demand the resource).
			e->classId = ClassIdForResourceName( r.name );   // default (extension heuristic)
			if ( JE::g_resource_system && r.name ) {
				void* tb = NULL;
				int tl = 0;
				if ( JE::g_resource_system->ReadResource( r.name, &tb, &tl ) ) {
					e->classId = ClassIdFromTemplateChunks( (const unsigned char*)tb, (unsigned)tl );
					free( tb );
				}
			}
			strncpy( e->name, r.name ? r.name : "", sizeof(e->name) - 1 );
			e->name[sizeof(e->name) - 1] = 0;
			// map identity: EME2 records live in one map; the pipeline compares mapId between
			// mover and target (entity+0x1EC). One map loaded per session slice -> id = 1.
			e->mapId = 1;

			if ( g_spawnedEntityCount < 128 ) {
				g_spawnedEntities[g_spawnedEntityCount++] = e;
				++spawned;
			} else {
				delete e;
			}
		}
		// verbatim log shape from Map::Create_0x57bc33
		JE::LogDebugString( NULL, "Map %s contains %d entities,\n",
		                   (char*)mapName, spawned );
		return spawned;
	}

	// sys.ini ([Mainmenu] "map name" / [Server] "Start map" + "Start map entry point").
	// Loads each map via JE::Map and spawns its entities. Deviation: the shipped flow queues
	// a map request through cls_0x5a42b0 (world+0x104) and the server tick performs the load;
	// we load inline at Initialize tail (request-manager reconstruction pending).
	// Full usable map load (see header).
	JE::Map* LoadCompleteMap( const char* resourceName ) {
		tracing::instrument( tracing::LOCATION, "resource=%s", resourceName );
		JE::Map* map = new JE::Map();
		if ( !map || !map->LoadFromFile( resourceName ) ) {
			return map;
		}
		// collision grid from the EMAP ".rle" name (CreateGridDims_0x57b2a0 equivalent)
		if ( map->mbr_0x4C && JE::g_resource_system ) {
			void* buf = NULL;
			int len = 0;
			if ( JE::g_resource_system->ReadResource( map->mbr_0x4C, &buf, &len ) ) {
				map->LoadRleGrid( (const unsigned char*)buf, (unsigned)len );
				free( buf );
			}
		}
		map->GetOctree();   // prefetch octree (GetOctree_0x56d960)
		return map;
	}

	void LoadStartupMaps() {
		tracing::instrument( tracing::LOCATION, "" );

		JE::VFX_Parameter* sys = static_cast<JE::VFX_Parameter*>( JE::Host::GetParameterTree() );
		JE::Map* map = new JE::Map();
		if ( !map ) {
			return;
		}

		const char* menuMap = sys ? sys->GetStringRaw_0x4df070( "Mainmenu", "map name" ) : NULL;
		if ( menuMap && *menuMap ) {
			JE::Map* m = LoadCompleteMap( menuMap );
			if ( m ) {
				SpawnMapEntities( m, menuMap );
			}
		}
		(void)map;
		// NOTE: the map object is intentionally retained (its records' names are referenced
		// by nothing after spawn, but the octree/grid belong to the startup world; leaking the
		// singleton until the map-request manager owns it is the conservative choice).
	}

	// test/game-side accessor: find a spawned entity by name prefix (e.g. "Default_PC")
	SpawnedEntity* FindSpawnedEntity( const char* prefix ) {
		size_t n = prefix ? strlen( prefix ) : 0;
		for ( int i = 0; i < g_spawnedEntityCount; ++i ) {
			if ( strncmp( g_spawnedEntities[i]->name, prefix, n ) == 0 ) {
				return g_spawnedEntities[i];
			}
		}
		return 0;
	}
} // namespace F3
