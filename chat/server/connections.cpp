#include "connections.hpp"

namespace server {

void ConnectionsHandler::StartListening(uint16_t port) {
    std::lock_guard LG(mutex_);
    if (_GetIsListening())
        throw std::logic_error("cant start listening when already listening");
    _Open();
    _BindToAddress({0, port});
    _MarkSocketAsListening(1);
    accepting_thread_ = std::thread([this]() {
        while (1) {
            mutex_.lock();
            Socket::RawDescriptorT desc = _AcceptConnection();
            if (desc == 0) return;
            std::lock_guard LG(mutex_);
            connections_.emplace_back(_ConnectionFactory(desc));
        }
    });
}
void ConnectionsHandler::StopListening() {
    std::lock_guard LG(mutex_);
    if (!_GetIsListening())
        throw std::logic_error("trying to stop listening when not listening");
    _Close();
    accepting_thread_.join();
}

void ConnectionsHandler::RemoveConnection(size_t ind) {
    std::lock_guard LG(mutex_);
    connections_.erase(connections_.begin() + ind);
}

std::unique_ptr<client::ConnectionHandler>
ConnectionsHandler::_ConnectionFactory(
    EventsHandler::ClientRawDescriptor desc) {
    return std::make_unique<client::ConnectionHandler>(this, desc);
}

}  // namespace server
