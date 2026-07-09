// Copyright 2024 OpenJE

#ifndef F3_GAMEDOOR_HPP_
#define F3_GAMEDOOR_HPP_

#include "F3.hpp"
#include "F3/GameUsableObject.hpp"

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
			void ctor_0x40e7f0();
			undefined4 virt_meth_0x40e8e0();
			undefined4 virt_meth_0x40e8f0();
			undefined4 virt_meth_0x40e910();
			undefined4 virt_meth_0x40e920();
			undefined4 virt_meth_0x40e930();
			undefined4 virt_meth_0x40e940();
			undefined4 virt_meth_0x40e950();
			undefined4 virt_meth_0x40e960();
			undefined4 virt_meth_0x40e970();
			undefined4 virt_meth_0x40e980();
			undefined4 virt_meth_0x40e990();
			undefined4 virt_meth_0x40e9a0();
			undefined4 virt_meth_0x40e9b0();
			undefined4 virt_meth_0x40e9c0();
			undefined4 virt_meth_0x40e9d0();
			undefined4 virt_meth_0x40e9e0();
			undefined4 virt_meth_0x40e9f0();
			undefined4 virt_meth_0x40ea00();
			undefined4 virt_meth_0x40ea10();
			undefined4 virt_meth_0x40ea20();
			undefined4 virt_meth_0x40ea40();
			undefined4 virt_meth_0x40f2f0();
			undefined4 virt_meth_0x40fc20();
			undefined4 virt_meth_0x40ffd0();
			undefined4 virt_meth_0x40ffe0();
			undefined4 virt_meth_0x4100a0();
	}; // GameDoor
} // namespace F3

#endif // F3_GAMEDOOR_HPP_
