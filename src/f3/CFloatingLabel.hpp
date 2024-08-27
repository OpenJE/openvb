// Copyright 2024 OpenJE

#ifndef F3_CFLOATINGLABEL_HPP_
#define F3_CFLOATINGLABEL_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Label.hpp"

namespace f3 {
	class CFloatingLabel : public GUI::Label {
		//: Members
		public:
			dword mbr_0x18;
			byte mbr_0x1b9c;
			dword mbr_0xe64;
			dword mbr_0xe68;
			dword mbr_0xe6c;
			dword mbr_0xe70;
			dword mbr_0xe74;
			dword mbr_0xe78;
			dword mbr_0xe7c;
			dword mbr_0xe80;
		//: Methods
		public:
			ctor_0x58fa00;
			dtor_0x58fa90;
			virt_meth_0x58faf0;
			virt_meth_0x58fc60;
			virt_meth_0x58fca0;
	}; // CFloatingLabel
} // namespace f3

#endif // F3_CFLOATINGLABEL_HPP_
