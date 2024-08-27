// Copyright 2024 OpenJE

#ifndef F3_NET_CONNECTION_EXCEPTION_HPP_
#define F3_NET_CONNECTION_EXCEPTION_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        namespace connection {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x5ab750;
            }; // struct exception
        } // namespace connection
    } // namespace net
} // namespace f3

#endif // F3_NET_CONNECTION_EXCEPTION_HPP_
