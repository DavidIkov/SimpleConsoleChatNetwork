#include "connection.hpp"

#include "chat/server/connections.hpp"
#include "spdlog/spdlog.h"

namespace client {
ConnectionHandler::ConnectionHandler(server::Base* server,
                                     ClientRawDescriptor desc)
    : Base(server, desc) {
    SPDLOG_INFO("Client joined: {}.", GetRemoteAddress());
}
ConnectionHandler::~ConnectionHandler() { SPDLOG_INFO("Client left."); }

void ConnectionHandler::Disconnect() {
    std::thread([server = (server::ConnectionsHandler*)server_, ts = this] {
        size_t i = 0;
        server->GetConnections(
            [&](std::vector<std::unique_ptr<Base>> const& cons) {
                for (auto const& con : cons) {
                    if (con.get() == ts) break;
                    ++i;
                }
            });
        server->RemoveConnection(i);
    }).detach();
    Base::Disconnect();
}
}  // namespace client
