// Copyright 2024 OpenJE

#ifndef F3_G3D_DIRECTIONALLIGHT_HPP_
#define F3_G3D_DIRECTIONALLIGHT_HPP_

#include "f3/global.hpp"
#include "f3/G3D_BaseLight.hpp"

namespace f3 {
	class G3D_DirectionalLight : public G3D_BaseLight {
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
		//: Methods
		public:
			ctor_0x4cb5e0;
			virt_meth_0x4cb6a0;
			meth_0x4ce980;
			virt_meth_0x4cec30;
			meth_0x4d06b0;
			meth_0x4d0750;
			virt_meth_0x4d1ac0;
			virt_meth_0x4d1b60;
			virt_meth_0x4d5ea0;
	}; // G3D_DirectionalLight
} // namespace f3

#endif // F3_G3D_DIRECTIONALLIGHT_HPP_
