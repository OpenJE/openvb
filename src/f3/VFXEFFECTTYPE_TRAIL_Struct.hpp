// Copyright 2024 OpenJE

#ifndef F3_VFXEFFECTTYPE_TRAIL_STRUCT_HPP_
#define F3_VFXEFFECTTYPE_TRAIL_STRUCT_HPP_

#include "f3/global.hpp"
#include "f3/VFXEFFECTTYPE_BASE_Struct.hpp"
#include "f3/cls_0x4d8d70.hpp"
#include "f3/cls_0x4d8d70.hpp"
#include "f3/cls_0x502110.hpp"
#include "f3/cls_0x50db20.hpp"
#include "f3/cls_0x4dff90.hpp"
#include "f3/cls_0x40a220.hpp"
#include "f3/cls_0x50db20.hpp"

namespace f3 {
	struct VFXEFFECTTYPE_TRAIL_Struct : public VFXEFFECTTYPE_BASE_Struct, public cls_0x4d8d70, public cls_0x50db20 {
		//: Members
			cls_0x4d8d70 mbr_0x70;
			cls_0x502110 mbr_0x80;
			cls_0x50db20 mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x9c;
			dword mbr_0xa0;
			cls_0x4dff90 mbr_0xa8;
			cls_0x40a220 mbr_0xbc;
			dword mbr_0xc0;
			dword mbr_0xd0;
			dword mbr_0xd4;
			dword mbr_0xdc;
			dword mbr_0xec;
			dword mbr_0xf0;
		//: Methods
			ctor_0x4f16d0;
			meth_0x4f1770;
			virt_meth_0x4f1810;
	}; // VFXEFFECTTYPE_TRAIL_Struct
} // namespace f3

#endif // F3_VFXEFFECTTYPE_TRAIL_STRUCT_HPP_
