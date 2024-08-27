// Copyright 2024 OpenJE

#ifndef F3_GUI_BASE_HPP_
#define F3_GUI_BASE_HPP_

#include "f3/global.hpp"

namespace f3 {
    namespace GUI {
        class Base {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                byte mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
                dword mbr_0xd30;
                dword mbr_0xd34;
            //: Methods
            public:
                meth_0x41dfa0;
                dtor_0x4aeff0;
                virt_deldtor_0x4af000;
                virt_meth_0x4af060;
                ctor_0x4b2720;
                virt_meth_0x4b2750;
                virt_meth_0x4b2770;
                virt_meth_0x4b2790;
                virt_meth_0x4b27c0;
                virt_meth_0x4b27e0;
                virt_meth_0x4b2800;
                virt_meth_0x4b2830;
                virt_meth_0x4b2850;
                virt_meth_0x4b2880;
        }; // class Base
    }; // namespace GUI
} // namespace f3

#endif // F3_GUI_BASE_HPP_
