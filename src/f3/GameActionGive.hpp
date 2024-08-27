// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONGIVE_HPP_
#define F3_GAMEACTIONGIVE_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct GameActionGive : public Action {
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
			virt_meth_0x404b60;
			ctor_0x4052a0;
			meth_0x4052d0;
			meth_0x4052f0;
			virt_meth_0x405300;
			virt_meth_0x405370;
	}; // GameActionGive
} // namespace f3

#endif // F3_GAMEACTIONGIVE_HPP_
