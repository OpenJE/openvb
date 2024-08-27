// Copyright 2024 OpenJE

#ifndef F3_BAD_TYPEID_HPP_
#define F3_BAD_TYPEID_HPP_

#include "f3/global.hpp"
#include "f3/exception.hpp"

namespace f3 {
	class bad_typeid : public exception {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			ctor_0x616c09;
			ctor_0x616c22;
			dtor_0x616c3a;
			virt_meth_0x616cad;
	}; // bad_typeid
} // namespace f3

#endif // F3_BAD_TYPEID_HPP_
