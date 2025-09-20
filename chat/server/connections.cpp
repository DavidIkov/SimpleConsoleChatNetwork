#include "connections.hpp"

#include "client/connection.hpp"
#include "spdlog/spdlog.h"

namespace server {

ConnectionsHandler::~ConnectionsHandler() {
    if (IsListening()) {
        Close();
        accepting_thread_.join();
    }
}

void ConnectionsHandler::StartListening(uint16_t port) {
    if (IsListening()) {
        SPDLOG_ERROR(
            "Failed to start listening on port {}. Connections handler is "
            "already listening.",
            port);
        throw std::exception();
    }
    Open();
    BindToAddress({0, port});
    MarkSocketAsListening(1);
    std::lock_guard LG(mutex_);
    accepting_thread_ = std::thread([this]() {
        try {
            while (1) {
                networking::RawDescriptorT desc = AcceptConnection();
                if (desc == 0) return;
                std::lock_guard LG(mutex_);
                connections_.emplace_back(_ClientFactory(desc));
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Caught exception in accepting thread. {}",
                         err.what());
        }
    });
}
void ConnectionsHandler::StopListening() {
    if (!IsListening()) {
        SPDLOG_ERROR(
            "Failed to stop listening. Connections handler is not listening.");
        throw std::exception();
    }
    Close();
    std::lock_guard LG(mutex_);
    accepting_thread_.join();
}

void ConnectionsHandler::RemoveConnection(size_t ind) {
    std::lock_guard LG(mutex_);
    connections_.erase(connections_.begin() + ind);
}

std::unique_ptr<client::Base> ConnectionsHandler::_ClientFactory(
    events::EventsProcessor::ClientRawDescriptor desc) {
    return std::make_unique<client::ConnectionHandler>(this, desc);
}

bool ConnectionsHandler::IsListening() const {
    std::lock_guard LG(mutex_);
    return accepting_thread_.joinable();
}

void ConnectionsHandler::GetConnections(
    std::function<void(
        std::vector<std::unique_ptr<client::Base>> const&)> const& callback)
    const {
    std::lock_guard LG(mutex_);
    callback(connections_);
}

void ConnectionsHandler::GetConnection(
    size_t ind,
    std::function<void(const std::unique_ptr<client::Base>&)> const& callback)
    const {
    std::lock_guard LG(mutex_);
    callback(connections_[ind]);
}

void ConnectionsHandler::GetConnection(
    size_t ind,
    std::function<void(const std::unique_ptr<client::Base>&)>& callback) {
    std::lock_guard LG(mutex_);
    callback(connections_[ind]);
}

}  // namespace server
