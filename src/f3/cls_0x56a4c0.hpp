// Copyright 2024 OpenJE

#ifndef F3_CLS_0X56A4C0_HPP_
#define F3_CLS_0X56A4C0_HPP_

#include "f3/global.hpp"
#include "f3/cls_0x401b00.hpp"
#include "f3/cls_0x50dbb0.hpp"

namespace f3 {
	class cls_0x56a4c0 : public cls_0x401b00, public cls_0x50dbb0 {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			ctor_0x56a4c0;
			meth_0x56a540;
			ctor_0x56a5e0;
	}; // 
} // namespace f3

#endif // F3_CLS_0X56A4C0_HPP_
