// Copyright 2024 OpenJE

#ifndef F3_G3D_PARTICLEEMITTER_HPP_
#define F3_G3D_PARTICLEEMITTER_HPP_

#include "f3/global.hpp"
#include "f3/G3D_BaseAttribute.hpp"

namespace f3 {
	class G3D_ParticleEmitter : public G3D_BaseAttribute {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x8;
		//: Methods
		public:
			virt_meth_0x4cac90;
			ctor_0x4cbb90;
			virt_meth_0x4cbbd0;
			dtor_0x4cbbf0;
			virt_meth_0x4cef30;
			virt_meth_0x4d2260;
			virt_meth_0x4d2330;
			virt_meth_0x4d23e0;
	}; // G3D_ParticleEmitter
} // namespace f3

#endif // F3_G3D_PARTICLEEMITTER_HPP_
