// Copyright 2024 OpenJE

#ifndef F3_SCRIPTINVENTORY_HPP_
#define F3_SCRIPTINVENTORY_HPP_

#include "F3.hpp"

#include <string.h>

namespace F3 {

	// PORT-SIDE INVENTORY MODEL - a declared stand-in, not a reversal of GameInventory.
	//
	// Why this exists at all: three script natives reach the inventory through `[entity + 0x168]` and then an
	// RTTI attempt `Inventory -> GameInventory`, and the semantics the natives actually depend on are narrow
	// enough to model honestly. Everything below is the part the BYTES prove; the rest of GameInventory
	// (item counts, weight, per-slot rules, the RB-tree the shipped matcher pushes into) is out of scope and
	// says so rather than inventing it.
	//
	// Proven shape, cited:
	//   - the equipped array is SIX slots at GameInventory+0x14:
	//     GameInventory::virt_meth_0x46a860 (vft slot 10, called at 0x46a7a9 `call dword ptr [eax+28h]`)
	//     walks `*(&this->mbr_0x14 + v3) == a2` with `while (v3 < 6)` (0x46a8cc) and returns the match flag.
	//     So "equipped" is membership in a six-pointer array, NOT a per-item boolean.
	//   - name matching is CASE-INSENSITIVE: the matcher GameInventory::meth_0x46a6c0 compares with
	//     `_stricmp` at 0x46a777. Item names differing only in case are the same query.
	//   - the matcher scans the WHOLE item list and keeps every hit, so "any match is equipped" is the answer
	//     a native sees; duplicate names with different equip state still return 1.
	//
	// Deviations, named: the item list has a fixed capacity (shipped walks a linked list with no cap), and the
	// shipped `Inventory -> GameInventory` cast is represented by bIsGameInventory because the port has no
	// two-level inventory hierarchy - without that flag the native's fifth failure message
	// ("... - Game Inventory not Found") would be unreachable and silently untestable.
	struct ScriptInventory {
		enum {
			// SIX is not a style choice: 0x46a8cc bounds the equipped scan with `v3 < 6`.
			SNCCI_EQUIPPED_SLOTS = 6,
			SNCCI_MAX_ITEMS = 32       // port capacity; shipped list is unbounded
		};

		struct Entry {
			const char *pcName;
			void *pItem;
		};

		Entry entries[ SNCCI_MAX_ITEMS ];
		int nCount;
		void *equipped[ SNCCI_EQUIPPED_SLOTS ];
		char bIsGameInventory;   // models the RTTI cast succeeding; see Deviations above

		ScriptInventory() {
			Reset();
		}

		void Reset() {
			for ( int i = 0; i < SNCCI_MAX_ITEMS; ++i ) {
				entries[ i ].pcName = 0;
				entries[ i ].pItem = 0;
			}
			for ( int s = 0; s < SNCCI_EQUIPPED_SLOTS; ++s ) {
				equipped[ s ] = 0;
			}
			nCount = 0;
			bIsGameInventory = 1;
		} // Reset

		void Add( const char *pcName, void *pItem ) {
			if ( nCount < SNCCI_MAX_ITEMS ) {
				entries[ nCount ].pcName = pcName;
				entries[ nCount ].pItem = pItem;
				++nCount;
			}
		} // Add

		// Slot bounds follow the shipped constant: an out-of-range slot is stored nowhere, because the
		// shipped scan could never observe it either.
		void Equip( int nSlot, void *pItem ) {
			if ( nSlot >= 0 && nSlot < SNCCI_EQUIPPED_SLOTS ) {
				equipped[ nSlot ] = pItem;
			}
		} // Equip

		int IsEquippedSlot( int nSlot ) const {
			if ( nSlot < 0 || nSlot >= SNCCI_EQUIPPED_SLOTS ) {
				return 0;
			}
			return equipped[ nSlot ] != 0;
		} // IsEquippedSlot

		void Unequip( int nSlot ) {
			if ( nSlot >= 0 && nSlot < SNCCI_EQUIPPED_SLOTS ) {
				equipped[ nSlot ] = 0;
			}
		} // Unequip

		// 0x46a777 semantics: case-insensitive name match, then 0x46a860's six-slot membership test.
		// Returns 1 if ANY item whose name matches is present in the equipped array.
		int FindEquipped( const char *pcName ) const {
			for ( int i = 0; i < nCount; ++i ) {
				if ( entries[ i ].pcName == 0 ) {
					continue;
				}
				if ( _stricmp( entries[ i ].pcName, pcName ) != 0 ) {
					continue;
				}
				for ( int s = 0; s < SNCCI_EQUIPPED_SLOTS; ++s ) {   // 0x46a8cc `while (v3 < 6)`
					if ( equipped[ s ] == entries[ i ].pItem ) {
						return 1;
					}
				}
			}
			return 0;
		} // FindEquipped

		int FindAny( const char *pcName ) const {
			for ( int i = 0; i < nCount; ++i ) {
				if ( entries[ i ].pcName && _stricmp( entries[ i ].pcName, pcName ) == 0 ) {
					return 1;
				}
			}
			return 0;
		} // FindAny
	}; // ScriptInventory

} // namespace F3

#endif // F3_SCRIPTINVENTORY_HPP_
