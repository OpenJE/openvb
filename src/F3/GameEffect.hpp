// Copyright 2024 OpenJE

#ifndef F3_GAMEEFFECT_HPP_
#define F3_GAMEEFFECT_HPP_

#include "F3.hpp"
#include "JE/Effect.hpp"

namespace F3 {
	struct GameEffect : public JE::Effect {
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
			undefined4 virt_meth_0x45c0a0();
			void ctor_0x462434();
			void dtor_0x4624e0();
			undefined4 virt_meth_0x4624f0();
			undefined4 virt_meth_0x4625c0();
			undefined4 virt_meth_0x4625e0();
			undefined4 virt_meth_0x462600();
			undefined4 virt_meth_0x462620();
			undefined4 virt_meth_0x4626a0();
			undefined4 virt_meth_0x462710();
			undefined4 virt_meth_0x462740();
			undefined4 virt_meth_0x462930();
			undefined4 meth_0x55ae10();
	}; // GameEffect
} // namespace F3

#endif // F3_GAMEEFFECT_HPP_
