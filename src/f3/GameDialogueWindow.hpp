// Copyright 2024 OpenJE

#ifndef F3_GAMEDIALOGUEWINDOW_HPP_
#define F3_GAMEDIALOGUEWINDOW_HPP_

#include "f3/global.hpp"
#include "je/DialogueWindow.hpp"

namespace F3 {
	class GameDialogueWindow : public JE::DialogueWindow {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xd50;
			dword mbr_0xd58;
		//: Methods
		public:
			ctor_0x437ed0;
			virt_meth_0x437f00;
			virt_meth_0x437fd0;
			virt_meth_0x438050;
	}; // GameDialogueWindow
} // namespace F3

#endif // F3_GAMEDIALOGUEWINDOW_HPP_
