// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>   // TurnOrderSignature: memcpy/strlen
#include "F3/CombatManager.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/HostHooks.hpp"
#include "JE.hpp"   // JE::LogDebugString

namespace F3 {
	// Registration. Last manager constructed owns the join seam and the active-manager pointer;
	// the app has exactly one (world+0x8C). MapEntitySpawn.cpp stores both, because it is the one
	// game-layer TU present in every suite that compiles GameActionAttack.cpp - see the seam's
	// declaration there for the link failure this arrangement exists to avoid.
	CombatManager::CombatManager() : mode0x4_(0), maintaining_(false), accMs_(0),
	                                 endIter_(false), dispatched_(0), count_(0),
	                                 head_(0), iter_(0) {
		SetActiveCombatManager( this );
		SetCombatJoinSeam( &CombatManager::JoinCombatThunk );
	}

	CombatManager::~CombatManager() {
		Flush();				   // 0x5a9440 via 0x5a9a10: unlink every node, ChangeState(4) each
		if ( GetActiveCombatManager() == this ) {
			SetActiveCombatManager( 0 );
			SetCombatJoinSeam( 0 );
		}
	}

	void CombatManager::JoinCombatThunk( SpawnedEntity* e ) {
		CombatManager* cm = GetActiveCombatManager();
		if ( cm ) {
			cm->JoinCombat( e );   // dedupe + insert + ChangeState(5), all inside - one operation
		}
	}

	void CombatManager::AddToTurns( SpawnedEntity* e ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( !e ) {
			return;
		}
		Node* n = (Node*)calloc( 1, sizeof( Node ) );
		if ( !n ) {
			return;
		}
		n->entity = e;
		if ( !head_ ) {
			head_ = n;
			n->next = n->prev = n;
			iter_ = n;
		} else {
			n->next = head_;
			n->prev = head_->prev;
			head_->prev->next = n;
			head_->prev = n;
		}
		++count_;
	}

	void CombatManager::RemoveFromTurns( SpawnedEntity* e ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( !head_ ) {
			return;
		}
		Node* cur = head_;
		do {
			if ( cur->entity == e ) {
				if ( cur->next == cur ) {
					head_ = 0;
					iter_ = 0;
				} else {
					cur->prev->next = cur->next;
					cur->next->prev = cur->prev;
					if ( head_ == cur ) {
						head_ = cur->next;
					}
					if ( iter_ == cur ) {
						iter_ = cur->next;
					}
				}
				free( cur );
				--count_;
				return;
			}
			cur = cur->next;
		} while ( cur != head_ );
	}

