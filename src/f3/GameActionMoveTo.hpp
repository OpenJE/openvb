// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONMOVETO_HPP_
#define F3_GAMEACTIONMOVETO_HPP_

#include "f3/global.hpp"
#include "f3/ActionMoveTo.hpp"

namespace f3 {
	struct GameActionMoveTo : public ActionMoveTo {
		//: Members
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x2c;
			dword mbr_0x30;
			byte mbr_0x3d;
		//: Methods
			ctor_0x4056f0;
			dtor_0x405710;
			virt_meth_0x405720;
			virt_meth_0x405770;
			virt_meth_0x405790;
			virt_meth_0x4057b0;
			virt_meth_0x4058b0;
			virt_meth_0x4060b0;
			virt_meth_0x406780;
			meth_0x55b350;
			meth_0x55b420;
	}; // GameActionMoveTo
} // namespace f3

#endif // F3_GAMEACTIONMOVETO_HPP_
