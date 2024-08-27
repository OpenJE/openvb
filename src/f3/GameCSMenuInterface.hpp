// Copyright 2024 OpenJE

#ifndef F3_GAMECSMENUINTERFACE_HPP_
#define F3_GAMECSMENUINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/GameCSSkillMenuInterface.hpp"

namespace f3 {
	class GameCSMenuInterface : public GameCSSkillMenuInterface {
		//: Members
		public:
			dword mbr_0x1ae4;
			dword mbr_0x4;
			dword mbr_0xd50;
		//: Methods
		public:
			virt_meth_0x435800;
			virt_meth_0x435810;
			virt_meth_0x435820;
			virt_meth_0x435830;
			ctor_0x435840;
			virt_meth_0x4359f0;
	}; // GameCSMenuInterface
} // namespace f3

#endif // F3_GAMECSMENUINTERFACE_HPP_
