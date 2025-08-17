#include "connection.hpp"

#include <iostream>

#include "chat/server/connections.hpp"

namespace client {
ConnectionHandler::ConnectionHandler(server::ConnectionsHandler* server,
                                       ClientRawDescriptor desc)
    : EventsHandler(desc), server_(server) {
    std::cout << "client joined" << std::endl;
}
ConnectionHandler::~ConnectionHandler() {
    if (!destruction_mutex_locked_) {
        destruction_mutex_locked_ = true;
        mutex_.lock();
    }
    std::cout << "client left" << std::endl;
}

void ConnectionHandler::_OnEvent(events::Type evTyp, void const* evData) {}

void ConnectionHandler::_OnDisconnect() {
    std::thread([server = server_, ts = this] {

        size_t i = 0;
        for (auto const& con : server->GetConnections().v_) {
            if (con.get() == ts) break;
            ++i;
        }
        server->RemoveConnection(i);
    }).detach();
}
}  // namespace client
