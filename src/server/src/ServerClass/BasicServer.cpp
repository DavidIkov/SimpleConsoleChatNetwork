#include"BasicServer.hpp"


void BasicServerC::_RemoveClient(BasicClientSlotC& client) {
    //todo handle client destructor deadlock
    for (size_t i = 0;i < Clients.size();i++) {
        if (&*Clients[i] == &client) { Clients.erase(Clients.begin() + i); return; }
    }
}
BasicClientSlotC& BasicServerC::ClientFactory() {
    ThreadLockC TL(this);
    return *Clients.emplace(Clients.begin(), new BasicClientSlotC(AsioContext.get()))->get();
}
void BasicServerC::StartAcceptingConnections() {
    ThreadLockC TL(this);
    BasicClientSlotC& client = ClientFactory();
    client.ListenForConnection(ConnectionsAcceptor);
    
}
BasicServerC::BasicServerC(asio::io_context& asioContext, asio::ip::port_type port) :
    AsioContext(asioContext), ConnectionsAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

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