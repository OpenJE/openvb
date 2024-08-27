// Copyright 2024 OpenJE

#ifndef F3_ACTIONWANDER_HPP_
#define F3_ACTIONWANDER_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionWander : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			byte mbr_0x4;
			dword mbr_0x40;
		//: Methods
			ActionWander(); // ctor_0x5a27e0
			virtual undefined4 virt_meth_0x5a28e0(); // virt_meth_0x5a28e0
			virtual undefined4 virt_meth_0x5a28f0(); // virt_meth_0x5a28f0
	}; // ActionWander
} // namespace f3

#endif // F3_ACTIONWANDER_HPP_
