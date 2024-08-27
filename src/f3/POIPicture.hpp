// Copyright 2024 OpenJE

#ifndef F3_POIPICTURE_HPP_
#define F3_POIPICTURE_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Picture.hpp"

namespace f3 {
	class POIPicture : public GUI::Picture {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xd2c;
			dword mbr_0xd30;
			dword mbr_0xd34;
			dword mbr_0xd38;
		//: Methods
		public:
			ctor_0x458780;
			dtor_0x4587c0;
			virt_meth_0x4589c0;
			virt_meth_0x4589e0;
	}; // POIPicture
} // namespace f3

#endif // F3_POIPICTURE_HPP_
