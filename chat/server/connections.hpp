#pragma once
#include <functional>
#include <vector>

#include "base.hpp"

namespace server {
class ConnectionsHandler : public Base {
public:
    ConnectionsHandler() = default;
    ConnectionsHandler(ConnectionsHandler const&) = delete;
    ConnectionsHandler& operator=(ConnectionsHandler const&) = delete;
    ConnectionsHandler(ConnectionsHandler&&) noexcept = delete;
    ConnectionsHandler& operator=(ConnectionsHandler&&) = delete;
    ~ConnectionsHandler();

    void StartListening(uint16_t port);
    void StopListening();

    bool IsListening() const;

    void GetConnections(
        std::function<void(
            std::vector<std::unique_ptr<client::Base>> const&)> const& callback)
        const;

    void GetConnection(
        size_t ind,
        std::function<void(const std::unique_ptr<client::Base>&)> const&
            callback) const;
    void GetConnection(
        size_t ind,
        std::function<void(const std::unique_ptr<client::Base>&)>& callback);

    void RemoveConnection(size_t ind);

protected:
    std::unique_ptr<client::Base> _ClientFactory(
        events::EventsProcessor::ClientRawDescriptor desc) override;

private:
    mutable std::mutex mutex_;

    std::vector<std::unique_ptr<client::Base>> connections_;

    std::thread accepting_thread_;
};

}  // namespace server
