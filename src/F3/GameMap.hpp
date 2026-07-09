// Copyright 2024 OpenJE

#ifndef F3_GAMEMAP_HPP_
#define F3_GAMEMAP_HPP_

#include "F3.hpp"
#include "JE/Map.hpp"

namespace F3 {
	struct GameMap : public JE::Map {
		//: Members
			byte mbr_0x15c;
			dword mbr_0x168;
			dword mbr_0x16c;
			dword mbr_0x174;
			dword mbr_0x30;
			dword mbr_0x4;
			dword mbr_0x44;
			byte mbr_0x8;
		//: Methods
			void ctor_0x46c8e0();
			void dtor_0x46c910();
			undefined4 virt_meth_0x46c920();
			undefined4 virt_meth_0x46c980();
			undefined4 virt_meth_0x46cae0();
			undefined4 virt_meth_0x46cb90();
			undefined4 virt_meth_0x46cbb0();
	}; // GameMap
} // namespace F3

#endif // F3_GAMEMAP_HPP_
