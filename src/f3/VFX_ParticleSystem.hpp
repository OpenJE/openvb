// Copyright 2024 OpenJE

#ifndef F3_VFX_PARTICLESYSTEM_HPP_
#define F3_VFX_PARTICLESYSTEM_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/Gfx_LineSystem.hpp"
#include "f3/VFXEFFECTTYPE_PARTICLESYSTEM_Struct.hpp"

namespace f3 {
	class VFX_ParticleSystem : public VFX_EffectInstance, public Gfx_LineSystem, public VFXEFFECTTYPE_PARTICLESYSTEM_Struct {
		//: Members
		public:
			dword mbr_0x138;
			byte mbr_0x1b4;
			dword mbr_0x1f0;
			dword mbr_0x200;
			dword mbr_0x204;
			dword mbr_0x238;
			dword mbr_0x248;
			dword mbr_0x24c;
			dword mbr_0x2a0;
			dword mbr_0x2a4;
			byte mbr_0x2c0;
			dword mbr_0x4;
			dword mbr_0x5d8;
			dword mbr_0x5dc;
			dword mbr_0x5e0;
			dword mbr_0x5e4;
			dword mbr_0x5e8;
			dword mbr_0x5ec;
			dword mbr_0x5f0;
			dword mbr_0x74;
			dword mbr_0x78;
			byte mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0xb8;
			byte mbr_0xc0;
			byte mbr_0xc1;
			dword mbr_0xc4;
			dword mbr_0xf0;
		//: Methods
		public:
			meth_0x4e1d20;
			meth_0x4f69b0;
			virt_meth_0x4f69e0;
			virt_meth_0x4f6a20;
			virt_meth_0x4f6a50;
			virt_meth_0x4f6b90;
			virt_meth_0x4f6bb0;
			virt_meth_0x4f6bd0;
			virt_meth_0x4f6c20;
			meth_0x4f70a0;
			ctor_0x4f7110;
			dtor_0x4f71b0;
			virt_meth_0x4f72a0;
			virt_meth_0x4f72c0;
			virt_meth_0x4f74e0;
			virt_meth_0x4f76b0;
			meth_0x4f87e0;
			virt_meth_0x4f94e0;
			meth_0x4f9850;
	}; // VFX_ParticleSystem
} // namespace f3

#endif // F3_VFX_PARTICLESYSTEM_HPP_
