// Copyright 2024 OpenJE

#ifndef F3_NET_BASE_HPP_
#define F3_NET_BASE_HPP_

#include "f3/global.hpp"
#include "f3/net/critical_section.hpp"

namespace f3 {
    namespace net {
        class base {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                dword mbr_0x1c;
                dword mbr_0x20;
                byte mbr_0x24;
                byte mbr_0x25;
                byte mbr_0x26;
                byte mbr_0x27;
                word mbr_0x28;
                word mbr_0x2c;
                byte mbr_0x2e;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                dword mbr_0x3c;
                net::critical_section mbr_0x4;
                dword mbr_0x40;
                dword mbr_0x44;
                dword mbr_0x48;
                dword mbr_0x4c;
                dword mbr_0x50;
                dword mbr_0xc;
            //: Methods
            public:
                meth_0x59d530;
                virt_meth_0x59d650;
                ctor_0x59d6d0;
        }; // class base
    } // namespace net
} // namespace f3

#endif // F3_NET_BASE_HPP_
