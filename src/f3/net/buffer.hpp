// Copyright 2024 OpenJE

#ifndef F3_NET_BUFFER_HPP_
#define F3_NET_BUFFER_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        class buffer {
            //: Members
            public:
                dword mbr_0x14;
                dword mbr_0x18;
                byte mbr_0x4;
            //: Methods
            public:
                virt_deldtor_0x45f240;
                dtor_0x45f280;
        }; // class buffer
    } // namespace net
} // namespace f3

#endif // F3_NET_BUFFER_HPP_
