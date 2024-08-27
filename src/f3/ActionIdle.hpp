// Copyright 2024 OpenJE

#ifndef F3_ACTIONIDLE_HPP_
#define F3_ACTIONIDLE_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionIdle : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionIdle(); // ctor_0x560180
	}; // ActionIdle
} // namespace f3

#endif // F3_ACTIONIDLE_HPP_
