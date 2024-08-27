// Copyright 2024 OpenJE

#ifndef F3_GAMEUSABLEOBJECT_HPP_
#define F3_GAMEUSABLEOBJECT_HPP_

#include "f3/global.hpp"
#include "f3/GameEntity.hpp"

namespace f3 {
	struct GameUsableObject : public GameEntity {
		//: Members
			dword mbr_0x4;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xc0;
		//: Methods
			ctor_0x412420;
			virt_meth_0x412610;
			virt_meth_0x412620;
			virt_meth_0x412630;
			virt_meth_0x412640;
			virt_meth_0x412650;
			virt_meth_0x412660;
			virt_meth_0x412670;
			virt_meth_0x412680;
			virt_meth_0x412690;
			virt_meth_0x4126a0;
			virt_meth_0x4126b0;
			virt_meth_0x4126c0;
			virt_meth_0x4126d0;
			virt_meth_0x4126e0;
			virt_meth_0x4126f0;
			virt_meth_0x412700;
			virt_meth_0x412710;
			virt_meth_0x412720;
			virt_meth_0x412730;
			virt_meth_0x412740;
			virt_meth_0x412750;
			virt_meth_0x4127c0;
			virt_meth_0x412a80;
			virt_meth_0x412a90;
	}; // GameUsableObject
} // namespace f3

#endif // F3_GAMEUSABLEOBJECT_HPP_
