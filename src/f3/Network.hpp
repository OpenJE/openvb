// Copyright 2024 OpenJE

#ifndef F3_NETWORK_HPP_
#define F3_NETWORK_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Network {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			word mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x4;
			word mbr_0x8;
			dword mbr_0xc;
		//: Methods
			dtor_0x543710;
			virt_meth_0x543730;
			virt_meth_0x543770;
			virt_meth_0x5437b0;
			virt_meth_0x543820;
			ctor_0x543890;
			virt_meth_0x543910;
			meth_0x543930;
			virt_meth_0x543a40;
			virt_meth_0x543ac0;
			virt_meth_0x5441f0;
	}; // Network
} // namespace f3

#endif // F3_NETWORK_HPP_
