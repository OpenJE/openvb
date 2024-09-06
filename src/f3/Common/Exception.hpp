// Copyright 2024 OpenJE

#ifndef F3_COMMON_EXCEPTION_HPP_
#define F3_COMMON_EXCEPTION_HPP_

#include "f3/global.hpp"

namespace f3 {
    namespace Common {
        struct Exception {
            //: Members
                dword mbr_0x4;
            //: Methods
                Exception(); // ctor_0x4011b0
                virtual ~Exception(); // dtor_0x4011a0, virt_deldtor_0x401010
        }; // struct Exception
    } // namespace Common
} // namespace f3

#endif // F3_COMMON_EXCEPTION_HPP_
