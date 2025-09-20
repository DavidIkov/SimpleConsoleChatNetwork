#include "base.hpp"

#include "spdlog/spdlog.h"

namespace client {

Base::Base(server::Base* server, ClientRawDescriptor desc)
    : EventsProcessor(desc), server_(server) {}

auto Base::_ProcessRequest(IncomingRequest const& pack) -> OutgoingRespond {
    SPDLOG_ERROR("Failed to process request respond, no one handled it. {}.",
                 pack);
    throw std::exception();
}

}  // namespace client
