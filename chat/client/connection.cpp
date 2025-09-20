#include "connection.hpp"

#include "spdlog/spdlog.h"

namespace client {

void ConnectionHandler::Disconnect() {
    SPDLOG_INFO("Disconnected from server.");
    Base::Disconnect();
}
}  // namespace client
