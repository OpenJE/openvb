// Copyright 2024 OpenJE

#ifndef F3_GAMEHOTKEYS_HPP_
#define F3_GAMEHOTKEYS_HPP_

#include "f3/global.hpp"
#include "je/HotKeys.hpp"

namespace F3 {
	struct GameHotKeys : public JE::HotKeys {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x45f2b0;
			meth_0x468ae0;
			virt_meth_0x469090;
			meth_0x599690;
			meth_0x5997b0;
			meth_0x599860;
	}; // GameHotKeys
} // namespace F3

#endif // F3_GAMEHOTKEYS_HPP_
