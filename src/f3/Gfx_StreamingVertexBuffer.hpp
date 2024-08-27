// Copyright 2024 OpenJE

#ifndef F3_GFX_STREAMINGVERTEXBUFFER_HPP_
#define F3_GFX_STREAMINGVERTEXBUFFER_HPP_

#include "f3/global.hpp"
#include "f3/Gfx_VertexBuffer.hpp"

namespace f3 {
	class Gfx_StreamingVertexBuffer : public Gfx_VertexBuffer {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x5053b0;
			ctor_0x5055e0;
			virt_meth_0x505600;
			dtor_0x505620;
			virt_meth_0x5056f0;
	}; // Gfx_StreamingVertexBuffer
} // namespace f3

#endif // F3_GFX_STREAMINGVERTEXBUFFER_HPP_
