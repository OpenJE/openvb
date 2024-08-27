// Copyright 2024 OpenJE

#ifndef F3_EFFECTTREETRIGGER_HPP_
#define F3_EFFECTTREETRIGGER_HPP_

#include "f3/global.hpp"
#include "f3/Trigger.hpp"

namespace f3 {
	struct EffectTreeTrigger : public Trigger {
		//: Members
			dword mbr_0x10;
			dword mbr_0x18;
			dword mbr_0x1c;
			byte mbr_0x24;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			byte mbr_0x74;
			dword mbr_0x78;
			byte mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x5aa530;
			virt_meth_0x5aa630;
			virt_meth_0x5aa750;
			virt_meth_0x5aa7e0;
			dtor_0x5aab40;
			ctor_0x5aad50;
			virt_meth_0x5aae20;
			virt_meth_0x5aae40;
			meth_0x5b16d0;
	}; // EffectTreeTrigger
} // namespace f3

#endif // F3_EFFECTTREETRIGGER_HPP_
