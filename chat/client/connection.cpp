#include "connection.hpp"

#include <iostream>

namespace client {
ConnectionHandler::~ConnectionHandler() {
    if (!destruction_mutex_locked_) {
        destruction_mutex_locked_ = true;
        mutex_.lock();
    }
}

void ConnectionHandler::_OnEvent(events::Type evTyp, void const *evData) {}
void ConnectionHandler::_OnDisconnect() {
    std::cout << "other side disconnected" << std::endl;
}
}  // namespace client
