// Copyright 2024 OpenJE

#ifndef F3_VFX_LIGHTNING_HPP_
#define F3_VFX_LIGHTNING_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_LIGHTNING_Struct.hpp"

namespace f3 {
	class VFX_Lightning : public VFX_EffectInstance, public VFXEFFECTTYPE_LIGHTNING_Struct {
		//: Members
		public:
			dword mbr_0x10c;
			dword mbr_0x110;
			byte mbr_0x130;
			dword mbr_0x134;
			byte mbr_0x138;
			byte mbr_0x139;
			dword mbr_0x144;
			dword mbr_0x150;
			dword mbr_0x154;
			dword mbr_0x158;
			dword mbr_0x15c;
			dword mbr_0x16c;
			dword mbr_0x170;
			byte mbr_0x174;
			dword mbr_0x178;
			dword mbr_0x17c;
			dword mbr_0x180;
			byte mbr_0x1a4;
			dword mbr_0x1b0;
			dword mbr_0x1b4;
			dword mbr_0x1c4;
			dword mbr_0x1c8;
			byte mbr_0x1dc;
			byte mbr_0x1dd;
			byte mbr_0x1de;
			byte mbr_0x1df;
			byte mbr_0x1e0;
			byte mbr_0x1e1;
			byte mbr_0x1e2;
			byte mbr_0x1e3;
			byte mbr_0x1e4;
			byte mbr_0x1e5;
			byte mbr_0x1e6;
			dword mbr_0x1e8;
			byte mbr_0x1f4;
			byte mbr_0x1f5;
			byte mbr_0x1f6;
			byte mbr_0x1f7;
			byte mbr_0x1f8;
			dword mbr_0x1fc;
			byte mbr_0x208;
			byte mbr_0x209;
			byte mbr_0x20a;
			byte mbr_0x20b;
			byte mbr_0x20c;
			dword mbr_0x210;
			byte mbr_0x21c;
			byte mbr_0x21d;
			byte mbr_0x21e;
			dword mbr_0x220;
			dword mbr_0x22c;
			dword mbr_0x230;
			dword mbr_0x234;
			byte mbr_0x238;
			dword mbr_0x23c;
			dword mbr_0x240;
			byte mbr_0x24c;
			dword mbr_0x250;
			dword mbr_0x254;
			dword mbr_0x258;
			dword mbr_0x25c;
			dword mbr_0x260;
			dword mbr_0x264;
			dword mbr_0x268;
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
			dword mbr_0xcc;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xf0;
			dword mbr_0xfc;
		//: Methods
		public:
			virt_meth_0x4e8e60;
			meth_0x4e8e90;
			virt_meth_0x4e8f30;
			virt_meth_0x4e8f70;
			virt_meth_0x4e9170;
			virt_meth_0x4e9530;
			dtor_0x4ea090;
			virt_meth_0x4ea2e0;
			virt_meth_0x4ea370;
			meth_0x4ea6f0;
			meth_0x4ebbb0;
			virt_meth_0x4ec990;
			virt_meth_0x4eceb0;
			ctor_0x4ecfb0;
	}; // VFX_Lightning
} // namespace f3

#endif // F3_VFX_LIGHTNING_HPP_
