// Copyright 2024 OpenJE

#ifndef F3_GUI_BUTTON_HPP_
#define F3_GUI_BUTTON_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Window.hpp"

namespace f3 {
    namespace GUI {
        class Button : public Window {
            //: Members
            public:
                dword mbr_0x18;
                dword mbr_0x40;
                dword mbr_0xd1c;
                byte mbr_0xd20;
                byte mbr_0xd21;
                dword mbr_0xd24;
                dword mbr_0xd28;
                dword mbr_0xd38;
                dword mbr_0xd3c;
                dword mbr_0xd40;
                dword mbr_0xd44;
                dword mbr_0xd54;
                dword mbr_0xd58;
                dword mbr_0xd5c;
                dword mbr_0xd60;
                dword mbr_0xd70;
                dword mbr_0xd74;
            //: Methods
            public:
                Button( uint param_1 ); // ctor_0x4a6ed0
                ~Button(); // dtor_0x4a6fe0
                int meth_0x4a6d70(); // meth_0x4a6d70
                bool virt_meth_0x4a6a10(); // virt_meth_0x4a6a10
                bool virt_meth_0x4a6ac0(); // virt_meth_0x4a6ac0
                uint virt_meth_0x4a6b60(); // virt_meth_0x4a6b60
                undefined4 virt_meth_0x4a6da0( int * param_1 ); // virt_meth_0x4a6da0
                undefined4 Get_mbr_0xd1c(); // virt_meth_0x4a6fa0
                void virt_meth_0x4a6fb0(); // virt_meth_0x4a6fb0
                int * virt_meth_0x4a6fc0( byte param_1 ); // virt_meth_0x4a6fc0
                void virt_meth_0x4a70d0( undefined * * param_1 ); // virt_meth_0x4a70d0
        }; // class Button
    } // namespace GUI
} // namespace f3

#endif // F3_GUI_BUTTON_HPP_
