// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONMOVETO_HPP_
#define F3_GAMEACTIONMOVETO_HPP_

#include "F3.hpp"
#include "JE/ActionMoveTo.hpp"

namespace F3 {
	// Server-side action ids created via the world-server interface (0x668494 vft[5], +20):
	//   type 2  = GameActionMoveTo   (observed at 0x4065B9/0x406628/0x406518)
	//   type 5  = GameActionWait     (observed at 0x406581; duration arg 1000 ms)
	//   type 10 = GameActionOperateDoor? (observed new 0x28 + ctor_0x4066c0 - separate class)
	enum {
		kWorldActionMoveTo = 2,
		kWorldActionWait   = 5
	};

	// F3::GameActionMoveTo - JE::ActionMoveTo plus the server-side steering hook
	// OnServerUpdate_0x4060B0 (0x5e1 bytes, fully decompiled; structure below).
	//
	// ORIGINAL OnServerUpdate_0x4060B0 flow (blocker response pipeline):
	//   1. JE base OnServerUpdate; grid = *(job + 80)  -> if NULL: RETURN (PathJob doubles as
	//      the node context; our NodeContext carries the same field, grid unmodeled).
	//   2. map = worldServer(0x668494)->vft[21](+84)(mover->mapId /*+492*/)  (GetMapById ✓)
	//   3. WorldToOctreeCell_0x560680(mover pos) then JE::SpatialGrid::QueryEntities_0x405F80
	//      over map+364+12 results into an RbTree(0x14) set.
	//   4. per blocker: switch (*(blocker + 0x15C /*classTag*/)):
	//        1 (creature/player): if blocker moving(+930)  -> push Wait(1000ms) onto mover
	//                              + push FRESH MoveTo (copies job fields: +24 job, +36..44
	//                              goal xyz, +48 flag) and null this job; if blocker
	//                              state(+480)==5 && !in entitySet -> blocker path; if ==4
	//                              -> re-issue; else skip.
	//        2 / 4 (usable/container): mover not interacting -> re-issue fresh MoveTo.
	//        3 (door, RTTI GameDoor): door-open byte (+188) -> skip; else segment-plane test
	//             vs path nodes (door pos +4..+36, yaw@+496 deg->rad 0.017453292, normal
	//             dot tests at 0x4062F2-0x406395) -> crossing blocked: push OperateDoor +
	//             Wait(1000) + fresh MoveTo.
	//   5. completion hook (this->__vftable[1]+0x10) + job=NULL + finish(1).
	//
	// PORT: action-list plumbing implemented; the blocker pipeline degrades EXACTLY like the
	// original with grid==NULL (step 1 return) until the spatial grid (map+364) lands.
	struct GameActionMoveTo : public JE::ActionMoveTo {
		GameActionMoveTo() : speed(8.0f) {}   // 8.0f = GE+0x94 ctor default (candidate source)

		// JE hook (secondary vft +0x14): the original routes it into the F3
		// OnServerUpdate_0x4060b0 through the entity's action loop; ms = entity ServerUpdate
		// timestamp (v16 world time in the original).
		virtual void OnMoveServerUpdate( unsigned ms ) { OnServerUpdate_0x4060b0( ms ); }

		// Movement speed source. The un-named GE+0x94 (ctor 8.0f, discovery session 1 §2 —
		// "needs first consumer") is our leading candidate for the entity's move speed; the
		// JE::Entity-side consumer is unconfirmed, so F3 carries an explicit field until the
		// first read site proves the mapping (then it becomes a mover+0x94 load).
		float speed;
		virtual float GetMoveSpeed() { return speed; }

		// OnServerUpdate vft slot on the entity-facing interface; called by the world pump
		// after the JE movement update.
		virtual int OnServerUpdate_0x4060b0( unsigned ms );

		// fresh re-issue copy (original writes +24 job, +36..44 goal, +48 flag on the new
		// action): copies target snapshot fields from a predecessor.
		void CopyTargetFrom( const GameActionMoveTo* other );
	};

	// F3::GameActionWait - the type-5 world action: entity idles for N ms (created with
	// duration 1000 in the blocker paths; field via ScrollBar::meth_0x568420 alias =
	// SetDuration(ms) in our model).
	struct GameActionWait {
		unsigned startMs;
		unsigned durationMs;
		GameActionWait() : startMs(0), durationMs(0) {}
		void Start( unsigned ms, unsigned dur ) { startMs = ms; durationMs = dur; }
		bool Expired( unsigned ms ) const { return ms - startMs >= durationMs; }
	};
} // namespace F3

#endif // F3_GAMEACTIONMOVETO_HPP_
