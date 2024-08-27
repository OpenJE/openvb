// Copyright 2024 OpenJE

#ifndef F3_TRANSITIONTRIGGER_HPP_
#define F3_TRANSITIONTRIGGER_HPP_

#include "f3/global.hpp"
#include "f3/Trigger.hpp"

namespace f3 {
	struct TransitionTrigger : public Trigger {
		//: Members
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			byte mbr_0x4d;
		//: Methods
			ctor_0x56e3c0;
			virt_meth_0x56e400;
			dtor_0x56e420;
			meth_0x5b1680;
			virt_meth_0x5b1980;
			virt_meth_0x5b1a60;
	}; // TransitionTrigger
} // namespace f3

#endif // F3_TRANSITIONTRIGGER_HPP_
