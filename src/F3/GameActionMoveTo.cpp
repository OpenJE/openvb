// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <string.h>
#include "F3/GameActionMoveTo.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/Map.hpp"
#include "JE/PathManager.hpp"
#include "JE/SpatialGrid.hpp"

namespace F3 {
	// vft 0x4060B0 (size 0x5e1) - server-side blocker response. Full structure documented in
	// the header. Port status: step 1 early-return is FAITHFUL while the map spatial grid
	// (map+364, JE::SpatialGrid::QueryEntities_0x405F80) is unmodeled - the original itself
	// returns immediately when the grid pointer is NULL (0x4060D9). The classTag switch, the
	// Wait(1000ms)+fresh-MoveTo re-issue pattern and the door OperateDoor/segment-plane test
	// are queued behind the grid reconstruction; evidence EAs inline below in comments.
	int GameActionMoveTo::OnServerUpdate_0x4060b0( unsigned ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%u", ms );
		(void)ms;

		if ( !job ) {
			// original dereferences *(job+80) unconditionally at 0x4060D4; a null job would
			// fault there too. Our completed actions clear job before finishing, so bail.
			return 0;
		}

		// 0x4060D4 grid probe; our grid lives on the active map (single-map seam).
		JE::Map* map = GetActiveMap();
		if ( !map || !map->Grid() ) {
			return 0;   // original NULL-grid return at 0x4060D9
		}
		// 0x406107: map = worldServer->vft[21](mover->mapId) — seam: active map must match
		if ( mover->mapId != 0u && !FindLoadedMap( (int)mover->mapId ) ) {
			return 0;
		}

		// 0x406145-0x406181: cell = WorldToOctreeCell(mover pos); query occupants there
		int cr[2];
		JE::WorldToOctreeCell_0x560680( cr, mover->pos, map );
		JE::SpatialGrid::Run run = { cr[0], 1 };
		void* hits[8];
		unsigned n = map->Grid()->QueryEntities( &run, 1, cr[0], cr[1], hits, 8 );

		for ( unsigned i = 0; i < n; ++i ) {
			SpawnedEntity* b = static_cast<SpawnedEntity*>( hits[i] );
			if ( b == static_cast<SpawnedEntity*>( mover ) ) {
				continue;   // 0x4063BC self-exclusion
			}
			switch ( b->classIdTag ) {
				case kClassGameCreature:
					// 0x4063C5: moving creature sharing our cell -> step-aside dance
					if ( !b->movingFlag ) {
						continue;
					}
					{
						static GameActionWait s_waits[8];
						static int s_wi = 0;
						GameActionWait& w = s_waits[ s_wi = ( s_wi + 1 ) & 7 ];
						w.Start( (unsigned)ms, 1000 );
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 2, &w );  // 0x406581-593
						GameActionMoveTo* next = new GameActionMoveTo();                        // 0x4065B9 (factory type 2)
						next->speed = speed;
						next->CopyTargetFrom( this );
						next->mover = mover;
						next->job   = job;            // 0x4065DD: +24 = job pointer (ownership moves)
						next->finished = 0;
						next->requestedFlag = 1;      // mid-move handover (0x4065B9 path skips OnAdd)
						next->lastUpdateMs  = (int)ms;   // handover instant, not the stale pre-stamp value
						next->lastRequestMs = lastRequestMs;
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 1, next );
						job = 0;                    // 0x4065F5
						finished = 1;               // 0x40665E meth_0x55b420(1)
						return 1;
					}
				case kClassGameUsableObject:
				case kClassGameContainer:
					// 0x406404-0x406659: re-issue (original gates on interaction/containment
					// state we do not model yet); hand the job to a fresh action, no Wait.
					{
						GameActionMoveTo* next = new GameActionMoveTo();
						next->speed = speed;
						next->CopyTargetFrom( this );
						next->mover = mover;
						next->job   = job;
						next->requestedFlag = 1;
						next->lastUpdateMs  = lastUpdateMs;
						next->lastRequestMs = lastRequestMs;
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 1, next );
						job = 0;
						finished = 1;
						return 1;
					}
				case kClassGameDoor:
					// 0x4061C4-0x406659 segment-plane test (door yaw @+496*0.017453292,
					// open byte +188, dot tests 0x4062F2-0x406395) -> OperateDoor + Wait +
					// fresh MoveTo. Needs GameDoor geometry: TODO, door occupant ignored.
					continue;
				default:
					continue;   // 0x4061AF switch default: skip
			}
		}
		return 0;
	}

	// Field copy used by the re-issue paths (0x4065B9+ writes on the freshly created action:
	// +24 = predecessor's job... ours: goal snapshot + target identity).
	void GameActionMoveTo::CopyTargetFrom( const GameActionMoveTo* other ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( !other ) {
			return;
		}
		target = other->target;
		targetPos[0] = other->targetPos[0];
		targetPos[1] = other->targetPos[1];
		targetPos[2] = other->targetPos[2];
		arriveDistance = other->arriveDistance;
		lastTargetId = other->lastTargetId;
		map = other->map;
	}
} // namespace F3
