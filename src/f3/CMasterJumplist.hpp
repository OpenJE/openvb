// Copyright 2024 OpenJE

#ifndef F3_CMASTERJUMPLIST_HPP_
#define F3_CMASTERJUMPLIST_HPP_

#include "f3/global.hpp"
#include "f3/CSoundFile.hpp"

namespace f3 {
	class CMasterJumplist : public CSoundFile {
		//: Members
		public:
			dword mbr_0x14;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			meth_0x4c7180;
			meth_0x4c7e80;
			ctor_0x539120;
			meth_0x539190;
			dtor_0x539210;
			virt_meth_0x539a00;
	}; // CMasterJumplist
} // namespace f3

#endif // F3_CMASTERJUMPLIST_HPP_
