// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONATTACK_HPP_
#define F3_GAMEACTIONATTACK_HPP_

#include "f3/global.hpp"
#include "je/Action.hpp"

namespace F3 {
	struct GameActionAttack : public JE::Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			byte mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			byte mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			byte mbr_0x58;
			byte mbr_0x59;
			byte mbr_0x5a;
			byte mbr_0x5b;
			byte mbr_0x5c;
			byte mbr_0x5d;
			byte mbr_0x5e;
			dword mbr_0x60;
			byte mbr_0x64;
			byte mbr_0x65;
			dword mbr_0x68;
			byte mbr_0x6c;
			byte mbr_0x6d;
			byte mbr_0x6e;
			byte mbr_0x6f;
			byte mbr_0x70;
			byte mbr_0x71;
			byte mbr_0x72;
			dword mbr_0x73;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			byte mbr_0xa4;
			byte mbr_0xa5;
			byte mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			dword mbr_0xc0;
			dword mbr_0xc4;
		//: Methods
			meth_0x401100;
			virt_meth_0x4011d0;
			meth_0x4012c0;
			meth_0x4012e0;
			meth_0x401410;
			meth_0x401790;
			meth_0x401890;
			meth_0x401a10;
			virt_meth_0x402080;
			virt_meth_0x4021b0;
			meth_0x402260;
			meth_0x4023c0;
			meth_0x402680;
			meth_0x402e00;
			meth_0x403300;
			meth_0x403320;
			meth_0x403770;
			dtor_0x403c70;
			virt_meth_0x403d50;
			ctor_0x403e40;
			virt_meth_0x403f80;
			meth_0x403fa0;
			meth_0x404060;
			meth_0x404270;
			virt_meth_0x404320;
	}; // GameActionAttack
} // namespace F3

#endif // F3_GAMEACTIONATTACK_HPP_
