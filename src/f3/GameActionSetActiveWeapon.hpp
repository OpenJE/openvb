// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONSETACTIVEWEAPON_HPP_
#define F3_GAMEACTIONSETACTIVEWEAPON_HPP_

#include "f3/global.hpp"
#include "f3/ActionSetActiveWeapon.hpp"

namespace f3 {
	struct GameActionSetActiveWeapon : public ActionSetActiveWeapon {
		//: Members
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			byte mbr_0x30;
			dword mbr_0x34;
			byte mbr_0x38;
			dword mbr_0x3c;
			byte mbr_0x40;
			byte mbr_0x41;
		//: Methods
			ctor_0x4075d0;
			dtor_0x407610;
			virt_meth_0x407620;
			meth_0x407710;
			meth_0x4077b0;
			virt_meth_0x407850;
			virt_meth_0x4078f0;
			virt_meth_0x407910;
	}; // GameActionSetActiveWeapon
} // namespace f3

#endif // F3_GAMEACTIONSETACTIVEWEAPON_HPP_
