#pragma once
#include <vector>

#include "chat/synchronized_value.hpp"
#include "client/connection.hpp"
#include "socket/socket_tcp.hpp"

namespace server {
class ConnectionsHandler : private Socket::Socket_TCP {
public:
    void StartListening(uint16_t port);
    void StopListening();

    inline bool GetIsListening() const;

    inline SynchronizedValue<
        const std::vector<std::unique_ptr<client::ConnectionHandler>>&>
    GetConnections() const;

    inline SynchronizedValue<const std::unique_ptr<client::ConnectionHandler>&>
    GetConnection(size_t ind) const;
    inline SynchronizedValue<std::unique_ptr<client::ConnectionHandler>&>
    GetConnection(size_t ind);

    void RemoveConnection(size_t ind);

protected:
    virtual std::unique_ptr<client::ConnectionHandler> _ConnectionFactory(
        EventsHandler::ClientRawDescriptor desc);

    inline const std::vector<std::unique_ptr<client::ConnectionHandler>>&
    _GetConnections() const;

    inline const std::unique_ptr<client::ConnectionHandler>& _GetConnection(
        size_t ind) const;
    inline std::unique_ptr<client::ConnectionHandler>& _GetConnection(
        size_t ind);

    inline bool _GetIsListening() const;

    using Socket_TCP::mutex_;

private:
    std::vector<std::unique_ptr<client::ConnectionHandler>> connections_;

    std::thread accepting_thread_;
};

SynchronizedValue<
    const std::vector<std::unique_ptr<client::ConnectionHandler>>&>
ConnectionsHandler::GetConnections() const {
    return {mutex_, connections_};
}

SynchronizedValue<const std::unique_ptr<client::ConnectionHandler>&>
ConnectionsHandler::GetConnection(size_t ind) const {
    return {mutex_, connections_[ind]};
}

SynchronizedValue<std::unique_ptr<client::ConnectionHandler>&>
ConnectionsHandler::GetConnection(size_t ind) {
    return {mutex_, connections_[ind]};
}

bool ConnectionsHandler::GetIsListening() const {
    std::lock_guard LG(mutex_);
    return _GetIsListening();
}

bool ConnectionsHandler::_GetIsListening() const {
    return accepting_thread_.joinable();
}

const std::vector<std::unique_ptr<client::ConnectionHandler>>&
ConnectionsHandler::_GetConnections() const {
    return connections_;
}

const std::unique_ptr<client::ConnectionHandler>&
ConnectionsHandler::_GetConnection(size_t ind) const {
    return connections_[ind];
}

std::unique_ptr<client::ConnectionHandler>& ConnectionsHandler::_GetConnection(
    size_t ind) {
    return connections_[ind];
}

}  // namespace server
