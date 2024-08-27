// Copyright 2024 OpenJE

#ifndef F3_TRIGGER_HPP_
#define F3_TRIGGER_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Trigger {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x24;
			dword mbr_0x4;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x56e3f0;
			virt_meth_0x5b1520;
			virt_meth_0x5b1750;
			meth_0x5b17e0;
			virt_meth_0x5b1880;
			ctor_0x5b18a0;
			virt_meth_0x5b1960;
	}; // Trigger
} // namespace f3

#endif // F3_TRIGGER_HPP_
