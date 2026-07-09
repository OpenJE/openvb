// Copyright 2024 OpenJE

#ifndef F3_GAMENETWORK_HPP_
#define F3_GAMENETWORK_HPP_

#include "F3.hpp"
#include "JE/Network.hpp"

namespace F3 {
	struct GameNetwork : public JE::Network {
		//: Members
			word mbr_0x1c;
			word mbr_0x8;
		//: Methods
			undefined4 virt_meth_0x45dfd0();
			undefined4 virt_meth_0x45e040();
			undefined4 virt_meth_0x45e0b0();
			void dtor_0x45e0d0();
	}; // GameNetwork
} // namespace F3

#endif // F3_GAMENETWORK_HPP_
