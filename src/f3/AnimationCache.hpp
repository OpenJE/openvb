// Copyright 2024 OpenJE

#ifndef F3_ANIMATIONCACHE_HPP_
#define F3_ANIMATIONCACHE_HPP_

#include "f3/global.hpp"

namespace f3 {
	class AnimationCache {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			AnimationCache(); // ctor_0x59bbe0
			~AnimationCache(); // dtor_0x59ba80
			virtual void virt_meth_0x59afe0(); // virt_meth_0x59afe0
			virtual void virt_meth_0x59b120(); // virt_meth_0x59b120
			virtual void Startup(); // virt_meth_0x59b1e0
			virtual undefined4 * virt_meth_0x59bca0(byte param_1); // virt_meth_0x59bca0
	}; // AnimationCache
} // namespace f3

#endif // F3_ANIMATIONCACHE_HPP_
