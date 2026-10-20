// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <string.h>
#include "F3/GameActionMoveTo.hpp"
#include "JE/PathManager.hpp"

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

		// 0x4060D4: grid = *(job + 80). Our PathJob carries no resident grid (spatial grid
		// unmodeled) => identical control flow to the original's NULL-grid return.
		// TODO(grid): query mover cell -> blockers; per blocker classTag (Entity+0x15C):
		//   1 + moving(+930):    PushActionOntoEntity(mover, kWait=1000ms); fresh MoveTo
		//                        copy (0x4065B9-0x4065F5 writes +24 job, +36..44 goal, +48)
		//   2/4:                 re-issue fresh MoveTo (0x406628-0x406659)
		//   3 (door): open byte +188 skip; else segment-plane test (yaw@+496 * 0.017453292,
		//              dots at 0x4062F2-0x406395) -> OperateDoor + Wait + fresh MoveTo.
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
