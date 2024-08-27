// Copyright 2024 OpenJE

#ifndef F3_VFX_MOTIONBLUR_HPP_
#define F3_VFX_MOTIONBLUR_HPP_

#include "f3/global.hpp"
#include "f3/VFX_EffectInstance.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"

namespace f3 {
	class VFX_MotionBlur : public VFX_EffectInstance, public VFXEFFECTTYPE_BASE_Struct {
		//: Members
		public:
			dword mbr_0x134;
			dword mbr_0x138;
			dword mbr_0x13c;
			byte mbr_0x140;
			byte mbr_0x144;
			dword mbr_0xf0;
		//: Methods
		public:
			virt_meth_0x4f6780;
			dtor_0x4f6860;
			ctor_0x4f6890;
			virt_meth_0x4f68f0;
			meth_0x4f6910;
			virt_meth_0x4f6970;
	}; // VFX_MotionBlur
} // namespace f3

#endif // F3_VFX_MOTIONBLUR_HPP_
