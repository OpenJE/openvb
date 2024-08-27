// Copyright 2024 OpenJE

#ifndef F3_CLIENT_SERVER_HPP_
#define F3_CLIENT_SERVER_HPP_

#include "f3/global.hpp"
#include "f3/net/client/server.hpp"

namespace f3 {
	namespace Client {
        class Server : public net::client::server {}; // class Server
    }; // namespace Client
} // namespace f3

#endif // F3_CLIENT::SERVER_HPP_
