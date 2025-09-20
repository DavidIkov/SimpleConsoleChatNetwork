
#pragma once

#include "base.hpp"

namespace client {

class ConnectionHandler : public Base {
public:
    ConnectionHandler() = default;
    ~ConnectionHandler() = default;
    ConnectionHandler(ConnectionHandler const &) = delete;
    ConnectionHandler &operator=(ConnectionHandler const &) = delete;
    ConnectionHandler(ConnectionHandler &&) noexcept = delete;
    ConnectionHandler &operator=(ConnectionHandler &&) noexcept = delete;

    void Disconnect() override;
};
}  // namespace client
