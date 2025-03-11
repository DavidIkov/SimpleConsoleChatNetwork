#include"BasicServer.hpp"


void BasicServerC::RemoveClient(BasicClientSlotC& client) {
    ThreadLockC TL(this);
    for (size_t i = 0;i < Clients.size();i++) {
        if (&*Clients[i] == &client) { Clients.erase(Clients.begin() + i); return; }
    }
}
BasicClientSlotC& BasicServerC::ClientFactory(asio::io_context& context) {
    return *Clients.emplace_back(new BasicClientSlotC(context)).get();
}
void BasicServerC::SetUpCallbacksForNewClient(BasicClientSlotC& client) {
    client.sOnConnectionAcceptErrorCallback(this, [](BasicClientSlotC* client, void* ptr) {
        BasicServerC* serv = (BasicServerC*)ptr;
        ThreadLockC TL(serv); if (!TL) return;
        serv->OnAcceptConnectionFailure();
        client->ListenForConnection(serv->ConnectionsAcceptor);
        });
    client.sOnDisconnectCallback(this, [](BasicClientSlotC* client, void* ptr, DisconnectReasonE reason) {
        BasicServerC* serv = (BasicServerC*)ptr;
        ThreadLockC TL(serv); if (!TL) return;
        serv->OnDisconnect(*client, reason);
        });
    client.sOnConnectCallback(this, [](BasicClientSlotC* client, void* ptr) {
        BasicServerC* serv = (BasicServerC*)ptr;
        ThreadLockC TL(serv); if (!TL) return;
        serv->OnConnect(*client);
        serv->StartAcceptingConnections();
        });
}
void BasicServerC::StartAcceptingConnections() {
    ThreadLockC TL(this); if (!TL) return;
    BasicClientSlotC& client = ClientFactory(AsioContext);
    SetUpCallbacksForNewClient(client);
    client.ListenForConnection(ConnectionsAcceptor);
}
BasicServerC::BasicServerC(asio::io_context& asioContext, asio::ip::port_type port) :
    AsioContext(asioContext), ConnectionsAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

void BasicServerC::Shutdown() {
    ThreadLockC TL(this); if (!TL) return;
    if (ConnectionsAcceptor.is_open()) {
        ConnectionsAcceptor.close();
        for (auto& client : Clients) if (client->gIsConnected()) client->Disconnect(false);
        Clients.clear();
    }
}