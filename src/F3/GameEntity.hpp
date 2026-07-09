// Copyright 2024 OpenJE

#ifndef F3_GAMEENTITY_HPP_
#define F3_GAMEENTITY_HPP_

#include "F3.hpp"
#include "JE/Entity.hpp"

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
			undefined4 virt_meth_0x4080c0();
			void ctor_0x4678a0();
			undefined4 virt_meth_0x467a00();
			undefined4 virt_meth_0x467a10();
			undefined4 virt_meth_0x467a20();
			undefined4 virt_meth_0x467a30();
			undefined4 virt_meth_0x467a40();
			undefined4 virt_meth_0x467a50();
			undefined4 virt_meth_0x467a60();
			undefined4 virt_meth_0x467a70();
			undefined4 virt_meth_0x467a80();
			undefined4 virt_meth_0x467a90();
			undefined4 virt_meth_0x467aa0();
			undefined4 virt_meth_0x467ab0();
			undefined4 virt_meth_0x467ac0();
			undefined4 virt_meth_0x467ad0();
			undefined4 virt_meth_0x467ae0();
			undefined4 virt_meth_0x467e30();
			undefined4 virt_meth_0x467e40();
			undefined4 virt_meth_0x468300();
			undefined4 virt_meth_0x468710();
			undefined4 virt_meth_0x468720();
			undefined4 virt_meth_0x468ad0();
	}; // GameEntity
} // namespace F3

#endif // F3_GAMEENTITY_HPP_
