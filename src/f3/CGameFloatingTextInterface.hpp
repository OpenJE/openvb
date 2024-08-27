// Copyright 2024 OpenJE

#ifndef F3_CGAMEFLOATINGTEXTINTERFACE_HPP_
#define F3_CGAMEFLOATINGTEXTINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/CFloatingTextInterface.hpp"
#include "f3/cls_0x4395a0.hpp"

namespace f3 {
	class CGameFloatingTextInterface : public CFloatingTextInterface, public cls_0x4395a0 {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			byte mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x24;
			dword mbr_0x37554;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
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
			dword mbr_0xe4;
			dword mbr_0xe8;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
		public:
			virt_meth_0x438f80;
			virt_meth_0x439070;
			virt_meth_0x439080;
			ctor_0x4390d0;
			virt_meth_0x439320;
	}; // CGameFloatingTextInterface
} // namespace f3

#endif // F3_CGAMEFLOATINGTEXTINTERFACE_HPP_
