// Copyright 2024 OpenJE

#ifndef F3_G3D_WATER_HPP_
#define F3_G3D_WATER_HPP_

#include "f3/global.hpp"
#include "f3/G3D_BaseAttribute.hpp"
#include "f3/cls_0x4d8d70.hpp"

namespace f3 {
	class G3D_Water : public G3D_BaseAttribute, public cls_0x4d8d70 {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			byte mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x64;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x8;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
		//: Methods
		public:
			meth_0x4c8d70;
			meth_0x4ce730;
			virt_meth_0x4cef90;
			virt_meth_0x4cf090;
			virt_meth_0x4d24a0;
			virt_meth_0x4d2620;
			virt_meth_0x4d26d0;
			ctor_0x4d8120;
			dtor_0x4d81c0;
			virt_meth_0x4d9110;
			ctor_0x4d92a0;
	}; // G3D_Water
} // namespace f3

#endif // F3_G3D_WATER_HPP_
