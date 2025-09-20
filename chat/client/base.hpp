
#pragma once

#include "events/processor/processor.hpp"
#include <SQLiteCpp/SQLiteCpp.h>

namespace client {

class Base : protected events::EventsProcessor {
public:
    Base();
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
    SQLite::Database db_;
};
}  // namespace client
