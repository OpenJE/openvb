// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONINITIATEDIALOGUE_HPP_
#define F3_GAMEACTIONINITIATEDIALOGUE_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct GameActionInitiateDialogue : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x18;
			byte mbr_0x19;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x4;
		//: Methods
			ctor_0x404b40;
			virt_meth_0x404b80;
			virt_meth_0x404be0;
	}; // GameActionInitiateDialogue
} // namespace f3

#endif // F3_GAMEACTIONINITIATEDIALOGUE_HPP_
