// Copyright 2024 OpenJE

#ifndef F3_GAMEDIALOGUEINTERFACE_HPP_
#define F3_GAMEDIALOGUEINTERFACE_HPP_

#include "f3/global.hpp"
#include "je/DialogueInterface.hpp"
#include "je/cls_0x4395a0.hpp"

namespace F3 {
	class GameDialogueInterface : public JE::DialogueInterface, public JE::cls_0x4395a0 {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x124;
			dword mbr_0x128;
			dword mbr_0x12c;
			dword mbr_0x130;
			byte mbr_0x134;
			dword mbr_0x138;
			dword mbr_0x154;
			dword mbr_0x158;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x17c;
			dword mbr_0x180;
			dword mbr_0x184;
			dword mbr_0x188;
			byte mbr_0x18c;
			dword mbr_0x190;
			dword mbr_0x1ac;
			dword mbr_0x1c8;
			dword mbr_0x1cc;
			dword mbr_0x1d0;
			dword mbr_0x1d4;
			dword mbr_0x1d8;
			dword mbr_0x1dc;
			dword mbr_0x1e0;
			dword mbr_0x1fc;
			dword mbr_0x218;
			dword mbr_0x21c;
			dword mbr_0x220;
			dword mbr_0x224;
			dword mbr_0x228;
			dword mbr_0x22c;
			dword mbr_0x230;
			dword mbr_0x234;
			dword mbr_0x24;
			dword mbr_0x27c;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x4;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			byte mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x7c;
			dword mbr_0x8;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xcc;
			dword mbr_0xe8;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
		public:
			virt_meth_0x437fe0;
			ctor_0x438080;
			virt_meth_0x438e00;
			virt_meth_0x438e10;
			virt_meth_0x438e20;
			virt_meth_0x438e30;
	}; // GameDialogueInterface
} // namespace F3

#endif // F3_GAMEDIALOGUEINTERFACE_HPP_
