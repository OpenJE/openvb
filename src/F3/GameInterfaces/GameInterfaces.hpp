// Copyright 2024 OpenJE

#ifndef F3_GAMEINTERFACES_HPP_
#define F3_GAMEINTERFACES_HPP_

#include "f3/global.hpp"
#include "je/Interfaces.hpp"

namespace F3 {
	struct GameInterfaces : public JE::Interfaces {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			dtor_0x4396b0;
			virt_meth_0x4396c0;
			virt_meth_0x4396f0;
			virt_meth_0x439720;
			virt_meth_0x439770;
			virt_meth_0x4397e0;
			virt_meth_0x439810;
			virt_meth_0x439850;
			virt_meth_0x4398c0;
			ctor_0x4398e0;
	}; // GameInterfaces
} // namespace F3

#endif // F3_GAMEINTERFACES_HPP_
