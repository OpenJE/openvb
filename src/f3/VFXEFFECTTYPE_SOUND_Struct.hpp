// Copyright 2024 OpenJE

#ifndef F3_VFXEFFECTTYPE_SOUND_STRUCT_HPP_
#define F3_VFXEFFECTTYPE_SOUND_STRUCT_HPP_

#include "f3/global.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"
#include "f3/cls_0x40a220.hpp"
#include "f3/cls_0x40a220.hpp"

namespace f3 {
	struct VFXEFFECTTYPE_SOUND_Struct : public VFXEFFECTTYPE_BASE_Struct, public cls_0x40a220 {
		//: Members
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x4;
			byte mbr_0x50;
			byte mbr_0x51;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			byte mbr_0x6c;
			byte mbr_0x6d;
			cls_0x40a220 mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xac;
			dword mbr_0xbc;
			dword mbr_0xc0;
			dword mbr_0xc4;
			dword mbr_0xc8;
			dword mbr_0xd8;
			dword mbr_0xdc;
			byte mbr_0xe0;
			byte mbr_0xe1;
		//: Methods
			meth_0x4e1c60;
			ctor_0x4e3000;
			meth_0x4e3060;
			virt_meth_0x4e3120;
			meth_0x4e3430;
	}; // VFXEFFECTTYPE_SOUND_Struct
} // namespace f3

#endif // F3_VFXEFFECTTYPE_SOUND_STRUCT_HPP_
