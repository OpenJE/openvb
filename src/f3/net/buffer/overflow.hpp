// Copyright 2024 OpenJE

#ifndef F3_NET_BUFFER_OVERFLOW_HPP_
#define F3_NET_BUFFER_OVERFLOW_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        namespace buffer {
            struct overflow : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x45e450;
                    ctor_0x45f090;
            }; // struct overflow
        } // namespce buffer
    } // namespace net
} // namespace f3

#endif // F3_NET_BUFFER_OVERFLOW_HPP_
