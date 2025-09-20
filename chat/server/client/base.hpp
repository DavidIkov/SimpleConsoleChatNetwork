
#pragma once

#include "events/processor/processor.hpp"

namespace server {
class Base;
}

namespace client {

class Base : protected events::EventsProcessor {
public:
    Base(server::Base *server, ClientRawDescriptor desc);
    ~Base() = default;
    Base(Base const &) = delete;
    Base &operator=(Base const &) = delete;
    Base(Base &&) noexcept = delete;
    Base &operator=(Base &&) noexcept = delete;

    using EventsProcessor::Connect;
    using EventsProcessor::Disconnect;
    using EventsProcessor::GetLocalAddress;
    using EventsProcessor::GetRemoteAddress;
    using EventsProcessor::IsConnected;
    using EventsProcessor::SendRequest;
    using EventsProcessor::WaitTillEndOfDataProcessing;
    using EventsProcessor::OutgoingRequest;
    using EventsProcessor::OutgoingRespond;
    using EventsProcessor::IncomingRequest;
    using EventsProcessor::IncomingRespond;



protected:
    OutgoingRespond _ProcessRequest(IncomingRequest const &pack) override;
    server::Base *server_;
};
}  // namespace client