	// 0x5A94A0 - the whole routine, not just its first step. Lines cited individually below.
	// `secondsPerRound` is the value the shipped line at 0x5a95aa multiplies by 1000 (world+0x40,
	// the immutable [World] Seconds Per Round config): it is added to the +10 accumulator ONCE PER
	// DISPATCHED QUEUE EVENT, which is what makes the manager clock round-based rather than wall-
	// based, and why it is a parameter and not a read of JE::Gregorian here.
	void CombatManager::Tick( int dtMs, unsigned secondsPerRound ) {
		tracing::instrument( tracing::LOCATION, "dt=%d spr=%u", dtMs, secondsPerRound );

		// 0x5a94a3: the maintenance pass runs only when we are not ALREADY walking the list, and
		// it sets +8 for the rest of this call. +8 is the flag the world's step-5 entity loop
		// reads (0x545e23), so combatants are not also pumped by the generic loop.
		if ( !maintaining_ ) {
			maintaining_ = true;             // 0x5a94ab
			// 0x5a94af-0x5a94f2: state-5 nodes get TurnPrep, every OTHER state unlinks its node
			// (entities leave the queue exactly one frame after leaving SPAWNED, session 14 §3).
			Node* cur = head_;
			if ( cur ) {
				Node* const stop = head_;
				for (;;) {
					Node* next = cur->next;
					SpawnedEntity* e = cur->entity;
					if ( e && e->entityState == kStateSpawned ) {
						e->frameDt0x300 = dtMs;      // vft+96 TurnPrep (0x549090) writes +0x300...
						e->actedLatch0x319 = 0;      // ...and clears the +0x319 acted latch
					} else {
						RemoveFromTurns( e );        // 0x5a94d1-0x5a94ef: unlink + count--
						if ( count_ == 0 ) {
							break;
						}
					}
					cur = next;
					if ( cur == stop ) {
						break;
					}
				}
			}
			// 0x5a9508-0x5a9511: iterator refresh to the first node after the sentinel.
			iter_ = head_;
		}

		// 0x5a9514-0x5a951b: nothing listed => release +8 and out.
		if ( count_ == 0 ) {
			maintaining_ = false;   // 0x5a9552
			return;
		}

		// 0x5a951d: the MODE test. +4 == 0 pumps EVERY listed entity with a plain ServerUpdate(dt)
		// - free-run, not "paused". The turn machinery below is the +4 != 0 branch.
		if ( mode0x4_ == 0 ) {
			if ( iter_ ) {
				Node* const stop = head_;
				for (;;) {
					// 0x5a953b: F3::ServerUpdate(iter->entity, param_1) - the world frame ms,
					// with NO accMs added: the free-run branch does not consume the round clock.
					F3::TickEntityServerUpdate( iter_->entity, dtMs );
					iter_ = iter_->next;          // 0x5a9543-0x5a9545
					if ( iter_ == stop || !iter_ ) {
						break;                    // 0x5a954f
					}
				}
			}
			maintaining_ = false;                 // 0x5a9552 (LABEL_19)
			return;
		}

		// ---- the round walk: AT MOST ONE entity is dispatched per call (0x5a955a-0x5a95de) ----
		if ( !iter_ ) {
			iter_ = head_;
		}
		SpawnedEntity* e = iter_ ? iter_->entity : 0;
		if ( !e ) {
			maintaining_ = false;
			return;
		}

		// `v9` at 0x5a9517 starts as ZERO, and 0x5a95ac writes it back unconditionally. That is
		// the quirk this port must not "fix": taking the LATCH path leaves the accumulator at 0,
		// i.e. skipping an entity that already acted RESETS the round clock.
		unsigned acc = 0;
		if ( e->actedLatch0x319 != 0 ) {
			// 0x5a955d: +0x319 set => this entity already acted. It gets its deadline cleared
			// (0x5a9567 writes +0x29C = 0 - the explicit-time domain of step 4: the round clock is
			// NOT consulted for this event) and is skipped for this call.
			e->turnDeadline0x29C = 0;
			iter_ = iter_->next;                                   // 0x5a9572
			if ( endIter_ ) {
				// 0x5a9575-0x5a957f: SetMode(0) and RETURN - and note that this return path does
				// NOT clear +8, so the world's step-5 loop stays suppressed after a mode switch.
				SetRoundMode( 0 );
				return;
			}
			if ( iter_ == head_ ) {
				maintaining_ = false;                              // 0x5a958f -> LABEL_19
				return;
			}
			e = iter_->entity;                                     // 0x5a9591
		} else {
			// 0x5a95aa: accMs += 1000 * (world +0x40) - the quantum, once, for THIS queue event.
			acc = accMs_ + 1000u * secondsPerRound;
		}
		accMs_ = acc;                                              // 0x5a95ac

		// 0x5a95c1 gate, T-039's reading stated plainly: +0x314 is the queued-action COUNT of the
		// entity's own action deque, so the term is "HAS ACTIONS QUEUED" - a pending order FORCES
		// the turn even when +0x1E4 is set. +0x1E4's own meaning is still open (T-041); the test
		// is mirrored as shipped, which is the only thing the port is entitled to reproduce.
		if ( e->flag0x1E4 == 0 || e->QueuedActionCount() != 0u ) {
			// 0x5a95ce: arm the deadline in SECONDS from the round accumulator...
			e->turnDeadline0x29C = (int)( accMs_ / 1000u );
			// 0x5a95de: ...and hand the entity its turn on `dt + accMs`. The round clock and the
			// frame ms are added only at this boundary, which is the ONE place the two domains
			// meet - never mix them anywhere else (T-011 step 4).
			dispatched_ = e;
			F3::TickEntityServerUpdate( e, dtMs + (int)accMs_ );
		}
		// 0x5a9551: the dispatch path DOES release +8 (only the SetRoundMode return above keeps
		// it set), so the generic entity loop runs again the same frame - for entities whose state
		// is not 5. A combatant is not double-ticked because the world loop skips state 5 at
		// 0x545e8e, not because anything here remembers it.
		maintaining_ = false;
	}

