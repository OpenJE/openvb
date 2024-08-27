// Copyright 2024 OpenJE

#ifndef F3_ACTIONEQUIP_HPP_
#define F3_ACTIONEQUIP_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionEquip : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionEquip(); // ctor_0x55fb40
	}; // ActionEquip
} // namespace f3

#endif // F3_ACTIONEQUIP_HPP_
