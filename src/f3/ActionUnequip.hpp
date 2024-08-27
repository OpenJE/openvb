// Copyright 2024 OpenJE

#ifndef F3_ACTIONUNEQUIP_HPP_
#define F3_ACTIONUNEQUIP_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionUnequip : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionUnequip(); // ctor_0x5638b0
	}; // ActionUnequip
} // namespace f3

#endif // F3_ACTIONUNEQUIP_HPP_
