// Copyright 2024 OpenJE

#ifndef F3_GFX_STREAMINGINDEXBUFFER_HPP_
#define F3_GFX_STREAMINGINDEXBUFFER_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_IndexBuffer.hpp"

namespace f3 {
	class Gfx_StreamingIndexBuffer : public Gfx_IndexBuffer {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x505650;
			virt_meth_0x505670;
			virt_meth_0x505690;
			dtor_0x5056b0;
			virt_meth_0x505730;
	}; // Gfx_StreamingIndexBuffer
} // namespace f3

#endif // F3_GFX_STREAMINGINDEXBUFFER_HPP_
