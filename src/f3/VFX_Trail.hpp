// Copyright 2024 OpenJE

#ifndef F3_VFX_TRAIL_HPP_
#define F3_VFX_TRAIL_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_TRAIL_Struct.hpp"

namespace f3 {
	class VFX_Trail : public VFX_EffectInstance, public VFXEFFECTTYPE_TRAIL_Struct {
		//: Members
		public:
			byte mbr_0x130;
			byte mbr_0x134;
			byte mbr_0x135;
			byte mbr_0x136;
			byte mbr_0x137;
			byte mbr_0x13c;
			byte mbr_0x13d;
			byte mbr_0x13e;
			byte mbr_0x13f;
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x16c;
			dword mbr_0x170;
			dword mbr_0x174;
			dword mbr_0x17c;
			dword mbr_0x1b8;
			dword mbr_0x1bc;
			dword mbr_0x1c4;
			dword mbr_0x1c8;
			byte mbr_0x1cc;
			dword mbr_0x1d0;
			dword mbr_0x1d4;
			dword mbr_0x1d8;
			byte mbr_0x1dc;
			dword mbr_0x1e0;
			byte mbr_0x1e4;
			dword mbr_0x1e8;
			dword mbr_0x1ec;
			dword mbr_0x1f0;
			dword mbr_0x1f4;
			dword mbr_0x1f8;
			dword mbr_0x1fc;
			dword mbr_0x200;
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
			dword mbr_0xf0;
		//: Methods
		public:
			meth_0x4f11b0;
			meth_0x4f1470;
			meth_0x4f1d80;
			dtor_0x4f2550;
			virt_meth_0x4f26e0;
			virt_meth_0x4f2be0;
			meth_0x4f2cb0;
			virt_meth_0x4f2d30;
			ctor_0x4f2dd0;
	}; // VFX_Trail
} // namespace f3

#endif // F3_VFX_TRAIL_HPP_
