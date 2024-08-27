// Copyright 2024 OpenJE

#ifndef F3_OPTIONCHECKBOX_HPP_
#define F3_OPTIONCHECKBOX_HPP_

#include "f3/global.hpp"
#include "f3/OptionDial.hpp"

namespace f3 {
	class OptionCheckBox : public OptionDial {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x4432d0;
			dtor_0x4432f0;
			virt_meth_0x443530;
			virt_meth_0x443e90;
	}; // OptionCheckBox
} // namespace f3

#endif // F3_OPTIONCHECKBOX_HPP_
