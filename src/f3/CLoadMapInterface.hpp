// Copyright 2024 OpenJE

#ifndef F3_CLOADMAPINTERFACE_HPP_
#define F3_CLOADMAPINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Interface.hpp"

namespace f3 {
	class CLoadMapInterface : public GUI::Interface {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x138;
			dword mbr_0x13c;
			dword mbr_0x140;
			dword mbr_0x150;
			dword mbr_0x1a0;
			dword mbr_0x1a4;
			dword mbr_0x1a8;
			dword mbr_0x1ac;
			dword mbr_0x1b0;
			dword mbr_0x1b4;
			byte mbr_0x20;
			dword mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0xfc;
		//: Methods
		public:
			dtor_0x43ff40;
			virt_meth_0x43ffb0;
			ctor_0x440050;
			virt_meth_0x4404a0;
			meth_0x456ee0;
			meth_0x457010;
			meth_0x457260;
			meth_0x457440;
			meth_0x4574a0;
			meth_0x4580c0;
			meth_0x458710;
			meth_0x495220;
			meth_0x4955b0;
			meth_0x495740;
			meth_0x495b50;
	}; // CLoadMapInterface
} // namespace f3

#endif // F3_CLOADMAPINTERFACE_HPP_
