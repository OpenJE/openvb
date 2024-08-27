// Copyright 2024 OpenJE

#ifndef F3_EFFECT_HPP_
#define F3_EFFECT_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Effect {
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
			byte mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			byte mbr_0x8;
			byte mbr_0x9;
			byte mbr_0xa;
			byte mbr_0xb;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x45c090;
			virt_meth_0x55a2a0;
			virt_meth_0x55a300;
			virt_meth_0x55ac10;
			dtor_0x55b130;
			ctor_0x55b230;
			virt_meth_0x55b2e0;
	}; // Effect
} // namespace f3

#endif // F3_EFFECT_HPP_
