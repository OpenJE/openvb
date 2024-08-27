// Copyright 2024 OpenJE

#ifndef F3_VFX_LIGHT_HPP_
#define F3_VFX_LIGHT_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/Gfx_LineSystem.hpp"
#include "f3/VFXEFFECTTYPE_LIGHT_Struct.hpp"

namespace f3 {
	class VFX_Light : public VFX_EffectInstance, public Gfx_LineSystem, public VFXEFFECTTYPE_LIGHT_Struct {
		//: Members
		public:
			dword mbr_0x134;
			dword mbr_0x138;
			byte mbr_0x144;
			byte mbr_0x145;
			byte mbr_0x146;
			byte mbr_0x14c;
			byte mbr_0x14d;
			byte mbr_0x14e;
			byte mbr_0x14f;
			byte mbr_0x150;
			byte mbr_0x151;
			dword mbr_0x154;
			dword mbr_0x158;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x164;
			dword mbr_0x168;
			dword mbr_0x16c;
			byte mbr_0x174;
			byte mbr_0x176;
			byte mbr_0x177;
			byte mbr_0x178;
			byte mbr_0x179;
			byte mbr_0x17a;
			dword mbr_0x17c;
			byte mbr_0x188;
			byte mbr_0x189;
			byte mbr_0x18a;
			byte mbr_0x18b;
			byte mbr_0x18c;
			dword mbr_0x190;
			byte mbr_0x19c;
			byte mbr_0x19d;
			byte mbr_0x19e;
			byte mbr_0x19f;
			byte mbr_0x1a0;
			dword mbr_0x1a4;
			byte mbr_0x1b0;
			byte mbr_0x1b1;
			dword mbr_0x1b8;
			dword mbr_0x1c4;
			dword mbr_0x1c8;
			dword mbr_0x1cc;
			dword mbr_0x80;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
		//: Methods
		public:
			virt_meth_0x4e7470;
			virt_meth_0x4e7950;
			dtor_0x4e8040;
			ctor_0x4e80d0;
			virt_meth_0x4e8140;
			meth_0x4e83b0;
			virt_meth_0x4e83f0;
	}; // VFX_Light
} // namespace f3

#endif // F3_VFX_LIGHT_HPP_
