// Copyright 2024 OpenJE

#ifndef F3_CFLOATINGTEXTINTERFACE_HPP_
#define F3_CFLOATINGTEXTINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/CEngineInterface.hpp"

namespace f3 {
	class CFloatingTextInterface : public CEngineInterface {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			byte mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x140;
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x24;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			byte mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x8;
			byte mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xc0;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfb8;
			dword mbr_0xfc;
			dword mbr_0xfc8;
			dword mbr_0xfcc;
		//: Methods
		public:
			virt_meth_0x58f9b0;
			meth_0x58f9c0;
			virt_meth_0x5900c0;
			virt_meth_0x590120;
			ctor_0x590360;
			meth_0x590500;
			virt_meth_0x590750;
			virt_meth_0x5907e0;
	}; // CFloatingTextInterface
} // namespace f3

#endif // F3_CFLOATINGTEXTINTERFACE_HPP_
