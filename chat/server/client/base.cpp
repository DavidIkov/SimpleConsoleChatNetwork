#include "base.hpp"

#include <iostream>

namespace client {

Base::Base(server::Base* server, ClientRawDescriptor desc)
    : EventsHandler(desc), server_(server) {}

void Base::_OnEvent(EventData const&) {
    std::cout << "received unhandled event" << std::endl;
}

}  // namespace client
