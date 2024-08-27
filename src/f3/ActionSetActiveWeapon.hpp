// Copyright 2024 OpenJE

#ifndef F3_ACTIONSETACTIVEWEAPON_HPP_
#define F3_ACTIONSETACTIVEWEAPON_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionSetActiveWeapon : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionSetActiveWeapon(); // ctor_0x563890
	}; // ActionSetActiveWeapon
} // namespace f3

#endif // F3_ACTIONSETACTIVEWEAPON_HPP_
