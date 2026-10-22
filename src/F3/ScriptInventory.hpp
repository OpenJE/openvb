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

		// The shipped wildcard token is the string "*" (global byte_65DC00 in the original .rdata,
		// read from the image: 2A 00). An inline accessor, not a static const member: a header-only
		// struct in a C++98 static link emitted one definition per TU and the linker said so
		// (multiple definition of F3::ScriptInventory::SNCCI_WILDCARD) - which is the correct fix
		// telling, not a workaround.
		static const char *Wildcard() {
			return "*";
		} // Wildcard


		struct Entry {
			const char *pcName;
			void *pItem;
			// Weapon support for GetWeaponRange (0x45b620). Shipped reaches the range in TWO hops: the
			// item at the creature's active slot is RTTI-cast GameItem -> GameWeapon and read at +0x408
			// (bytes 8b 80 08 04 00 00, 0x45b727 - a POINTER), and that pointer is then dereferenced at
			// +0x2C (bytes 8b 40 2c, 0x45b740) for the actual range. pRangeRecord models the +0x408
			// target, so the two-hop shape survives instead of being flattened into a single int.
			int *pRangeRecord;
			char bWeapon;
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
				entries[ i ].pRangeRecord = 0;
				entries[ i ].bWeapon = 0;
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
				entries[ nCount ].pRangeRecord = 0;
				entries[ nCount ].bWeapon = 0;
				++nCount;
			}
		} // Add

		// A weapon, with the record its +0x408 pointer would address (range lives at +0x2C of it).
		void AddWeapon( const char *pcName, void *pItem, int *pRangeRecord ) {
			if ( nCount < SNCCI_MAX_ITEMS ) {
				entries[ nCount ].pcName = pcName;
				entries[ nCount ].pItem = pItem;
				entries[ nCount ].pRangeRecord = pRangeRecord;
				entries[ nCount ].bWeapon = 1;
				++nCount;
			}
		} // AddWeapon

		// 0x4693d0 - the equipped cell at a slot, RTTI'd to GameItem, returning 0 if it is not one.
		// DEVIATION, named because it is a real difference: the shipped accessor reads
		// `*(this + 0x14 + slot*4)` with NO bound check at all (0x4693d4), so slot 6 or 9 inspects
		// whatever follows the six-cell array. The port bounds it, because unbounded reads of
		// neighboring port members would be a different - and less reproducible - wrong answer.
		const Entry *ItemAtSlot( int nSlot ) const {
			if ( nSlot < 0 || nSlot >= SNCCI_EQUIPPED_SLOTS ) {
				return 0;
			}
			for ( int i = 0; i < nCount; ++i ) {
				if ( entries[ i ].pItem == equipped[ nSlot ] && equipped[ nSlot ] != 0 ) {
					return &entries[ i ];
				}
			}
			return 0;
		} // ItemAtSlot


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

		// UnequipItem's two paths do NOT share a comparison, and that is shipped behavior worth the
		// trouble of modelling: the named-slot path uses strcmp (CASE-SENSITIVE, 0x45a8e7) while the
		// slot-7 "everywhere" path goes through meth_0x46a6c0, whose matcher is _stricmp (0x46a777).
		// So "Hunting Rifle" unequips from slot 0 but "HUNTING RIFLE" does not - while slot 7 accepts
		// either spelling. Porting one comparison for both paths would be a silent divergence.
		//
		// The shipped wildcard is the string "*" (global byte_65DC00, read from .rdata of the original
		// image: 2A 00), matched against the REQUESTED name at 0x45a932. Note what it does NOT do:
		// the wildcard branch clears the slot and then falls through to `return 0`, while the exact-name
		// branch returns 1. Unequipping by wildcard therefore SUCCEEDS AND REPORTS FAILURE - preserved
		// below because it is observable, not because it is sensible.
		int UnequipNamed( const char *pcName, int nSlot ) {
			if ( nSlot != 7 ) {
				if ( nSlot < 0 || nSlot >= SNCCI_EQUIPPED_SLOTS ) {
					return 0;
				}
				void *pItem = equipped[ nSlot ];
				if ( pItem == 0 ) {
					return 0;
				}
				const char *pcHave = NameOfItem( pItem );
				if ( pcHave && strcmp( pcHave, pcName ) == 0 ) {   // 0x45a8e7: case-SENSITIVE
					equipped[ nSlot ] = 0;
					return 1;
				}
				if ( strcmp( pcName, Wildcard() ) == 0 ) {     // 0x45a932: byte_65DC00 == "*"
					equipped[ nSlot ] = 0;
					return 0;   // shipped: cleared, but the native answers 0
				}
				return 0;
			}
			int nCleared = 0;
			for ( int i = 0; i < nCount; ++i ) {
				if ( entries[ i ].pcName == 0 || _stricmp( entries[ i ].pcName, pcName ) != 0 ) {
					continue;   // 0x46a777 path: case-INSENSITIVE
				}
				for ( int s = 0; s < SNCCI_EQUIPPED_SLOTS; ++s ) {
					if ( equipped[ s ] == entries[ i ].pItem ) {
						equipped[ s ] = 0;
						++nCleared;
					}
				}
			}
			return nCleared ? 1 : 0;
		} // UnequipNamed

		const char *NameOfItem( const void *pItem ) const {
			for ( int i = 0; i < nCount; ++i ) {
				if ( entries[ i ].pItem == pItem ) {
					return entries[ i ].pcName;
				}
			}
			return 0;
		} // NameOfItem

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
