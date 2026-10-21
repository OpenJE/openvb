// Copyright 2024 OpenJE

#ifndef F3_COMBATMANAGER_HPP_
#define F3_COMBATMANAGER_HPP_

#include "types.hpp"

namespace F3 {
	struct SpawnedEntity;

	// F3::CombatManager - port of the combat/initiative manager (JE cls_0x5a9330,
	// world+0x8C). Structure + turn protocol from discoveries
	// session6-classid-readers / session12 / session14 and the raw 0x5A94A0 decompile:
	//   {+4 ROUND-MODE switch (NOT "paused" - see the mode note), +8 re-entry /
	//    list-maintaining, +C iterator, +10 accMs, +14 end-iter flag, +1C turn-list
	//    head, +20 count}
	// Per world frame (ServerUpdate step 3, BEFORE PathManager::ProcessJobs at 0x545E18):
	//   1. guarded list maintenance: nodes whose entity is state-5 get TurnPrep
	//      (vft+0x60 = 0x549090: writes e+0x300=dt, clears the +0x319 latch); any other
	//      state UNLINKS the node (entities leave the queue exactly one frame after
	//      leaving SPAWNED, session 14 §3).
	//   2. with count>0 and not paused(+4): per-node dispatch - entities holding the
	//      +0x319 latch get their deadline cleared (+0x29C=0) and are skipped this
	//      frame; otherwise, when (+0x1E4==0 || +0x314!=0) AT 0x5a95c1, the deadline is armed.
	//      T-039 settled the second term: +0x314 is the ELEMENT COUNT of the action deque at
	//      Entity+0x304 (push `++` at 0x54ea71; erase `--` at 0x54b637/0x54b672; zeroed in the
	//      ctor at 0x5515e6), and `Entity::virt_meth_0x4084c0` is literally
	//      `return *(this + 0x314) == 0`. So the term reads "HAS ACTIONS QUEUED", not "is busy"
	//      - a queued order FORCES the turn even when the +0x1E4 test is false. Our port derives
	//      that count from actions[] (SpawnedEntity::QueuedActionCount, T-039) because our
	//      actions[4] is the deque's first block only (deviation D-2).
	//      +0x1E4's MEANING is still open (DB size 4; its other readers gate per-map entity
	//      counts at 0x54bc3a), so mirror the test at 0x5a95c1 - do not name it after a guess.
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
	//
	// MODE, manager+0x04 (T-011, read at 0x5a951d and written at 0x5a936e/0x5a93ad). The old
	// label "paused" was wrong and is retired: `if (!mbr_0x4)` is the branch that pumps EVERY
	// listed entity with a plain ServerUpdate(dt) each frame (0x5a951d-0x5a954f), and a NON-ZERO
	// mode is what selects the one-entity-per-event round walk (latch-skip -> quantum -> gate ->
	// dispatch). 0 is therefore "free-run", not "stopped". The switch is meth_0x5a9330(this,mode),
	// which first walks the GLOBAL entity list (byte_70BE0C) calling each entity's round hook
	// (0x548ee0 / 0x548f30 - NOT ported) before storing +4 and clearing +14. So SetRoundMode()
	// below stores and clears only, and says so.
	//
	// WHO RAISES +4 IN THE ORIGINAL IS NOT ESTABLISHED. The DB records no nonzero writer of +4
	// and only two zero-stores of +14 (0x5a9371/0x5a93b0) plus one read (0x5a9575). The port
	// therefore STARTS in free-run mode and never raises the mode by itself: that is a routed
	// question (T-011 open item), not licence to invent a round timer, an initiative order or an
	// AP spend. Tests that want the round-walk branch call SetRoundMode() explicitly and say why.
	class CombatManager {
	public:
		// Defined in CombatManager.cpp: it registers this manager as the one attacks join through
		// AND installs the join seam. MapEntitySpawn.hpp explains why the join is a POINTER and not
		// a call - GameActionAttack.cpp is compiled into suites that do not link CombatManager.cpp,
		// and a direct call there is a link error in someone else's suite. The last manager
		// constructed owns both; the app has exactly one (world+0x8C), and a test that stacks
		// managers must say so out loud.
		CombatManager();

		// The shipped destructor is 0x5a9a10, and Flush's other xref (0x5a95f0) is how teardown
		// reaches the list. Freeing the nodes on teardown is therefore what the original does, and
		// this port additionally UNREGISTERS itself: a stack manager going out of scope in a test is
		// exactly how the join seam could resolve to freed memory, and a dangling seam would be a
		// crash that reads like a combat bug.
		~CombatManager();

