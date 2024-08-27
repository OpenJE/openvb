// Copyright 2024 OpenJE

#ifndef F3_CSOUNDGROUP_HPP_
#define F3_CSOUNDGROUP_HPP_

#include "f3/global.hpp"
#include "f3/CSoundFile.hpp"

namespace f3 {
	class CSoundGroup : public CSoundFile {
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
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x52dbf0;
			dtor_0x52dc90;
			virt_meth_0x52dd00;
	}; // CSoundGroup
} // namespace f3

#endif // F3_CSOUNDGROUP_HPP_
