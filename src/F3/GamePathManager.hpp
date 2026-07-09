// Copyright 2024 OpenJE

#ifndef F3_GAMEPATHMANAGER_HPP_
#define F3_GAMEPATHMANAGER_HPP_

#include "F3.hpp"
#include "JE/PathManager.hpp"

namespace F3 {
	class GamePathManager : public JE::PathManager {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			void ctor_0x45ba40();
			void dtor_0x45ba60();
			undefined4 virt_meth_0x45ba70();
			undefined4 virt_meth_0x45ba90();
			undefined4 virt_meth_0x45baf0();
			undefined4 virt_meth_0x45bb10();
			undefined4 virt_meth_0x45bc90();
	}; // GamePathManager
} // namespace F3

#endif // F3_GAMEPATHMANAGER_HPP_
