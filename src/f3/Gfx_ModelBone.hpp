// Copyright 2024 OpenJE

#ifndef F3_GFX_MODELBONE_HPP_
#define F3_GFX_MODELBONE_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_BaseObject.hpp"

namespace f3 {
	class Gfx_ModelBone : public Gfx_BaseObject {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
		//: Methods
		public:
			dtor_0x524330;
			virt_meth_0x524370;
			ctor_0x524640;
	}; // Gfx_ModelBone
} // namespace f3

#endif // F3_GFX_MODELBONE_HPP_
