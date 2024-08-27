// Copyright 2024 OpenJE

#ifndef F3_WATER_HPP_
#define F3_WATER_HPP_

#include "f3/global.hpp"
#include "f3/DynamicObject.hpp"

namespace f3 {
	struct Water : public DynamicObject {
		//: Members
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x164;
			dword mbr_0x168;
			dword mbr_0x16c;
			dword mbr_0x170;
			dword mbr_0x178;
			dword mbr_0x188;
			dword mbr_0x18c;
			dword mbr_0x194;
			dword mbr_0x1a4;
			dword mbr_0x1a8;
		//: Methods
			virt_meth_0x5ab040;
			virt_meth_0x5ab070;
			virt_meth_0x5ab0a0;
			virt_meth_0x5ab0c0;
			virt_meth_0x5ab0e0;
			virt_meth_0x5ab0f0;
			dtor_0x5ab250;
			ctor_0x5ab310;
			virt_meth_0x5ab380;
	}; // Water
} // namespace f3

#endif // F3_WATER_HPP_
