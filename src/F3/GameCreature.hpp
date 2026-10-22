// Copyright 2024 OpenJE

#ifndef F3_GAMECREATURE_HPP_
#define F3_GAMECREATURE_HPP_

#include "F3.hpp"
#include "F3/GameEntity.hpp"

namespace F3 {
	struct GameCreature : public GameEntity {
		//: Members
			dword mbr_0x114;
			dword mbr_0x25c;
			dword mbr_0x260;
			dword mbr_0x264;
			dword mbr_0x268;
			dword mbr_0x26c;
			dword mbr_0x270;
			dword mbr_0x274;
			byte mbr_0x27c;
			dword mbr_0x28c;
			dword mbr_0x290;
			dword mbr_0x294;
			byte mbr_0x29c;
			dword mbr_0x2ac;
			dword mbr_0x2b0;
			dword mbr_0x2b4;
			byte mbr_0x2b8;
			dword mbr_0x2bc;
			dword mbr_0x4;
			dword mbr_0x40c;
			dword mbr_0x428;
			dword mbr_0x6f0;
			dword mbr_0x6f4;
			dword mbr_0x6f8;
			dword mbr_0x6fc;
			dword mbr_0x700;
			dword mbr_0x704;
			dword mbr_0x708;
			dword mbr_0x70c;
			dword mbr_0x710;
			dword mbr_0x730;
			dword mbr_0x748;
			dword mbr_0x7b0;
			dword mbr_0x7c8;
			dword mbr_0x7ec;
			dword mbr_0x7f0;
			dword mbr_0x7f4;
			dword mbr_0x7f8;
			dword mbr_0x7fc;
			dword mbr_0x804;
			dword mbr_0xa0;
			dword mbr_0xbc;
			dword mbr_0xc0;
			dword mbr_0xc4;
			dword mbr_0xc8;
			dword mbr_0xcc;
			dword mbr_0xd0;
			dword mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
		// ---- the primary-attribute band, shipped GameCreature+0x760..0x778 (slice B13) -----------
		// Appended AFTER every existing member on purpose: per item 69 these generated headers are
		// name-sorted excerpts, so inserting mid-list would silently move every later member's PORT
		// offset. Declared in shipped order and contiguously because GetPrimaryAttribute indexes them
		// as an array (`mov eax,[eax+ecx*4+760h]` at 0x45acf2) while the derived-attribute switch
		// reads them by NAME - both shapes are shipped, neither is style. Names come from the IDB
		// (strength/perception/endurance/charisma/intelligence/agility/luck) because the meanings are
		// load-bearing for the formulas, not decoration. charisma_0x76c is read by NO derived case -
		// a hole in the switch - and exists here for the index path.
		// Typed int (not the file's dword) because every shipped formula is signed arithmetic
		// (imul/add on agility/str) and these are read by natives returning int.
			int strength_0x760;
			int perception_0x764;
			int endurance_0x768;
			int charisma_0x76c;
			int intelligence_0x770;
			int agility_0x774;
			int luck_0x778;
			// EVASION reads this SEPARATE field at +0x7FC, not the band - the IDB's own name says
			// "maybe_agility", so the derived switch's EVASION case does not alias agility_0x774.
			int maybe_agility_0x7fc;
		// ---- the base-skill and perk-rank bands (slice B14), as ONE adjacent pair ----------------
		// Sized by EVIDENCE, not by a guess, and both sizes agree twice over:
		//   baseSkills: the band runs +0xE0..+0x114, because the perk band starts at +0x114
		//               (GetPerkRank 0x45abd2 `mov eax,[eax+ecx*4+114h]`), giving (0x114-0xE0)/4
		//               = 13 ints - and GetTotalSkill's switch has EXACTLY 13 cases
		//               (cmp edx,0Ch at 0x40983e). Two independent counts, same answer.
		//   perkRanks:  GameCreature::ctor_0x40d010 zero-clears this band with
		//               `memset(<mbr_0x114>, 0, 0x34)` (0x40d277; the IDB attributes usage
		//               0x40d279 to mbr_0x114) = 13 ints.
		// Neither native bounds-checks the index, so these sizes describe the DATA, not a guard:
		// shipped happily reads past them. Appended at the end like the primary band, for the same
		// item-69 reason - inserting mid-list would move every later member's PORT offset.
			// ONE backing array, not two, and that is deliberate: shipped lays the bands out ADJACENT
			// (skills +0xE0..+0x114, perks +0x114..+0x148), so an out-of-range SKILL index does not
			// fault - it reads the PERK band. GetTotalSkill calls the base-skill accessor BEFORE its
			// switch (0x409837), so skill index 13 returns perk rank 0 unchanged, and index 14 perk
			// rank 1, etc. Modelling them as two separate arrays would make that shipped behavior
			// undefined behavior in C++ and quietly untestable; one array keeps it defined and
			// observable. SKILL_AT / PERK_AT below are the readable spellings of the two halves.
			int statBands_0xE0[ 26 ];   // [0..12] = base skills at +0xE0, [13..25] = perk ranks at +0x114
			// The creature's INVENTORY, shipped at GameCreature+0x168 and read by THREE natives from
			// the PRE-CAST entity base (IsItemEquipped 0x45b821 `mov eax,[esi+168h]`, GetWeaponRange
			// 0x45b6af, EquipItem) before any Inventory -> GameInventory RTTI attempt. Declared as the
			// port's own model type rather than a raw dword because the port has no GameInventory
			// implementation to point at - see src/F3/ScriptInventory.hpp for what stands in, and why
			// naming it a stand-in is the honest move rather than pretending the era exists.
			class ScriptInventory *inventory_0x168;
			// GetWeaponRange's two inputs, appended for the same item-69 reason as the bands above.
			// +0x98 is read through the tiny getter at 0x463b60 (`return this->mbr_0x98') and used as
			// the SLOT passed to the inventory's item-at-slot accessor (0x45b702-0x45b70a). The same
			// field is what Equip compares against 2 and 3 (0x46c170/0x46c18a), so it is the creature's
			// active weapon slot, not a weapon id.
			int weaponSlot_0x98;
			// +0x6F4 is the FALLBACK range pointer, used only when no item occupies the slot
			// (0x45b732: `mov edi,[edi+6F4h]' off the POST-CAST base, unlike +0x168 which is read
			// pre-cast). It is a POINTER that gets dereferenced at +0x2C, so "range 0" and "no fallback
			// pointer" are again indistinguishable by value to a script.
			int *rangeFallback_0x6f4;
		//: Methods
			void ctor_0x40d010();
			undefined4 virt_meth_0x40d300();
			undefined4 virt_meth_0x40d310();
			undefined4 virt_meth_0x40d320();
			undefined4 virt_meth_0x40d330();
			undefined4 virt_meth_0x40d340();
			undefined4 virt_meth_0x40d350();
			undefined4 virt_meth_0x40d360();
			undefined4 virt_meth_0x40d370();
			undefined4 virt_meth_0x40d380();
			undefined4 virt_meth_0x40d390();
			undefined4 virt_meth_0x40d3a0();
			undefined4 virt_meth_0x40d3b0();
			undefined4 virt_meth_0x40d3c0();
			undefined4 virt_meth_0x40d3d0();
			undefined4 virt_meth_0x40d3e0();
			undefined4 virt_meth_0x40d3f0();
			undefined4 virt_meth_0x40d400();
			undefined4 virt_meth_0x40d410();
			undefined4 virt_meth_0x40d420();
			undefined4 virt_meth_0x40d430();
			undefined4 virt_meth_0x40d440();
			undefined4 virt_meth_0x40d450();
			undefined4 virt_meth_0x40d460();
			undefined4 virt_meth_0x40d470();
			undefined4 virt_meth_0x40d480();
			undefined4 virt_meth_0x40e340();
			undefined4 virt_meth_0x40e350();
			undefined4 virt_meth_0x40e590();
	}; // GameCreature
} // namespace F3

#endif // F3_GAMECREATURE_HPP_
