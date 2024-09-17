// Copyright 2024 OpenJE

#ifndef F3_GAMECONTAINER_HPP_
#define F3_GAMECONTAINER_HPP_

#include "f3/global.hpp"
#include "f3/GameUsableObject.hpp"

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
			ctor_0x408c20;
			virt_meth_0x408d10;
			virt_meth_0x408d20;
			virt_meth_0x408d40;
			virt_meth_0x408d50;
			virt_meth_0x408d60;
			virt_meth_0x408d70;
			virt_meth_0x408d80;
			virt_meth_0x408d90;
			virt_meth_0x408da0;
			virt_meth_0x408db0;
			virt_meth_0x408dc0;
			virt_meth_0x408dd0;
			virt_meth_0x408de0;
			virt_meth_0x408df0;
			virt_meth_0x408e00;
			virt_meth_0x408e10;
			virt_meth_0x408e20;
			virt_meth_0x408e30;
			virt_meth_0x408e40;
			virt_meth_0x408e50;
			virt_meth_0x408e60;
			virt_meth_0x409150;
			virt_meth_0x409260;
			virt_meth_0x40e900;
	}; // GameContainer
} // namespace F3

#endif // F3_GAMECONTAINER_HPP_
