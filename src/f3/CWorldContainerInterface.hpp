// Copyright 2024 OpenJE

#ifndef F3_CWORLDCONTAINERINTERFACE_HPP_
#define F3_CWORLDCONTAINERINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Interface.hpp"
#include "f3/CSound.hpp"

namespace f3 {
	class CWorldContainerInterface : public GUI::Interface, public CSound {
		//: Members
		public:
			dword mbr_0x138;
			dword mbr_0x13c;
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x150;
			dword mbr_0x154;
			dword mbr_0x1a0;
			dword mbr_0x1a4;
			dword mbr_0x1a8;
			dword mbr_0x1ac;
			dword mbr_0x1b0;
			dword mbr_0x1b4;
			byte mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
		//: Methods
		public:
			meth_0x4574b0;
			meth_0x4574f0;
			meth_0x457520;
			virt_meth_0x4575d0;
			virt_meth_0x4577d0;
			meth_0x457a10;
			dtor_0x457d20;
			ctor_0x457db0;
			virt_meth_0x458030;
			meth_0x495290;
			meth_0x4952b0;
	}; // CWorldContainerInterface
} // namespace f3

#endif // F3_CWORLDCONTAINERINTERFACE_HPP_
