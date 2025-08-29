#pragma once
#include <vector>

#include "base.hpp"

namespace server {
class ConnectionsHandler : public Base {
public:
    void StartListening(uint16_t port);
    void StopListening();

    inline bool IsListening() const;

    inline const std::vector<std::unique_ptr<client::Base>>& GetConnections()
        const;

    inline const std::unique_ptr<client::Base>& GetConnection(size_t ind) const;
    inline std::unique_ptr<client::Base>& GetConnection(size_t ind);

    void RemoveConnection(size_t ind);

    void StopThreads() override;

protected:
    std::unique_ptr<client::Base> _ConnectionFactory(
        EventsHandler::ClientRawDescriptor desc) override;

private:
    std::vector<std::unique_ptr<client::Base>> connections_;

    std::thread accepting_thread_;
};

const std::vector<std::unique_ptr<client::Base>>&
ConnectionsHandler::GetConnections() const {
    return connections_;
}

const std::unique_ptr<client::Base>& ConnectionsHandler::GetConnection(
    size_t ind) const {
    return connections_[ind];
}

std::unique_ptr<client::Base>& ConnectionsHandler::GetConnection(size_t ind) {
    return connections_[ind];
}

bool ConnectionsHandler::IsListening() const {
    return accepting_thread_.joinable();
}

}  // namespace server
