// Copyright 2024 OpenJE

#ifndef F3_GAMECURSORMANAGER_HPP_
#define F3_GAMECURSORMANAGER_HPP_

#include "F3.hpp"
#include "JE/CursorManager.hpp"

namespace F3 {
	struct GameCursorManager : public JE::CursorManager {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			byte mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			byte mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x8;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			byte mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xc;
		//: Methods
			undefined4 meth_0x4361f0();
			undefined4 meth_0x436210();
			void dtor_0x436270();
			undefined4 meth_0x436300();
			void ctor_0x436330();
			undefined4 virt_meth_0x4363f0();
			undefined4 meth_0x4364c0();
			undefined4 meth_0x436840();
			undefined4 virt_meth_0x437ab0();
			undefined4 virt_meth_0x437b20();
			undefined4 virt_meth_0x437b40();
			undefined4 virt_meth_0x437bb0();
			undefined4 virt_meth_0x437be0();
			undefined4 virt_meth_0x437d80();
			undefined4 virt_meth_0x5915d0();
			undefined4 meth_0x591670();
			undefined4 meth_0x591780();
			undefined4 meth_0x5919d0();
	}; // GameCursorManager
} // namespace F3

#endif // F3_GAMECURSORMANAGER_HPP_
