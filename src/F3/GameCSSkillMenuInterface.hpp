// Copyright 2024 OpenJE

#ifndef F3_GAMECSSKILLMENUINTERFACE_HPP_
#define F3_GAMECSSKILLMENUINTERFACE_HPP_

#include "F3.hpp"
#include "JE/CSMenuInterface.hpp"
#include "JE/cls_0x4395a0.hpp"

namespace F3 {
	class GameCSSkillMenuInterface : public JE::CSMenuInterface, public JE::cls_0x4395a0 {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xd24;
			dword mbr_0xd28;
			dword mbr_0xd2c;
			dword mbr_0xd30;
			dword mbr_0xd58;
		//: Methods
		public:
			void ctor_0x435c90();
			undefined4 virt_meth_0x435f20();
			undefined4 virt_meth_0x435f30();
			undefined4 virt_meth_0x435f40();
			undefined4 virt_meth_0x435f50();
			undefined4 virt_meth_0x435fa0();
			undefined4 meth_0x569080();
	}; // GameCSSkillMenuInterface
} // namespace F3

#endif // F3_GAMECSSKILLMENUINTERFACE_HPP_
