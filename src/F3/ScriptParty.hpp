// Copyright 2024 OpenJE

#ifndef F3_SCRIPTPARTY_HPP_
#define F3_SCRIPTPARTY_HPP_

#include "F3.hpp"
#include "JE/Entity.hpp"

namespace F3 {

	// PORT-SIDE PARTY MODEL - a declared stand-in for GameParty, in the same shape as ScriptInventory.
	//
	// Two script natives reach a party and nothing else about it, and both need exactly two facts:
	//   - the LEADER, read at GameParty+0x14 (Protagonist 0x45a9c7 `mov eax,[eax+14h]`) and then
	//     dereferenced at Entity+0x1DC for its net handle (0x45a9e0 `mov eax,[eax+1DCch]' equivalent,
	//     bytes 8b 80 dc 01 00 00) - the same field Create stamps and SetHP resolves through.
	//   - an ADD operation, which is GameParty VFT SLOT 0 called with the resolved entity as its single
	//     argument (AddCNPCToParty 0x45a370: mov edx,[eax]; push entity; mov ecx,eax; call [edx]).
	//
	// Everything else about the party - recruitment rules, reputation, per-member state, the server
	// object the ctor stamps at GameEntity+0x28 - is out of scope and unmodeled, so Add below answers
	// "was there room and was it not already a member", which is the most that can be claimed from
	// these two call sites. Do not read capacity or duplicate policy into it as fact.
	struct ScriptParty {
		enum { SNCP_MAX_MEMBERS = 16 };   // port capacity; shipped bound is not observable from here

		// Models the shipped `Party -> GameParty` RTTI attempt succeeding. The port has no two-level
		// party hierarchy, and without this flag both natives' party-failure messages would be
		// unreachable - a dead string in a port that claims to preserve shipped text.
		char bIsGameParty;
		JE::Entity *pLeader;
		JE::Entity *members[ SNCP_MAX_MEMBERS ];
		int nCount;

		ScriptParty() {
			Reset();
		}

		void Reset() {
			bIsGameParty = 1;
			pLeader = 0;
			for ( int i = 0; i < SNCP_MAX_MEMBERS; ++i ) {
				members[ i ] = 0;
			}
			nCount = 0;
		} // Reset

		// GameParty vft slot 0, as the native uses it. 1 = added, 0 = already a member or no room.
		int Add( JE::Entity *pEntity ) {
			if ( pEntity == 0 ) {
				return 0;
			}
			for ( int i = 0; i < nCount; ++i ) {
				if ( members[ i ] == pEntity ) {
					return 0;   // already in the party
				}
			}
			if ( nCount >= SNCP_MAX_MEMBERS ) {
				return 0;
			}
			members[ nCount++ ] = pEntity;
			if ( pLeader == 0 ) {
				pLeader = pEntity;   // port convenience ONLY: nothing here proves the first member leads
			}
			return 1;
		} // Add

		int IsMember( const JE::Entity *pEntity ) const {
			for ( int i = 0; i < nCount; ++i ) {
				if ( members[ i ] == pEntity ) {
					return 1;
				}
			}
			return 0;
		} // IsMember
	}; // ScriptParty

} // namespace F3

#endif // F3_SCRIPTPARTY_HPP_
