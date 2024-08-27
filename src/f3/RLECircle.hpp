// Copyright 2024 OpenJE

#ifndef F3_RLECIRCLE<NEAR STRUCT ENTITY *>_HPP_
#define F3_RLECIRCLE<NEAR STRUCT ENTITY *>_HPP_

#include "f3/global.hpp"
#include "f3/RLEPrimitive.hpp"

namespace f3 {
	class RLECircle<near struct Entity *> : public RLEPrimitive<near struct Entity *> {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
		//: Methods
		public:
			meth_0x40a1b0;
			virt_meth_0x54b770;
			ctor_0x54fea0;
			virt_meth_0x54ff10;
			virt_meth_0x54ff30;
			virt_meth_0x54ff50;
			virt_meth_0x54ffd0;
			virt_meth_0x591690;
	}; // RLECircle<near struct Entity *>
} // namespace f3

#endif // F3_RLECIRCLE<NEAR STRUCT ENTITY *>_HPP_
