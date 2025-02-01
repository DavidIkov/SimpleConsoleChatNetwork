#include"Server.hpp"
#include"ConsoleOutFormatting.hpp"

void ServerC::_StartReading(ClientS& client) {
    client.Socket.async_read_some(asio::buffer(client.ReadBuffer, sizeof(client.ReadBuffer)), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                std::cout << PutBeforeLastString << "Client disconnected, stopping reading" << std::endl << RestoreCursorPos;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                ActiveClients.remove_if([&](auto& v)->bool {return &v.Socket == &client.Socket;});
                return;
            }
            else if (ec == asio::error::operation_aborted) {
                std::cout << PutBeforeLastString << "Server forcefully closed socket, stopping reading" << std::endl << RestoreCursorPos;
                //no need for shutdown and close and removing of socket since this code will happen only on shutdown, 
                //so its done forcefully and i can rely on caller to do allat
                OnDisconnect(client);
                return;
            }
            else if (ec == asio::error::connection_reset) {
                std::cout << PutBeforeLastString << "Client reseted connection, stopping reading" << std::endl << RestoreCursorPos;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                ActiveClients.remove_if([&](auto& v)->bool {return &v.Socket == &client.Socket;});
                return;
            }
            else {
                std::cout << PutBeforeLastString << "Unhandled error occured in socket, stopping reading " <<
                    ec.value() << ' ' << ec.message() << std::endl << RestoreCursorPos;
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
                OnDisconnect(client);
                ActiveClients.remove_if([&](auto& v)->bool {return &v.Socket == &client.Socket;});
                return;
            }
        }
        OnRead(bytes);
        _StartReading(client);
        });
}
void ServerC::_AcceptConnection() {
    ClientS& client = ActiveClients.emplace_front(AsioContext.get());
    ConnectionsAcceptor.async_accept(client.Socket, [&](asio::error_code ec) {
        if (ec) {
            if (ec == asio::error::operation_aborted) {
                std::cout << PutBeforeLastString << "Server closed socket, closing acceptor" << std::endl << RestoreCursorPos;
                return;
            } else {
                std::cout << PutBeforeLastString << "Unhandled error occured in socket, closing acceptor " <<
                    ec.value() << ' ' << ec.message() << std::endl << RestoreCursorPos;
                return;
            }
        }
        std::cout << PutBeforeLastString << "Connected! " << client.Socket.remote_endpoint().address().to_string() << std::endl << RestoreCursorPos;
        OnConnect();
        _StartReading(client);
        _AcceptConnection();
        });

}
ServerC::ServerC(asio::io_context& asioContext, asio::ip::port_type port) :
    AsioContext(asioContext), ConnectionsAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    std::cout << PutBeforeLastString << "Server is up" << std::endl << RestoreCursorPos;
    _AcceptConnection();
}
ServerC::~ServerC() {
    Shutdown();
}
void ServerC::WriteToSocket(asio::ip::tcp::socket& socketToWrite, const std::string_view& data) {
    if (!data.empty())
        socketToWrite.async_write_some(asio::buffer((char*)data.data(), data.size()), [&](asio::error_code ec, size_t bytesWritten) {
            if (ec) return;
            WriteToSocket(socketToWrite, std::string_view(data.data() + bytesWritten, data.size() - bytesWritten));
            });
}
void ServerC::Shutdown() {
    if (ConnectionsAcceptor.is_open()) {
        ConnectionsAcceptor.close();
        for (auto& client : ActiveClients)
            if (client.Socket.is_open()) {
                client.Socket.shutdown(client.Socket.shutdown_both);
                client.Socket.close();
            }
        ActiveClients.clear();
        std::cout << PutBeforeLastString << "Server shutdown" << std::endl << RestoreCursorPos;
    }
}