// Copyright 2024 OpenJE

#ifndef F3_VFX_EFFECTINSTANCE_HPP_
#define F3_VFX_EFFECTINSTANCE_HPP_

#include "f3/global.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"

namespace f3 {
	class VFX_EffectInstance {
		//: Members
		public:
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			VFXEFFECTTYPE_BASE_Struct mbr_0x4;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			byte mbr_0x70;
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
			byte mbr_0xc0;
			byte mbr_0xc1;
		//: Methods
		public:
			ctor_0x4e17c0;
			meth_0x4e18d0;
			virt_meth_0x4e18e0;
			virt_meth_0x4e19b0;
	}; // VFX_EffectInstance
} // namespace f3

#endif // F3_VFX_EFFECTINSTANCE_HPP_
