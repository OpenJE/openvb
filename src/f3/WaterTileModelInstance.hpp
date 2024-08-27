// Copyright 2024 OpenJE

#ifndef F3_WATERTILEMODELINSTANCE_HPP_
#define F3_WATERTILEMODELINSTANCE_HPP_

#include "f3/global.hpp"
#include "f3/ModelInstance.hpp"

namespace f3 {
	class WaterTileModelInstance : public ModelInstance {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
		//: Methods
		public:
			virt_meth_0x5639a0;
			meth_0x563bb0;
			meth_0x563e30;
			meth_0x563ff0;
			meth_0x564050;
			dtor_0x564620;
			virt_meth_0x564900;
			ctor_0x565540;
			meth_0x565bb0;
			meth_0x565f50;
	}; // WaterTileModelInstance
} // namespace f3

#endif // F3_WATERTILEMODELINSTANCE_HPP_
