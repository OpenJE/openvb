// Copyright 2024 OpenJE

#ifndef F3_VFX_SPRITE_HPP_
#define F3_VFX_SPRITE_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_SPRITE_Struct.hpp"

namespace f3 {
	class VFX_Sprite : public VFX_EffectInstance, public VFXEFFECTTYPE_SPRITE_Struct {
		//: Members
		public:
			dword mbr_0x10c;
			dword mbr_0x110;
			byte mbr_0x134;
			dword mbr_0x13c;
			dword mbr_0x14c;
			dword mbr_0x150;
			dword mbr_0x158;
			dword mbr_0x168;
			dword mbr_0x16c;
			dword mbr_0x170;
			dword mbr_0x174;
			dword mbr_0x178;
			dword mbr_0x17c;
			dword mbr_0x188;
			byte mbr_0x18c;
			byte mbr_0x18d;
			byte mbr_0x18e;
			byte mbr_0x18f;
			byte mbr_0x190;
			byte mbr_0x191;
			byte mbr_0x192;
			byte mbr_0x193;
			byte mbr_0x194;
			byte mbr_0x195;
			dword mbr_0x198;
			byte mbr_0x1a4;
			byte mbr_0x1a5;
			byte mbr_0x1a6;
			byte mbr_0x1a7;
			dword mbr_0x1a8;
			byte mbr_0x1b4;
			byte mbr_0x1b5;
			dword mbr_0x1c8;
			byte mbr_0x1d4;
			dword mbr_0x1d8;
			dword mbr_0x218;
			dword mbr_0x21c;
			byte mbr_0x220;
			dword mbr_0x224;
			dword mbr_0x228;
			dword mbr_0x22c;
			dword mbr_0x230;
			dword mbr_0x234;
			dword mbr_0x238;
			dword mbr_0x23c;
			byte mbr_0x240;
			dword mbr_0x244;
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
			dword mbr_0xcc;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xfc;
		//: Methods
		public:
			virt_meth_0x4f4c50;
			virt_meth_0x4f4ca0;
			meth_0x4f4eb0;
			virt_meth_0x4f4fc0;
			meth_0x4f5400;
			dtor_0x4f5db0;
			virt_meth_0x4f6190;
			meth_0x4f6390;
			virt_meth_0x4f63f0;
			virt_meth_0x4f6410;
			ctor_0x4f6490;
	}; // VFX_Sprite
} // namespace f3

#endif // F3_VFX_SPRITE_HPP_
