// Copyright 2024 OpenJE

#ifndef F3_GAMEPATHMANAGER_HPP_
#define F3_GAMEPATHMANAGER_HPP_

#include "f3/global.hpp"
#include "f3/PathManager.hpp"

namespace f3 {
	class GamePathManager : public PathManager {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x45ba40;
			dtor_0x45ba60;
			virt_meth_0x45ba70;
			virt_meth_0x45ba90;
			virt_meth_0x45baf0;
			virt_meth_0x45bb10;
			virt_meth_0x45bc90;
	}; // GamePathManager
} // namespace f3

#endif // F3_GAMEPATHMANAGER_HPP_
