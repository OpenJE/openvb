// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONOPERATEDOOR_HPP_
#define F3_GAMEACTIONOPERATEDOOR_HPP_

#include "f3/global.hpp"
#include "je/Action.hpp"

namespace F3 {
	struct GameActionOperateDoor : public JE::Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x24;
			byte mbr_0x25;
			byte mbr_0x4;
		//: Methods
			ctor_0x4066c0;
			meth_0x406700;
			virt_meth_0x406720;
			meth_0x406730;
			virt_meth_0x4067c0;
	}; // GameActionOperateDoor
} // namespace F3

#endif // F3_GAMEACTIONOPERATEDOOR_HPP_
