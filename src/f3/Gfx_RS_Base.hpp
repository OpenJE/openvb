// Copyright 2024 OpenJE

#ifndef F3_GFX_RS_BASE_HPP_
#define F3_GFX_RS_BASE_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_RenderStyle.hpp"

namespace f3 {
	class Gfx_RS_Base : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x535ed0;
			virt_meth_0x535f00;
			virt_meth_0x535f20;
			dtor_0x536ac0;
			virt_meth_0x536b30;
			virt_meth_0x536b40;
			virt_meth_0x536b50;
			virt_meth_0x536b60;
			virt_meth_0x536b80;
			virt_meth_0x536b90;
			virt_meth_0x536bb0;
			virt_meth_0x536be0;
			virt_meth_0x536c00;
			ctor_0x53b9f0;
			virt_meth_0x53ba20;
			virt_meth_0x53bc00;
	}; // Gfx_RS_Base
} // namespace f3

#endif // F3_GFX_RS_BASE_HPP_
