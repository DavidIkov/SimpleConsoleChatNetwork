
#pragma once

#include "chat/events_handler.hpp"

namespace server {
class ConnectionsHandler;
}

namespace client {

class ConnectionHandler : protected EventsHandler {
public:
    ConnectionHandler(server::ConnectionsHandler *server,
                       ClientRawDescriptor desc);
    ~ConnectionHandler();
    ConnectionHandler(ConnectionHandler const &) = delete;
    ConnectionHandler &operator=(ConnectionHandler const &) = delete;
    ConnectionHandler(ConnectionHandler &&) noexcept = default;
    ConnectionHandler &operator=(ConnectionHandler &&) noexcept = default;

    using EventsHandler::GetLocalAddress;
    using EventsHandler::GetRemoteAddress;

protected:
    server::ConnectionsHandler *server_;

    void _OnEvent(events::Type evTyp, void const *evData) override;
    void _OnDisconnect() override final;
};
}  // namespace client
