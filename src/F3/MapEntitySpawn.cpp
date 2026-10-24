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

	// Flagless convenience form of the ext resolver cited above (0x46FB20, vb 0x668494
	// slot[12]): the shipped call sites pass the player flag explicitly; our default is
	// .CRT -> creature.
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

	// Port-side stand-in for the shipped PARSED-TEMPLATE class id that feeds
	// CreateGameEntityByClass_0x46f530: the original reads the class from the GCRE-style chunk of
	// the .CRT/.ITM/... blob (spawn-time template load, EME2 record handler 0x573f30, session 21
	// §3). Our fourcc-marker inference agrees with every shipped EME2 name but stays OUR reading
	// until template parsing is reconstructed — the marker set itself is verbatim.
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

	// Getter over the port's OWN 1..16 map-id table (ids minted by the Map::Create loader
	// meth_0x57b8a0 @0x57B8A0, IDB-verified 2026-10-22; the loader body carries the call order).
	// No shipped counterpart function: the original hands the map pointer around through the
	// map-request manager, not by id.
	JE::Map* FindLoadedMap( int id ) {
		if ( id < 1 || id > s_loadedMapCount ) {
			return 0;
		}
		return s_loadedMaps[id];
	}

	// Registry depth for the test harness; no shipped counterpart (same port-side bookkeeping
	// family as FindLoadedMap above, 0x57B8A0 loader era).
	int LoadedMapCount() {
		return s_loadedMapCount;
	}

	// Entry-point ring = the shipped EMEP chunk @0x573B00 (u8 flag + 6x3 u32 positions + 6 u32
	// -> 96-byte payload; our reader JE/Map.cpp ReadEMEP). ringSlot <= 5 mirrors the shipped
	// 6-slot ring; writing e->pos from it is the spawn-time SetPosition step of the EME2 handler
	// chain @0x573f30 (session 21 verbatim: lookup52 -> create24 -> Create -> SetPosition).
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
		case 6:   // the idle resident (T-071): OUR struct carries the owner at the real shipped
		         // +0x14 (GameActionIdle mbr_0x14, usage 0x40563d reads it in the activation), so
		         // this is the one kind where the uniform store 0x550737 has an honest target.
			static_cast< SpawnedEntity::IdleResident* >( action )->owner0x14 = entity;
			if ( linked ) { *linked = 1; }
			break;
		default:
			// kind 3 (GameActionOperateDoor, MapEntitySpawn.hpp:192) is likewise a flat port-side
			// pair { door, startMs } with no owner member.
			if ( refused ) { *refused = 1; }
			break;
		}
	}

	// Port enqueue seam = the shipped store+refresh pair: uniform owner store 0x550737 (link
	// policy per kind lives in LinkActionOwner above) and the deferred GUI/net refresh
	// meth_0x54ef30 @0x550741, deliberately NOT ported (docs/f3/EntityActionQueueSeam.md).
	// The null guard is ours — no shipped null-order source.
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
	// First-block slot writer: our actions[4] is the deque's FIRST BLOCK only (T-074 §1a
	// container anatomy; shipped record stride 24B via the sub_54EB40 allocator @0x54eb80, T-085
	// §1). The original APPENDS blocks when full — we refuse and return 0, logged by callers.
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

	// T-071 seam: mirror of JE::Entity::meth_0x54c440 - walk the deque, erase, deldtor(1) each.
	// The head gets its [vft+8] cancel hook BEFORE the walk (0x54c49a); shipped idle answers it
	// with ret 0 (0x404b70) and our other kinds have no port-side cancel hook at all, so the
	// hook is cited-and-omitted rather than faked: nothing on the port side reads it.
	void ClearActionQueue( SpawnedEntity* e ) {
		if ( !e ) {
			return;
		}
		for ( int i = 0; i < 4; ++i ) {
			e->actions[i].kind = 0;
			e->actions[i].action = 0;
		}
	}

	// T-071 seam: the drain-then-feed-idle rebase. Shipped does it at the two membership/command
	// edges - Entity::SetInMap_0x551ba0 (drain @0x551bbe -> CreateGameAction case 1 @0x551bde ->
	// funnel @0x551be4, reached from Server::meth_0x555750 and Client::virt_meth_0x58c660) and
	// the Server stop-packet (virt_meth_0x556ea0 case 0x3B: drain @0x55763b -> create(1)
	// @0x557657 -> funnel @0x55765d). T-056 run H saw the result as the first insert on 12/12
	// entities. The object is armed fresh each time (start-pending 0x560184, stamp-flag 0x405605,
	// done-byte stays 0 at 0x56018d) - shipped allocates anew per rebase; in-place re-arming is
	// the same observable state. Owner link goes through EnqueueEntityAction so the 0x550737
	// analog fires exactly like the shipped funnel does.
	void RebaseOntoIdle( SpawnedEntity* e, unsigned int worldMsUnused ) {
		if ( !e ) {
			return;
		}
		(void)worldMsUnused;   // the stamp is applied at the resident's FIRST PUMP tick (the
		                       // shipped start runs inside the pump at sub_55A1C0, not at the
		                       // feeder) - kept in the signature so call sites read as the
		                       // shipped chain reads: rebase at world-time.
		ClearActionQueue( e );
		e->idleResident = SpawnedEntity::IdleResident();   // ctor defaults ARE the re-arm values
		EnqueueEntityAction( e, &e->idleResident, 6 );     // slot 0 after the drain; links owner
		// P-REV: the arm COMPLETES on the entity's next processed tick (0x5a9070 raw - see the
		// IdleResident block); rebase is the insert-window's start, not a permanent occupant.
	}

	// ---------------------------------------------------------------------------
	// P-CMBT Step 1 - the effect inbox (shipped Entity+0x2E0 deque, T-074).
	// ---------------------------------------------------------------------------

	// Fire site. Two DIFFERENT slots, and conflating them is the error this comment exists to
	// prevent (evidence: NiceTiger c1fdb7a + TrueRaven ea53985, re-verified here against the pinned
	// exe by reading GameEffectDamage's vft 0x66073c as raw dwords):
	//   slot 11 / byte +0x2C = 0x404b70 = `c3`, a BARE RETN. This is what the generic finish path
	//     meth_0x55a4f0 calls (`ff 50 2c` @0x55a501), so for GameEffectDamage the generic finish
	//     path applies NOTHING - the slot is a no-op.
	//   slot 10 / byte +0x28 = 0x45c6b0 = the real wrapper: it sets ready (`c6 46 0b 01` @0x45c6b3,
	//     mov byte [esi+0Bh],1) BEFORE its gate call (`e8 64 5f 00 00` @0x45c6b7 -> 0x462620), then
	//     tail-jmps `e9 76 fa ff ff` @0x45c6c5 to 0x45c140, the apply body. 0x45c140 appears in NO
	//     vft cell anywhere - it is reachable only through that wrapper.
	// CONSEQUENCE: damage HP is applied on the PUSH side (inside the enqueue call, via slot 10),
	// which is why a damage effect never sits in the deque. Step 1's port-side fire is therefore an
	// OBSERVABILITY seam for the walk's order; it must NOT be read as "where shipped writes HP".
	// The pending gate itself is real either way: 0x55a4f0 tests +0xA (`test al,al` @0x55a4f6,
	// `je` @0x55a4f8), clears it, then makes the call - so the fire is one-shot.
	static void FirePendingEffect( SpawnedEntity* e, SpawnedEntity::PendingEffect* pe ) {
		if ( !pe->pending0xA ) {
			return;                       // 0x55a4f8: not pending, no [vft+0x2C] call
		}
		pe->pending0xA = 0;               // cleared before the call, as 0x55a4f0 does
		e->effectMark0x171 = true;        // P-CMBT Step 3: the fire site's busy mark (0x54c631
		                                  // family: `mov byte $1,[ent+0x171]`); cleared by the
		                                  // world step's re-gate walk AFTER the pump, below
		e->effectsFired++;
		e->lastFireKind = pe->kind;
		e->lastFireDmg = pe->dmg;
		e->lastFireSource = pe->source;
	}

	// The public enqueue API, mirror of JE::Entity::meth_0x5506c0 (0x5506c0-0x55071a).
	// ORDER MATTERS and is byte-pinned: the preamble stamps/propagates (0x55a3a0 owner stamp
	// @0x5506d1, 0x55a460 @0x5506d8, 0x55a4b0 @0x5506df, 0x55ae10 propagate @0x5506e6), THEN the
	// done-predicate is tested (`call DWORD PTR [edx]` @0x5506ef, `test al,al` @0x5506f1) and if
	// ALREADY DONE the object is handled INLINE via meth_0x55a570 @0x5506f7 and `ret 4` @0x5506fe -
	// it NEVER ENTERS THE DEQUE. Only the not-done branch reaches `lea ecx,[edi+0x2e0]` @0x550706 +
	// push_back @0x55070c. T-074 §3: an effect whose ready gate is already satisfied fires
	// synchronously. Our v1 effects are always ready-on-arrive, so the inline branch is the one
	// that runs, and it is placed FIRST to keep shipped's order.
	//
	// Returns 1 = appended, 0 = inline-fired (never queued), -1 = REFUSED at capacity.
	// Shipped grows unbounded (push_back allocates a new 0x10 block @0x54ea49 when
	// `((count+start)&3)==0 && base <= (count+4)>>2`); our block is fixed at 4 and refuses - the
	// same declared D-2 divergence the action queue carries, cited rather than hidden.
	int EnqueueEffect( SpawnedEntity* target, const SpawnedEntity::PendingEffect& in ) {
		tracing::instrument( tracing::LOCATION, "kind=%d", in.kind );
		if ( !target ) {
			return -1;
		}
		SpawnedEntity::PendingEffect pe = in;   // the preamble operates on the object, then it is
		                                        // either fired-inline or stored
		// NO pending0xA STAMP HERE. The shipped preamble calls meth_0x55a460 @0x5506d8 and
		// meth_0x55a4b0 @0x5506df before the predicate test, and T-074 §3 does not name what either
		// writes. Forcing +0xA=1 would be inventing a stamp to make a test pass AND it would defeat
		// the one-shot rule at 0x55a4f0 (a re-finish would re-fire). So +0xA arrives as the CALLER
		// built the effect - the caller owns the object's bytes, exactly as shipped's constructors
		// do - and Step 2's damage factory is where the stamp legitimately belongs.

		// 0x5506ef-0x5506fe: done already -> fire + dispose, do NOT enqueue.
		// For GameEffectDamage this is not a branch that happens to be taken, it is the ONLY one:
		// its wrapper sets +0xB unconditionally @0x45c6b3 BEFORE the gate @0x45c6b7, so the
		// predicate always reads ready and the enqueue at 0x550701 is UNREACHABLE FOR DAMAGE.
		// THE OPEN QUESTION IS NOW CLOSED (NiceTiger 79b6c81; re-verified here against the pinned
		// exe, not pasted). CodeRefsTo(push_back 0x54ea00) = exactly THREE sites image-wide, with
		// zero indirect enablers (no imm32 of that EA anywhere in .text, so the census is closed):
		//   0x55070c -> +0x2E0 via THIS API (readiness-gated, so damage never satisfies it)
		//   0x55072e -> +0x304 action seam (`lea ecx,[esi+304h]` @0x550728)
		//   0x552692 -> +0x2E0 from DELTA-RECORD REPLAY, in JE::Entity::virt_meth_0x552420 = Entity
		//     vft slot 10 @0x692084 (extent 0x33c), and this site has NO slot-0 readiness test:
		//     factory `call [edx+28h]` @0x552664 -> assign-copy `call [eax+8]` @0x552676 (slot 2 =
		//     0x45c0c0) -> stamp `call 0x55a3a0` @0x552682 -> `lea ecx,[esi+2e0h]` @0x55268c ->
		//     push @0x552692 -> wake `call 0x54ef30` @0x552699. A replayed effect that is NOT ready
		//     therefore sits pending, which is exactly the shape walk #1's skip-past branch (@0x54c63d
		//     inc / @0x54c63e jmp) exists to serve. The container is live-code territory, fed by
		//     net/replay rather than by the attack feeder.
		// TWO CITE CORRECTIONS, both byte-checked here before repeating the finding:
		//   (i) the gate is `call 0x5b4f30` + `test al,1` + `je 0x5526ab` @0x552644/0x55264b, NOT
		//       0x5525d3. That address holds `8b 87 f0 02 00 00 85 c0` = mov eax,[edi+2F0h];
		//       test eax,eax - the SOURCE deque's COUNT check. T-074 §2 carried the same off-by-0x71
		//       cite, so it propagates unless corrected at the source.
		//  (ii) "no fire-walk in the loop" is right; "no fire-ELIGIBLE call" is not. @0x5526a0 calls
		//       meth_0x55af80, head `56 8b f1 8a 46 0a 84 c0 74 15` = a +0xA-gated [vft+0x1C]
		//       predicate path, so a replayed entry can be reached immediately after the push
		//       without waiting for a pump - the same push-side law damage obeys.
		// PORT DEBT, named and era-blocked with the delta-record machinery: this is a SECOND +0x2E0
		// writer the port eventually owes. Step 1 ports the container law only; the replay feeder is
		// not implemented and is not invented here.
		if ( pe.done0xB ) {
			// P-CMBT Step 2: damage applies PUSH-SIDE, at the effect's fire path. Shipped calls the
			// slot-10 apply wrapper (0x45c6b0 -> tail-jmp 0x45c140) at the finisher's creation site
			// @0x403842, before the push @0x403906 - synchronous within the attacker's execution
			// either way, so the port runs it here, in the inline branch, and the per-hit math did
			// not move. The walk/drain fire sites below do NOT apply damage: shipped's generic
			// finish slot [vft+0x2C] is the bare `retn` 0x404b70 for GameEffectDamage - a queued
			// damage entry is ticked, never applied (NiceTiger 9a32ecd). A return of 0 from the
			// apply (the DEAD gate) is observed by the finisher's loop post-apply, not here.
			if ( pe.kind == 1 ) {
				(void)ApplyDamageEffect( target, pe );
			}
			FirePendingEffect( target, &pe );
			target->effectsFreed++;            // 0x55a570's [vft+0x0C](obj,1) deleting-dtor
			target->inboxTrace[0] = 1;         // fire
			target->inboxTrace[1] = 2;         // free; no erase - it was never queued
			target->inboxTrace[2] = 0;
			return 0;
		}

		// 0x550706-0x55070c: append into the deque.
		for ( int i = 0; i < 4; ++i ) {
			if ( target->inbox.fx[ i ].kind == 0 ) {
				target->inbox.fx[ i ] = pe;
				target->inbox.fxCount = (int)target->InboxCount();
				return 1;
			}
		}
		return -1;   // CAPACITY: shipped would allocate a second block here (0x54ea49)
	}

	// Pump walk #1: the FIRST thing the entity pump does (T-074 §4a; the world-clock fetch
	// vft+0x98 @0x54c591 precedes it, then this loop @0x54c597-0x54c640, then the OnMessage pop
	// @0x54c643 and only then walk #2, the +0x304 action deque, @0x54c77c).
	//
	// The semantics that make this NOT the action walk: no entry is ever blocked on. Raw per entry:
	//   tick            `call 0x55afb0`      @0x54c5f8
	//   done-predicate  `call [edx]` / test  @0x54c601 / 0x54c603
	//   done  -> erase `call 0x54b5c0` @0x54c61d, then finish `call 0x55a570` @0x54c62c,
	//            then `mov BYTE [ebp+0x171],1` @0x54c631, then `jmp` @0x54c638 WITHOUT advancing
	//            the cursor (the next entry slides into the freed slot - erase closes the ring);
	//   not done -> `inc esi` @0x54c63d and `jmp` back @0x54c63e: SKIP PAST, whole deque visited.
	// Contrast walk #2's `BREAK` @0x54c7f6, which is the head-of-line behavior T-070 found.
	//
	// NOTE ON PROSE: the design doc's acceptance (c) reads "fire before erase-visible state". The
	// BYTES order it as erase (@0x54c61d) THEN finish/fire (@0x54c62c). This walk follows the bytes
	// and the trace pins erase->fire->free; the doc wording is flagged back to its author rather
	// than silently implemented either way.
	static void RunInboxWalk( SpawnedEntity* e ) {
		int i = 0;                       // the +0x2EC start cursor
		while ( i < 4 ) {
			SpawnedEntity::PendingEffect pe = e->inbox.fx[ i ];   // COPY, not a reference: the erase
			                                                   // below shifts the NEXT entry into this
			                                                   // slot, and shipped finishes the object it
			                                                   // erased (@0x54c62c uses the entry pulled
			                                                   // before the @0x54c61d erase), not whatever
			                                                   // slid into the hole
			if ( pe.kind == 0 ) {
				++i;                    // empty slot: nothing to tick (defensive; our block is
					                      // head-packed by the erase below)
				continue;
			}
			// tick: shipped meth_0x55afb0 recurses the child tree at +0x1C. v1 effects are FLAT
			// (design §"honest gaps" 4), so the tick has nothing to do yet - named, not invented.
			if ( !pe.done0xB ) {
				++i;                    // 0x54c63d: skip past, keep scanning - no head-of-line
				continue;
			}
			// 0x54c61d erase first...
			for ( int j = i; j < 3; ++j ) { e->inbox.fx[ j ] = e->inbox.fx[ j + 1 ]; }
			e->inbox.fx[ 3 ] = SpawnedEntity::PendingEffect();
			e->inboxTrace[0] = 3;         // erase
			// ...then 0x54c62c finish: fire [vft+0x2C] and dispose. `pe` is the COPY saved above.
			// CAUTION: for GameEffectDamage that +0x2C slot is 0x404b70, a bare retn - see the
			// FirePendingEffect header. The generic path's call is real; what it APPLIES depends on
			// which class fills the slot, and damage's apply runs push-side through slot 10 instead.
			FirePendingEffect( e, &pe );
			e->inboxTrace[1] = 1;         // fire
			e->effectsFreed++;
			e->inboxTrace[2] = 2;         // free (0x55a570 -> [vft+0x0C](obj,1) + j__free)
			e->inbox.fxCount = (int)e->InboxCount();
			// DO NOT advance i: the erase slid the next entry into this slot (@0x54c638).
		}
		// 0x54c631 sets [ent+0x171] on every fire; Step 3 owns that mark, so it is NOT stored here.
	}

	// Full drain, mirror of meth_0x54c390 (0x54c390-0x54c42e) - reached ONLY from entity
	// reset/destroy (its sole caller is meth_0x54fff0 @0x5501a2; T-074 §4b found no combat-round,
	// idle-feeder or net-ingest caller). It erases every entry (@0x54c3fa) and finishes each with
	// meth_0x55a570 (@0x54c409), so pending effects FIRE on the drain.
	//
	// THIS IS NOT ClearActionQueue and the difference is asserted, not unified: the action drain
	// meth_0x54c440 gives the head its [vft+8] CANCEL hook BEFORE walking (@0x54c49a, T-071) while
	// the effect drain has NO cancel pass at all - T-074 §4b says the [vft+8] analog does not exist
	// for effects, so v1 is fires-never-cancels and no cancel is invented here.
	void DrainEffectInbox( SpawnedEntity* e ) {
		if ( !e ) {
			return;
		}
		while ( e->InboxCount() != 0u ) {
			SpawnedEntity::PendingEffect pe = e->inbox.fx[ 0 ];   // COPY, same reason as walk #1
			for ( int j = 0; j < 3; ++j ) { e->inbox.fx[ j ] = e->inbox.fx[ j + 1 ]; }
			e->inbox.fx[ 3 ] = SpawnedEntity::PendingEffect();
			e->inboxTrace[0] = 3;         // erase @0x54c3fa
			FirePendingEffect( e, &pe );  // finish @0x54c409 -> fire then free
			e->inboxTrace[1] = 1;
			e->effectsFreed++;
			e->inboxTrace[2] = 2;
		}
		e->inbox.fxCount = (int)e->InboxCount();
		e->effectMark0x171 = true;   // 0x54c429 sets +0x171 at the drain tail (Step 3, NOW LANDED;
		                             // the tail's OTHER call - the GUI-busy de-reg meth_0x54bf00
		                             // @0x54c422 - stays cited-and-absent with the unported GUI seam,
		                             // NOT faked); cleared by the world step after the pump
	}

	static void PumpEntity( SpawnedEntity* e, int ms, int combatantDispatch );

	static void TickOneEntity( SpawnedEntity* e, int ms ) {
		PumpEntity( e, ms, 0 );   // the world loop's own step-5 pump (0x545e71-0x545ebd)
		// P-CMBT Step 3: the world-step's re-gate walk meth_0x54aa20 runs AFTER the pump and
		// clears the effect busy mark @0x54aa82 (walks +0x2E0 firing gate-satisfied entries
		// @0x54aa78). The mark is set by fire sites and cleared ONCE here - set/clear-once, no
		// toggling (the fixture asserts both directions).
		if ( e ) { e->effectMark0x171 = false; }
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

	// Per-entity frame pump mirroring shipped virt_meth_0x54c570 (walk#1 +0x2E0 -> mailbox
	// +0x37C pop -> walk#2 +0x304 + raiser pair -> anim-deque paths A/B, raw 0x54c891-0x54ca31,
	// T-085 §4), reached per frame through F3::ServerUpdate @0x549130's state gate
	// ([ent+0x78] in {1..7}; state 7 passes ONLY when [ent+0x1BC] != 0).
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
			int actedSlot = -1;   // T-048: the slot the FIFO took; its state AFTER the branch
			                      // decides whether the +0x319 raiser fires (see store site below)
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
			// P-CMBT Step 1: walk #1, the effect inbox, runs FIRST - after the lifecycle switch and
			// strictly BEFORE the action walk (shipped 0x54c597 precedes the OnMessage pop 0x54c643
			// and the +0x304 action walk 0x54c77c; T-074 §4a). The mailbox pop (Step 5) and the anim
			// deque (Step 6) will slot between/after these, per the same order sentence.
			RunInboxWalk( e );
			// The walk, P-REV shape (0x54c77c loop / 0x54c7f6 predicate / erase-and-ADVANCE):
			// process the head; if it FINISHES this tick the shipped walk erases and
			// CONTINUES to the next entry in the SAME frame (instant finishers - idle's
			// 0x5a9070, self-finishing engine kinds - chain through one walk); a head that
			// does NOT finish BREAKS the walk (head-of-line - the step-aside Wait gates the
			// MoveTo behind it). Our fixed block must STAY head-packed for "first free slot"
			// insert to equal the shipped deque's tail append, so a completion COMPACTS the
			// array (shipped erase closes the ring; our holes would reorder FIFO and are a
			// single-block modeling limit worth avoiding rather than documenting forever).
			for ( int s = 0; s < 4; ) {
				SpawnedEntity::ActionEntry& a = e->actions[s];
				if ( a.kind == 0 ) {
					++s;            // only reachable past the packed head (defensive)
					continue;
				}
				acted = 1;
				actedSlot = s;
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
				} else if ( a.kind == 6 ) {
					// The idle ARM, P-REV transient semantics (T-082 S1 refutation of the
					// never-finishes reading - raw 0x5a9070 is `c6 41 18 01`, an unconditional
					// write of the SAME byte predicate 0x404b60 reads). One runner call does
					// start, update, and the walk's predicate retest (sub_55A1C0), so idle is
					// processed, arms its observables, and erases in ONE tick; the loop then
					// compacts and CONTINUES - entries behind the arm run in the same frame,
					// exactly as the shipped walk does for instant finishers.
					SpawnedEntity::IdleResident* ir =
					    static_cast<SpawnedEntity::IdleResident*>( a.action );
					if ( ir->startPending0x10 ) {
						ir->startPending0x10 = 0;   // cleared BEFORE the start (sub_55A1C0:20)
						if ( ir->stampOnce0x20 ) {  // ctor 0x405605 sets 1, read at 0x405651
							e->actionStartMs0x184 = ms;   // 0x548f90 store @0x548fa2/0x548fb8
						}
						// QueueAnimEvent_0x4673D0 pair: (ent, 9, 0) @0x40566d THEN (ent, 1,
						// [arm+0x1C]) @0x40567a. The port seam is a single last-id field (no
						// anim reader yet; consumers checked - writers only at
						// GameActionAttack.cpp:312/378), so the observable is the LAST id,
						// with both EAs cited rather than a silent 1.
						e->queuedAnimEvent = 1;
					}
					ir->done0x18 = 1;               // 0x5a9070 raw - the done byte itself,
					                              // written unconditionally on this tick
					a.kind = 0; a.action = 0;       // predicate retest in the same runner call
					                              // reads done -> erase, and the walk continues
				}
				if ( a.kind != 0 ) {
					break;              // unfinished head: the shipped 0x54c7f6 false-path break
					                  // (head-of-line block) - behind THIS, nothing runs today
				}
				// finished: erase-and-advance, compaction keeps the block head-packed so the
				// next entry runs in THIS walk (0x54c80a-0x54c819 erase then loop)
				for ( int i = s; i < 3; ++i ) { e->actions[i] = e->actions[i + 1]; }
				e->actions[3].kind = 0; e->actions[3].action = 0;
			}
			// T-048: the +0x319 RAISER, mirroring shipped `Entity::virt_meth_0x54c570`, whose
			// complete writer set is closed (docs/f3/CombatTurnDispatch.md "+0x319 raiser"):
			//   0x54c833 - an animated-event item was processed this tick and did NOT finish
			//     (remaining time fell below `event+0xC`). Port analog per the row: the FIFO slot
			//     taken above is STILL OCCUPIED after its branch ran - mv->finished false,
			//     at->Update != 1, w->Expired false, die->Update != 1. "Processed, not done"
			//     is exactly "more work pending".
			//   0x54c864 - the event list was EXHAUSTED while the dt budget at Entity+0x179 was
			//     still nonzero ("the pump stopped early"). UNEXPRESSIBLE IN THE PORT, cited and
			//     absent per honest scope: we have no +0x179 dt-budget field at all (the region is
			//     a zeroed gap byte array in JE::Mover, read by nothing) and our pump consumes no
			//     per-tick budget - it runs exactly ONE action slot per entity per tick, so the
			//     "list drained with budget left" state has no analog to construct. Do NOT invent
			//     a budget counter to hang a second store on; that would be the same class of
			//     invention this whole row exists to remove (T-042's 2253 lesson).
			// The slot-DROP path above deliberately does NOT clear: shipped leaves the latch as
			// the EARLIER branch decided; the only zeros anywhere near it are TurnPrep 0x549094
			// (ported at CombatManager.cpp:113, state-5 nodes only) and the ctor 0x5515ff. The
			// sibling byte stores at 0x54c85d/0x54c86d belong to THE RAISER'S OWN
			// exhausted-with-budget pair on this same action walk, not to the animated-event
			// deque - attribution AMENDED per T-085 §4 E1-E3 (docs only, the decision below is
			// unchanged): E1 0x54c845-51 is the precondition (head null = deque exhausted AND
			// [ent+0x174] budget remained); E2 0x54c86d is `movb $1,[ent+0x318]` (arm) on the
			// FIRST exhausted-with-budget occurrence; E3 0x54c85d + 0x54c864 clears +0x318 and
			// sets +0x319 on the SECOND consecutive occurrence - i.e. "twice in a row with budget
			// left", which is the raiser's own double-occurrence detector. My earlier line called
			// them `+0x2E0+0x34` deque traffic because the old +0x2E0 = animated-event-deque
			// reading was still load-bearing here; T-074/T-085 settled +0x2E0 as the EFFECT INBOX
			// and the anim deque as +0x1A8, so that arithmetic is retired too.
			// NOT replicated either way, and for the honest reason now: the pair needs a budget
			// counter ([ent+0x174], decremented only by this walk at 0x54c7e7) that the port does
			// not model - see the no-invention note above, which stands untouched.
			// With the store live the manager's
			// skip-advance (0x5a955d -> 0x5a9572) becomes reachable exactly as shipped - through
			// RE-ENTRANCY, which the port does not model (row boundary; lockstep question routed
			// by T-011). No rotation is invented here: the PathManager T-048 block pins BOTH
			// the raise and the stable head.
			if ( acted && actedSlot >= 0 && e->actions[ actedSlot ].kind != 0 ) {
				// P-REV note: T-071's `kind != 6` guard retires as VACUOUS, not reversed - with
				// the transient reading (0x5a9070 raw) the idle branch ALWAYS clears its slot,
				// so "slot still occupied after processing" can never be true of kind 6 here.
				// The shipped raiser 0x54c833 sits on the ANIM-EVENT deque; WildZenith's T-085
				// census confirmed idle can never be the not-finished head across frames.
				e->actedLatch0x319 = 1;
			}
		}
	} // PumpEntity

	// The join seam's storage. Declared in MapEntitySpawn.hpp; kept in THIS file because it is the
	// one game-layer TU present in every suite that compiles GameActionAttack.cpp.
	static CombatJoinFn g_combatJoinSeam = 0;
	void SetCombatJoinSeam( CombatJoinFn fn ) { g_combatJoinSeam = fn; }
	// Seam indirection ONTO shipped Entity::JoinCombat @0x5490b0 (Entity vft +0x30, idx12;
	// server-gated — the original throws the world-server string when no server instance, session
	// 23 §1). The fn-pointer seam itself is port-side plumbing (this TU must compile without
	// src/F3.cpp); the join semantics belong to 0x5490b0 and CombatManager owns the list.
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
			// T-071: add-to-world is the port's membership edge. Shipped fires the rebase
			// from the net path that follows spawn (Entity::SetInMap_0x551ba0: drain
			// @0x551bbe -> CreateGameAction(1) @0x551bde -> funnel @0x551be4, driven by
			// Server::meth_0x555750 / Client::virt_meth_0x58c660 ADDTOMAP), and T-056 run H
			// observed the result as GameActionIdle at the FIRST insert on 12/12 entities.
			// The head-of-line consequence is the shipped baseline behavior: script natives
			// enqueue BEHIND the resident (that is why run H's Attack never fired); the
			// command-boundary packets drain first (virt_meth_0x556ea0 cases 0x1A/0x3B/0x3C).
			RebaseOntoIdle( e, 0 );
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
	// Test/game-side accessor: prefix walk over the port's spawn registry. NO shipped counterpart
	// located via ea-index 2026-10-22 — the original reaches entities through the busy-list walk
	// (byte_70BE24 loop @0x545e71..0x545ecc, World step virt_meth_0x545d60 era) and name-keyed
	// _Tree lookups; our registry contents ARE the 0x573f30-era spawns.
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
