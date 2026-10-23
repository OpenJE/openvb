// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONMOVETO_HPP_
#define F3_GAMEACTIONMOVETO_HPP_

#include "F3.hpp"
#include "JE/ActionMoveTo.hpp"
#include "F3/MapEntitySpawn.hpp"

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
		GameActionMoveTo() : speed(8.0f), lastRequestKind(0), lastRequestA0(0), lastRequestA4(0), moverGate0x90(0) {}   // 8.0f = GE+0x94 ctor default (candidate source)

		// JE hook (secondary vft +0x14): the original routes it into the F3
		// OnServerUpdate_0x4060b0 through the entity's action loop; ms = entity ServerUpdate
		// timestamp (v16 world time in the original).
		virtual void OnMoveServerUpdate( unsigned ms ) { OnServerUpdate_0x4060b0( ms ); }

		// Movement speed source. The un-named GE+0x94 (ctor 8.0f, discovery session 1 §2 —
		// "needs first consumer") is our leading candidate for the entity's move speed; the
		// JE::Entity-side consumer is unconfirmed, so F3 carries an explicit field until the
		// first read site proves the mapping (then it becomes a mover+0x94 load).
		float speed;
		// Port of slot 7 (0x405720): RTTI-casts the action's mover to GameEntity and returns
		// [mover+0x94], falling back to 10.0 when non-positive (single GE-based +0x94 reader;
		// discoveries ge94-speed doc §1). Our mover is always SpawnedEntity in this lane.
		virtual float GetMoveSpeed() {
			if ( mover ) {
				float v = static_cast<SpawnedEntity*>( mover )->moveSpeed0x94;
				if ( !( v <= 0.0f ) ) {   // NaN-safe like the original's `if <=0`
					return v;
				}
				return 10.0f;
			}
			return speed;   // no mover (unit tests driving bare actions): explicit field
		}

		// OnServerUpdate vft slot on the entity-facing interface; called by the world pump
		// after the JE movement update.
		virtual int OnServerUpdate_0x4060b0( unsigned ms );

		// ---- batch C rows: the four entity-facing vft slots GameActionMoveTo adds over
		// ActionMoveTo (derived vft 0x649c1c cells +0x20/+0x24/+0x28/+0x2C), ported per
		// docs/f3/GameActionMoveToEvidence.md (all bodies read whole, raw bytes).
		// Destructor cells need no port-side code: 0x405710 is vft-restore + tail-jmp to
		// ActionMoveTo::dtor_0x55b460 (compiler's job here), 0x4058b0 is the scalar deleting
		// dtor (call dtor; flag&1 -> free) = operator delete semantics. Whole-body read
		// found NO own-side frees in either.
		//
		// Twin forwarders (0x405770/0x405790): shipped bodies are byte-identical except the
		// kind word - call [mover+0x14]'s vft slot 18 (+0x48) as (kind, arg0, 0, arg4),
		// retn 8, with NO null test (B8-family unguarded deref @0x405777). The mover-side
		// resolver (GameEntity [vft+0x48] = 0x467ae0 thunk -> 0x467830: anim-pair fill via
		// [mover+0xC0] set + AnimMeshInstance::0x467780 hit-test + dword_6F3960 translate
		// loop, kinds read as anim request ids) is NOT ported (Gfx anim era), so the port
		// records the request as an observable on the ACTION instead of faking the resolve.
		// STATUS: the forwarder CALL is a named gap; the recording fields are port-side
		// observability, NOT shipped behavior parity. Kinds 0x1B2/0x1A0 stay opaque (law 1).
		virtual void MoverRequest_0x405770( unsigned a0, unsigned a4 ) {
			lastRequestKind = 0x1B2;   // 434
			lastRequestA0 = a0;
			lastRequestA4 = a4;
		}
		virtual void MoverRequest_0x405790( unsigned a0, unsigned a4 ) {
			lastRequestKind = 0x1A0;   // 416
			lastRequestA0 = a0;
			lastRequestA4 = a4;
		}
		int lastRequestKind;   // 0 = never; else 0x1B2/0x1A0
		unsigned lastRequestA0, lastRequestA4;

		// Slot 10 (0x4057b0): RTTI Entity->GameEntity cast of the mover with SILENT return
		// on failure (null is the testable failure arm); stamps the server clock
		// (0x548f90 -> Entity+0x184, store @0x548fb8/0x548fa2); drains the mover ring
		// (0x54bd90 over [+0x1B0 size, +0x1B4 cursor, +0x1B8 count]) - RING UNMODELED in the
		// port (no SpawnedEntity +0x1B0 trio), drain is a cited no-op; co-caller of the same
		// drain is GameActionDie::virt_meth_0x404e90 @0x404ed4. Then queues anim 29 when
		// mover GE+0x90 == 1, else anim 32 (QueueAnimEvent_0x4673d0 seam =
		// SpawnedEntity::queuedAnimEvent record-field, precedent MapEntitySpawn.cpp:745).
		// The +0x90 gate is mirrored on the action (SpawnedEntity has no +0x90 field yet);
		// same "explicit field until the first read site proves the mapping" convention as
		// `speed` above; retires with the GameEntity-side field.
		virtual void OnAnimChoice_0x4057b0( unsigned ms );

		// Slot 11 (0x406780): cast (silent fail) + clock stamp + QueueAnimEvent(1, 0).
		// Whole-body contrast vs slot 10, byte-verified: NO ring drain, NO +0x90 gate.
		virtual void OnAnimStamp1_0x406780( unsigned ms );

		int moverGate0x90;   // port mirror of GE+0x90 (see slot-10 note); drives 29 vs 32

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
