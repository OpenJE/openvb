// Copyright 2024 OpenJE

#ifndef F3_NET_SERVER_HPP_
#define F3_NET_SERVER_HPP_

#include "f3/global.hpp"
#include "f3/net/base.hpp"
#include "f3/net/connection.hpp"

namespace f3 {
    namespace net {
        class server : public base {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                word mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x40;
                dword mbr_0x44;
                dword mbr_0x54;
                byte mbr_0x58;
                byte mbr_0x59;
                dword mbr_0x5c;
                dword mbr_0x60;
                dword mbr_0x64;
                dword mbr_0x70;
                dword mbr_0x74;
                dword mbr_0xc;
            //: Methods
            public:
                virt_meth_0x552980;
                virt_meth_0x5a07d0;
                virt_meth_0x5a0800;
                meth_0x5a0830;
                virt_meth_0x5a08d0;
                virt_meth_0x5a0900;
                virt_meth_0x5a0990;
                meth_0x5a0a30;
                meth_0x5a0ad0;
                meth_0x5a0b20;
                ctor_0x5a0b40;
                virt_meth_0x5a0bb0;
                virt_meth_0x5a0bc0;
                meth_0x5a0bf0;
                meth_0x5a0c20;
                meth_0x5a0d60;
                meth_0x5a0f30;
                meth_0x5a1410;
                meth_0x5a1550;
                meth_0x5a16a0;
                meth_0x5a1800;
                meth_0x5a18c0;
                meth_0x5a18f0;
                meth_0x5a1920;
                meth_0x5a19a0;
                virt_meth_0x5a19c0;
                meth_0x5a19f0;
                meth_0x5a1ab0;
                meth_0x5a1b50;
                virt_meth_0x5a1b80;
                virt_meth_0x5a1bd0;
                virt_meth_0x5a1c00;
                meth_0x5a1dc0;
                virt_meth_0x5a1e30;
        }; // class server
    } // namespace net
} // namespace f3

#endif // F3_NET_SERVER_HPP_
