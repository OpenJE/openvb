// Copyright 2024 OpenJE

#ifndef F3_GAMEPROJECTILE_HPP_
#define F3_GAMEPROJECTILE_HPP_

#include "F3.hpp"
#include "F3/GameEntity.hpp"

namespace F3 {
	struct GameProjectile : public GameEntity {
		//: Members
			dword mbr_0x4;
			byte mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xbc;
		//: Methods
			void ctor_0x411c60();
			undefined4 virt_meth_0x411ef0();
			undefined4 virt_meth_0x411f00();
			undefined4 virt_meth_0x411f10();
			undefined4 virt_meth_0x411f20();
			undefined4 virt_meth_0x411f30();
			undefined4 virt_meth_0x411f40();
			undefined4 virt_meth_0x411f50();
			undefined4 virt_meth_0x411f60();
			undefined4 virt_meth_0x411f70();
			undefined4 virt_meth_0x411f80();
			undefined4 virt_meth_0x411f90();
			undefined4 virt_meth_0x411fa0();
			undefined4 virt_meth_0x411fb0();
			undefined4 virt_meth_0x411fc0();
			undefined4 virt_meth_0x411fd0();
			undefined4 virt_meth_0x411fe0();
			undefined4 virt_meth_0x411ff0();
			undefined4 virt_meth_0x412000();
			undefined4 virt_meth_0x412010();
			undefined4 virt_meth_0x412020();
			undefined4 virt_meth_0x412030();
			undefined4 virt_meth_0x412040();
			undefined4 virt_meth_0x412400();
			undefined4 virt_meth_0x412410();
	}; // GameProjectile
} // namespace F3

#endif // F3_GAMEPROJECTILE_HPP_
