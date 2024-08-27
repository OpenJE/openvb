// Copyright 2024 OpenJE

#ifndef F3_G3D_POINTLIGHT_HPP_
#define F3_G3D_POINTLIGHT_HPP_

#include "f3/global.hpp"
#include "f3/G3D_BaseLight.hpp"

namespace f3 {
	class G3D_PointLight : public G3D_BaseLight {
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
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
		//: Methods
		public:
			ctor_0x4cb740;
			virt_meth_0x4cb780;
			virt_meth_0x4ceca0;
			virt_meth_0x4d1c10;
			virt_meth_0x4d1d30;
			virt_meth_0x4d6150;
	}; // G3D_PointLight
} // namespace f3

#endif // F3_G3D_POINTLIGHT_HPP_
