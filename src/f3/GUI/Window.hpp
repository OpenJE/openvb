// Copyright 2024 OpenJE

#ifndef F3_GUI_WINDOW_HPP_
#define F3_GUI_WINDOW_HPP_

#include "f3/global.hpp"
#include "f3/GUI/Base.hpp"
#include "f3/GUI/HeapTracker.hpp"

namespace f3 {
    namespace GUI {
        class Window : public Base, public HeapTracker {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x1c;
                dword mbr_0x1e8;
                dword mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x340;
                dword mbr_0x38;
                dword mbr_0x39;
                dword mbr_0x3a;
                byte mbr_0x3b;
                byte mbr_0x3c;
                dword mbr_0x4;
                dword mbr_0x40;
                byte mbr_0x44;
                dword mbr_0x498;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                byte mbr_0x5f;
                dword mbr_0x5f0;
                dword mbr_0x60;
                dword mbr_0x64;
                byte mbr_0x68;
                dword mbr_0x6c;
                dword mbr_0x70;
                dword mbr_0x74;
                dword mbr_0x748;
                dword mbr_0x78;
                dword mbr_0x7c;
                dword mbr_0x8;
                dword mbr_0x80;
                dword mbr_0x84;
                dword mbr_0x88;
                dword mbr_0x8a0;
                dword mbr_0x8c;
                dword mbr_0x90;
                dword mbr_0x9f8;
                dword mbr_0xb50;
                dword mbr_0xc;
                dword mbr_0xca8;
                dword mbr_0xcac;
                dword mbr_0xcb0;
                dword mbr_0xcb4;
                dword mbr_0xcb8;
                byte mbr_0xcbc;
                dword mbr_0xccc;
                dword mbr_0xcd0;
                dword mbr_0xcd4;
                dword mbr_0xcd8;
                byte mbr_0xcdc;
                dword mbr_0xcec;
                dword mbr_0xcf0;
                dword mbr_0xcf4;
                dword mbr_0xcf8;
                byte mbr_0xcfc;
                dword mbr_0xd0c;
                dword mbr_0xd10;
                dword mbr_0xd14;
                dword mbr_0xd18;
            //: Methods
            public:
                virt_meth_0x420990;
                meth_0x495a50;
                virt_meth_0x4af020;
                virt_meth_0x4af040;
                virt_meth_0x4af080;
                virt_meth_0x4af170;
                virt_meth_0x4af190;
                virt_meth_0x4af1a0;
                virt_meth_0x4af1b0;
                virt_meth_0x4af1e0;
                virt_meth_0x4af1f0;
                virt_meth_0x4af250;
                virt_meth_0x4af260;
                virt_meth_0x4af270;
                virt_meth_0x4af280;
                virt_meth_0x4af290;
                virt_meth_0x4af2d0;
                virt_meth_0x4af2e0;
                virt_meth_0x4af2f0;
                virt_meth_0x4af300;
                virt_meth_0x4af310;
                virt_meth_0x4af320;
                virt_meth_0x4af380;
                virt_meth_0x4af390;
                virt_meth_0x4af460;
                virt_meth_0x4af470;
                virt_meth_0x4af480;
                virt_meth_0x4af4c0;
                virt_meth_0x4af4d0;
                virt_meth_0x4af4e0;
                virt_meth_0x4af4f0;
                virt_meth_0x4af580;
                virt_meth_0x4af590;
                virt_meth_0x4af5a0;
                virt_meth_0x4af5b0;
                virt_meth_0x4af810;
                virt_meth_0x4af820;
                virt_meth_0x4af830;
                virt_meth_0x4af890;
                virt_meth_0x4af8a0;
                virt_meth_0x4af8f0;
                virt_meth_0x4af980;
                virt_meth_0x4af9c0;
                virt_meth_0x4afa90;
                virt_meth_0x4afb30;
                virt_meth_0x4afc00;
                virt_meth_0x4afc70;
                virt_meth_0x4aff30;
                virt_meth_0x4affa0;
                virt_meth_0x4b0000;
                virt_meth_0x4b0040;
                virt_meth_0x4b0160;
                virt_meth_0x4b0280;
                virt_meth_0x4b02c0;
                virt_meth_0x4b02e0;
                virt_meth_0x4b0480;
                virt_meth_0x4b0520;
                virt_meth_0x4b05a0;
                virt_meth_0x4b0640;
                virt_meth_0x4b0660;
                virt_meth_0x4b0680;
                virt_meth_0x4b06a0;
                virt_meth_0x4b06c0;
                ctor_0x4b0810;
                virt_meth_0x4b0a40;
                virt_meth_0x4b0ca0;
                virt_meth_0x4b0d50;
                virt_meth_0x4b0df0;
                virt_meth_0x4b0e10;
                virt_meth_0x4b10f0;
                virt_meth_0x4b13f0;
                meth_0x4b2890;
                virt_meth_0x5a9320;
                virt_meth_0x5c0350;
        }; // class Window
    } // namespace GUI
} // namespace f3

#endif // F3_GUI_WINDOW_HPP_
