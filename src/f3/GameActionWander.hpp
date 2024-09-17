// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONWANDER_HPP_
#define F3_GAMEACTIONWANDER_HPP_

#include "f3/global.hpp"
#include "je/ActionWander.hpp"

namespace F3 {
	struct GameActionWander : public JE::ActionWander {
		//: Members
			dword mbr_0x14;
		//: Methods
			virt_meth_0x407f80;
			virt_meth_0x407f90;
			ctor_0x46f140;
	}; // GameActionWander
} // namespace F3

#endif // F3_GAMEACTIONWANDER_HPP_
