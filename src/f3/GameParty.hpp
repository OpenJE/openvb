// Copyright 2024 OpenJE

#ifndef F3_GAMEPARTY_HPP_
#define F3_GAMEPARTY_HPP_

#include "f3/global.hpp"
#include "je/Party.hpp"

namespace F3 {
	struct GameParty : public JE::Party {
		//: Members
			dword mbr_0x14;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			ctor_0x46cd00;
			virt_meth_0x46cd30;
	}; // GameParty
} // namespace F3

#endif // F3_GAMEPARTY_HPP_
