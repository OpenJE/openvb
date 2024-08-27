// Copyright 2024 OpenJE

#ifndef F3_VFXEFFECTTYPE_SPRITE_STRUCT_HPP_
#define F3_VFXEFFECTTYPE_SPRITE_STRUCT_HPP_

#include "f3/global.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"
#include "f3/cls_0x40a220.hpp"
#include "f3/cls_0x50db20.hpp"
#include "f3/cls_0x4d8d70.hpp"

namespace f3 {
	struct VFXEFFECTTYPE_SPRITE_Struct : public VFXEFFECTTYPE_BASE_Struct, public cls_0x40a220, public cls_0x50db20 {
		//: Members
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			byte mbr_0x70;
			dword mbr_0x78;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			dword mbr_0xc0;
			dword mbr_0xc4;
			byte mbr_0xc8;
			byte mbr_0xc9;
			cls_0x4d8d70 mbr_0xca;
			byte mbr_0xcc;
			byte mbr_0xcd;
			byte mbr_0xce;
			dword mbr_0xcf;
			byte mbr_0xd1;
			dword mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			byte mbr_0xe0;
			byte mbr_0xe1;
			byte mbr_0xe2;
			byte mbr_0xe3;
			dword mbr_0xe4;
			dword mbr_0xe8;
			dword mbr_0xec;
			byte mbr_0xf0;
			byte mbr_0xf1;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
			ctor_0x4f4fe0;
			meth_0x4f5050;
			virt_meth_0x4f50c0;
			meth_0x4f5fc0;
	}; // VFXEFFECTTYPE_SPRITE_Struct
} // namespace f3

#endif // F3_VFXEFFECTTYPE_SPRITE_STRUCT_HPP_
