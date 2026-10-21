// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <new>
#include <stdlib.h>
#include <string.h>
#include "F3/MapEntitySpawn.hpp"
#include "F3/CombatManager.hpp"   // the world-server stand-in for JoinCombat (see GetActiveCombatManager)
#include "F3/GameActionAttack.hpp"
#include "JE/ActionMoveTo.hpp"   // kind 1: mover lives at the shipped +0x11, not +0x14
#include "JE/Map.hpp"
#include "JE/SpatialGrid.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/PathManager.hpp"
#include "JE/HostHooks.hpp"
#include "JE/ActionMoveTo.hpp"
#include "F3/GameActionMoveTo.hpp"
#include "F3/GameActionAttack.hpp"   // kind 4 attack + kind 5 die actions (pump below)
#include "JE/Scripting.hpp"          // OnDeath / OnDamage script hooks
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
	// GDOR payload (session 9 §2): {u8 @0xBC, u8 @0xBD, string(key, 0xC8 base per ctor),
	// v>=1: u32 @0xC0 + jitter}. Missing/failed fields stay at ctor defaults.
	static void ParseGdor( const unsigned char* data, unsigned len,
	                       unsigned char* bBC, unsigned char* bBD, int* lock ) {
		unsigned pos = 0;
		while ( pos + 12 <= len ) {
			dword fourcc, ver, sz;
			memcpy( &fourcc, data + pos, 4 );
			memcpy( &ver,    data + pos + 4, 4 );
			memcpy( &sz,     data + pos + 8, 4 );
			if ( sz < 12 || pos + sz > len ) { break; }
			if ( fourcc == 0x524F4447u /*GDOR*/ ) {
				const unsigned char* b = data + pos + 12;
				unsigned bl = sz - 12;
				if ( bl >= 1 ) { *bBC = b[0]; }
				if ( bl >= 2 ) { *bBD = b[1]; }
				// key string: u16 length at +2 (StreamReadString format), may be empty
				unsigned o = 2;
				if ( o + 2 <= bl ) {
					unsigned short sl; memcpy( &sl, b + o, 2 );
					o += 2 + sl;
				}
				if ( ver >= 1 && o + 4 <= bl ) { memcpy( lock, b + o, 4 ); }
				return;
			}
			pos += sz;
		}
	}

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

	// The owner back-link. Shipped does this as ONE uniform store - 0x550737 `*(param_1 + 20) =
	// this` has no type knowledge, because every shipped action class really does carry the field
	// at +0x14. WE CANNOT COPY THAT STORE: our action structs are only partly shipped-layout, and
	// a blind dword write at +0x14 landed in the middle of unrelated members (it corrupted
	// JE::ActionMoveTo and silently failed to set GameActionAttack::actor, which sits at offset 0 -
	// its "+0x14" comment records the SHIPPED offset, not the port offset). So the seam links the
	// kinds whose owner field this port actually has, and says plainly which ones it cannot.
	// Measured offsets were taken from the compiled layout, not from the comments.
	static void LinkActionOwner( void* action, int kind, SpawnedEntity* entity, int* linked,
	                             int* refused ) {
		switch ( kind ) {
		case 4:   // GameActionAttack - the drain calls Update(ms) with no entity argument, so the
		          // action can only reach its attacker through this field (103/130/157).
			static_cast< F3::GameActionAttack* >( action )->actor = entity;
			if ( linked ) { *linked = 1; }
			break;
		case 2:    // F3::GameActionWait (the pump-side model, NOT the DB-layout header struct) is
		case 5:    // { startMs, durationMs } / { damageType, material, flagsByte0x28 }: there is no
			if ( refused ) { *refused = 1; }   // owner field to write, and inventing one is a fake
			break;
		case 1: {
			// JE::ActionMoveTo carries its mover at the shipped +0x11 ("*this->gap11"), NOT +0x14,
			// and the existing registration path already sets `mover`. Overwriting it on the
			// strength of an offset analogy would be a guess, so the seam only VERIFIES here and
			// reports a mismatch instead of silently changing behaviour.
			JE::ActionMoveTo* mv = static_cast< JE::ActionMoveTo* >( action );
			if ( mv->mover && mv->mover != entity ) {
				JE::LogDebugString( NULL,
				    "EnqueueEntityAction: mover != queueing entity (kind 1, unlinked)\r\n" );
			}
			if ( refused ) { *refused = 1; }
			break;
		}
		default:
			// kind 3 (GameActionOperateDoor, MapEntitySpawn.hpp:192) is likewise a flat port-side
			// pair { door, startMs } with no owner member.
			if ( refused ) { *refused = 1; }
			break;
		}
	}

	int EnqueueEntityAction( SpawnedEntity* entity, void* action, int kind ) {
		tracing::instrument( tracing::LOCATION, "kind=%d", kind );
		if ( !entity || !action ) {
			return 0;   // the original would fault here; we have no null-order source
		}

		int linked = 0, refused = 0;
		const int pushed = PushActionOntoEntity( entity, kind, action );
		if ( pushed ) {
			LinkActionOwner( action, kind, entity, &linked, &refused );   // 0x550737, after the push
		}
		if ( pushed && refused ) {
			// Once, not per call: the shipped store is uniform, ours is not, and a reader of the
			// log must be able to see that SOME kinds were queued without an owner back-link.
			static bool warned = false;
			if ( !warned ) {
				warned = true;
				JE::LogDebugString( NULL,
				    "EnqueueEntityAction: kind queued without owner link (no owner field port-side; "
				    "shipped 0x550737 stores +0x14 uniformly)\r\n" );
			}
		}
		if ( pushed && linked && !refused ) {
			// 0x550741 JE::Entity::meth_0x54ef30 - the deferred GUI/net refresh latch: raises a
			// per-entity flag, links the entity into the global list at byte_70BE24, pokes
			// F3::global_gui_interface_0x70be20 via meth_0x401dd0(1), sets byte_70BDE8. NOT PORTED
			// and deliberately not stubbed with something that looks like a latch - an absent call
			// with the EA cited is honest; a flag nobody reads is a false model. Recorded in
			// docs/f3/EntityActionQueueSeam.md.
		}
		return pushed;
	}

	// The one place a slot is written (owner linking lives in EnqueueEntityAction, which is what
	// new code should call; these call sites predate the seam and keep their own setup).
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
		return 0;   // our actions[4] is the deque's FIRST BLOCK; the original appends blocks
	}

	static void PumpEntity( SpawnedEntity* e, int ms, int combatantDispatch );

	static void TickOneEntity( SpawnedEntity* e, int ms ) {
		PumpEntity( e, ms, 0 );   // the world loop's own step-5 pump (0x545e71-0x545ebd)
	}

	void TickWorldForEntity( SpawnedEntity* e, int ms ) { TickOneEntity( e, ms ); }

	// F3::ServerUpdate at 0x5a953b / 0x5a95de - what CombatManager dispatches to for a
	// combatant. This is the SAME body as the world-loop pump with ONE difference: the state-5
	// gate at 0x545e8e is inverted, because the world loop skips state 5 precisely so that the
	// combat manager owns it. Two copies of this function would drift, and the drift would read
	// as combat behaviour, so there is one body and a flag.
	void TickEntityServerUpdate( SpawnedEntity* e, int ms ) {
		if ( e ) { PumpEntity( e, ms, 1 ); }
	}

	void TickWorldEntities( int ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%d", ms );
		for ( int i = 0; i < g_spawnedEntityCount; ++i ) {
			SpawnedEntity* e = g_spawnedEntities[i];
			if ( !e ) {
				continue;
			}
			TickOneEntity( e, ms );
		}
	}

	static void PumpEntity( SpawnedEntity* e, int ms, int combatantDispatch ) {
		{
			RegisterOccupancy( e, GetActiveMap() );
			// engine-anim countdown (E+0x94 semantics): elapsed accumulates toward 0; on
			// completion the door byte flips (GE-side completion, ge94 §2)
			if ( e->animCountdown94 < 0 ) {
				e->animCountdown94 += ( ms - e->animLastMs );
				e->animLastMs = ms;
				if ( e->animCountdown94 >= 0 ) {
					e->animCountdown94 = 0;
					e->doorByte0xBC = ( e->doorByte0xBC == 0 ) ? 1 : 0;
				}
			}
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
				return;
			}
			switch ( e->entityState ) {
				case kStateInvalid:
					JE::LogDebugString( NULL,
					    "Trying to update entity when its state is invalid\n" );
					return;
				case kStateCreated:
				case kStateCreatedAlt:
					e->entityState = kStateActive;   // handlers end in ChangeState(4)
					return;                          // do not also pump this frame (fidelity)
				case kStateDying:
					// OnDeath_0x5509b0 (session 12 §2): RunScript("OnDeath") -> world-server
					// registry dereg (net broadcast unported) -> ChangeState(7). The 6->7 edge is
					// SERVER-ONLY in the original (it throws otherwise), so it is gated here too.
					JE::EntityRunScript( e, "OnDeath", ms, 0, 0 );
					if ( JE::Host::IsServerContext() ) {
						e->entityState = kStateDead;   // the one legal 6->7 site
					}
					return;
				case kStateDead:
					if ( !e->worldLinked ) {
						return;
					}
					break;      // DEAD still pumps normal update while linked
				case kStateRemoved:
					return;   // 0x550ad0 deep tail unmodeled
				case kStateSpawned:
					// 0x545e8e: `if (*(entity + 480) != 5)` - the WORLD loop skips state 5, because a
					// state-5 entity belongs to the combat manager (it joined through 0x5490b0, and
					// 0x5a953b/0x5a95de dispatch its ServerUpdate). CombatManager has already proven
					// the entity is a state-5 node before it calls, so only that caller pumps here.
					if ( !combatantDispatch ) {
						return;
					}
					break;      // dispatched as a combatant: pump it
				default:
					break;      // ACTIVE
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
				} else if ( a.kind == 3 ) {
					GameActionOperateDoor* od = static_cast<GameActionOperateDoor*>( a.action );
					// Issue the engine door anim (0x40ffe0 shape): PlayAnim(id 514, open/close)
					// then arm the E+0x94 countdown for -durMs. Model/Gfx anim durations are
					// unmodeled => 1000 ms fallback (documented deviation; GetDuration_4a26d0
					// is the seam once models load). RunScript("OnUse") awaits the script layer.
					if ( od->door && od->door->animCountdown94 == 0 ) {
						od->door->animCountdown94 = -1000;
						od->door->animLastMs = ms;
					}
					a.kind = 0; a.action = 0;   // action itself completes on issue, as the
					                              // original's OperateDoor returns once queued
				} else if ( a.kind == 2 ) {
					F3::GameActionWait* w = static_cast<F3::GameActionWait*>( a.action );
					if ( w->startMs == 0u && w->durationMs ) {
						w->startMs = (unsigned)ms;   // lazily stamp start on first tick
					}
					if ( w->Expired( (unsigned)ms ) ) {
						a.kind = 0; a.action = 0;
					}
				} else if ( a.kind == 4 ) {
					// kind 4 = GameActionAttack (session 21 state machine). gameMs is the world
					// frame ms, the same clock the original feeds OnServerUpdate.
					GameActionAttack* at = static_cast<GameActionAttack*>( a.action );
					if ( at->Update( (unsigned)ms ) == 1 ) {
						a.kind = 0; a.action = 0;   // caller-owned (stack or pool) object
					}
				} else if ( a.kind == 5 ) {
					// kind 5 = GameActionDie, queued by the damage Apply onto the VICTIM's list
					// ("mode 5 = accepts queued actions", session 7 §3 step 10).
					GameActionDie* die = static_cast<GameActionDie*>( a.action );
					if ( die->Update( e ) == 1 ) {
						a.kind = 0; a.action = 0;   // static pool: no delete
					}
				}
			}
		}
	} // PumpEntity

	// The join seam's storage. Declared in MapEntitySpawn.hpp; kept in THIS file because it is the
	// one game-layer TU present in every suite that compiles GameActionAttack.cpp.
	static CombatJoinFn g_combatJoinSeam = 0;
	void SetCombatJoinSeam( CombatJoinFn fn ) { g_combatJoinSeam = fn; }
	int JoinCombatThroughSeam( SpawnedEntity* e ) {
		if ( !g_combatJoinSeam || !e ) {
			return 0;   // no manager: the caller reports it, it does not pretend to have joined
		}
		g_combatJoinSeam( e );
		return 1;
	}

	// The world-server stand-in (CombatManager.hpp explains why the seam is defined HERE rather
	// than by calling F3::GetWorldServerInstance() from the action: GameActionAttack.cpp is
	// compiled into suites that link neither the world nor src/F3.cpp).
	static CombatManager* g_activeCombatManager = 0;
	CombatManager* GetActiveCombatManager() { return g_activeCombatManager; }
	void SetActiveCombatManager( CombatManager* cm ) { g_activeCombatManager = cm; }

	// Entity::meth_0x5507f0 ChangeState. Legality read from 0x55080c-0x550920:
	//   0->{1,3}  1->{4}  3->{4}  2->{8}  4->{5,6,2}  5->{4,6,2}  6->{7,2}  7->{2}
	// An illegal pair THROWS "Invalid state change in Entity::ChangeState." in the original
	// (the string is at 0x550820 .. 0x550934, one site per state). The port has no exception
	// plumbing on this path, so it logs the SAME string and does not store: silently accepting
	// an illegal pair is what would let the manager's list and the entity's state disagree.
	// The shipped tail (vft[+27] at 0x55094b, then meth_0x54ef30 / meth_0x54bf00) is the
	// deferred GUI/net refresh pair - CITED AND ABSENT, the same call the enqueue seam already
	// refuses to fake (docs/f3/EntityActionQueueSeam.md).
	int ChangeEntityState( SpawnedEntity* e, int next ) {
		tracing::instrument( tracing::LOCATION, "next=%d", next );
		if ( !e ) {
			return 0;
		}
		int legal = 0;
		switch ( e->entityState ) {
			case kStateInvalid:    legal = ( next == kStateCreated || next == kStateCreatedAlt ); break;  // 0x55080c
			case kStateCreated:    legal = ( next == kStateActive ); break;                                // 0x55083a
			case kStateCreatedAlt: legal = ( next == kStateActive ); break;                                // 0x55083a
			case kStateDestroy:    legal = ( next == kStateRemoved ); break;                               // 0x550863
			case kStateActive:     legal = ( next == kStateSpawned || next == kStateDying
			                               || next == kStateDestroy ); break;                              // 0x550896
			case kStateSpawned:    legal = ( next == kStateActive || next == kStateDying
			                               || next == kStateDestroy ); break;                              // 0x5508c9
			case kStateDying:      legal = ( next == kStateDead || next == kStateDestroy ); break;         // 0x5508f7
			case kStateDead:       legal = ( next == kStateDestroy ); break;                               // 0x550920
			default:               legal = 0; break;   // REMOVED(8) has no legal exit in the table
		}
		if ( !legal ) {
			JE::LogDebugString( NULL, "Invalid state change in Entity::ChangeState.\r\n" );
			return 0;
		}
		e->entityState = next;   // 0x550945 stores Entity+0x1E0
		return 1;
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
			// EME2 u8 flag -> Create(..., flag!=0, 1) + Entity+0x3A0 pump byte
			// (discovery session 21 §spawn-handler + session 22 §1 closed C-12 question).
			// Shipped crt values: record[0] (Default_PC) flag=1 -> pumps; all others 0 ->
			// pump-disabled offline (network-owned entities). Matches T-004 assertions.
			e->pumpEnabled = ( r.flag != 0 ) ? 1 : 0;
			// Ground-truth class from the template's chunk signature (demand the resource).
			e->classId = ClassIdForResourceName( r.name );   // default (extension heuristic)
			// Template demand goes through the managed cache (original: Demand(managed=1)
			// during entity file load; multi-instance critters read their .CRT once).
			if ( JE::g_resource_system && r.name ) {
				JE::ResourceSystem::Res* t = JE::g_resource_system->Demand_0x49b750( r.name, 1 );
				if ( t ) {
					e->classId = ClassIdFromTemplateChunks( static_cast<const unsigned char*>( t->data ), (unsigned)t->size );
					if ( e->classId == kClassGameDoor ) {
						ParseGdor( static_cast<const unsigned char*>( t->data ), (unsigned)t->size,
						           &e->doorByte0xBC, &e->doorByte0xBD, &e->doorLock0xC0 );
					}
					int gf[8];
					if ( ParseGentBlock( static_cast<const unsigned char*>( t->data ), (unsigned)t->size, gf ) ) {
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
			e->entityState = kStateActive;   // add-to-world activation (map load leaves
			                               // entities at ctor birth 5; the pump head skips
			                               // them until this flip - session 14 §2)
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
		                   const_cast<char*>( mapName ), spawned );
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
				map->LoadRleGrid( static_cast<const unsigned char*>( buf ), (unsigned)len );
				free( buf );
			}
		}
		map->GetOctree();   // prefetch octree (GetOctree_0x56d960)
		return map;
	}

	void LoadStartupMaps() {
		tracing::instrument( tracing::LOCATION, "" );

		const JE::VFX_Parameter* sys = static_cast<const JE::VFX_Parameter*>( JE::Host::GetParameterTree() );
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
