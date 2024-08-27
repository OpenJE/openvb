// Copyright 2024 OpenJE

#ifndef F3_G3D_NODE_HPP_
#define F3_G3D_NODE_HPP_

#include "f3/global.hpp"
#include "f3/G3D_Transform.hpp"
#include "f3/cls_0x48fce0.hpp"

namespace f3 {
	class G3D_Node {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x110;
			dword mbr_0x114;
			dword mbr_0x11c;
			dword mbr_0x120;
			dword mbr_0x124;
			byte mbr_0x128;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			G3D_Transform mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x8;
			cls_0x48fce0 mbr_0xa0;
			dword mbr_0xc;
		//: Methods
		public:
			meth_0x4caaa0;
			virt_meth_0x4cab30;
			dtor_0x4ccef0;
			virt_meth_0x4cea90;
			virt_meth_0x4ceab0;
			ctor_0x4d7600;
			meth_0x4d80f0;
			meth_0x4d9400;
			virt_meth_0x4da230;
			virt_meth_0x4da760;
			virt_meth_0x4dabd0;
	}; // G3D_Node
} // namespace f3

#endif // F3_G3D_NODE_HPP_
