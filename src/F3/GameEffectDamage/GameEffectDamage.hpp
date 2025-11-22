// Copyright 2024 OpenJE

#ifndef F3_GAMEEFFECTDAMAGE_HPP_
#define F3_GAMEEFFECTDAMAGE_HPP_

#include "f3/global.hpp"
#include "f3/GameEffect.hpp"

namespace F3 {
	struct GameEffectDamage : public GameEffect {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x68;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			byte mbr_0xac;
			byte mbr_0xad;
			byte mbr_0xb;
		//: Methods
			ctor_0x45c040;
			dtor_0x45c0b0;
			virt_meth_0x45c0c0;
			virt_meth_0x45c100;
			virt_meth_0x45c120;
			virt_meth_0x45c6b0;
			meth_0x462860;
	}; // GameEffectDamage
} // namespace F3

#endif // F3_GAMEEFFECTDAMAGE_HPP_
