// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONIDLE_HPP_
#define F3_GAMEACTIONIDLE_HPP_

#include "f3/global.hpp"
#include "je/ActionIdle.hpp"

namespace F3 {
	struct GameActionIdle : public JE::ActionIdle {
		//: Members
			dword mbr_0x14;
			dword mbr_0x1c;
			byte mbr_0x20;
		//: Methods
			ctor_0x4055f0;
			dtor_0x405610;
			virt_meth_0x405620;
			virt_meth_0x405690;
	}; // GameActionIdle
} // namespace F3

#endif // F3_GAMEACTIONIDLE_HPP_
