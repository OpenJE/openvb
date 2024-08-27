// Copyright 2024 OpenJE

#ifndef F3_CMASTERGROUPLIST_HPP_
#define F3_CMASTERGROUPLIST_HPP_

#include "f3/global.hpp"
#include "f3/CSoundFile.hpp"

namespace f3 {
	class CMasterGrouplist : public CSoundFile {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x4;
			dword mbr_0x470;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x52dfb0;
			virt_meth_0x52e040;
			meth_0x52e220;
			dtor_0x52e390;
	}; // CMasterGrouplist
} // namespace f3

#endif // F3_CMASTERGROUPLIST_HPP_
