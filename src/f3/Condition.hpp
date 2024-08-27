// Copyright 2024 OpenJE

#ifndef F3_CONDITION_HPP_
#define F3_CONDITION_HPP_

#include "f3/global.hpp"
#include "f3/Effect.hpp"

namespace f3 {
	struct Condition : public Effect {
		//: Members
			dword mbr_0x14;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x38;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			byte mbr_0x6c;
			byte mbr_0x6d;
			byte mbr_0xa;
			byte mbr_0xb;
		//: Methods
			virt_meth_0x5a8c00;
			virt_meth_0x5a8c20;
			virt_meth_0x5a8c40;
			meth_0x5a8c50;
			ctor_0x5a8d30;
			virt_meth_0x5a8d70;
			dtor_0x5a8d80;
			virt_meth_0x5a8dc0;
			virt_meth_0x5a8e00;
			virt_meth_0x5a8eb0;
			virt_meth_0x5a8ed0;
			virt_meth_0x5a8f20;
	}; // Condition
} // namespace f3

#endif // F3_CONDITION_HPP_
