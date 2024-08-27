// Copyright 2024 OpenJE

#ifndef F3_GUI::CPKTMANAGER_HPP_
#define F3_GUI::CPKTMANAGER_HPP_

#include "f3/global.hpp"

namespace f3 {
    namespace GUI {
        class CPktManager {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                dword mbr_0x1c;
                dword mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                dword mbr_0x3c;
                dword mbr_0x4;
                dword mbr_0x40;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                ctor_0x4b6200;
                dtor_0x4b6340;
                virt_meth_0x4b64f0;
        }; // class CPktManager
    } // namespace GUI
} // namespace f3

#endif // F3_GUI::CPKTMANAGER_HPP_
