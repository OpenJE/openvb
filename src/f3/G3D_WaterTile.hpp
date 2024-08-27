// Copyright 2024 OpenJE

#ifndef F3_G3D_WATERTILE_HPP_
#define F3_G3D_WATERTILE_HPP_

#include "f3/global.hpp"
#include "f3/G3D_Mesh.hpp"

namespace f3 {
	class G3D_WaterTile : public G3D_Mesh {
		//: Members
		public:
			dword mbr_0x20;
			byte mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x4;
			byte mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			byte mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
		//: Methods
		public:
			virt_meth_0x4d6bc0;
			virt_meth_0x4d6c10;
			ctor_0x4d98f0;
			meth_0x4d9dd0;
			virt_meth_0x4db010;
			virt_meth_0x4dd060;
			virt_meth_0x4dd0c0;
			virt_meth_0x4dd120;
	}; // G3D_WaterTile
} // namespace f3

#endif // F3_G3D_WATERTILE_HPP_
