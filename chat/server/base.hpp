#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include <functional>

#include "client/base.hpp"
#include "networking/socket/tcp.hpp"

namespace server {
class Base : protected networking::socket::TCP {
public:
    Base();

protected:
    virtual std::unique_ptr<client::Base> _ClientFactory(
        events::EventsProcessor::ClientRawDescriptor desc) = 0;

    void GetDB(std::function<void(SQLite::Database&)> const& callback);

private:
    std::mutex mutex_;
    SQLite::Database db_;
};
}  // namespace server
