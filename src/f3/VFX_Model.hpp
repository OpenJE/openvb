// Copyright 2024 OpenJE

#ifndef F3_VFX_MODEL_HPP_
#define F3_VFX_MODEL_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_MODEL_Struct.hpp"

namespace f3 {
	class VFX_Model : public VFX_EffectInstance, public VFXEFFECTTYPE_MODEL_Struct {
		//: Members
		public:
			dword mbr_0x150;
			dword mbr_0x154;
			dword mbr_0x164;
			dword mbr_0x168;
			dword mbr_0x188;
			dword mbr_0x18c;
			dword mbr_0x19c;
			dword mbr_0x1a0;
			dword mbr_0x1c0;
			dword mbr_0x1c4;
			dword mbr_0x1d4;
			dword mbr_0x1d8;
			dword mbr_0x1dc;
			dword mbr_0x1e0;
			dword mbr_0x1e4;
			dword mbr_0x1e8;
			byte mbr_0x1ec;
			byte mbr_0x1ed;
			byte mbr_0x1ee;
			dword mbr_0x1f0;
			byte mbr_0x1f4;
			byte mbr_0x1f5;
			byte mbr_0x1f6;
			byte mbr_0x1f7;
			byte mbr_0x1f8;
			byte mbr_0x1f9;
			byte mbr_0x1fa;
			byte mbr_0x1fb;
			byte mbr_0x1fc;
			dword mbr_0x200;
			byte mbr_0x20c;
			byte mbr_0x20d;
			byte mbr_0x20e;
			byte mbr_0x20f;
			byte mbr_0x210;
			byte mbr_0x211;
			byte mbr_0x212;
			byte mbr_0x213;
			byte mbr_0x214;
			dword mbr_0x218;
			byte mbr_0x224;
			byte mbr_0x225;
			byte mbr_0x226;
			byte mbr_0x227;
			byte mbr_0x228;
			dword mbr_0x22c;
			byte mbr_0x238;
			byte mbr_0x239;
			dword mbr_0x24;
			dword mbr_0x248;
			dword mbr_0x254;
			dword mbr_0x258;
			dword mbr_0x25c;
			dword mbr_0x26c;
			dword mbr_0x270;
			dword mbr_0x274;
			dword mbr_0x278;
			dword mbr_0x288;
			dword mbr_0x28c;
			dword mbr_0x290;
			dword mbr_0x294;
			dword mbr_0x2a4;
			dword mbr_0x2a8;
			dword mbr_0x2ac;
			byte mbr_0x2b0;
			dword mbr_0x2b4;
			dword mbr_0x2b8;
			dword mbr_0x2bc;
			dword mbr_0x2c0;
			dword mbr_0x2c4;
			dword mbr_0x2c8;
			dword mbr_0x2cc;
			dword mbr_0x2d0;
			dword mbr_0x2d4;
			byte mbr_0x2d8;
			byte mbr_0x2d9;
			byte mbr_0x2da;
			dword mbr_0x74;
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
			virt_meth_0x4f30c0;
			virt_meth_0x4f3690;
			virt_meth_0x4f36c0;
			virt_meth_0x4f36f0;
			virt_meth_0x4f3720;
			virt_meth_0x4f3740;
			virt_meth_0x4f3760;
			virt_meth_0x4f3770;
			virt_meth_0x4f3780;
			meth_0x4f3a00;
			meth_0x4f4000;
			dtor_0x4f4240;
			virt_meth_0x4f43d0;
			meth_0x4f43e0;
			virt_meth_0x4f45c0;
			ctor_0x4f4650;
			meth_0x4f47b0;
			virt_meth_0x4f4960;
	}; // VFX_Model
} // namespace f3

#endif // F3_VFX_MODEL_HPP_
