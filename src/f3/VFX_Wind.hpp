// Copyright 2024 OpenJE

#ifndef F3_VFX_WIND_HPP_
#define F3_VFX_WIND_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/cls_0x4dff90.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"

namespace f3 {
	class VFX_Wind : public VFX_EffectInstance, public cls_0x4dff90, public VFXEFFECTTYPE_BASE_Struct {
		//: Members
		public:
			dword mbr_0x134;
			dword mbr_0x13c;
			dword mbr_0x140;
			byte mbr_0x148;
			byte mbr_0x149;
			dword mbr_0x150;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x164;
			dword mbr_0x168;
			dword mbr_0x16c;
			dword mbr_0x170;
			dword mbr_0x174;
			dword mbr_0x178;
			dword mbr_0x17c;
			byte mbr_0x184;
			dword mbr_0x188;
			dword mbr_0x18c;
			dword mbr_0x190;
			dword mbr_0x194;
			dword mbr_0x198;
			byte mbr_0x7c;
		//: Methods
		public:
			dtor_0x4e3730;
			virt_meth_0x4e3790;
			ctor_0x4e37d0;
			virt_meth_0x4e3870;
			virt_meth_0x4e3980;
			meth_0x4e3e10;
			virt_meth_0x4e3e60;
			virt_meth_0x4e5ec0;
	}; // VFX_Wind
} // namespace f3

#endif // F3_VFX_WIND_HPP_
