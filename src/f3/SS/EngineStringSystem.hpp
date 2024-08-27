// Copyright 2024 OpenJE

#ifndef F3_SS_ENGINESTRINGSYSTEM_HPP_
#define F3_SS_ENGINESTRINGSYSTEM_HPP_

#include "f3/global.hpp"
#include "f3/SS/StringSystem.hpp"

namespace f3 {
    namespace SS {
        class EngineStringSystem : public StringSystem {
            //: Members
            public:
                dword mbr_0x16c;
                dword mbr_0x170;
                dword mbr_0x174;
            //: Methods
            public:
                virt_meth_0x56bca0;
                meth_0x56be30;
                virt_meth_0x56c260;
                ctor_0x56c360;
                dtor_0x56c3e0;
        }; // class EngineStringSystem
    } // namespace SS
} // namespace f3

#endif // F3_SS_ENGINESTRINGSYSTEM_HPP_
