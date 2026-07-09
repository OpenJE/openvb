// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONWAIT_HPP_
#define F3_GAMEACTIONWAIT_HPP_

#include "F3.hpp"
#include "JE/ActionWait.hpp"

namespace F3 {
	struct GameActionWait : public JE::ActionWait {
		//: Members
			dword mbr_0x14;
		//: Methods
			void ctor_0x407ed0();
			void dtor_0x407ef0();
			undefined4 virt_meth_0x407f00();
			undefined4 virt_meth_0x407f60();
	}; // GameActionWait
} // namespace F3

#endif // F3_GAMEACTIONWAIT_HPP_
