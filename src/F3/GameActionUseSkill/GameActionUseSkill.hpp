// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONUSESKILL_HPP_
#define F3_GAMEACTIONUSESKILL_HPP_

#include "f3/global.hpp"
#include "je/Action.hpp"

namespace F3 {
	class GameActionUseSkill : public JE::Action {
		//: Members
		public:
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			byte mbr_0x20;
		//: Methods
		public:
			ctor_0x407ca0;
			virt_meth_0x407cc0;
	}; // GameActionUseSkill
} // namespace F3

#endif // F3_GAMEACTIONUSESKILL_HPP_
