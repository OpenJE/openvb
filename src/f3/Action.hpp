// Copyright 2024 OpenJE

#ifndef F3_ACTION_HPP_
#define F3_ACTION_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Action {
		//: Members
			byte mbr_0x4;
		//: Methods
			//~Action(); // dtor_0x4066f0
			virtual ~Action(); // virt_deldtor_0x4010e0
			virtual byte Get_mbr_0x4 (); // virt_meth_0x4010d0
	}; // Action
} // namespace f3

#endif // F3_ACTION_HPP_
