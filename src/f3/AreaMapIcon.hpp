// Copyright 2024 OpenJE

#ifndef F3_AREAMAPICON_HPP_
#define F3_AREAMAPICON_HPP_

#include "f3/global.hpp"
#include "f3/cls_0x4abc60.hpp"

namespace f3 {
	class AreaMapIcon {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			byte mbr_0x10c;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			byte mbr_0x45;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			cls_0x4abc60 mbr_0x70;
			dword mbr_0x78;
			byte mbr_0x8;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			dword mbr_0xc;
			dword mbr_0xc0;
			dword mbr_0xc4;
			dword mbr_0xc8;
			dword mbr_0xcc;
			dword mbr_0xd0;
			dword mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xe4;
			dword mbr_0xe8;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
		public:
			AreaMapIcon(); // ctor_0x422a20
			~AreaMapIcon(); // dtor_0x4202e0
			void Get_mbrs_0xc_0x10_0x14_0x18( undefined4 * param_1, undefined4 * param_2, undefined4 * param_3, undefined4 * param_4 ); // meth_0x41d160
			void meth_0x41d190( int param_1 ); // meth_0x41d190
			void Set_mbr_0x34( undefined4 param_1 ); // meth_0x41d1c0
			void Set_mbr_0x38( undefined4 param_1 ); // meth_0x41d1e0
			undefined4 Get_mbr_0x38(); // meth_0x41d1f0
			void meth_0x41d5b0( undefined4 param_1, undefined param_2 ); // meth_0x41d5b0
			void meth_0x41d5f0(); // meth_0x41d5f0
			void meth_0x41e5f0( int param_1, int param_2, int param_3, int param_4 ); // meth_0x41e5f0
			void meth_0x41e640( float param_1, float param_2, float param_3, float param_4 ); // meth_0x41e640
			void meth_0x41f300( int param_1 ); // meth_0x41f300
			void meth_0x436ad0( int param_1 ); // meth_0x436ad0
			void meth_0x437940( int param_1, uint param_2 ); // meth_0x437940
			undefined meth_0x437b90 (); // meth_0x437b90
			void meth_0x437e70( int param_1 ); // meth_0x437e70
			bool Get_mbr_0x4(); // meth_0x5b4f30
			virtual undefined4 * virt_meth_0x421690( byte param_1 ); // virt_meth_0x421690
	}; // AreaMapIcon
} // namespace f3

#endif // F3_AREAMAPICON_HPP_
