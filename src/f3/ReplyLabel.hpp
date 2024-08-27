// Copyright 2024 OpenJE

#ifndef F3_REPLYLABEL_HPP_
#define F3_REPLYLABEL_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Label.hpp"

namespace f3 {
	class ReplyLabel : public GUI::Label {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xe64;
			dword mbr_0xe68;
		//: Methods
		public:
			ctor_0x591ba0;
			dtor_0x591c10;
			virt_meth_0x591f40;
			virt_meth_0x591f50;
			virt_meth_0x592680;
	}; // ReplyLabel
} // namespace f3

#endif // F3_REPLYLABEL_HPP_
