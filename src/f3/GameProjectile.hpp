// Copyright 2024 OpenJE

#ifndef F3_GAMEPROJECTILE_HPP_
#define F3_GAMEPROJECTILE_HPP_

#include "f3/global.hpp"
#include "f3/GameEntity.hpp"

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
			ctor_0x411c60;
			virt_meth_0x411ef0;
			virt_meth_0x411f00;
			virt_meth_0x411f10;
			virt_meth_0x411f20;
			virt_meth_0x411f30;
			virt_meth_0x411f40;
			virt_meth_0x411f50;
			virt_meth_0x411f60;
			virt_meth_0x411f70;
			virt_meth_0x411f80;
			virt_meth_0x411f90;
			virt_meth_0x411fa0;
			virt_meth_0x411fb0;
			virt_meth_0x411fc0;
			virt_meth_0x411fd0;
			virt_meth_0x411fe0;
			virt_meth_0x411ff0;
			virt_meth_0x412000;
			virt_meth_0x412010;
			virt_meth_0x412020;
			virt_meth_0x412030;
			virt_meth_0x412040;
			virt_meth_0x412400;
			virt_meth_0x412410;
	}; // GameProjectile
} // namespace F3

#endif // F3_GAMEPROJECTILE_HPP_
