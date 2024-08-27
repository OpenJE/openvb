// Copyright 2024 OpenJE

#ifndef F3_ACTIONDIE_HPP_
#define F3_ACTIONDIE_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionDie : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionDie(); // ctor_0x55fb10
			virtual undefined4 virt_meth_0x55fb30(); // virt_meth_0x55fb30
			virtual undefined4 virt_meth_0x5638d0(); // virt_meth_0x5638d0
	}; // ActionDie
} // namespace f3

#endif // F3_ACTIONDIE_HPP_
