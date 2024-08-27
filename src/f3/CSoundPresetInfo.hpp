// Copyright 2024 OpenJE

#ifndef F3_CSOUNDPRESETINFO_HPP_
#define F3_CSOUNDPRESETINFO_HPP_

#include "f3/global.hpp"
#include "f3/CSoundFile.hpp"

namespace f3 {
	class CSoundPresetInfo : public CSoundFile {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x52cef0;
			dtor_0x52cf80;
			virt_meth_0x52d550;
	}; // CSoundPresetInfo
} // namespace f3

#endif // F3_CSOUNDPRESETINFO_HPP_
