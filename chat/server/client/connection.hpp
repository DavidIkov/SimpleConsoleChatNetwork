
#pragma once

#include "base.hpp"

namespace client {

class ConnectionHandler : public Base {
public:
    ConnectionHandler(server::Base *server, ClientRawDescriptor desc);
    ~ConnectionHandler();
    ConnectionHandler(ConnectionHandler const &) = delete;
    ConnectionHandler &operator=(ConnectionHandler const &) = delete;
    ConnectionHandler(ConnectionHandler &&) noexcept = delete;
    ConnectionHandler &operator=(ConnectionHandler &&) noexcept = delete;

protected:
    void Disconnect() override;
};
}  // namespace client
