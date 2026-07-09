// Copyright 2024 OpenJE

#ifndef F3_GAMEUSABLEOBJECT_HPP_
#define F3_GAMEUSABLEOBJECT_HPP_

#include "F3.hpp"
#include "F3/GameEntity.hpp"

namespace F3 {
	struct GameUsableObject : public GameEntity {
		//: Members
			dword mbr_0x4;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xc0;
		//: Methods
			void ctor_0x412420();
			undefined4 virt_meth_0x412610();
			undefined4 virt_meth_0x412620();
			undefined4 virt_meth_0x412630();
			undefined4 virt_meth_0x412640();
			undefined4 virt_meth_0x412650();
			undefined4 virt_meth_0x412660();
			undefined4 virt_meth_0x412670();
			undefined4 virt_meth_0x412680();
			undefined4 virt_meth_0x412690();
			undefined4 virt_meth_0x4126a0();
			undefined4 virt_meth_0x4126b0();
			undefined4 virt_meth_0x4126c0();
			undefined4 virt_meth_0x4126d0();
			undefined4 virt_meth_0x4126e0();
			undefined4 virt_meth_0x4126f0();
			undefined4 virt_meth_0x412700();
			undefined4 virt_meth_0x412710();
			undefined4 virt_meth_0x412720();
			undefined4 virt_meth_0x412730();
			undefined4 virt_meth_0x412740();
			undefined4 virt_meth_0x412750();
			undefined4 virt_meth_0x4127c0();
			undefined4 virt_meth_0x412a80();
			undefined4 virt_meth_0x412a90();
	}; // GameUsableObject
} // namespace F3

#endif // F3_GAMEUSABLEOBJECT_HPP_
