// Copyright 2024 OpenJE

#ifndef F3_GAMECSSKILLMENUINTERFACE_HPP_
#define F3_GAMECSSKILLMENUINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/CSMenuInterface.hpp"
#include "f3/cls_0x4395a0.hpp"

namespace f3 {
	class GameCSSkillMenuInterface : public CSMenuInterface, public cls_0x4395a0 {
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
			ctor_0x435c90;
			virt_meth_0x435f20;
			virt_meth_0x435f30;
			virt_meth_0x435f40;
			virt_meth_0x435f50;
			virt_meth_0x435fa0;
			meth_0x569080;
	}; // GameCSSkillMenuInterface
} // namespace f3

#endif // F3_GAMECSSKILLMENUINTERFACE_HPP_
