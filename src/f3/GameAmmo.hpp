// Copyright 2024 OpenJE

#ifndef F3_GAMEAMMO_HPP_
#define F3_GAMEAMMO_HPP_

#include "f3/global.hpp"
#include "f3/GameItem.hpp"

namespace f3 {
	struct GameAmmo : public GameItem {
		//: Members
			dword mbr_0x3b0;
			dword mbr_0x3b4;
			dword mbr_0x3b8;
			dword mbr_0x3bc;
			dword mbr_0x3c0;
			dword mbr_0x3c4;
			dword mbr_0x3cc;
			dword mbr_0x3d0;
			dword mbr_0x3d8;
			dword mbr_0x4;
		//: Methods
			ctor_0x407fb0;
			virt_meth_0x4082a0;
			virt_meth_0x4082b0;
			virt_meth_0x4082c0;
			virt_meth_0x4082d0;
			virt_meth_0x4082e0;
			virt_meth_0x4082f0;
			virt_meth_0x408300;
			virt_meth_0x408310;
			virt_meth_0x408320;
			virt_meth_0x408330;
			virt_meth_0x408340;
			virt_meth_0x408350;
			virt_meth_0x408360;
			virt_meth_0x408370;
			virt_meth_0x408380;
			virt_meth_0x408390;
			virt_meth_0x4083a0;
			virt_meth_0x4083b0;
			virt_meth_0x4083c0;
			virt_meth_0x4083d0;
			virt_meth_0x4083e0;
			virt_meth_0x4083f0;
			virt_meth_0x408570;
	}; // GameAmmo
} // namespace f3

#endif // F3_GAMEAMMO_HPP_
