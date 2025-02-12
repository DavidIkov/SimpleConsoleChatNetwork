#include"BasicServer.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

void BasicServerC::_StartReading(ClientS& client) {
    client.Socket.async_read_some(asio::buffer(client.ReadBuffer, sizeof(client.ReadBuffer)), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                OutputMacro << "Client disconnected, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                ActiveClients.remove_if([&](auto& v)->bool {return &v.Socket == &client.Socket;});
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
                ActiveClients.remove_if([&](auto& v)->bool {return &v.Socket == &client.Socket;});
                return;
            }
            else {
                OutputMacro << "Unhandled error occured in socket, stopping reading " <<
                    ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                ActiveClients.remove_if([&](auto& v)->bool {return &v.Socket == &client.Socket;});
                return;
            }
        }
        OnRead(client, bytes);
        _StartReading(client);
        });
}
void BasicServerC::_AcceptConnection() {
    ClientS& client = ActiveClients.emplace_front(AsioContext.get());
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
template<> void BasicServerC::WriteToClient<std::string_view const&>(ClientS& client, const std::string_view& data) {
    if (!data.empty()) {
        size_t bytesOffset = 0;
        asio::error_code ec;
        while ((bytesOffset +=
            client.Socket.write_some(asio::buffer(data.data() + bytesOffset, data.size() - bytesOffset), ec)) != data.size())
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
        for (auto& client : ActiveClients)
            if (client.Socket.is_open()) {
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
            }
        ActiveClients.clear();
        OutputMacro << "Server shutdown"
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    }
}