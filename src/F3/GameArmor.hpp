// Copyright 2024 OpenJE

#ifndef F3_GAMEARMOR_HPP_
#define F3_GAMEARMOR_HPP_

#include "F3.hpp"
#include "F3/GameItem.hpp"

namespace F3 {
	struct GameArmor : public GameItem {
		//: Members
			dword mbr_0x3b0;
			dword mbr_0x3c8;
			dword mbr_0x3d0;
			dword mbr_0x4;
		//: Methods
			void ctor_0x408580();
			undefined4 virt_meth_0x408750();
			undefined4 virt_meth_0x408760();
			undefined4 virt_meth_0x408770();
			undefined4 virt_meth_0x408780();
			undefined4 virt_meth_0x408790();
			undefined4 virt_meth_0x4087a0();
			undefined4 virt_meth_0x4087b0();
			undefined4 virt_meth_0x4087c0();
			undefined4 virt_meth_0x4087d0();
			undefined4 virt_meth_0x4087e0();
			undefined4 virt_meth_0x4087f0();
			undefined4 virt_meth_0x408800();
			undefined4 virt_meth_0x408810();
			undefined4 virt_meth_0x408820();
			undefined4 virt_meth_0x408830();
			undefined4 virt_meth_0x408840();
			undefined4 virt_meth_0x408850();
			undefined4 virt_meth_0x408860();
			undefined4 virt_meth_0x408870();
			undefined4 virt_meth_0x408880();
			undefined4 virt_meth_0x408890();
			undefined4 virt_meth_0x4088a0();
			undefined4 virt_meth_0x408970();
	}; // GameArmor
} // namespace F3

#endif // F3_GAMEARMOR_HPP_