		// The seam's target. A free function taking only the entity, because the pointer type cannot
		// carry a `this`; it resolves the active manager, so no manager means no join happens.
		static void JoinCombatThunk( SpawnedEntity* e );

		bool IsActive() const { return count_ > 0; }

		// 0x5490b0 Entity::JoinCombat + 0x5a9810 Combat::AddToList as ONE operation. The shipped
		// pair is atomic for a reason: AddToList inserts the node and THEN stores state 5
		// (meth_0x5507f0 at 0x5a987b), while the world loop skips state-5 entities at 0x545e8e -
		// so a half-join either double-pumps a listed entity or leaves a state-5 entity that
		// NOTHING pumps. AddToList also dedupes BEFORE inserting (0x5a9826-0x5a9841: walk the list,
		// and if a node's entity matches, return - no insert AND no state store), which is why a
		// second attack on the same actor can neither duplicate nor re-state it.
		// The shipped guard at 0x5490c3 THROWS "Invalid attempt to retrieve world server instance."
		// with no server context; this port logs once and refuses - no exception plumbing here, and
		// a silent skip would hide the very condition the throw exists to report.
		void JoinCombat( class SpawnedEntity* e );

		// 0x5a9440 Combat::Flush: unlink and free EVERY node, --count per node, and store state 4
		// (meth_0x5507f0 at 0x5a9485) for each entity as it goes. Leaving rounds is a list
		// operation with a state side effect - never a bare state store.
		void Flush();

		// 0x5a9330 Combat::SetMode: stores +4, clears +14, and (UNPORTED) the per-entity round
		// hooks over the global entity list. See the MODE note above for who may call this.
		void SetRoundMode( unsigned mode );
		unsigned RoundMode() const { return mode0x4_; }

		// The entity the round walk last dispatched (0x5a95de). PORT-SIDE bookkeeping: the original
		// hands that entity back as its return value, and there is no shipped field for it. Tests
		// and the digest read it so "who was acted on" is a fact from the manager, not an inference
		// from an hp change.
		class SpawnedEntity* LastDispatched() const { return dispatched_; }

		// 0x5A94A0, the whole of it: list maintenance (step 1) plus the dispatch walk (step 2).
		// `secondsPerRound` is the operand at 0x5a95aa: the shipped line is
		// `1000 * (world+0x40) + accMs`, and world+0x40 is the IMMUTABLE config
		// "Seconds Per Round" (default 6 at 0x546b5b, atol'd from [World] at 0x546c8f; writer
		// census closed by T-010d). So it is added ONCE PER DISPATCHED QUEUE EVENT - a round-based
		// virtual clock, never wall-ms and never a per-frame accumulation. The previous
		// `accMs_ += clockSeconds * 1000` line was the C-23 placeholder and is retired here
		// (discoveries/2025-10-20-canonical-fact-sheet.md §13).
		void Tick(int dtMs, unsigned secondsPerRound);

		void AddToTurns(  class SpawnedEntity* e );
		void RemoveFromTurns( class SpawnedEntity* e );
		int  TurnCount() const { return count_; }

		// T-026 digest seam: the queued actors' names IN QUEUE ORDER, comma separated, truncated
		// safely and always NUL-terminated; returns the number written. The node list itself stays
		// private - the digest needs the ORDER as a fact (who acts when), not the linked list as an
		// API that later code could mutate.
		int  TurnOrderSignature( char* buf, int cap ) const;

		unsigned AccMs() const { return accMs_; }
		bool     MidTurn() const { return maintaining_; }   // mbr_0x8c[+8] gate

	private:
		struct Node {
			Node* next;
			Node* prev;
			class SpawnedEntity* entity;
		};

		unsigned  mode0x4_;      // +4 0 = free-run every listed entity; != 0 = the round walk
		bool      maintaining_;  // +8 re-entry/list-maintaining (gates step-5 pump too)
		unsigned  accMs_;        // +10 round-clock ms
		bool      endIter_;      // +14 end-of-walk flag (read 0x5a9575; no nonzero writer found)
		class SpawnedEntity* dispatched_;   // port-side only, see LastDispatched()
		int       count_;        // +20
		Node*     head_;         // +1C (circular sentinel)
		Node*     iter_;         // +C
	};
	// The manager the action layer joins through. Shipped resolves it through the world-server
	// singleton (0x5490f4: (*(world[1].vft + 32))(world+1)); the port stores it in
	// MapEntitySpawn.cpp for the same link reason as the seam above, and CombatManager's ctor
	// registers it. A null manager is REPORTED at the call site, never silently skipped.
	CombatManager* GetActiveCombatManager();
	void SetActiveCombatManager( CombatManager* cm );

} // namespace F3

#endif // F3_COMBATMANAGER_HPP_
