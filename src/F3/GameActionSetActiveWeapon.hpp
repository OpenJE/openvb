// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONSETACTIVEWEAPON_HPP_
#define F3_GAMEACTIONSETACTIVEWEAPON_HPP_

#include "F3.hpp"
#include "JE/ActionSetActiveWeapon.hpp"

namespace F3 {
	struct GameActionSetActiveWeapon : public JE::ActionSetActiveWeapon {
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
			void ctor_0x4075d0();
			void dtor_0x407610();
			undefined4 virt_meth_0x407620();
			undefined4 meth_0x407710();
			undefined4 meth_0x4077b0();
			undefined4 virt_meth_0x407850();
			undefined4 virt_meth_0x4078f0();
			undefined4 virt_meth_0x407910();
	}; // GameActionSetActiveWeapon
} // namespace F3

#endif // F3_GAMEACTIONSETACTIVEWEAPON_HPP_
