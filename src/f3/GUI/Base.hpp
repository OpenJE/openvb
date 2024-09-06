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
                Base(); // ctor_0x4b2720
                virtual ~Base(); // dtor_0x4aeff0, virt_deldtor_0x4af000
                uint meth_0x41dfa0( uint param_1, uint param_2, int * param_3, int * param_4, uint * param_5, uint * param_6, float * param_7, float * param_8, float * param_9, float * param_10 ); // meth_0x41dfa0
                virtual void virt_meth_0x4af060(); // virt_meth_0x4af060
                virtual void virt_meth_0x4b2750( undefined4 * param_1, undefined4 * param_2 ); // virt_meth_0x4b2750
                virtual void virt_meth_0x4b2770( int param_1, int param_2 ); // virt_meth_0x4b2770
                virtual void virt_meth_0x4b2790( int param_1, int param_2 ); // virt_meth_0x4b2790
                virtual void virt_meth_0x4b27c0(); // virt_meth_0x4b27c0
                virtual bool virt_meth_0x4b27e0( LONG param_1, LONG param_2 ); // virt_meth_0x4b27e0
                virtual undefined4 virt_meth_0x4b2800( FILE * param_1 ); // virt_meth_0x4b2800
                virtual undefined4 virt_meth_0x4b2830( FILE * param_1 ); // virt_meth_0x4b2830
                virtual void virt_meth_0x4b2850( int param_1 ); // virt_meth_0x4b2850
                virtual void virt_meth_0x4b2880(); // virt_meth_0x4b2880
        }; // class Base
    }; // namespace GUI
} // namespace f3

#endif // F3_GUI_BASE_HPP_
