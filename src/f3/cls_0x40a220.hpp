// Copyright 2024 OpenJE

#ifndef F3_CLS_0X40A220_HPP_
#define F3_CLS_0X40A220_HPP_

#include "f3/global.hpp"
#include "f3/cls_0x496bd0.hpp"
#include "f3/cls_0x4d8d70.hpp"
#include "f3/cls_0x4d8d70.hpp"
#include "f3/std/basic_streambuf.hpp"
#include "f3/std/basic_streambuf.hpp"

namespace f3 {
	class cls_0x40a220 : public cls_0x496bd0, public cls_0x4d8d70, public std::basic_streambuf<char, struct std::char_traits<char> > {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x4;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			cls_0x4d8d70 mbr_0x60;
			dword mbr_0x68;
			dword mbr_0x6c;
			std::basic_streambuf<char, struct std::char_traits<char> > mbr_0x8;
			dword mbr_0x8c;
			dword mbr_0x90;
		//: Methods
		public:
			ctor_0x40a220;
			ctor_0x497bb0;
			meth_0x497fb0;
			meth_0x498120;
			meth_0x4984e0;
			meth_0x498550;
			meth_0x499040;
			ctor_0x499500;
			meth_0x4e3260;
			meth_0x5a4b80;
			meth_0x605c76;
			meth_0x605db5;
	}; // 
} // namespace f3

#endif // F3_CLS_0X40A220_HPP_
