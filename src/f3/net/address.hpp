// Copyright 2024 OpenJE

#ifndef F3_NET_ADDRESS_HPP_
#define F3_NET_ADDRESS_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        class address {
            //: Members
            public:
                byte mbr_0x4;
                byte mbr_0x5;
                byte mbr_0x6;
                byte mbr_0x7;
                word mbr_0x8;
            //: Methods
            public:
                dtor_0x45e410;
                virt_deldtor_0x45e420;
                ctor_0x5a07b0;
        }; // class address
    } // namespace net
} // namespace f3

#endif // F3_NET_ADDRESS_HPP_
