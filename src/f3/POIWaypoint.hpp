// Copyright 2024 OpenJE

#ifndef F3_POIWAYPOINT_HPP_
#define F3_POIWAYPOINT_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Label.hpp"

namespace f3 {
	class POIWaypoint : public GUI::Label {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xe64;
			dword mbr_0xe68;
			dword mbr_0xe6c;
			dword mbr_0xe70;
			dword mbr_0xe74;
			dword mbr_0xe78;
			dword mbr_0xe7c;
		//: Methods
		public:
			ctor_0x4587e0;
			dtor_0x4588c0;
			virt_meth_0x458930;
			virt_meth_0x458950;
			virt_meth_0x458980;
			virt_meth_0x4589d0;
			virt_meth_0x458a00;
	}; // POIWaypoint
} // namespace f3

#endif // F3_POIWAYPOINT_HPP_
