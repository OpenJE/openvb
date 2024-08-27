// Copyright 2024 OpenJE

#ifndef F3_EDITTOKENINTERFACE_HPP_
#define F3_EDITTOKENINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/CEngineInterface.hpp"

namespace f3 {
	class EditTokenInterface : public CEngineInterface {
		//: Members
		public:
			dword mbr_0x38;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
		//: Methods
		public:
			ctor_0x5b3e60;
			virt_meth_0x5b3fd0;
			meth_0x5b3ff0;
			meth_0x5b4130;
			virt_meth_0x5b42a0;
	}; // EditTokenInterface
} // namespace f3

#endif // F3_EDITTOKENINTERFACE_HPP_
