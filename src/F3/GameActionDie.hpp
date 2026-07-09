// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONDIE_HPP_
#define F3_GAMEACTIONDIE_HPP_

#include "F3.hpp"
#include "JE/ActionDie.hpp"

namespace F3 {
	struct GameActionDie : public JE::ActionDie {
		//: Members
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			byte mbr_0x28;
			byte mbr_0x4;
		//: Methods
			void ctor_0x404e50();
			void dtor_0x404e80();
			undefined4 virt_meth_0x404e90();
			undefined4 virt_meth_0x405020();
			undefined4 virt_meth_0x405040();
	}; // GameActionDie
} // namespace F3

#endif // F3_GAMEACTIONDIE_HPP_
