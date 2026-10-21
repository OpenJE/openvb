// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONDIE_HPP_
#define F3_GAMEACTIONDIE_HPP_

#include "F3.hpp"
#include "JE/ActionDie.hpp"

// DOCUMENTATION-GRADE layout of the SHIPPED F3::GameActionDie (T-035, 2026-10-20).
// This is NOT the live type. The drain executes the pump-side model declared in
// F3/GameActionAttack.hpp (`struct GameActionDie { ... }`), which carries no base class and only
// the fields the port actually uses. Both headers used to declare `F3::GameActionDie` with
// different layouts - included together in one TU that is a redefinition error, and compiled
// separately it silently misleads any later lane that aligns the two (see T-035 and harness item
// 22: a +0xNN here is a SHIPPED offset, not a port offset). Renamed to GameActionDieShipped so
// both can coexist and the shipped record stays in-tree as the reference for fidelity work.
namespace F3 {
	struct GameActionDieShipped : public JE::ActionDie {
		//: Members
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			byte mbr_0x28;
			byte mbr_0x4;
		//: Methods
			void ctor_0x404e50();
			void dtor_0x404e80();
			undefined4 virt_meth_0x404e90();
			undefined4 virt_meth_0x405020();
			undefined4 virt_meth_0x405040();
	}; // GameActionDieShipped
} // namespace F3

#endif // F3_GAMEACTIONDIE_HPP_
