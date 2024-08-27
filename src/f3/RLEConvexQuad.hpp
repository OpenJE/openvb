// Copyright 2024 OpenJE

#ifndef F3_RLECONVEXQUAD<NEAR STRUCT ENTITY *>_HPP_
#define F3_RLECONVEXQUAD<NEAR STRUCT ENTITY *>_HPP_

#include "f3/global.hpp"
#include "f3/RLEPrimitive.hpp"

namespace f3 {
	class RLEConvexQuad<near struct Entity *> : public RLEPrimitive<near struct Entity *> {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
		//: Methods
		public:
			virt_meth_0x40a4b0;
			ctor_0x40c9b0;
			virt_meth_0x40cb80;
			virt_meth_0x40cbb0;
			virt_meth_0x40cbd0;
			virt_meth_0x40cbf0;
			virt_meth_0x40cc60;
			virt_meth_0x54ffb0;
	}; // RLEConvexQuad<near struct Entity *>
} // namespace f3

#endif // F3_RLECONVEXQUAD<NEAR STRUCT ENTITY *>_HPP_
