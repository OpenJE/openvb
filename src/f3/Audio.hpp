// Copyright 2024 OpenJE

#ifndef F3_AUDIO_HPP_
#define F3_AUDIO_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct Audio {
		//: Members
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x34;
			dword mbr_0x38;
			byte mbr_0x3c;
			byte mbr_0x3d;
			byte mbr_0x3e;
			dword mbr_0x4;
			dword mbr_0x40;
			byte mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x8;
		//: Methods
			meth_0x5909e0;
			ctor_0x590b10;
			virt_meth_0x590b50;
			virt_meth_0x590be0;
			virt_meth_0x591110;
			dtor_0x591520;
			virt_meth_0x5915b0;
	}; // Audio
} // namespace f3

#endif // F3_AUDIO_HPP_
