// Copyright 2024 OpenJE

#ifndef F3_GFX_RS_BASE_FOW_HPP_
#define F3_GFX_RS_BASE_FOW_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_RenderStyle.hpp"

namespace f3 {
	class Gfx_RS_Base_FOW : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x536180;
			virt_meth_0x5361d0;
			virt_meth_0x536200;
			dtor_0x537180;
			virt_meth_0x537210;
			virt_meth_0x537220;
			virt_meth_0x537230;
			virt_meth_0x537240;
			virt_meth_0x537260;
			virt_meth_0x537290;
			virt_meth_0x5372b0;
			ctor_0x53e0f0;
			virt_meth_0x53e120;
			virt_meth_0x53e570;
	}; // Gfx_RS_Base_FOW
} // namespace f3

#endif // F3_GFX_RS_BASE_FOW_HPP_
