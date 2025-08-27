#pragma once

#include "client/base.hpp"
#include "socket/socket_tcp.hpp"

namespace server {
class Base : protected Socket::Socket_TCP {
public:
    using Socket_TCP::AquireLock;

protected:
    virtual std::unique_ptr<client::Base> _ConnectionFactory(
        EventsHandler::ClientRawDescriptor desc) = 0;
};
}  // namespace server
