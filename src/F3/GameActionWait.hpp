// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONWAIT_HPP_
#define F3_GAMEACTIONWAIT_HPP_

#include "F3.hpp"
#include "JE/ActionWait.hpp"

namespace F3 {
	// T-035 (was a second F3::GameActionWait colliding with the live drain stand-in in
	// GameActionMoveTo.hpp:83 — proof-of-collision + proof-of-fix: tools/odr-demo.cpp).
	// This header is the ONLY in-tree record of the shipped Wait action's layout
	// (JE::ActionWait base, mbr_0x14, ctor_0x407ed0/dtor_0x407ef0/virt 0x407f00/0x407f60) —
	// kept deliberately, per coordinator ruling: delete would erase fidelity evidence.
	// No field offsets moved by the rename.
	struct GameActionWaitShipped : public JE::ActionWait {
		//: Members
			dword mbr_0x14;
		//: Methods
			void ctor_0x407ed0();
			void dtor_0x407ef0();
			undefined4 virt_meth_0x407f00();
			undefined4 virt_meth_0x407f60();
	}; // GameActionWaitShipped
} // namespace F3

#endif // F3_GAMEACTIONWAIT_HPP_
