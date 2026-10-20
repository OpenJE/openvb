// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include "F3/CombatManager.hpp"
#include "F3/MapEntitySpawn.hpp"

namespace F3 {
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

	// 0x5A94A0 structural port. Turn DISPATCH deliberately not invented here (C-23
	// cadence source unresolved); maintenance + accMs accounting are faithful.
	void CombatManager::Tick( int dtMs, unsigned clockSeconds ) {
		tracing::instrument( tracing::LOCATION, "dt=%d clock=%u", dtMs, clockSeconds );

		// accMs accumulation: documented reimpl clock source (Gregorian total seconds);
		// original line 0x5A95AA multiplies an artifact-y getter - pending lockstep (C-23).
		accMs_ += clockSeconds * 1000u;

		if ( maintaining_ ) {
			return;   // re-entrancy latch (mbr_0x8)
		}
		if ( count_ == 0 ) {
			return;
		}
		maintaining_ = true;

		// list maintenance pass (0x5A94A3-0x5A9511): state-5 nodes get TurnPrep
		// (0x549090: e+0x300 = dt; e+0x319 = 0), everyone else unlinks.
		Node* cur = head_;
		if ( cur ) {
			Node* const stop = head_;
			for (;;) {
				Node* next = cur->next;
				SpawnedEntity* e = cur->entity;
				if ( e && e->entityState == kStateSpawned ) {
					e->frameDt0x300 = dtMs;      // vft+96(+0x60) turn-prep writes
					e->actedLatch0x319 = 0;
				} else {
					RemoveFromTurns( e );        // unlink + count-- (0x5A94D1-0x5A94EF)
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

		// 0x5A9508: iterator refresh + latch release
		iter_ = head_;
		maintaining_ = false;
	}
} // namespace F3
