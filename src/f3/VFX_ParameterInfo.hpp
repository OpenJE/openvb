// Copyright 2024 OpenJE

#ifndef F3_VFX_PARAMETERINFO_HPP_
#define F3_VFX_PARAMETERINFO_HPP_

#include "f3/global.hpp"
#include "f3/VFX_Parameter.hpp"
#include "f3/VFX_Parameter.hpp"
#include "f3/VFX_Parameter.hpp"
#include "f3/VFX_Parameter.hpp"

namespace f3 {
	class VFX_ParameterInfo : public VFX_Parameter {
		//: Members
		public:
			byte mbr_0x110;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x64;
			dword mbr_0x68;
			VFX_Parameter mbr_0x6c;
			byte mbr_0xa0;
			VFX_Parameter mbr_0xa4;
			byte mbr_0xd8;
			VFX_Parameter mbr_0xdc;
		//: Methods
		public:
			ctor_0x4fb2d0;
			dtor_0x4fb370;
			virt_meth_0x4fb430;
			ctor_0x4fb5b0;
			meth_0x4fdfb0;
			meth_0x4fdfd0;
			meth_0x4fdff0;
			meth_0x4fe010;
			meth_0x4fe040;
			meth_0x4fe060;
			meth_0x4fe080;
			meth_0x4fe0a0;
			meth_0x4fe0c0;
			meth_0x4fe140;
			meth_0x4fe160;
			meth_0x4fe180;
			meth_0x4fe1b0;
			meth_0x4fe280;
			meth_0x4fe450;
			meth_0x4fe690;
			meth_0x4fe880;
			meth_0x4feb20;
			meth_0x4feef0;
			meth_0x4ff1b0;
			meth_0x4ff970;
			meth_0x4ffd10;
			meth_0x500150;
			meth_0x500760;
			meth_0x500bc0;
			meth_0x501000;
	}; // VFX_ParameterInfo
} // namespace f3

#endif // F3_VFX_PARAMETERINFO_HPP_
