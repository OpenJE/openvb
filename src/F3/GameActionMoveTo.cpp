// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <string.h>
#include "F3/GameActionMoveTo.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/Map.hpp"
#include "JE/PathManager.hpp"
#include <stdio.h>
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
					// Session-11 rule refinement (blocker state Entity+0x1E0 == raw +480):
					// 4 (ACTIVE) unconditionally, or 5 (SPAWNED) when not already in the
					// mover's interaction set (JE::EntitySet::Contains_0x54AEB0 unmodeled ->
					// treated as NOT contained, the blocking branch), trigger the plain
					// re-issue dance at 0x4065FE; MOVING creatures add the Wait first.
					if ( b->entityState != F3::kStateActive && b->entityState != F3::kStateSpawned ) {
						continue;   // dying/dead/removed creatures do not block movement
					}
					if ( !b->movingFlag ) {
						// 0x4065FE: re-issue fresh MoveTo WITHOUT Wait (LABEL_57)
						GameActionMoveTo* next = new GameActionMoveTo();
						next->speed = speed;
						next->CopyTargetFrom( this );
						next->mover = mover;
						next->job   = job;
						next->requestedFlag = 1;
						next->lastUpdateMs  = (int)ms;
						next->lastRequestMs = lastRequestMs;
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 1, next );
						job = 0;
						finished = 1;
						return 1;
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
					// 0x4061C4: RTTI GameDoor; `if (door[188]) goto LABEL_33` - the flag-set
					// door is SKIPPED. Flag-clear doors face the segment-plane test (hinge
					// points door[+0x10..+0x24], yaw +0x1F0 * 0.017453292, side-dot pair
					// 0x4062F2-0x406395). Point-field semantics NOT pinned (C-14 to
					// discovery) - conservative shipped-flow reading: flag-clear door in our
					// cell => dance OperateDoor + Wait(1000) + fresh MoveTo (0x406485-0x406659).
					if ( b->doorByte0xBC != 0 ) {
						continue;   // LABEL_33 skip path
					}
					{
						static GameActionOperateDoor s_ods[4];
						static GameActionWait s_ws2[4];
						static int s_oi = 0;
						s_oi = ( s_oi + 1 ) & 3;
						s_ods[s_oi].door = b;
						s_ods[s_oi].startMs = (unsigned)ms;
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 3, &s_ods[s_oi] );
						GameActionWait& w = s_ws2[s_oi];
						w.Start( (unsigned)ms, 1000 );
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 2, &w );
						GameActionMoveTo* next = new GameActionMoveTo();
						next->speed = speed;
						next->CopyTargetFrom( this );
						next->mover = mover;
						next->job   = job;
						next->requestedFlag = 1;
						next->lastUpdateMs  = (int)ms;
						next->lastRequestMs = lastRequestMs;
						PushActionOntoEntity( static_cast<SpawnedEntity*>( mover ), 1, next );
						job = 0;
						finished = 1;
						return 1;
					}
				default:
					continue;   // 0x4061AF switch default: skip
			}
		}
		return 0;
	}

	// ---- batch C rows: entity-facing anim slots (docs/f3/GameActionMoveToEvidence.md,
	// whole-body reads). Cast convention: shipped RTTI-casts the [this+0x14] mover
	// Entity->GameEntity and SILENTLY returns on failure; the port's mover is always
	// SpawnedEntity in this lane (GetMoveSpeed precedent), so the testable failure arm is
	// the null-mover silent return. STATUS (owners attached, per the relay law): the
	// forwarder CALL is a named gap (header note); stamp/anim arms consume live
	// SpawnedEntity seams; the drain arm is a cited no-op until the mover ring models.
	void GameActionMoveTo::OnAnimChoice_0x4057b0( unsigned ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%u gate=%d", ms, moverGate0x90 );
		if ( !mover ) {
			return;   // cast-fail arm, null form (shipped: silent retn @0x4057cf-era)
		}
		SpawnedEntity* ge = static_cast<SpawnedEntity*>( mover );
		ge->actionStartMs0x184 = (int)ms;   // 0x548f90 clock store @0x548fb8/0x548fa2
		// 0x54bd90 ring drain (mover +0x1B0 size / +0x1B4 cursor / +0x1B8 count):
		// UNMODELED in the port - no SpawnedEntity trio exists; no-op named gap. The
		// co-caller of the same drain is GameActionDie::virt_meth_0x404e90 @0x404ed4;
		// retires when the ring lands (same class as the +0x2E0/+0x304 deques, per the
		// type-vs-instance law kept in the evidence doc).
		ge->queuedAnimEvent = ( moverGate0x90 == 1 ) ? 29 : 32;   // 0x4057ef: cmp [esi+90h],1 -> 1Dh else 20h
	}

	void GameActionMoveTo::OnAnimStamp1_0x406780( unsigned ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%u", ms );
		if ( !mover ) {
			return;
		}
		SpawnedEntity* ge = static_cast<SpawnedEntity*>( mover );
		ge->actionStartMs0x184 = (int)ms;   // same clock arm as slot 10
		ge->queuedAnimEvent = 1;            // QueueAnimEvent(1, 0) tail; NO drain, NO gate (byte-verified contrast)
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
