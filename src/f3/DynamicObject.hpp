// Copyright 2024 OpenJE

#ifndef F3_DYNAMICOBJECT_HPP_
#define F3_DYNAMICOBJECT_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_LineSystem.hpp"

namespace f3 {
	struct DynamicObject {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x171;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x2e0;
			dword mbr_0x2e4;
			dword mbr_0x2e8;
			dword mbr_0x2ec;
			dword mbr_0x2f0;
			byte mbr_0x30;
			dword mbr_0x33c;
			dword mbr_0x34;
			dword mbr_0x37c;
			dword mbr_0x38;
			dword mbr_0x380;
			dword mbr_0x3c;
			dword mbr_0x3c0;
			dword mbr_0x3c4;
			dword mbr_0x3c8;
			dword mbr_0x3cc;
			dword mbr_0x3d0;
			dword mbr_0x3d4;
			dword mbr_0x4;
			Gfx_LineSystem mbr_0x40;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			meth_0x54b3f0;
			meth_0x54bf30;
			meth_0x54c180;
			meth_0x54c390;
			meth_0x5682c0;
			ctor_0x568350;
			virt_meth_0x568400;
	}; // DynamicObject
} // namespace f3

#endif // F3_DYNAMICOBJECT_HPP_
