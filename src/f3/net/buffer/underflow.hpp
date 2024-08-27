// Copyright 2024 OpenJE

#ifndef F3_NET_BUFFER_UNDERFLOW_HPP_
#define F3_NET_BUFFER_UNDERFLOW_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        namespace buffer {
            struct underflow : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x45e650;
            }; // struct underflow
        } // namespace buffer
    } // namespace net
} // namespace f3

#endif // F3_NET_BUFFER_UNDERFLOW_HPP_
