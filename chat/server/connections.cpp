#include "connections.hpp"

#include "client/connection.hpp"
namespace server {

void ConnectionsHandler::StartListening(uint16_t port) {
    if (IsListening())
        throw std::logic_error("cant start listening when already listening");
    Open();
    BindToAddress({0, port});
    MarkSocketAsListening(1);
    accepting_thread_ = std::thread([this]() {
        while (1) {
            mutex_.lock();
            Socket::RawDescriptorT desc = AcceptConnection();
            if (desc == 0) return;
            std::lock_guard LG(mutex_);
            connections_.emplace_back(_ConnectionFactory(desc));
        }
    });
}
void ConnectionsHandler::StopListening() {
    if (!IsListening())
        throw std::logic_error("trying to stop listening when not listening");
    Close();
    accepting_thread_.join();
}

void ConnectionsHandler::RemoveConnection(size_t ind) {
    {
        auto LG = connections_[ind]->AquireLock();
        connections_[ind]->StopThreads();
    }
    connections_.erase(connections_.begin() + ind);
}

void ConnectionsHandler::StopThreads() {
    if (IsListening()) {
        Close();
        accepting_thread_.join();
    }
}

std::unique_ptr<client::Base> ConnectionsHandler::_ConnectionFactory(
    EventsHandler::ClientRawDescriptor desc) {
    return std::make_unique<client::ConnectionHandler>(this, desc);
}

}  // namespace server
