// Copyright 2024 OpenJE

#ifndef F3_GUI_FONT_HPP_
#define F3_GUI_FONT_HPP_

#include "f3/global.hpp"

namespace f3 {
    namespace GUI {
        class Font {
            //: Members
            public:
                dword mbr_0x18;
                dword mbr_0x1c;
                dword mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x4;
                dword mbr_0x8;
            //: Methods
            public:
                Font(); // ctor_0x4b5500
                ~Font(); // dtor_0x4b55a0
                void meth_0x51e970(); // meth_0x51e970
                void meth_0x51ebd0( _String_base *param_1, dword param_2, dword param_3, dword param_4 ); // meth_0x51ebd0
                virtual undefined4 * virt_meth_0x4b5580( byte param_1 ); // virt_meth_0x4b5580
        }; // class Font
    } // namespace GUI
} // namespace f3

#endif // F3_GUI_FONT_HPP_
