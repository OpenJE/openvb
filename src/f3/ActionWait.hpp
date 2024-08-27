// Copyright 2024 OpenJE

#ifndef F3_ACTIONWAIT_HPP_
#define F3_ACTIONWAIT_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionWait : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x4;
		//: Methods
			ActionWait(); // ctor_0x5601a0
			virtual undefined4 Set_mbr_0x1c( undefined4 param_1 ); // virt_meth_0x5601c0
			virtual undefined4 virt_meth_0x5601d0(); // virt_meth_0x5601d0
	}; // ActionWait
} // namespace f3

#endif // F3_ACTIONWAIT_HPP_
