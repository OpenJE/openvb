// Copyright 2024 OpenJE

#ifndef F3_GFX_PARTICLEINFO_HPP_
#define F3_GFX_PARTICLEINFO_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_ParticleInfoBase.hpp"
#include "f3/cls_0x515ed0.hpp"

namespace f3 {
	class Gfx_ParticleInfo : public Gfx_ParticleInfoBase, public cls_0x515ed0 {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x8;
			byte mbr_0x80;
			byte mbr_0x81;
			dword mbr_0x84;
			dword mbr_0xc;
			dword mbr_0xc8;
			dword mbr_0xcc;
			byte mbr_0xd0;
		//: Methods
		public:
			meth_0x513af0;
			ctor_0x514bc0;
			dtor_0x514c30;
			virt_meth_0x514c80;
			ctor_0x516010;
	}; // Gfx_ParticleInfo
} // namespace f3

#endif // F3_GFX_PARTICLEINFO_HPP_
