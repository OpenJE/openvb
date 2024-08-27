// Copyright 2024 OpenJE

#ifndef F3_AREAMAP_HPP_
#define F3_AREAMAP_HPP_

#include "f3/global.hpp"

namespace f3 {
	class AreaMap {
		//: Members
		public:
			dword mbr_0x10;
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
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x8;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			byte mbr_0x94;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xc;
		//: Methods
		public:
			AreaMap( undefined4 param_1, undefined4 param_2, _String_base * param_3, undefined4 param_4, undefined4 param_5, undefined4 param_6, undefined4 param_7, undefined4 param_8, undefined4 param_9 ); // ctor_0x57fad0
			~AreaMap(); // dtor_0x57f760
			void meth_0x57ece0(); // meth_0x57ece0
			virtual void fwrite_file_from_mbrs( FILE * param_1 ); // virt_meth_0x57c140
			virtual undefined4 * virt_meth_0x57fcd0( byte param_1 ); // virt_meth_0x57fcd0
			virtual void fread_file_into_mbrs( FILE * param_1 ); // virt_meth_0x57ff40
	}; // AreaMap
} // namespace f3

#endif // F3_AREAMAP_HPP_
