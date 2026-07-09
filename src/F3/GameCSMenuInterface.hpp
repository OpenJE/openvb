// Copyright 2024 OpenJE

#ifndef F3_GAMECSMENUINTERFACE_HPP_
#define F3_GAMECSMENUINTERFACE_HPP_

#include "F3.hpp"
#include "F3/GameCSSkillMenuInterface.hpp"

namespace F3 {
	class GameCSMenuInterface : public GameCSSkillMenuInterface {
		//: Members
		public:
			dword mbr_0x1ae4;
			dword mbr_0x4;
			dword mbr_0xd50;
		//: Methods
		public:
			undefined4 virt_meth_0x435800();
			undefined4 virt_meth_0x435810();
			undefined4 virt_meth_0x435820();
			undefined4 virt_meth_0x435830();
			void ctor_0x435840();
			undefined4 virt_meth_0x4359f0();
	}; // GameCSMenuInterface
} // namespace F3

#endif // F3_GAMECSMENUINTERFACE_HPP_
