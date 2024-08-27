// Copyright 2024 OpenJE

#ifndef F3_VFX_VEG_HPP_
#define F3_VFX_VEG_HPP_

#include "f3/global.hpp"
#include "f3/VFX_ParameterHolder.hpp"

namespace f3 {
	class VFX_VEG : public VFX_ParameterHolder {
		//: Members
		public:
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
		//: Methods
		public:
			virt_meth_0x501340;
			meth_0x501430;
			dtor_0x501a60;
			meth_0x501ad0;
			virt_meth_0x501ae0;
			ctor_0x501ec0;
			virt_meth_0x501fa0;
	}; // VFX_VEG
} // namespace f3

#endif // F3_VFX_VEG_HPP_
