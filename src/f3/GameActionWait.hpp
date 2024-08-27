// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONWAIT_HPP_
#define F3_GAMEACTIONWAIT_HPP_

#include "f3/global.hpp"
#include "f3/ActionWait.hpp"

namespace f3 {
	struct GameActionWait : public ActionWait {
		//: Members
			dword mbr_0x14;
		//: Methods
			ctor_0x407ed0;
			dtor_0x407ef0;
			virt_meth_0x407f00;
			virt_meth_0x407f60;
	}; // GameActionWait
} // namespace f3

#endif // F3_GAMEACTIONWAIT_HPP_
