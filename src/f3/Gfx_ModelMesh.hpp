// Copyright 2024 OpenJE

#ifndef F3_GFX_MODELMESH_HPP_
#define F3_GFX_MODELMESH_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_NodeAttribute.hpp"
#include "f3/cls_0x4abc60.hpp"

namespace f3 {
	class Gfx_ModelMesh : public Gfx_NodeAttribute, public cls_0x4abc60 {
		//: Members
		public:
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			byte mbr_0x28;
			byte mbr_0x29;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x49ddd0;
			virt_meth_0x49de80;
			dtor_0x50e3d0;
			meth_0x50e4f0;
			meth_0x50e760;
	}; // Gfx_ModelMesh
} // namespace f3

#endif // F3_GFX_MODELMESH_HPP_
