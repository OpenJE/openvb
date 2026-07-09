// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONMOVETO_HPP_
#define F3_GAMEACTIONMOVETO_HPP_

#include "F3.hpp"
#include "JE/ActionMoveTo.hpp"

namespace F3 {
	struct GameActionMoveTo : public JE::ActionMoveTo {
		//: Members
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x2c;
			dword mbr_0x30;
			byte mbr_0x3d;
		//: Methods
			void ctor_0x4056f0();
			void dtor_0x405710();
			undefined4 virt_meth_0x405720();
			undefined4 virt_meth_0x405770();
			undefined4 virt_meth_0x405790();
			undefined4 virt_meth_0x4057b0();
			undefined4 virt_meth_0x4058b0();
			undefined4 virt_meth_0x4060b0();
			undefined4 virt_meth_0x406780();
			undefined4 meth_0x55b350();
			undefined4 meth_0x55b420();
	}; // GameActionMoveTo
} // namespace F3

#endif // F3_GAMEACTIONMOVETO_HPP_
