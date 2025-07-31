// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONUNEQUIP_HPP_
#define F3_GAMEACTIONUNEQUIP_HPP_

#include "f3/global.hpp"
#include "je/ActionUnequip.hpp"

namespace F3 {
	struct GameActionUnequip : public JE::ActionUnequip {
		//: Members
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x24;
		//: Methods
			ctor_0x407af0;
			dtor_0x407b10;
			virt_meth_0x407b20;
			meth_0x407bc0;
			virt_meth_0x407c20;
			virt_meth_0x407c30;
			virt_meth_0x407c50;
	}; // GameActionUnequip
} // namespace F3

#endif // F3_GAMEACTIONUNEQUIP_HPP_
