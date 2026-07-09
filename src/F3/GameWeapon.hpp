// Copyright 2024 OpenJE

#ifndef F3_GAMEWEAPON_HPP_
#define F3_GAMEWEAPON_HPP_

#include "F3.hpp"
#include "F3/GameItem.hpp"

namespace F3 {
	struct GameWeapon : public GameItem {
		//: Members
			dword mbr_0x3b0;
			dword mbr_0x3b4;
			dword mbr_0x3b8;
			dword mbr_0x3bc;
			dword mbr_0x3c0;
			dword mbr_0x3c4;
			dword mbr_0x3c8;
			byte mbr_0x3d0;
			dword mbr_0x3e0;
			dword mbr_0x3e4;
			dword mbr_0x3e8;
			dword mbr_0x3ec;
			dword mbr_0x3f0;
			dword mbr_0x3f4;
			dword mbr_0x3f8;
			dword mbr_0x3fc;
			dword mbr_0x4;
			dword mbr_0x400;
			dword mbr_0x404;
			dword mbr_0x408;
			dword mbr_0x40c;
			dword mbr_0x410;
			dword mbr_0x414;
			dword mbr_0x418;
			dword mbr_0x41c;
			dword mbr_0x420;
			dword mbr_0x424;
			dword mbr_0x428;
			dword mbr_0x430;
		//: Methods
			void ctor_0x414510();
			undefined4 virt_meth_0x414680();
			undefined4 virt_meth_0x414690();
			undefined4 virt_meth_0x4146a0();
			undefined4 virt_meth_0x4146b0();
			undefined4 virt_meth_0x4146c0();
			undefined4 virt_meth_0x4146d0();
			undefined4 virt_meth_0x4146e0();
			undefined4 virt_meth_0x4146f0();
			undefined4 virt_meth_0x414700();
			undefined4 virt_meth_0x414710();
			undefined4 virt_meth_0x414720();
			undefined4 virt_meth_0x414730();
			undefined4 virt_meth_0x414740();
			undefined4 virt_meth_0x414750();
			undefined4 virt_meth_0x414760();
			undefined4 virt_meth_0x414770();
			undefined4 virt_meth_0x414780();
			undefined4 virt_meth_0x414790();
			undefined4 virt_meth_0x4147a0();
			undefined4 virt_meth_0x4147b0();
			undefined4 virt_meth_0x4147c0();
			undefined4 virt_meth_0x415660();
			undefined4 virt_meth_0x416a60();
	}; // GameWeapon
} // namespace F3

#endif // F3_GAMEWEAPON_HPP_
