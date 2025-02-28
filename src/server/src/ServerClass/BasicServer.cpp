#include"BasicServer.hpp"

void BasicServerC::_RemoveClient(BasicClientS& client) {
    std::lock_guard lg(ServerMutex);
    for (size_t i = 0;i < Clients.size();i++) {
        if (&*Clients[i] == &client) { Clients.erase(Clients.begin() + i); return; }
    }
}
void BasicServerC::_StartReading(BasicClientS& client) {
    std::lock_guard lg(ServerMutex);
    client.Socket.async_read_some(asio::buffer(client.ReadBuffer, sizeof(client.ReadBuffer)), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                std::lock_guard lg(ServerMutex);
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client, DisconnectReasonE::ClientDisconnected);
                _RemoveClient(client);
                return;
            }
            else if (ec == asio::error::operation_aborted) {
                //no need for shutdown and close and removing of socket since this code will happen only on shutdown, 
                //so its done forcefully and i can rely on caller to do allat
                return;
            }
            else if (ec == asio::error::connection_reset) {
                std::lock_guard lg(ServerMutex);
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client, DisconnectReasonE::ClientResetedConnection);
                _RemoveClient(client);
                return;
            }
            else {
                std::lock_guard lg(ServerMutex);
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client, DisconnectReasonE::UnknownError);
                _RemoveClient(client);
                return;
            }
        }
        OnRead(client, bytes);
        _StartReading(client);
        });
}
BasicServerC::BasicClientS& BasicServerC::ClientFactory() {
    std::lock_guard lg(ServerMutex);
    return *Clients.emplace(Clients.begin(), new BasicClientS(AsioContext.get()))->get();
}
void BasicServerC::StartAcceptingConnections() {
    std::lock_guard lg(ServerMutex);
    BasicClientS& client = ClientFactory();
    ConnectionsAcceptor.async_accept(client.Socket, [&](asio::error_code ec) {
        if (ec) {
            if (ec == asio::error::operation_aborted) {
                OnAcceptConnectionError(OnAcceptConnectionErrorE::ServerClosedAcceptor);
                return;
            } else {
                OnAcceptConnectionError(OnAcceptConnectionErrorE::UnknownError);
                return;
            }
        }
        OnConnect();
        _StartReading(client);
        StartAcceptingConnections();
        });

}
BasicServerC::BasicServerC(asio::io_context& asioContext, asio::ip::port_type port) :
    AsioContext(asioContext), ConnectionsAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}
BasicServerC::~BasicServerC() {
    std::lock_guard lg(ServerMutex);
    Shutdown();
}
auto BasicServerC::_WriteToClient(BasicClientS& client, void const* arr, size_t lenInBytes) -> WriteToClientResultE {
    std::lock_guard lg(ServerMutex);
    if (!client.gIsActive()) return WriteToClientResultE::ClientIsNotActive;
    if (lenInBytes != 0) {
        size_t bytesOffset = 0;
        asio::error_code ec;
        while ((bytesOffset +=
            client.Socket.write_some(asio::buffer((char*)arr + bytesOffset, lenInBytes - bytesOffset), ec)) != lenInBytes)
            if (ec) {
                if (ec == asio::error::operation_aborted) return WriteToClientResultE::StoppedByServer;
                else return WriteToClientResultE::UknownError;
            }
    }
    return WriteToClientResultE::NoErrors;
}
void BasicServerC::Shutdown() {
    std::lock_guard lg(ServerMutex);
    if (ConnectionsAcceptor.is_open()) {
        ConnectionsAcceptor.close();
        for (auto& client : Clients)
            if (client.get()->gIsActive()) {
                OnDisconnect(*client.get(), DisconnectReasonE::ServerDisconnected);
                asio::error_code ec;
                //todo do some error handling here
                client.get()->Socket.shutdown(client.get()->Socket.shutdown_both, ec);
                client.get()->Socket.close(ec);
            }
        Clients.clear();
    }
}