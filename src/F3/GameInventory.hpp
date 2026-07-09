// Copyright 2024 OpenJE

#ifndef F3_GAMEINVENTORY_HPP_
#define F3_GAMEINVENTORY_HPP_

#include "F3.hpp"
#include "JE/Inventory.hpp"

namespace F3 {
	struct GameInventory : public JE::Inventory {
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
			undefined4 virt_meth_0x4690a0();
			undefined4 virt_meth_0x4691c0();
			undefined4 virt_meth_0x469240();
			undefined4 virt_meth_0x4692d0();
			undefined4 meth_0x4693c0();
			undefined4 meth_0x4693d0();
			undefined4 virt_meth_0x469410();
			undefined4 virt_meth_0x469550();
			undefined4 virt_meth_0x469670();
			undefined4 meth_0x469f90();
			undefined4 meth_0x46a6c0();
			undefined4 virt_meth_0x46a860();
			undefined4 meth_0x46b100();
			undefined4 virt_meth_0x46b350();
			void dtor_0x46b7b0();
			void ctor_0x46b970();
			undefined4 virt_meth_0x46ba10();
			undefined4 virt_meth_0x46be40();
	}; // GameInventory
} // namespace F3

#endif // F3_GAMEINVENTORY_HPP_
