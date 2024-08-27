// Copyright 2024 OpenJE

#ifndef F3_HOTKEYS_HPP_
#define F3_HOTKEYS_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct HotKeys {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			ctor_0x599810;
			virt_meth_0x5998c0;
			virt_meth_0x599a90;
			dtor_0x599cd0;
			virt_meth_0x599d50;
	}; // HotKeys
} // namespace f3

#endif // F3_HOTKEYS_HPP_
