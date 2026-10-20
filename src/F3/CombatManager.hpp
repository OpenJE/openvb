// Copyright 2024 OpenJE

#ifndef F3_COMBATMANAGER_HPP_
#define F3_COMBATMANAGER_HPP_

#include "types.hpp"

namespace F3 {
	struct SpawnedEntity;

	// F3::CombatManager - port of the combat/initiative manager (JE cls_0x5a9330,
	// world+0x8C). Structure + turn protocol from discoveries
	// session6-classid-readers / session12 / session14 and the raw 0x5A94A0 decompile:
	//   {+4 paused, +8 re-entry/list-maintaining, +C iterator, +10 accMs,
	//    +14 end-iter flag, +1C turn-list head, +20 count}
	// Per world frame (ServerUpdate step 3, BEFORE PathManager::ProcessJobs at 0x545E18):
	//   1. guarded list maintenance: nodes whose entity is state-5 get TurnPrep
	//      (vft+0x60 = 0x549090: writes e+0x300=dt, clears the +0x319 latch); any other
	//      state UNLINKS the node (entities leave the queue exactly one frame after
	//      leaving SPAWNED, session 14 §3).
	//   2. with count>0 and not paused(+4): per-node dispatch - entities holding the
	//      +0x319 latch get their deadline cleared (+0x29C=0) and are skipped this
	//      frame; otherwise, when (+0x1E4==0 || +0x314!=0), the deadline is armed
	//      (accMs/1000) and the entity receives its turn update F3::ServerUpdate(dt+accMs).
	//
	// CLOCK-CAVEAT (chat C-23): the original's accMs line multiplies an IWorldServer
	// slot-0 call that decompiles to a pointer getter - a confirmed artifact. This port
	// feeds accMs from JE::Gregorian::TotalSeconds() (1000x) as the DOCUMENTED reimpl
	// source until lockstep settles the original's true value.
	//
	// Combat-mode gate: while the +8 maintaining flag pattern implies activity, the
	// shipped World::ServerUpdate_0x545d60 step-5 entity loop runs ONLY when
	// *(mbr_0x8c+8)==0 (0x545E23) - mirrored via IsActive() gating the F3 pump hook.
	class CombatManager {
	public:
		CombatManager() : paused_(false), maintaining_(false), accMs_(0),
		                  count_(0), head_(0), iter_(0) {}

		bool IsActive() const { return count_ > 0; }

		// Structure-level port of 0x5A94A0 step 1 (list maintenance + TurnPrep).
		// Turn DISPATCH (step 2) stays structural until the cadence source is settled.
		void Tick(int dtMs, unsigned clockSeconds);

		void AddToTurns(  class SpawnedEntity* e );
		void RemoveFromTurns( class SpawnedEntity* e );
		int  TurnCount() const { return count_; }

		unsigned AccMs() const { return accMs_; }
		bool     MidTurn() const { return maintaining_; }   // mbr_0x8c[+8] gate

	private:
		struct Node {
			Node* next;
			Node* prev;
			class SpawnedEntity* entity;
		};

		bool      paused_;       // +4
		bool      maintaining_;  // +8 re-entry/list-maintaining (gates step-5 pump too)
		unsigned  accMs_;        // +10
		bool      endIter_;      // +14
		int       count_;        // +20
		Node*     head_;         // +1C (circular sentinel)
		Node*     iter_;         // +C
	};
} // namespace F3

#endif // F3_COMBATMANAGER_HPP_
