// Copyright 2024 OpenJE

#ifndef F3_NET_CLIENT_HPP_
#define F3_NET_CLIENT_HPP_

#include "f3/global.hpp"
#include "f3/net/base.hpp"
#include "f3/net/connection.hpp"

namespace f3 {
    namespace net {
        class client : public base {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                word mbr_0x28;
                word mbr_0x2c;
                byte mbr_0x2e;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                dword mbr_0x3c;
                dword mbr_0x40;
                dword mbr_0x44;
                dword mbr_0x48;
                dword mbr_0x4c;
                dword mbr_0x50;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                byte mbr_0x60;
                byte mbr_0x61;
                byte mbr_0x62;
                byte mbr_0x63;
                word mbr_0x64;
                dword mbr_0x68;
                dword mbr_0x6c;
                dword mbr_0xc;
            //: Methods
            public:
                virt_meth_0x409bb0;
                virt_meth_0x55bdb0;
                ctor_0x59c2e0;
                virt_meth_0x59c320;
                virt_meth_0x59c330;
                virt_meth_0x59c340;
                virt_meth_0x59c360;
                virt_meth_0x59c380;
                virt_meth_0x59c410;
                virt_meth_0x59c440;
                virt_meth_0x59c450;
                virt_meth_0x59c4d0;
                meth_0x59c560;
                meth_0x59c5d0;
                meth_0x59c670;
                meth_0x59c6f0;
                meth_0x59c780;
                meth_0x59c790;
                meth_0x59c7f0;
                virt_meth_0x59c880;
                virt_meth_0x59c8a0;
                meth_0x59c9c0;
                meth_0x59cb40;
                meth_0x59ccf0;
                meth_0x59cf10;
                meth_0x59d040;
                virt_meth_0x59d180;
                meth_0x59d1a0;
                meth_0x59d260;
                virt_meth_0x59d2d0;
                virt_meth_0x59d300;
                meth_0x59d5d0;
                meth_0x59d5f0;
                meth_0x59d630;
                meth_0x59d7f0;
                meth_0x59d930;
        }; // class client
    } // namespace net
} // namespace f3

#endif // F3_NET_CLIENT_HPP_
