// Copyright 2024 OpenJE

#ifndef F3_GAMEJOURNAL_HPP_
#define F3_GAMEJOURNAL_HPP_

#include "F3.hpp"
#include "JE/Journal.hpp"

namespace F3 {
	struct GameJournal : public JE::Journal {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			void ctor_0x46c410();
			undefined4 meth_0x46c480();
			void dtor_0x46c550();
			undefined4 virt_meth_0x46c5e0();
			undefined4 virt_meth_0x46c5f0();
	}; // GameJournal
} // namespace F3

#endif // F3_GAMEJOURNAL_HPP_
