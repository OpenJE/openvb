// Copyright 2024 OpenJE

#ifndef F3_AREAMAPNOTE_HPP_
#define F3_AREAMAPNOTE_HPP_

#include "f3/global.hpp"

namespace f3 {
	struct AreaMapNote {
		//: Members
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x5d;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x8;
		//: Methods
			AreaMapNote(); //ctor_0x57c400
			 AreaMapNote( int param_1 ); //ctor_0x57d7c0
			~AreaMapNote(); //dtor_0x57c450
			void meth_0x57c060(); //meth_0x57c060
			void meth_0x57d2e0(); //meth_0x57d2e0
			int meth_0x57d3a0( int param_1 ); //meth_0x57d3a0
			virtual void virt_meth_0x57c8d0(); //virt_meth_0x57c8d0
			virtual void virt_meth_0x57d440( FILE * param_1 ); //virt_meth_0x57d440
	}; // AreaMapNote
} // namespace f3

#endif // F3_AREAMAPNOTE_HPP_
