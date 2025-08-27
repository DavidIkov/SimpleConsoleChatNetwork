#include "connection.hpp"

#include <iostream>

#include "chat/server/connections.hpp"

namespace client {
ConnectionHandler::ConnectionHandler(server::Base* server,
                                     ClientRawDescriptor desc)
    : Base(server, desc) {
    std::cout << "client joined" << std::endl;
}
ConnectionHandler::~ConnectionHandler() {
    std::cout << "client left" << std::endl;
}

void ConnectionHandler::_OnDisconnect() {
    std::thread([server = (server::ConnectionsHandler*)server_, ts = this,
                 mutex = &mutex_] {
        auto LG = server->AquireLock();
        size_t i = 0;
        for (auto const& con : server->GetConnections()) {
            if (con.get() == ts) break;
            ++i;
        }
        server->RemoveConnection(i);
    }).detach();
}
}  // namespace client
