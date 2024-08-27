// Copyright 2024 OpenJE

#ifndef F3_CJUMPLIST_HPP_
#define F3_CJUMPLIST_HPP_

#include "f3/global.hpp"
#include "f3/CSoundFile.hpp"

namespace f3 {
	class CJumplist : public CSoundFile {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x538b70;
			dtor_0x538bb0;
			virt_meth_0x538bf0;
	}; // CJumplist
} // namespace f3

#endif // F3_CJUMPLIST_HPP_
