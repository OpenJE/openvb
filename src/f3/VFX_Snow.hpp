// Copyright 2024 OpenJE

#ifndef F3_VFX_SNOW_HPP_
#define F3_VFX_SNOW_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_SNOW_Struct.hpp"

namespace f3 {
	class VFX_Snow : public VFX_EffectInstance, public VFXEFFECTTYPE_SNOW_Struct {
		//: Members
		public:
			dword mbr_0x154;
			dword mbr_0x168;
			dword mbr_0x16c;
			dword mbr_0x170;
			dword mbr_0x174;
			dword mbr_0x178;
			dword mbr_0x180;
			dword mbr_0x184;
			dword mbr_0x188;
			dword mbr_0x18c;
			dword mbr_0x190;
			dword mbr_0x194;
			dword mbr_0x198;
			dword mbr_0x19c;
			dword mbr_0x1a0;
			byte mbr_0x1a4;
			byte mbr_0x1a5;
			byte mbr_0x1a6;
			byte mbr_0x1a7;
			byte mbr_0x1a8;
			dword mbr_0x1ac;
			byte mbr_0x1b0;
			dword mbr_0x1b4;
			dword mbr_0x1cc;
			dword mbr_0x1d0;
			dword mbr_0x1d4;
			dword mbr_0x1d8;
			dword mbr_0x1dc;
			dword mbr_0x1e0;
			dword mbr_0x1e4;
			dword mbr_0x1e8;
			dword mbr_0x1ec;
			dword mbr_0x1f0;
			dword mbr_0x1f4;
			dword mbr_0x1f8;
			byte mbr_0x1fc;
			dword mbr_0x204;
			dword mbr_0x208;
			byte mbr_0x20c;
			dword mbr_0x210;
			dword mbr_0x250;
			dword mbr_0x254;
			byte mbr_0x258;
			dword mbr_0x25c;
			dword mbr_0x260;
			dword mbr_0x264;
			dword mbr_0x268;
			dword mbr_0x26c;
			byte mbr_0x270;
			byte mbr_0x271;
			dword mbr_0x74;
			byte mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
		//: Methods
		public:
			virt_meth_0x4e1890;
			virt_meth_0x4e3eb0;
			virt_meth_0x4e3f10;
			ctor_0x4e4100;
			dtor_0x4e4310;
			meth_0x4e4680;
			meth_0x4e4700;
			virt_meth_0x4e4db0;
			virt_meth_0x4e4dd0;
			meth_0x4e4f00;
			virt_meth_0x4e5d20;
	}; // VFX_Snow
} // namespace f3

#endif // F3_VFX_SNOW_HPP_
