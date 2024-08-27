// Copyright 2024 OpenJE

#ifndef F3_G3D_SPOTLIGHT_HPP_
#define F3_G3D_SPOTLIGHT_HPP_

#include "f3/global.hpp"
#include "f3/G3D_PointLight.hpp"

namespace f3 {
	class G3D_SpotLight : public G3D_PointLight {
		//: Members
		public:
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
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
		//: Methods
		public:
			ctor_0x4cb890;
			virt_meth_0x4cb950;
			virt_meth_0x4ced80;
			virt_meth_0x4d1e50;
			virt_meth_0x4d1ff0;
			virt_meth_0x4d64e0;
	}; // G3D_SpotLight
} // namespace f3

#endif // F3_G3D_SPOTLIGHT_HPP_
