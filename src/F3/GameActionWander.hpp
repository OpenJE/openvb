// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONWANDER_HPP_
#define F3_GAMEACTIONWANDER_HPP_

#include "F3.hpp"
#include "JE/ActionWander.hpp"

namespace F3 {
	struct GameActionWander : public JE::ActionWander {
		//: Members
			dword mbr_0x14;
		//: Methods
			undefined4 virt_meth_0x407f80();
			undefined4 virt_meth_0x407f90();
			void ctor_0x46f140();
	}; // GameActionWander
} // namespace F3

#endif // F3_GAMEACTIONWANDER_HPP_
