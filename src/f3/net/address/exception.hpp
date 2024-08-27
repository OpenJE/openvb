// Copyright 2024 OpenJE

#ifndef F3_NET_ADDRESS_EXCEPTION_HPP_
#define F3_NET_ADDRESS_EXCEPTION_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        namespace address {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x5ac9c0;
                    ctor_0x5acbe0;
            }; // struct exception
        } // namespace address
    } // namespace net
} // namespace f3

#endif // F3_NET_ADDRESS_EXCEPTION_HPP_
