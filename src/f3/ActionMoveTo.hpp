// Copyright 2024 OpenJE

#ifndef F3_ACTIONMOVETO_HPP_
#define F3_ACTIONMOVETO_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
	struct ActionMoveTo : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			byte mbr_0x34;
			byte mbr_0x35;
			dword mbr_0x38;
			byte mbr_0x3c;
			byte mbr_0x3d;
			byte mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
		//: Methods
			ActionMoveTo(); // ctor_0x55b300
			~ActionMoveTo(); // dtor_0x55b460
			virtual void virt_meth_0x406e90(); // virt_meth_0x406e90
			virtual float10 virt_meth_0x55b340();// virt_meth_0x55b340
			virtual void virt_meth_0x55b370(); // virt_meth_0x55b370
			virtual void virt_meth_0x55b3b0(); // virt_meth_0x55b3b0
			virtual undefined4 virt_meth_0x55b3f0(); // virt_meth_0x55b3f0
			virtual byte Get_mbr_0x3d(); // virt_meth_0x55b440
			virtual void virt_meth_0x55b450(); // virt_meth_0x55b450
			virtual undefined4 virt_meth_0x55b4f0(); // virt_meth_0x55b4f0
			virtual undefined4 * virt_meth_0x55b5e0(byte param_1); // virt_meth_0x55b5e0
			virtual undefined4 virt_meth_0x55b600(); // virt_meth_0x55b600
	}; // ActionMoveTo
} // namespace f3

#endif // F3_ACTIONMOVETO_HPP_
