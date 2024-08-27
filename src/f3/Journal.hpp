// Copyright 2024 OpenJE

#ifndef F3_JOURNAL_HPP_
#define F3_JOURNAL_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Journal {
		//: Members
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			ctor_0x5a0520;
			dtor_0x5a0560;
			virt_meth_0x5a05c0;
			virt_meth_0x5a05f0;
	}; // Journal
} // namespace f3

#endif // F3_JOURNAL_HPP_
