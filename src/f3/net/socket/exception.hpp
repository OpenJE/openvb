// Copyright 2024 OpenJE

#ifndef F3_NET_SOCKET_EXCEPTION_HPP_
#define F3_NET_SOCKET_EXCEPTION_HPP_

#include "f3/global.hpp"
#include "f3/net/exception.hpp"

namespace f3 {
    namespace net {
        namespace socket {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    virt_meth_0x45f220;
                    ctor_0x5b8d20;
                    ctor_0x5b8e40;
            }; // struct exception
        } // namespace socket
    } // namespace net
} // namespace f3

#endif // F3_NET_SOCKET_EXCEPTION_HPP_
