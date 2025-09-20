#include "base.hpp"

#include "spdlog/spdlog.h"

namespace client {

Base::Base() : db_(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {}
auto Base::_ProcessRequest(IncomingRequest const &pack)
    -> OutgoingRespond {
    SPDLOG_ERROR("Failed to process request respond, no one handled it. {}.", pack);
    throw std::exception();
}

}  // namespace client
