
#pragma once

#include "chat/events_handler.hpp"

namespace client {

class ConnectionHandler : protected EventsHandler {
public:
    ConnectionHandler() = default;
    ~ConnectionHandler();
    ConnectionHandler(ConnectionHandler const &) = delete;
    ConnectionHandler &operator=(ConnectionHandler const &) = delete;
    ConnectionHandler(ConnectionHandler &&) noexcept = default;
    ConnectionHandler &operator=(ConnectionHandler &&) noexcept = default;

    using EventsHandler::Connect;
    using EventsHandler::Disconnect;
    using EventsHandler::GetLocalAddress;
    using EventsHandler::GetRemoteAddress;

protected:
    void _OnEvent(events::Type evTyp, void const *evData) override;
    void _OnDisconnect() override final;
};
}  // namespace client
