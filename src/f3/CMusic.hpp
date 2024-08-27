// Copyright 2024 OpenJE

#ifndef F3_CMUSIC_HPP_
#define F3_CMUSIC_HPP_

#include "f3/global.hpp"
#include "f3/CSound.hpp"

namespace f3 {
	class CMusic : public CSound {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x115;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
		public:
			ctor_0x539030;
			dtor_0x539070;
			virt_meth_0x539570;
	}; // CMusic
} // namespace f3

#endif // F3_CMUSIC_HPP_
