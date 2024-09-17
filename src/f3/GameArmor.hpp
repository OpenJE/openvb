// Copyright 2024 OpenJE

#ifndef F3_GAMEARMOR_HPP_
#define F3_GAMEARMOR_HPP_

#include "f3/global.hpp"
#include "f3/GameItem.hpp"

namespace F3 {
	struct GameArmor : public GameItem {
		//: Members
			dword mbr_0x3b0;
			dword mbr_0x3c8;
			dword mbr_0x3d0;
			dword mbr_0x4;
		//: Methods
			ctor_0x408580;
			virt_meth_0x408750;
			virt_meth_0x408760;
			virt_meth_0x408770;
			virt_meth_0x408780;
			virt_meth_0x408790;
			virt_meth_0x4087a0;
			virt_meth_0x4087b0;
			virt_meth_0x4087c0;
			virt_meth_0x4087d0;
			virt_meth_0x4087e0;
			virt_meth_0x4087f0;
			virt_meth_0x408800;
			virt_meth_0x408810;
			virt_meth_0x408820;
			virt_meth_0x408830;
			virt_meth_0x408840;
			virt_meth_0x408850;
			virt_meth_0x408860;
			virt_meth_0x408870;
			virt_meth_0x408880;
			virt_meth_0x408890;
			virt_meth_0x4088a0;
			virt_meth_0x408970;
	}; // GameArmor
} // namespace F3

#endif // F3_GAMEARMOR_HPP_
