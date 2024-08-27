// Copyright 2024 OpenJE

#ifndef F3_ACTIONANIM_HPP_
#define F3_ACTIONANIM_HPP_

#include "f3/global.hpp"
#include "f3/Action.hpp"
#include "f3/Entity.hpp"

namespace f3 {
	struct ActionAnim : public Action {
		//: Members
			byte mbr_0x10;
			Entity* entity;
			dword mbr_0x18;
			dword mbr_0x1c;
			word mbr_0x20;
			byte mbr_0x22;
			byte mbr_0x23;
			dword mbr_0x24;
			byte mbr_0x4;
		//: Methods
			ActionAnim(); // ctor_0x5a9230
			virtual undefined4* virt_meth_0x405350( byte param_1 ); // virt_meth_0x405350
			virtual byte Get_mbr_0x23 (); // virt_meth_0x5a9260
			virtual void virt_meth_0x5a9270(); // virt_meth_0x5a9270
			virtual undefined4 virt_meth_0x5a9280( int param1 ); // virt_meth_0x5a9280
			virtual undefined4 virt_meth_0x5a92f0(); // virt_meth_0x5a92f0
			virtual undefined4 virt_meth_0x5a9310(); // virt_meth_0x5a9310
	}; // ActionAnim
} // namespace f3

#endif // F3_ACTIONANIM_HPP_
