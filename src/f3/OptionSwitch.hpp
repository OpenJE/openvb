// Copyright 2024 OpenJE

#ifndef F3_OPTIONSWITCH_HPP_
#define F3_OPTIONSWITCH_HPP_

#include "f3/global.hpp"
#include "f3/OptionDial.hpp"

namespace f3 {
	class OptionSwitch : public OptionDial {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x4432a0;
			dtor_0x4432c0;
			virt_meth_0x443510;
			virt_meth_0x443d70;
	}; // OptionSwitch
} // namespace f3

#endif // F3_OPTIONSWITCH_HPP_
