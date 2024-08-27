// Copyright 2024 OpenJE

#ifndef F3_GAMEEFFECT_HPP_
#define F3_GAMEEFFECT_HPP_

#include "f3/global.hpp"
#include "f3/Effect.hpp"

namespace f3 {
	struct GameEffect : public Effect {
		//: Members
			dword mbr_0x1c;
			dword mbr_0x24;
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
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			byte mbr_0xa;
			byte mbr_0xb;
		//: Methods
			virt_meth_0x45c0a0;
			ctor_0x462434;
			dtor_0x4624e0;
			virt_meth_0x4624f0;
			virt_meth_0x4625c0;
			virt_meth_0x4625e0;
			virt_meth_0x462600;
			virt_meth_0x462620;
			virt_meth_0x4626a0;
			virt_meth_0x462710;
			virt_meth_0x462740;
			virt_meth_0x462930;
			meth_0x55ae10;
	}; // GameEffect
} // namespace f3

#endif // F3_GAMEEFFECT_HPP_
