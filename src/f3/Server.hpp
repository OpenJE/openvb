// Copyright 2024 OpenJE

#ifndef F3_SERVER_HPP_
#define F3_SERVER_HPP_

#include "f3/global.hpp"
#include "f3/net/server.hpp"
#include "f3/net/packet.hpp"

namespace f3 {
	class Server : public net::server, public net::packet {
		//: Members
		public:
			dword mbr_0x14;
			dword mbr_0x5064;
			dword mbr_0x5068;
			byte mbr_0x5070;
			dword mbr_0x5078;
			dword mbr_0x507c;
			dword mbr_0x5080;
			dword mbr_0x5084;
			dword mbr_0x54;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x70a4;
			dword mbr_0x70ac;
			dword mbr_0x70bc;
			dword mbr_0x70c0;
			byte mbr_0x70c4;
			dword mbr_0x70c8;
			dword mbr_0x70cc;
			dword mbr_0x70dc;
			dword mbr_0x70e0;
			byte mbr_0x70e4;
			byte mbr_0x70e5;
			byte mbr_0x70e6;
			dword mbr_0x70e8;
			byte mbr_0x70ec;
			byte mbr_0x70ed;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x46cea0;
			meth_0x54a5f0;
			virt_meth_0x5529a0;
			virt_meth_0x552a60;
			virt_meth_0x552ac0;
			virt_meth_0x552b60;
			virt_meth_0x552c70;
			virt_meth_0x552ca0;
			meth_0x552e20;
			meth_0x552f20;
			meth_0x553850;
			meth_0x553990;
			meth_0x553b00;
			meth_0x553c30;
			meth_0x553cf0;
			meth_0x553dd0;
			meth_0x554300;
			meth_0x554f70;
			meth_0x555050;
			meth_0x555170;
			virt_meth_0x5552c0;
			virt_meth_0x555380;
			meth_0x5553c0;
			meth_0x5554b0;
			meth_0x5555d0;
			meth_0x555640;
			meth_0x555680;
			meth_0x5556c0;
			meth_0x555750;
			meth_0x555890;
			virt_meth_0x556250;
			meth_0x556270;
			meth_0x5562f0;
			meth_0x556370;
			meth_0x5563d0;
			meth_0x556450;
			ctor_0x5564d0;
			meth_0x5566b0;
			meth_0x556b60;
			virt_meth_0x556ea0;
	}; // Server
} // namespace f3

#endif // F3_SERVER_HPP_
