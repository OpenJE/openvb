// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONPICKUP_HPP_
#define F3_GAMEACTIONPICKUP_HPP_

#include "f3/global.hpp"
#include "f3/ActionPickUp.hpp"

namespace f3 {
	struct GameActionPickUp : public ActionPickUp {
		//: Members
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x28;
			byte mbr_0x2c;
			byte mbr_0x2d;
		//: Methods
			meth_0x406a30;
			ctor_0x406e70;
			dtor_0x406eb0;
			meth_0x406ec0;
			virt_meth_0x406f10;
			virt_meth_0x4071a0;
	}; // GameActionPickUp
} // namespace f3

#endif // F3_GAMEACTIONPICKUP_HPP_
