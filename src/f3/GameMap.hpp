// Copyright 2024 OpenJE

#ifndef F3_GAMEMAP_HPP_
#define F3_GAMEMAP_HPP_

#include "f3/global.hpp"
#include "f3/Map.hpp"

namespace f3 {
	struct GameMap : public Map {
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
			ctor_0x46c8e0;
			dtor_0x46c910;
			virt_meth_0x46c920;
			virt_meth_0x46c980;
			virt_meth_0x46cae0;
			virt_meth_0x46cb90;
			virt_meth_0x46cbb0;
	}; // GameMap
} // namespace f3

#endif // F3_GAMEMAP_HPP_
