// Copyright 2024 OpenJE

#ifndef F3_GAMEENTITY_HPP_
#define F3_GAMEENTITY_HPP_

#include "f3/global.hpp"
#include "je/Entity.hpp"

namespace F3 {
	struct GameEntity : public JE::Entity {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			byte mbr_0x9c;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x4080c0;
			ctor_0x4678a0;
			virt_meth_0x467a00;
			virt_meth_0x467a10;
			virt_meth_0x467a20;
			virt_meth_0x467a30;
			virt_meth_0x467a40;
			virt_meth_0x467a50;
			virt_meth_0x467a60;
			virt_meth_0x467a70;
			virt_meth_0x467a80;
			virt_meth_0x467a90;
			virt_meth_0x467aa0;
			virt_meth_0x467ab0;
			virt_meth_0x467ac0;
			virt_meth_0x467ad0;
			virt_meth_0x467ae0;
			virt_meth_0x467e30;
			virt_meth_0x467e40;
			virt_meth_0x468300;
			virt_meth_0x468710;
			virt_meth_0x468720;
			virt_meth_0x468ad0;
	}; // GameEntity
} // namespace F3

#endif // F3_GAMEENTITY_HPP_
