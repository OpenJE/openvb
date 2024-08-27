// Copyright 2024 OpenJE

#ifndef F3_ACTIONMESSAGE_HPP_
#define F3_ACTIONMESSAGE_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionMessage : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			byte mbr_0x4;
		//: Methods
			ActionMessage(); // ctor_0x5a8ff0
			virtual undefined4 virt_meth_0x5a9070(); // virt_meth_0x5a9070
			virtual undefined4 virt_meth_0x5a9080(); // virt_meth_0x5a9080
	}; // ActionMessage
} // namespace f3

#endif // F3_ACTIONMESSAGE_HPP_
