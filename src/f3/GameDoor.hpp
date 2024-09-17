// Copyright 2024 OpenJE

#ifndef F3_GAMEDOOR_HPP_
#define F3_GAMEDOOR_HPP_

#include "f3/global.hpp"
#include "f3/GameUsableObject.hpp"

namespace F3 {
	struct GameDoor : public GameUsableObject {
		//: Members
			dword mbr_0x4;
			byte mbr_0xbc;
			byte mbr_0xbd;
			byte mbr_0xc8;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xe4;
			dword mbr_0xe8;
			dword mbr_0xf0;
		//: Methods
			ctor_0x40e7f0;
			virt_meth_0x40e8e0;
			virt_meth_0x40e8f0;
			virt_meth_0x40e910;
			virt_meth_0x40e920;
			virt_meth_0x40e930;
			virt_meth_0x40e940;
			virt_meth_0x40e950;
			virt_meth_0x40e960;
			virt_meth_0x40e970;
			virt_meth_0x40e980;
			virt_meth_0x40e990;
			virt_meth_0x40e9a0;
			virt_meth_0x40e9b0;
			virt_meth_0x40e9c0;
			virt_meth_0x40e9d0;
			virt_meth_0x40e9e0;
			virt_meth_0x40e9f0;
			virt_meth_0x40ea00;
			virt_meth_0x40ea10;
			virt_meth_0x40ea20;
			virt_meth_0x40ea40;
			virt_meth_0x40f2f0;
			virt_meth_0x40fc20;
			virt_meth_0x40ffd0;
			virt_meth_0x40ffe0;
			virt_meth_0x4100a0;
	}; // GameDoor
} // namespace F3

#endif // F3_GAMEDOOR_HPP_
