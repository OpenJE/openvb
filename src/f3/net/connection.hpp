// Copyright 2024 OpenJE

#ifndef F3_NET_CONNECTION_HPP_
#define F3_NET_CONNECTION_HPP_

#include "f3/global.hpp"
#include "f3/net/packet.hpp"
#include "f3/cls_0x5abd80.hpp"
#include "f3/net/packet.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        class connection {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                word mbr_0x18;
                dword mbr_0x1c;
                byte mbr_0x20;
                net::packet mbr_0x20a4;
                byte mbr_0x20a8;
                dword mbr_0x20b4;
                dword mbr_0x20b8;
                dword mbr_0x20bc;
                word mbr_0x20c8;
                dword mbr_0x20cc;
                dword mbr_0x20d4;
                byte mbr_0x21;
                byte mbr_0x22;
                byte mbr_0x23;
                word mbr_0x24;
                word mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                dword mbr_0x4;
                dword mbr_0x40;
                cls_0x5abd80 mbr_0x40dc;
                dword mbr_0x4124;
                dword mbr_0x4128;
                dword mbr_0x412c;
                dword mbr_0x4130;
                dword mbr_0x4134;
                dword mbr_0x4138;
                dword mbr_0x4140;
                dword mbr_0x4148;
                dword mbr_0x414c;
                dword mbr_0x4150;
                dword mbr_0x4154;
                dword mbr_0x4158;
                dword mbr_0x415c;
                dword mbr_0x4160;
                dword mbr_0x44;
                dword mbr_0x48;
                dword mbr_0x4c;
                dword mbr_0x50;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                dword mbr_0x60;
                dword mbr_0x64;
                dword mbr_0x68;
                net::packet mbr_0x6c;
                byte mbr_0x70;
                dword mbr_0x8;
                dword mbr_0x80;
                dword mbr_0x84;
                dword mbr_0xc;
            //: Methods
            public:
                meth_0x5ab790;
                meth_0x5ab7a0;
                meth_0x5ab8e0;
                meth_0x5abab0;
                meth_0x5abaf0;
                meth_0x5abc00;
                meth_0x5abc70;
                ctor_0x5abf30;
                dtor_0x5abf90;
                virt_meth_0x5ac210;
                meth_0x5ac450;
                meth_0x5ac500;
        }; // class connection
    } // namespace net
} // namespace f3

#endif // F3_NET_CONNECTION_HPP_
