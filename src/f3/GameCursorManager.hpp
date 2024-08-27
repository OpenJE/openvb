// Copyright 2024 OpenJE

#ifndef F3_GAMECURSORMANAGER_HPP_
#define F3_GAMECURSORMANAGER_HPP_

#include "f3/global.hpp"
#include "f3/CursorManager.hpp"

namespace f3 {
	struct GameCursorManager : public CursorManager {
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
			meth_0x4361f0;
			meth_0x436210;
			dtor_0x436270;
			meth_0x436300;
			ctor_0x436330;
			virt_meth_0x4363f0;
			meth_0x4364c0;
			meth_0x436840;
			virt_meth_0x437ab0;
			virt_meth_0x437b20;
			virt_meth_0x437b40;
			virt_meth_0x437bb0;
			virt_meth_0x437be0;
			virt_meth_0x437d80;
			virt_meth_0x5915d0;
			meth_0x591670;
			meth_0x591780;
			meth_0x5919d0;
	}; // GameCursorManager
} // namespace f3

#endif // F3_GAMECURSORMANAGER_HPP_
