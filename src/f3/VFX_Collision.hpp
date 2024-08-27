// Copyright 2024 OpenJE

#ifndef F3_VFX_COLLISION_HPP_
#define F3_VFX_COLLISION_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/cls_0x4dff90.hpp"
#include "f3/Gfx_LineSystem.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"

namespace f3 {
	class VFX_Collision : public VFX_EffectInstance, public cls_0x4dff90, public Gfx_LineSystem, public VFXEFFECTTYPE_BASE_Struct {
		//: Members
		public:
			dword mbr_0x134;
			dword mbr_0x140;
			dword mbr_0x144;
			dword mbr_0x154;
			dword mbr_0x158;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x164;
			dword mbr_0x168;
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
			virt_meth_0x4ee8b0;
			virt_meth_0x4eea10;
			dtor_0x4f0ef0;
			ctor_0x4f0f90;
			virt_meth_0x4f10a0;
			meth_0x4f1120;
			virt_meth_0x4f1160;
	}; // VFX_Collision
} // namespace f3

#endif // F3_VFX_COLLISION_HPP_
