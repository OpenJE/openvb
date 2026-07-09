// Copyright 2024 OpenJE

#ifndef F3_GAMESERVER_HPP_
#define F3_GAMESERVER_HPP_

#include "F3.hpp"
#include "JE/Server.hpp"

namespace F3 {
	struct GameServer : public JE::Server {
		//: Members
			dword mbr_0x14;
			dword mbr_0x5064;
			dword mbr_0x54;
			byte mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x70a8;
			byte mbr_0x70c4;
			dword mbr_0x70f0;
			dword mbr_0x70f4;
			dword mbr_0x70f8;
			dword mbr_0x70fc;
			dword mbr_0x7100;
			dword mbr_0x7104;
			dword mbr_0xc;
		//: Methods
			void dtor_0x46ce90();
			undefined4 virt_meth_0x46ceb0();
			undefined4 meth_0x46cf10();
			undefined4 virt_meth_0x46d000();
			undefined4 meth_0x46d020();
			undefined4 meth_0x46d110();
			undefined4 meth_0x46d260();
			undefined4 meth_0x46d390();
			undefined4 meth_0x46d560();
			undefined4 meth_0x46d700();
			undefined4 meth_0x46d7f0();
			undefined4 meth_0x46d8e0();
			undefined4 meth_0x46d9e0();
			undefined4 meth_0x46dad0();
			undefined4 meth_0x46dbb0();
			undefined4 meth_0x46dc90();
			undefined4 virt_meth_0x46dd70();
			undefined4 meth_0x46ebd0();
			undefined4 meth_0x46ec30();
			undefined4 meth_0x46ecd0();
			undefined4 meth_0x46ed30();
			undefined4 meth_0x46ed90();
			void ctor_0x46ee70();
			undefined4 meth_0x552d40();
			undefined4 meth_0x553010();
			undefined4 meth_0x553180();
			undefined4 meth_0x5532f0();
			undefined4 meth_0x5533b0();
			undefined4 meth_0x553470();
			undefined4 meth_0x553550();
			undefined4 meth_0x553600();
			undefined4 meth_0x553790();
			undefined4 meth_0x553e90();
			undefined4 meth_0x553f90();
			undefined4 meth_0x5540c0();
			undefined4 meth_0x5541d0();
			undefined4 meth_0x5543e0();
			undefined4 meth_0x554580();
			undefined4 meth_0x554670();
			undefined4 meth_0x554720();
			undefined4 meth_0x5547d0();
			undefined4 meth_0x554970();
			undefined4 meth_0x554a50();
			undefined4 meth_0x554b40();
			undefined4 meth_0x554c30();
			undefined4 meth_0x554d30();
			undefined4 meth_0x554e50();
			undefined4 meth_0x5555a0();
			undefined4 meth_0x555790();
			undefined4 meth_0x5557d0();
			undefined4 meth_0x555810();
			undefined4 meth_0x555850();
			undefined4 meth_0x555920();
			undefined4 meth_0x5559f0();
			undefined4 meth_0x555a90();
			undefined4 meth_0x555b00();
			undefined4 meth_0x555b70();
			undefined4 meth_0x555c40();
			undefined4 meth_0x555cd0();
			undefined4 meth_0x555d80();
			undefined4 meth_0x555e00();
			undefined4 meth_0x555e80();
			undefined4 meth_0x556000();
			undefined4 meth_0x556040();
			undefined4 meth_0x556080();
			undefined4 meth_0x556100();
			undefined4 meth_0x5a09d0();
			undefined4 meth_0x5a0a00();
	}; // GameServer
} // namespace F3

#endif // F3_GAMESERVER_HPP_
