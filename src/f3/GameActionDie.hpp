// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONDIE_HPP_
#define F3_GAMEACTIONDIE_HPP_

#include "f3/global.hpp"
#include "f3/ActionDie.hpp"

namespace f3 {
	struct GameActionDie : public ActionDie {
		//: Members
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			byte mbr_0x28;
			byte mbr_0x4;
		//: Methods
			ctor_0x404e50;
			dtor_0x404e80;
			virt_meth_0x404e90;
			virt_meth_0x405020;
			virt_meth_0x405040;
	}; // GameActionDie
} // namespace f3

#endif // F3_GAMEACTIONDIE_HPP_
