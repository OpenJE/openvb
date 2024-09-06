// Copyright 2024 OpenJE

#ifndef F3_GUI_EDIT_HPP_
#define F3_GUI_EDIT_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Label.hpp"
#include "f3/cls_0x4abc60.hpp"

namespace f3 {
    namespace GUI {
        class Edit : public Label {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x1000;
                dword mbr_0x1004;
                dword mbr_0x1008;
                byte mbr_0x100c;
                dword mbr_0x1010;
                dword mbr_0x1014;
                byte mbr_0x1018;
                byte mbr_0x1019;
                byte mbr_0x101a;
                dword mbr_0x101c;
                dword mbr_0x1020;
                dword mbr_0x1024;
                dword mbr_0x1028;
                dword mbr_0x102c;
                dword mbr_0x1030;
                dword mbr_0x1034;
                byte mbr_0x1038;
                dword mbr_0x14;
                dword mbr_0x18;
                dword mbr_0x8;
                dword mbr_0xc;
                dword mbr_0xd20;
                dword mbr_0xd24;
                dword mbr_0xd38;
                dword mbr_0xd3c;
                dword mbr_0xd58;
                dword mbr_0xd5c;
                dword mbr_0xd60;
                dword mbr_0xd6c;
                dword mbr_0xe1c;
                dword mbr_0xe20;
                dword mbr_0xe24;
                dword mbr_0xe28;
                dword mbr_0xe48;
                dword mbr_0xe50;
                dword mbr_0xe54;
                dword mbr_0xe5c;
                dword mbr_0xe60;
                byte mbr_0xe64;
                dword mbr_0xe68;
                dword mbr_0xe6c;
                dword mbr_0xe70;
                dword mbr_0xe74;
                dword mbr_0xe78;
                dword mbr_0xe7c;
                dword mbr_0xe80;
                dword mbr_0xe84;
                dword mbr_0xe88;
                dword mbr_0xe8c;
                cls_0x4abc60 mbr_0xe90;
                dword mbr_0xe9c;
                dword mbr_0xea0;
                dword mbr_0xea4;
                dword mbr_0xea8;
                dword mbr_0xeac;
                dword mbr_0xeb0;
                dword mbr_0xeb4;
                dword mbr_0xeb8;
                dword mbr_0xed4;
                dword mbr_0xed8;
                dword mbr_0xedc;
                dword mbr_0xee0;
                dword mbr_0xee4;
                dword mbr_0xee8;
                dword mbr_0xeec;
                dword mbr_0xef0;
                dword mbr_0xef4;
                dword mbr_0xef8;
                dword mbr_0xefc;
                dword mbr_0xf00;
                dword mbr_0xf04;
                dword mbr_0xf08;
                dword mbr_0xf0c;
                dword mbr_0xf10;
                dword mbr_0xf14;
                dword mbr_0xf18;
                dword mbr_0xf1c;
                dword mbr_0xf20;
                dword mbr_0xf24;
                dword mbr_0xf28;
                dword mbr_0xf2c;
                dword mbr_0xf30;
                dword mbr_0xf34;
                byte mbr_0xf38;
                dword mbr_0xf3c;
                dword mbr_0xf40;
                dword mbr_0xf44;
                dword mbr_0xf48;
                dword mbr_0xf4c;
                dword mbr_0xf50;
                dword mbr_0xf54;
                dword mbr_0xf58;
                dword mbr_0xf5c;
                dword mbr_0xf60;
                dword mbr_0xf64;
                dword mbr_0xf70;
                dword mbr_0xf74;
                dword mbr_0xf78;
                dword mbr_0xf7c;
                dword mbr_0xf80;
                dword mbr_0xf84;
                dword mbr_0xf88;
                dword mbr_0xf8c;
                dword mbr_0xfa8;
                dword mbr_0xfac;
                dword mbr_0xfb0;
                dword mbr_0xfb4;
                dword mbr_0xfb8;
                dword mbr_0xfbc;
                dword mbr_0xfc0;
                dword mbr_0xfc4;
                dword mbr_0xfc8;
                dword mbr_0xfcc;
                dword mbr_0xfd0;
                dword mbr_0xfd4;
                dword mbr_0xfd8;
                dword mbr_0xfdc;
                dword mbr_0xfe0;
                dword mbr_0xfe4;
                dword mbr_0xfe8;
                dword mbr_0xfec;
                dword mbr_0xff0;
                dword mbr_0xff4;
                dword mbr_0xff8;
                dword mbr_0xffc;
            //: Methods
            public:
                Edit( uint param_1 ); // ctor_0x4c51b0
                ~Edit(); // dtor_0x4c1240
                int meth_0x434220(); // meth_0x434220
                void meth_0x4a7db0( int param_1, int *param_2, int *param_3, int *param_4, uint *param_5 ); // meth_0x4a7db0
                int meth_0x4a8200( int param_1, void *param_2, int param_3 ); // meth_0x4a8200
                int meth_0x4a87b0( int param_1 ); // meth_0x4a87b0
                int meth_0x4a8800( int param_1 ); // meth_0x4a8800
                void meth_0x4a8900( int param_1, int param_2 ); // meth_0x4a8900
                void meth_0x4a89c0( int param_1 ); // meth_0x4a89c0
                undefined4 meth_0x4a8b90( int param_1, void *param_2 ); // meth_0x4a8b90
                float10 * meth_0x4c0510( float10 *__return_storage_ptr__, float param_1, float param_2 ); // meth_0x4c0510
                uint meth_0x4c05a0(); // meth_0x4c05a0
                void meth_0x4c05c0( int param_1 ); // meth_0x4c05c0
                void meth_0x4c0660( int param_1 ); // meth_0x4c0660
                void meth_0x4c09a0( int param_1 ); // meth_0x4c09a0
                void meth_0x4c0ef0(); // meth_0x4c0ef0
                void meth_0x4c13d0(); // meth_0x4c13d0
                void meth_0x4c14d0( int param_1 ); // meth_0x4c14d0
                void meth_0x4c1960(); // meth_0x4c1960
                void meth_0x4c1fa0( char param_1 ); // meth_0x4c1fa0
                void meth_0x4c2360( int param_1, int param_2 ); // meth_0x4c2360
                void meth_0x4c2390(); // meth_0x4c2390
                void meth_0x4c2f20(); // meth_0x4c2f20
                void meth_0x4c3af0( int param_1 ); // meth_0x4c3af0
                void meth_0x4c3af0( int param_1 ); // meth_0x4c4ed0
                virtual void virt_meth_0x4c0330(); // virt_meth_0x4c0330
                virtual void virt_meth_0x4c0360(); // virt_meth_0x4c0360
                virtual void virt_meth_0x4c0380(); // virt_meth_0x4c0380
                virtual void virt_meth_0x4c03b0(); // virt_meth_0x4c03b0
                virtual void virt_meth_0x4c03d0(); // virt_meth_0x4c03d0
                virtual void virt_meth_0x4c03f0(); // virt_meth_0x4c03f0
                virtual void virt_meth_0x4c0410(); // virt_meth_0x4c0410
                virtual void virt_meth_0x4c0430(); // virt_meth_0x4c0430
                virtual void virt_meth_0x4c0450(); // virt_meth_0x4c0450
                virtual void virt_meth_0x4c0470(); // virt_meth_0x4c0470
                virtual void virt_meth_0x4c0490(); // virt_meth_0x4c0490
                virtual void virt_meth_0x4c04b0(); // virt_meth_0x4c04b0
                virtual void virt_meth_0x4c04d0(); // virt_meth_0x4c04d0
                virtual void virt_meth_0x4c04f0(); // virt_meth_0x4c04f0
                virtual void virt_meth_0x4c0590(); // virt_meth_0x4c0590
                virtual uint virt_meth_0x4c0720(); // virt_meth_0x4c0720
                virtual void virt_meth_0x4c07d0(); // virt_meth_0x4c07d0
                virtual undefined4 virt_meth_0x4c13b0(); // virt_meth_0x4c13b0
                virtual void virt_meth_0x4c13c0(); // virt_meth_0x4c13c0
                virtual int * virt_meth_0x4c2250( byte param_1 ); // virt_meth_0x4c2250
                virtual void virt_meth_0x4c2270(); // virt_meth_0x4c2270
                virtual void virt_meth_0x4c22a0(); // virt_meth_0x4c22a0
                virtual void virt_meth_0x4c22d0(); // virt_meth_0x4c22d0
                virtual void virt_meth_0x4c2300(); // virt_meth_0x4c2300
                virtual void virt_meth_0x4c2330(); // virt_meth_0x4c2330
                virtual void virt_meth_0x4c3ac0(); // virt_meth_0x4c3ac0
                virtual undefined4 virt_meth_0x4c4ee0(); // virt_meth_0x4c4ee0
        }; // class Edit
    } // namespace GUI
} // namespace f3

#endif // F3_GUI_EDIT_HPP_