	// 0x5490b0 -> 0x5a9810. One operation, in the shipped order: DEDUPE, insert, then store 5.
	void CombatManager::JoinCombat( SpawnedEntity* e ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( !e ) {
			return;
		}
		// 0x5490c3: shipped THROWS "Invalid attempt to retrieve world server instance." unless the
		// AreaMapIcon guard bit is set. There is no exception plumbing on this path in the port, so
		// the condition is reported and the join refused - a silent skip would hide exactly the
		// case the shipped throw exists to announce.
		if ( !JE::Host::IsServerContext() ) {
			static bool warned = false;
			if ( !warned ) {
				warned = true;
				JE::LogDebugString( NULL,
				    "Invalid attempt to retrieve world server instance.\r\n" );   // 0x5490d7 verbatim
			}
			return;
		}
		// 0x5a9813-0x5a9841 AddToList: walk the circular list and RETURN on a match - no insert,
		// and crucially NO state store. That is what makes a second attack on the same actor a
		// no-op instead of a duplicate node.
		if ( head_ ) {
			Node* cur = head_;
			do {
				if ( cur->entity == e ) {
					return;
				}
				cur = cur->next;
			} while ( cur && cur != head_ );
		}
		AddToTurns( e );                       // 0x5a9843-0x5a9871 node insert
		// 0x5a987b: JE::Entity::meth_0x5507f0(e, 5) - ChangeState(5), legal only from 4. If the
		// entity is not in a state that may enter rounds, the shipped code throws; ChangeEntityState
		// logs the same string and refuses, and THIS caller must not leave the node in the list
		// behind a state that was never stored - that is the half-join the atomicity prevents.
		if ( !ChangeEntityState( e, kStateSpawned ) ) {
			RemoveFromTurns( e );
			return;
		}
		// 0x5a9868 cls_0x401b00::meth_0x401dd0(manager+0x18, 1) and the world-server call at
		// 0x5a9893: the deferred GUI/net refresh. CITED AND ABSENT, the same call the enqueue seam
		// already refuses to fake (docs/f3/EntityActionQueueSeam.md).
	}

	// 0x5a9440 Combat::Flush. Callers found: 0x5a95f0 (via sub_546870 - the world/round setup)
	// and the manager's own destructor 0x5a9a10.
	void CombatManager::Flush() {
		tracing::instrument( tracing::LOCATION, "" );
		while ( head_ ) {
			Node* n = head_;
			SpawnedEntity* e = n->entity;
			// 0x5a945e-0x5a9483: unlink this node first (splice + free + count--), THEN the state
			// store - the order matters because the store can itself reach back into the list.
			if ( n->next == n ) {
				head_ = 0;
				iter_ = 0;
			} else {
				n->prev->next = n->next;
				n->next->prev = n->prev;
				if ( head_ == n ) {
					head_ = n->next;
				}
				if ( iter_ == n ) {
					iter_ = n->next;
				}
			}
			free( n );
			--count_;
			if ( e ) {
				ChangeEntityState( e, kStateActive );   // 0x5a9485: ChangeState(4)
			}
		}
	}

	// 0x5a9330 Combat::SetMode. Stores +4 and clears +14 - and NOTHING else: the shipped function
	// first walks the global entity list (byte_70BE0C) calling meth_0x548ee0 / meth_0x548f30 per
	// entity (0x548ee0's gate at +0x296, 0x548f30's at +0x438), and those per-entity round hooks
	// are not ported. Cited and absent, so this is NOT a silent partial port.
	void CombatManager::SetRoundMode( unsigned mode ) {
		tracing::instrument( tracing::LOCATION, "mode=%u", mode );
		mode0x4_ = mode;      // 0x5a936e / 0x5a93ad
		endIter_ = false;     // 0x5a9371 / 0x5a93b0
	}

	// Traverses the circular turn list exactly the way Tick does (start at head_, follow next
	// until back to head_), so the signature reports the order the pump will actually act in, not
	// an insertion order the pump never sees. Empty list -> empty string and 0 written.
	int CombatManager::TurnOrderSignature( char* buf, int cap ) const {
		if ( !buf || cap <= 0 ) { return 0; }
		buf[0] = 0;
		if ( !head_ ) { return 0; }
		int written = 0, used = 0;
		Node* cur = head_;
		do {
			const char* name = ( cur->entity && cur->entity->name[0] ) ? cur->entity->name : "-";
			if ( used > 0 && used < cap - 1 ) { buf[used++] = ','; buf[used] = 0; }
			int len = (int)strlen( name );
			if ( used + len > cap - 1 ) { len = cap - 1 - used; }
			if ( len > 0 ) { memcpy( buf + used, name, (size_t)len ); used += len; buf[used] = 0; }
			++written;
			cur = cur->next;
		} while ( cur && cur != head_ );
		return written;
	}

} // namespace F3
