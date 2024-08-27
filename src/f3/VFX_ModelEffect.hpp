// Copyright 2024 OpenJE

#ifndef F3_VFX_MODELEFFECT_HPP_
#define F3_VFX_MODELEFFECT_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_MODELEFFECT_Struct.hpp"

namespace f3 {
	class VFX_ModelEffect : public VFX_EffectInstance, public VFXEFFECTTYPE_MODELEFFECT_Struct {
		//: Members
		public:
			byte mbr_0x134;
			byte mbr_0x135;
			dword mbr_0x138;
			dword mbr_0x13c;
			byte mbr_0x140;
			byte mbr_0x141;
			byte mbr_0x142;
			byte mbr_0x143;
			byte mbr_0x144;
			byte mbr_0x145;
			byte mbr_0x146;
			byte mbr_0x147;
			byte mbr_0x148;
			byte mbr_0x149;
			dword mbr_0x14c;
			byte mbr_0x158;
			byte mbr_0x159;
			byte mbr_0x15a;
			byte mbr_0x15b;
			byte mbr_0x15c;
			byte mbr_0x15d;
			byte mbr_0x15e;
			byte mbr_0x15f;
			byte mbr_0x160;
			dword mbr_0x164;
			byte mbr_0x170;
			byte mbr_0x171;
			byte mbr_0x172;
			byte mbr_0x173;
			byte mbr_0x174;
			dword mbr_0x178;
			byte mbr_0x184;
			dword mbr_0x188;
			dword mbr_0xf0;
		//: Methods
		public:
			virt_meth_0x4e8650;
			dtor_0x4e8be0;
			ctor_0x4e8c00;
			virt_meth_0x4e8c60;
			meth_0x4e8e00;
			virt_meth_0x4e8e40;
			virt_meth_0x4f6800;
			virt_meth_0x4f6880;
	}; // VFX_ModelEffect
} // namespace f3

#endif // F3_VFX_MODELEFFECT_HPP_
