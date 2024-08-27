// Copyright 2024 OpenJE

#ifndef F3_EFFECTTREERINGTRIGGER_HPP_
#define F3_EFFECTTREERINGTRIGGER_HPP_

#include "f3/global.hpp"
#include "f3/EffectTreeTrigger.hpp"

namespace f3 {
	struct EffectTreeRingTrigger : public EffectTreeTrigger {
		//: Members
			dword mbr_0x2c;
			dword mbr_0x3c;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x78;
			byte mbr_0x7c;
			dword mbr_0x88;
			dword mbr_0x8c;
		//: Methods
			ctor_0x5b1fa0;
			dtor_0x5b1fd0;
			virt_meth_0x5b1fe0;
			virt_meth_0x5b2050;
			virt_meth_0x5b20b0;
	}; // EffectTreeRingTrigger
} // namespace f3

#endif // F3_EFFECTTREERINGTRIGGER_HPP_
