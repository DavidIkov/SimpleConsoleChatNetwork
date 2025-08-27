
#pragma once

#include "base.hpp"
#include "chat/events_handler.hpp"

namespace client {

class ConnectionHandler : public Base {
public:
    ConnectionHandler(server::Base *server, ClientRawDescriptor desc);
    ~ConnectionHandler();
    ConnectionHandler(ConnectionHandler const &) = delete;
    ConnectionHandler &operator=(ConnectionHandler const &) = delete;
    ConnectionHandler(ConnectionHandler &&) noexcept = delete;
    ConnectionHandler &operator=(ConnectionHandler &&) noexcept = delete;

    using EventsHandler::GetLocalAddress;
    using EventsHandler::GetRemoteAddress;

protected:
    void _OnDisconnect() override final;
};
}  // namespace client
