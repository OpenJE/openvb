// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONRELOAD_HPP_
#define F3_GAMEACTIONRELOAD_HPP_

#include "f3/global.hpp"
#include "je/Action.hpp"

namespace F3 {
	struct GameActionReload : public JE::Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			byte mbr_0x2c;
			byte mbr_0x2d;
			byte mbr_0x2e;
			dword mbr_0x30;
			byte mbr_0x34;
			dword mbr_0x38;
			byte mbr_0x4;
		//: Methods
			ctor_0x4071c0;
			virt_meth_0x407200;
			virt_meth_0x407210;
			meth_0x407250;
			virt_meth_0x407270;
	}; // GameActionReload
} // namespace F3

#endif // F3_GAMEACTIONRELOAD_HPP_
