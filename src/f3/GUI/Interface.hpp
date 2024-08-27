// Copyright 2024 OpenJE

#ifndef F3_GUI_INTERFACE_HPP_
#define F3_GUI_INTERFACE_HPP_

#include "f3/global.hpp"
#include "f3/GUI/HeapTracker.hpp"

namespace f3 {
    namespace GUI {
        class Interface : public HeapTracker {
            //: Members
            public:
                dword mbr_0x18;
                dword mbr_0x1c;
                byte mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                byte mbr_0x30;
                dword mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                meth_0x4058d0;
                meth_0x405950;
                meth_0x405c20;
                meth_0x405ce0;
                meth_0x405ec0;
                meth_0x406080;
                meth_0x40aaf0;
                meth_0x40b5a0;
                meth_0x40f240;
                meth_0x41f750;
                meth_0x4211f0;
                meth_0x429c70;
                virt_meth_0x430790;
                meth_0x4431e0;
                meth_0x4431f0;
                meth_0x44a7e0;
                meth_0x44ab00;
                meth_0x44b380;
                meth_0x44b5e0;
                meth_0x44e2f0;
                meth_0x44e3e0;
                meth_0x450050;
                meth_0x48ddf0;
                meth_0x48e2a0;
                meth_0x48eaa0;
                meth_0x48eae0;
                meth_0x493ac0;
                meth_0x493c00;
                virt_meth_0x4953c0;
                meth_0x495500;
                virt_meth_0x4955e0;
                ctor_0x495600;
                virt_meth_0x4958d0;
                meth_0x49e980;
                meth_0x49f560;
                meth_0x4a0620;
                meth_0x4b9cb0;
                meth_0x4b9d60;
                meth_0x4b9e30;
                meth_0x4c0df0;
                meth_0x4cf7c0;
                meth_0x4cf870;
                meth_0x4f6d40;
                meth_0x546560;
                meth_0x547a60;
                meth_0x547ab0;
                meth_0x55be20;
                meth_0x55be80;
                meth_0x55c520;
                meth_0x55c850;
                meth_0x55caa0;
                meth_0x55cb60;
                meth_0x55cdf0;
                meth_0x55cf40;
                meth_0x564920;
                meth_0x56cf70;
                meth_0x56d730;
                meth_0x56e060;
                meth_0x56ec50;
                meth_0x56f8a0;
                meth_0x56fe80;
                meth_0x570740;
                meth_0x571380;
                meth_0x571850;
                meth_0x57c090;
                meth_0x5b15b0;
                meth_0x5b1780;
                meth_0x5b1d70;
                meth_0x5b1ea0;
        }; // class Interface
    } // namespace GUI
} // namespace f3

#endif // F3_GUI_INTERFACE_HPP_
