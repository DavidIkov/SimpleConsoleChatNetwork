#include"BasicServer.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

void BasicServerC::_RemoveClient(BasicClientS& client) {
    for (size_t i = 0;i < Clients.size();i++) {
        if (&*Clients[i] == &client) { Clients.erase(Clients.begin() + i); return; }
    }
}
void BasicServerC::_StartReading(BasicClientS& client) {
    client.Socket.async_read_some(asio::buffer(client.ReadBuffer, sizeof(client.ReadBuffer)), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                OutputMacro << "Client disconnected, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                _RemoveClient(client);
                return;
            }
            else if (ec == asio::error::operation_aborted) {
                OutputMacro << "Server forcefully closed socket, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                //no need for shutdown and close and removing of socket since this code will happen only on shutdown, 
                //so its done forcefully and i can rely on caller to do allat
                OnDisconnect(client);
                return;
            }
            else if (ec == asio::error::connection_reset) {
                OutputMacro << "Client reseted connection, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                _RemoveClient(client);
                return;
            }
            else {
                OutputMacro << "Unhandled error occured in socket, stopping reading " <<
                    ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                _RemoveClient(client);
                return;
            }
        }
        OnRead(client, bytes);
        _StartReading(client);
        });
}
BasicServerC::BasicClientS& BasicServerC::ClientFactory() {
    return *Clients.emplace(Clients.begin(), new BasicClientS(AsioContext.get()))->get();
}
void BasicServerC::_AcceptConnection() {
    std::unique_ptr<int> k(new int);
    BasicClientS& client = ClientFactory();
    ConnectionsAcceptor.async_accept(client.Socket, [&](asio::error_code ec) {
        if (ec) {
            if (ec == asio::error::operation_aborted) {
                OutputMacro << "Server closed acceptor"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                return;
            } else {
                OutputMacro << "Unhandled error occured in acceptor" <<
                    ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                return;
            }
        }
        OutputMacro << "Connected! " << client.Socket.remote_endpoint().address().to_string()
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        OnConnect();
        _StartReading(client);
        _AcceptConnection();
        });

}
BasicServerC::BasicServerC(asio::io_context& asioContext, asio::ip::port_type port) :
    AsioContext(asioContext), ConnectionsAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    OutputMacro << "Server is up"
        << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    _AcceptConnection();
}
BasicServerC::~BasicServerC() {
    Shutdown();
}
void BasicServerC::_WriteToClient(BasicClientS& client, void const* arr, size_t lenInBytes) {
    if (lenInBytes != 0) {
        size_t bytesOffset = 0;
        asio::error_code ec;
        while ((bytesOffset +=
            client.Socket.write_some(asio::buffer((char*)arr + bytesOffset, lenInBytes - bytesOffset), ec)) != lenInBytes)
            if (ec) {
                if (ec == asio::error::operation_aborted) OutputMacro << "Canceled writing to socket" <<
                    ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                else OutputMacro << "Unhandled error occured while writing to socket" <<
                    ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
    }
}
void BasicServerC::Shutdown() {
    if (ConnectionsAcceptor.is_open()) {
        ConnectionsAcceptor.close();
        for (auto& client : Clients)
            if (client.get()->Socket.is_open()) {
                client.get()->Socket.shutdown(client.get()->Socket.shutdown_both);
                client.get()->Socket.close();
            }
        Clients.clear();
        OutputMacro << "Server shutdown"
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    }
}