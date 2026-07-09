// Copyright 2024 OpenJE

#ifndef F3_GAMECONTAINER_HPP_
#define F3_GAMECONTAINER_HPP_

#include "F3.hpp"
#include "F3/GameUsableObject.hpp"

namespace F3 {
	struct GameContainer : public GameUsableObject {
		//: Members
			dword mbr_0x100;
			dword mbr_0x4;
			byte mbr_0xbc;
			byte mbr_0xbd;
			dword mbr_0xc0;
			byte mbr_0xc8;
			dword mbr_0xd8;
			dword mbr_0xdc;
			byte mbr_0xe4;
			dword mbr_0xf4;
			dword mbr_0xf8;
		//: Methods
			void ctor_0x408c20();
			undefined4 virt_meth_0x408d10();
			undefined4 virt_meth_0x408d20();
			undefined4 virt_meth_0x408d40();
			undefined4 virt_meth_0x408d50();
			undefined4 virt_meth_0x408d60();
			undefined4 virt_meth_0x408d70();
			undefined4 virt_meth_0x408d80();
			undefined4 virt_meth_0x408d90();
			undefined4 virt_meth_0x408da0();
			undefined4 virt_meth_0x408db0();
			undefined4 virt_meth_0x408dc0();
			undefined4 virt_meth_0x408dd0();
			undefined4 virt_meth_0x408de0();
			undefined4 virt_meth_0x408df0();
			undefined4 virt_meth_0x408e00();
			undefined4 virt_meth_0x408e10();
			undefined4 virt_meth_0x408e20();
			undefined4 virt_meth_0x408e30();
			undefined4 virt_meth_0x408e40();
			undefined4 virt_meth_0x408e50();
			undefined4 virt_meth_0x408e60();
			undefined4 virt_meth_0x409150();
			undefined4 virt_meth_0x409260();
			undefined4 virt_meth_0x40e900();
	}; // GameContainer
} // namespace F3

#endif // F3_GAMECONTAINER_HPP_
