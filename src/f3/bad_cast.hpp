// Copyright 2024 OpenJE

#ifndef F3_BAD_CAST_HPP_
#define F3_BAD_CAST_HPP_

#include "f3/global.hpp"
#include "f3/exception.hpp"

namespace f3 {
	class bad_cast : public exception {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			ctor_0x616bcd;
			ctor_0x616be6;
			dtor_0x616bfe;
			virt_meth_0x616c91;
	}; // bad_cast
} // namespace f3

#endif // F3_BAD_CAST_HPP_
