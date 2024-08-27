// Copyright 2024 OpenJE

#ifndef F3_GAMEINVENTORY_HPP_
#define F3_GAMEINVENTORY_HPP_

#include "f3/global.hpp"
#include "f3/Inventory.hpp"

namespace f3 {
	struct GameInventory : public Inventory {
		//: Members
			dword mbr_0x14;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x4690a0;
			virt_meth_0x4691c0;
			virt_meth_0x469240;
			virt_meth_0x4692d0;
			meth_0x4693c0;
			meth_0x4693d0;
			virt_meth_0x469410;
			virt_meth_0x469550;
			virt_meth_0x469670;
			meth_0x469f90;
			meth_0x46a6c0;
			virt_meth_0x46a860;
			meth_0x46b100;
			virt_meth_0x46b350;
			dtor_0x46b7b0;
			ctor_0x46b970;
			virt_meth_0x46ba10;
			virt_meth_0x46be40;
	}; // GameInventory
} // namespace f3

#endif // F3_GAMEINVENTORY_HPP_
