// Copyright 2024 OpenJE

#ifndef F3_VISUALEFFECT_HPP_
#define F3_VISUALEFFECT_HPP_

#include "f3/global.hpp"
#include "f3/DynamicObject.hpp"

namespace f3 {
	struct VisualEffect : public DynamicObject {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x164;
			byte mbr_0x168;
			dword mbr_0x16c;
			byte mbr_0x170;
			dword mbr_0x174;
			dword mbr_0x178;
			dword mbr_0x17c;
			dword mbr_0x18;
			byte mbr_0x180;
			dword mbr_0x184;
			dword mbr_0x1c;
			byte mbr_0x1c4;
			byte mbr_0x1c5;
			byte mbr_0x1c6;
			byte mbr_0x1c7;
			dword mbr_0x20;
			dword mbr_0x24;
			byte mbr_0x30;
		//: Methods
			virt_meth_0x5682a0;
			dtor_0x59f6f0;
			virt_meth_0x59f760;
			virt_meth_0x59f770;
			virt_meth_0x59f7a0;
			virt_meth_0x59f7f0;
			virt_meth_0x59f810;
			virt_meth_0x59f830;
			virt_meth_0x59f840;
			meth_0x59f8d0;
			meth_0x59f8e0;
			virt_meth_0x59f930;
			meth_0x59f950;
			meth_0x59fa40;
			meth_0x59fc70;
			ctor_0x59fcb0;
			virt_meth_0x59fd70;
			virt_meth_0x59fdb0;
			meth_0x59ff20;
	}; // VisualEffect
} // namespace f3

#endif // F3_VISUALEFFECT_HPP_
