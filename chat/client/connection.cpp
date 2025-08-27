#include "connection.hpp"

#include <iostream>

namespace client {

void ConnectionHandler::_OnDisconnect() {
    std::cout << "other side disconnected" << std::endl;
}
void ConnectionHandler::Disconnect() {
    std::cout << "client disconnected" << std::endl;
    Base::Disconnect();
}
}  // namespace client
