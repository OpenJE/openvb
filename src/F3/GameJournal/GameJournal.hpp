// Copyright 2024 OpenJE

#ifndef F3_GAMEJOURNAL_HPP_
#define F3_GAMEJOURNAL_HPP_

#include "f3/global.hpp"
#include "je/Journal.hpp"

namespace F3 {
	struct GameJournal : public JE::Journal {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			ctor_0x46c410;
			meth_0x46c480;
			dtor_0x46c550;
			virt_meth_0x46c5e0;
			virt_meth_0x46c5f0;
	}; // GameJournal
} // namespace F3

#endif // F3_GAMEJOURNAL_HPP_
