// Copyright 2024 OpenJE

#ifndef F3_GFX_BASEPACKETDATA_HPP_
#define F3_GFX_BASEPACKETDATA_HPP_

#include "f3/global.hpp"

namespace f3 {
	class Gfx_BasePacketData {
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
			byte mbr_0x4;
			byte mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			dtor_0x422950;
			meth_0x4b7660;
			ctor_0x4b7990;
	}; // Gfx_BasePacketData
} // namespace f3

#endif // F3_GFX_BASEPACKETDATA_HPP_
