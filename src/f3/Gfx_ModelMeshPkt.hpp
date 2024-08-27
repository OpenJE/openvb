// Copyright 2024 OpenJE

#ifndef F3_GFX_MODELMESHPKT_HPP_
#define F3_GFX_MODELMESHPKT_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_NodeAttribute.hpp"

namespace f3 {
	class Gfx_ModelMeshPkt : public Gfx_NodeAttribute {
		//: Members
		public:
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			byte mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			byte mbr_0x44;
			byte mbr_0x45;
			byte mbr_0x46;
			byte mbr_0x47;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x49e1d0;
			virt_meth_0x49ebe0;
			dtor_0x49ec00;
	}; // Gfx_ModelMeshPkt
} // namespace f3

#endif // F3_GFX_MODELMESHPKT_HPP_
