// Copyright 2024 OpenJE

#ifndef F3_GAMEHOTKEYS_HPP_
#define F3_GAMEHOTKEYS_HPP_

#include "F3.hpp"
#include "JE/HotKeys.hpp"

namespace F3 {
	struct GameHotKeys : public JE::HotKeys {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0xc;
		//: Methods
			undefined4 virt_meth_0x45f2b0();
			undefined4 meth_0x468ae0();
			undefined4 virt_meth_0x469090();
			undefined4 meth_0x599690();
			undefined4 meth_0x5997b0();
			undefined4 meth_0x599860();
	}; // GameHotKeys
} // namespace F3

#endif // F3_GAMEHOTKEYS_HPP_
