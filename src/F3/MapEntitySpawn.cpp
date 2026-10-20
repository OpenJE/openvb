// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <new>
#include <stdlib.h>
#include <string.h>
#include "F3/MapEntitySpawn.hpp"
#include "JE/Map.hpp"
#include "JE/SpatialGrid.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/PathManager.hpp"
#include "JE/HostHooks.hpp"
#include "JE/ActionMoveTo.hpp"
#include "F3/GameActionMoveTo.hpp"
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
	// EXT RESOLVER 0x46FB20 (JE::World vb 0x668494 slot[12]/+48), verbatim semantics:
	//   .CRT -> (flag ? GamePlayer(10) : GameCreature(1))  [player = creature + flag]
	//   .itm 5, .wea 6, .arm 7, .dor 3, .use 2, .pro 8, .amo 9, .con 4; else fallback chain.
	// (duplicate .use compare in the original is dead code; ".pro" NOT ".por".)
	int ClassIdForResourceNameEx( const char* name, int playerFlag ) {
		if ( !name ) {
			return kClassGameEntity;
		}
		const char* dot = strrchr( name, '.' );
		if ( !dot ) {
			return kClassGameEntity;
		}
		++dot;
		if ( _stricmp( dot, "CRT" ) == 0 ) {
			return playerFlag ? kClassGamePlayer : kClassGameCreature;
		}
		if ( _stricmp( dot, "ITM" ) == 0 ) return kClassGameItem;
		if ( _stricmp( dot, "WEA" ) == 0 ) return kClassGameWeapon;
		if ( _stricmp( dot, "ARM" ) == 0 ) return kClassGameArmor;
		if ( _stricmp( dot, "DOR" ) == 0 ) return kClassGameDoor;
		if ( _stricmp( dot, "USE" ) == 0 ) return kClassGameUsableObject;
		if ( _stricmp( dot, "PRO" ) == 0 ) return kClassGameProjectile;
		if ( _stricmp( dot, "AMO" ) == 0 ) return kClassGameAmmo;
		if ( _stricmp( dot, "CON" ) == 0 ) return kClassGameContainer;
		return kClassGameEntity;
	}

	int ClassIdForResourceName( const char* name ) {
		return ClassIdForResourceNameEx( name, 0 );   // flagless default = creature for .CRT
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

	// Registry: loaded maps with stable ids starting at 1 (GetMapById equivalent).
	static JE::Map* s_loadedMaps[17] = { 0 };   /* 1-based ids: slot 0 unused */
	static int      s_loadedMapCount = 0;

	int MapCreate( const char* resourceName ) {
		tracing::instrument( tracing::LOCATION, "resource=%s", resourceName );
		// 0x57B8A0 order: LoadFromFile_0x574f60 -> name assignment -> CreateGridDims
		// (rle dims) -> GetOctree prefetch -> (entity instantiation happens in the chunk
		// readers themselves; ours spawn from records separately). Throw strings from the
		// original are logged (FatalError-on-read-failure deferred):
		//   "Error reading map from Map::Create() method." (0x57B95E)
		JE::Map* map = LoadCompleteMap( resourceName );
		if ( !map || !map->mbr_0x2C ) {
			if ( map ) {
				JE::LogDebugString( NULL, "Error reading map from Map::Create() method.\n" );
			}
			return 0;
		}
		if ( s_loadedMapCount >= 16 ) {   // ids 1..16 into [17]
			return 0;
		}
		s_loadedMaps[s_loadedMapCount + 1] = map;
		++s_loadedMapCount;
		return s_loadedMapCount;   // id = 1-based
	}

	JE::Map* FindLoadedMap( int id ) {
		if ( id < 1 || id > s_loadedMapCount ) {
			return 0;
		}
		return s_loadedMaps[id];
	}

	int LoadedMapCount() {
		return s_loadedMapCount;
	}

	int PlaceEntityAtEntryPoint( SpawnedEntity* e, JE::Map* map, unsigned entryId, unsigned ringSlot ) {
		tracing::instrument( tracing::LOCATION, "entry=%u slot=%u", entryId, ringSlot );
		if ( !e || !map || ringSlot > 5 ) {
			return 0;
		}
		float ring[18];
		if ( map->GetEntryPoint( entryId, ring ) == 0 ) {
			return 0;
		}
		e->pos[0] = ring[ringSlot * 3 + 0];
		e->pos[1] = ring[ringSlot * 3 + 1];
		e->pos[2] = ring[ringSlot * 3 + 2];
		return 1;
	}

	static JE::Map* s_activeMap = NULL;
	void SetActiveMap( JE::Map* map ) { s_activeMap = map; }
	JE::Map* GetActiveMap() { return s_activeMap; }

	// (Re)register an entity's occupancy at its current cell (original syncs entity+0x16C
	// footprint links through JE::Entity position setters; ours refreshes per pump tick).
	static void RegisterOccupancy( SpawnedEntity* e, JE::Map* map ) {
		if ( !map || !map->Grid() ) {
			return;
		}
		int cr[2];
		JE::WorldToOctreeCell_0x560680( cr, e->pos, map );
		if ( e->regCol == cr[0] && e->regRow == cr[1] ) {
			return;
		}
		if ( e->regCol >= 0 ) {
			map->Grid()->Remove( e->regCol, e->regRow, e );
		}
		if ( map->Grid()->Insert( cr[0], cr[1], e ) ) {
			e->regCol = cr[0];
			e->regRow = cr[1];
		} else {
			e->regCol = e->regRow = -1;
		}
	}

	int PushActionOntoEntity( SpawnedEntity* e, int kind, void* action ) {
		tracing::instrument( tracing::LOCATION, "kind=%d", kind );
		if ( !e ) {
			return 0;
		}
		for ( int i = 0; i < 4; ++i ) {
			if ( e->actions[i].kind == 0 ) {
				e->actions[i].kind = kind;
				e->actions[i].action = action;
				if ( kind == 2 ) {
					static_cast<F3::GameActionWait*>( action )->Start( (unsigned)0,
						static_cast<F3::GameActionWait*>( action )->durationMs );
				}
				return 1;
			}
		}
		return 0;   // original list is unbounded; 4-slot cap documented
	}

	void TickWorldEntities( int ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%d", ms );
		for ( int i = 0; i < g_spawnedEntityCount; ++i ) {
			SpawnedEntity* e = g_spawnedEntities[i];
			if ( !e ) {
				continue;
			}
			RegisterOccupancy( e, GetActiveMap() );
			int acted = 0;
			// Lifecycle gate (session 11/12 pump switch on Entity+0x1E0):
			//  - byte +0x3A0 off => never pump (0x549157 pre-switch check)
			//  - state 0 INVALID => original THROWS; we log+skip (deviation)
			//  - 1/3 => CREATED/CREATED-ALT handler advances to ACTIVE(4) (script RunScript
			//    elided - no script system yet)
			//  - 4/5 => normal update;  6 DYING => OnDeath (6->7 flip is SERVER-ONLY and
			//    unmodeled: script hook + netId dereg, session 12 §2)
			//  - 7 DEAD => pumps only while worldLinked (+0x1BC) per the switch
			//  - 8 REMOVED => deep cleanup tail (0x550ad0); actions not pumped here yet
			if ( !e->pumpEnabled ) {
				continue;
			}
			switch ( e->entityState ) {
				case kStateInvalid:
					JE::LogDebugString( NULL,
					    "Trying to update entity when its state is invalid\n" );
					continue;
				case kStateCreated:
				case kStateCreatedAlt:
					e->entityState = kStateActive;   // handlers end in ChangeState(4)
					continue;                        // do not also pump this frame (fidelity)
				case kStateDying:
					continue;   // OnDeath_0x5509b0: script + server-only dereg TODO
				case kStateDead:
					if ( !e->worldLinked ) {
						continue;
					}
					break;      // DEAD still pumps normal update while linked
				case kStateRemoved:
					continue;   // 0x550ad0 deep tail unmodeled
				default:
					break;      // ACTIVE/SPAWNED
			}
			// FIFO: the original runs ONE active action per entity (head of its action
			// queue; the step-aside Wait must gate the MoveTo behind it). Process the
			// first non-empty slot only.
			for ( int s = 0; s < 4 && !acted; ++s ) {
				SpawnedEntity::ActionEntry& a = e->actions[s];
				if ( a.kind == 0 ) {
					continue;
				}
				acted = 1;
				if ( a.kind == 1 ) {
					JE::ActionMoveTo* mv = static_cast<JE::ActionMoveTo*>( a.action );
					mv->Update_0x55b600( (unsigned)ms, 0 );
					if ( mv->finished ) {
						a.kind = 0; a.action = 0;   // slot drop (original deletes here)
					}
				} else if ( a.kind == 2 ) {
					F3::GameActionWait* w = static_cast<F3::GameActionWait*>( a.action );
					if ( w->startMs == 0u && w->durationMs ) {
						w->startMs = (unsigned)ms;   // lazily stamp start on first tick
					}
					if ( w->Expired( (unsigned)ms ) ) {
						a.kind = 0; a.action = 0;
					}
				}
			}
		}
	}

	int ParseGentBlock( const unsigned char* data, unsigned len, int f[8] ) {
		tracing::instrument( tracing::LOCATION, "" );
		unsigned pos = 0;
		while ( pos + 12 <= len ) {
			dword fourcc, ver, sz;
			memcpy( &fourcc, data + pos, 4 );
			memcpy( &ver,    data + pos + 4, 4 );
			memcpy( &sz,     data + pos + 8, 4 );
			if ( sz < 12 || pos + sz > len ) {
				break;
			}
			if ( fourcc == 0x544E4547u /*GENT*/ ) {
				const unsigned char* b = data + pos + 12;
				unsigned body = sz - 12;
				for ( int i = 0; i < 6 && (unsigned)(i + 1) * 4 <= body; ++i ) {
					memcpy( &f[i], b + 4 * i, 4 );
				}
				f[6] = 1;   /* max */ f[7] = 0;   /* cur  */
				if ( ver >= 1 && body >= 32 ) {
					memcpy( &f[6], b + 24, 4 );
					memcpy( &f[7], b + 28, 4 );
					if ( f[7] > f[6] ) {
						f[7] = f[6];   // clamp per 0x463910
					}
				}
				return 1;
			}
			pos += sz;
		}
		return 0;
	}

	// World-server serial generator (original: WorldServer->vft[8] called from GameEntity ctor,
	// discovery note §2 "+0x28 world-server-assigned id"). Monotonic per spawn.
	// 29-bit id space: F3 entity/net ids are `x & 0x1FFFFFFF`, stored at GameEntity+0x28.
	// Counter accessor = IWorldServer 0x66856C slot +8 (0x46f280 reads world.mbr_0x88;
	// masked setter nearby) - discoveries gameworld-interfaces §2/§6 + class-vft-tables.
	static int NextNetId() {
		static int s_next = 1;
		int id = s_next & 0x1FFFFFFF;
		s_next = ( s_next + 1 ) & 0x1FFFFFFF;
		return id;
	}

	// Records -> entities. Each: operator new(SpawnedEntity) + pos/rot/mapId/name from the
	// record (0x551880-equivalent init + SetPosition_0x54A8A0 + SetRotation_0x549FE0). The
	// entity's map id mirrors the loaded map (original: template/creation via world-server
	// subobject; here the caller supplies mapId through the map's context).
	int SpawnMapEntities( JE::Map* map, const char* mapName ) {
		tracing::instrument( tracing::LOCATION, "map=%s", mapName ? mapName : "?" );

		if ( !map ) {
			return 0;
		}
		// original: entity pushed into the map Rb container (map+0x134); our flat list.
		// mapId assigned from the registry id of this map when registered via MapCreate.
		int assignedMapId = 1;
		for ( int q = 1; q <= s_loadedMapCount; ++q ) {
			if ( s_loadedMaps[q] == map ) {
				assignedMapId = q;
				break;
			}
		}
		int spawned = 0;
		for ( unsigned i = 0; i < map->m_recordCount; ++i ) {
			const JE::Map::EntityRecord& r = map->m_records[i];
			// Must run the constructor (not bare operator new): SpawnedEntity is polymorphic
			// (JE::Mover vft) and the movement pipeline calls its virtuals.
			SpawnedEntity* e = new SpawnedEntity();   // ctor defaults: gentF {0,0,0,0,0,0},
		                                                 // statMax/statCur 0/0 (GENT overwrites)
			e->pos[0] = r.pos[0];
			e->pos[1] = r.pos[1];
			e->pos[2] = r.pos[2];
			e->yaw   = r.rotDeg[0];
			e->yawRad = 0.0f;
			// Ground-truth class from the template's chunk signature (demand the resource).
			e->classId = ClassIdForResourceName( r.name );   // default (extension heuristic)
			// Template demand goes through the managed cache (original: Demand(managed=1)
			// during entity file load; multi-instance critters read their .CRT once).
			if ( JE::g_resource_system && r.name ) {
				JE::ResourceSystem::Res* t = JE::g_resource_system->Demand_0x49b750( r.name, 1 );
				if ( t ) {
					e->classId = ClassIdFromTemplateChunks( (const unsigned char*)t->data, (unsigned)t->size );
					int gf[8];
					if ( ParseGentBlock( (const unsigned char*)t->data, (unsigned)t->size, gf ) ) {
						e->gentF[0] = gf[0]; e->gentF[1] = gf[1]; e->gentF[2] = gf[2];
						e->gentF[3] = gf[3]; e->gentF[4] = gf[4]; e->gentF[5] = gf[5];
						e->statMax = gf[6];
						e->statCur = gf[7];
					}
					JE::g_resource_system->Release_0x498eb0( t );
				}
			}
			e->classIdTag = e->classId;   // Entity+0x15C runtime tag mirrors the factory case
			strncpy( e->name, r.name ? r.name : "", sizeof(e->name) - 1 );
			e->name[sizeof(e->name) - 1] = 0;
			// map identity: EME2 records live in one map; the pipeline compares mapId between
			// mover and target (entity+0x1EC). One map loaded per session slice -> id = 1.
			e->mapId = (unsigned)assignedMapId;
			e->netId = NextNetId();
			if ( map->m_entityCount < 128 ) {
				map->m_entities[map->m_entityCount++] = e;
			}
			RegisterOccupancy( e, map );
			if ( map == GetActiveMap() ) { /* keep latest active registration fresh */ }

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
			int id = MapCreate( menuMap );
			JE::Map* m = FindLoadedMap( id );
			if ( m ) {
				SetActiveMap( m );
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
