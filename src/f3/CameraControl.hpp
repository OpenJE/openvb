// Copyright 2024 OpenJE

#ifndef F3_CAMERACONTROL_HPP_
#define F3_CAMERACONTROL_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_CamCtrl_Orbit.hpp"

namespace f3 {
	struct CameraControl {
		//: Members
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x110;
			byte mbr_0x114;
			dword mbr_0x118;
			dword mbr_0x11c;
			dword mbr_0x4;
			Gfx_CamCtrl_Orbit mbr_0x8;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			byte mbr_0xc;
			word mbr_0xc0;
			word mbr_0xc2;
			byte mbr_0xc3;
			dword mbr_0xc4;
			dword mbr_0xc8;
			dword mbr_0xcc;
			dword mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
		//: Methods
			meth_0x585e20;
			meth_0x585e60;
			meth_0x585fd0;
			meth_0x586110;
			meth_0x586330;
			meth_0x586410;
			dtor_0x586570;
			meth_0x586590;
			meth_0x586710;
			meth_0x586760;
			meth_0x5867c0;
			meth_0x586830;
			meth_0x5869c0;
			ctor_0x586f70;
			virt_meth_0x587030;
	}; // CameraControl
} // namespace f3

#endif // F3_CAMERACONTROL_HPP_
