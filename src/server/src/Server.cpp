#include"Server.hpp"
#include"iostream"

void ServerC::_StartReading(asio::ip::tcp::socket& socket) {
    socket.async_read_some(asio::buffer(ReadBuffer.Data, ReadBuffer.Size), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                std::cout << "Client disconnected, removing socket" << std::endl;
                socket.shutdown(socket.shutdown_both);
                socket.close();
                ActiveSockets.remove_if([&](auto& v)->bool {return &v == &socket;});
                return;
            }
            else if (ec == asio::error::operation_aborted) {
                std::cout << "Server closed socket, stopping reading" << std::endl;
                return;
            }
            else if (ec == asio::error::connection_reset) {
                std::cout << "Client reseted connection, removing socket" << std::endl;
                socket.shutdown(socket.shutdown_both);
                socket.close();
                ActiveSockets.remove_if([&](auto& v)->bool {return &v == &socket;});
                return;
            }
            else {
                std::cout << "Unhandled error occured while reading data from server's socket " <<
                    ec.value() << ' ' << ec.message() << std::endl; return;
            }

        };
        OnRead(bytes);
        std::cout << "Server received message containing " << bytes << " bytes: ";
        for (size_t i = 0; i < bytes; i++)
            std::cout << ReadBuffer.Data[i];
        std::cout << std::endl;

        socket.async_write_some(asio::buffer(ReadBuffer.Data, bytes), [](asio::error_code ec, size_t bytes){std::cout << "Resended all of it\n";});
        using namespace std::chrono_literals; std::this_thread::sleep_for(100ms);
        _StartReading(socket);
        });
}
void ServerC::_AcceptConnection() {
    asio::ip::tcp::socket& socket = ActiveSockets.emplace_front(*AsioContext);
    ConnectionsAcceptor.async_accept(socket, [&](asio::error_code err) {
        if (err) {
            if (err == asio::error::operation_aborted) {
                std::cout << "Server closed acceptor" << std::endl;
                return;
            }
            else {
                std::cout << "Unhandled error occured while trying to accept connection " <<
                    err.value() << ' ' << err.message() << std::endl;
                return;
            }
        }
        std::cout << "Connected! " << socket.remote_endpoint().address().to_string() << std::endl;
        OnConnect();
        _StartReading(socket);
        _AcceptConnection();
        });

}
ServerC::ServerC(asio::io_context& asioContext, asio::ip::port_type port, ReadBufferS readBuffer) :
    AsioContext(&asioContext), ConnectionsAcceptor(asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    ReadBuffer(readBuffer) {
    std::cout << "Server is up" << std::endl;
    _AcceptConnection();
}
ServerC::~ServerC() {
    Shutdown();
}
void ServerC::WriteToSocket(asio::ip::tcp::socket& socketToWrite, const ReadBufferS& data) {
    socketToWrite.async_write_some(asio::buffer(data.Data, data.Size), [](asio::error_code,size_t){});
}
void ServerC::Shutdown() {
    if (ConnectionsAcceptor.is_open()) {
        ConnectionsAcceptor.close();
        for (auto& socket : ActiveSockets)
            if (socket.is_open()) {
                socket.shutdown(socket.shutdown_both);
                socket.close();
            }
        ActiveSockets.clear();
        std::cout << "Server shutdown" << std::endl;
    }
}