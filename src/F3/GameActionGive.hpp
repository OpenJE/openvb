// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONGIVE_HPP_
#define F3_GAMEACTIONGIVE_HPP_

#include "F3.hpp"
#include "JE/Action.hpp"

namespace F3 {
	struct GameActionGive : public JE::Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			byte mbr_0x4;
		//: Methods
			undefined4 virt_meth_0x404b60();
			void ctor_0x4052a0();
			undefined4 meth_0x4052d0();
			undefined4 meth_0x4052f0();
			undefined4 virt_meth_0x405300();
			undefined4 virt_meth_0x405370();
	}; // GameActionGive
} // namespace F3

#endif // F3_GAMEACTIONGIVE_HPP_
