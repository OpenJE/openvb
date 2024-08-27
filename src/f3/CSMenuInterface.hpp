// Copyright 2024 OpenJE

#ifndef F3_CSMENUINTERFACE_HPP_
#define F3_CSMENUINTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/CEngineInterface.hpp"

namespace f3 {
	class CSMenuInterface : public CEngineInterface {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xd24;
			dword mbr_0xd28;
			dword mbr_0xd2c;
			dword mbr_0xd30;
			dword mbr_0xd34;
			dword mbr_0xd38;
			dword mbr_0xd3c;
			dword mbr_0xd40;
			dword mbr_0xd44;
			dword mbr_0xd48;
			dword mbr_0xd4c;
		//: Methods
		public:
			virt_meth_0x568b40;
			virt_meth_0x568b50;
			virt_meth_0x568b60;
			ctor_0x568e50;
	}; // CSMenuInterface
} // namespace f3

#endif // F3_CSMENUINTERFACE_HPP_
