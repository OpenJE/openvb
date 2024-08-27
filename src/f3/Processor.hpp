// Copyright 2024 OpenJE

#ifndef F3_PROCESSOR_HPP_
#define F3_PROCESSOR_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Processor {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			dtor_0x580a70;
			ctor_0x580ac0;
			virt_meth_0x580ae0;
	}; // Processor
} // namespace f3

#endif // F3_PROCESSOR_HPP_
