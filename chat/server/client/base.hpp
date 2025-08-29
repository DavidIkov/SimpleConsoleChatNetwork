
#pragma once

#include <condition_variable>

#include "chat/events_handler.hpp"

namespace server {
class Base;
}

namespace client {

class Base : protected EventsHandler {
public:
    Base(server::Base *server, ClientRawDescriptor desc);
    ~Base() = default;
    Base(Base const &) = delete;
    Base &operator=(Base const &) = delete;
    Base(Base &&) noexcept = delete;
    Base &operator=(Base &&) noexcept = delete;

    using EventsHandler::AquireLock;
    using EventsHandler::GetLocalAddress;
    using EventsHandler::GetRemoteAddress;
    using EventsHandler::IsConnected;
    using EventsHandler::StopThreads;

protected:
    server::Base *server_;

    void _OnEvent(EventData const &ev_data) override;

    // cv that can be used to wait until some event reponse is processed.
    std::condition_variable event_respond_cv_;
};
}  // namespace client
