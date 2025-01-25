#include"Client.hpp"
#include"ConsoleOutFormatting.hpp"
void ClientC::_StartReading_Async() {
    Socket.async_read_some(asio::buffer((char*)WriteBuffer.data(), WriteBuffer.size()), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) { std::cout << PutBeforeLastString << "Server disconnected, stopping reading" << std::endl << RestoreCursorPos; return; }
            else if (ec == asio::error::operation_aborted) { std::cout << PutBeforeLastString << "Client closed socket, stopping reading" << std::endl << RestoreCursorPos; return; }
            else if (ec == asio::error::connection_reset) { std::cout << PutBeforeLastString << "Server reseted connection, stopping reading" << std::endl << RestoreCursorPos; return; }
            else { std::cout << PutBeforeLastString << "Unhandled error occured while reading " << ec.value() << ' ' << ec.message() << std::endl << RestoreCursorPos; return; }
        }
        OnRead(bytes);
        _StartReading_Async();
        });
}
ClientC::ClientC(asio::io_context& context, std::string_view writeBuffer) :Context(context),
Socket(context), WriteBuffer(writeBuffer) { }
void ClientC::Connect(asio::ip::tcp::endpoint ep) {
    if (Socket.is_open()) { std::cout << PutBeforeLastString << "Cant connect socket, its already connected" << std::endl << RestoreCursorPos; return; }
    asio::error_code ec;
    Socket.connect(ep, ec);
    if (ec) {
        Socket = asio::ip::tcp::socket(Context.get());
        std::cout << PutBeforeLastString << "Failed to connecting to endpoint " << ep.address().to_string() << ':' << ep.port() << std::endl << RestoreCursorPos;
        return;
    }
    else {
        std::cout << PutBeforeLastString << "Connected to server at " << ep.address().to_string() << ':' << ep.port() << std::endl << RestoreCursorPos;
        OnConnect();
        _StartReading_Async();
    }
}
void ClientC::Disconnect() {
    if (!Socket.is_open()) {
        std::cout << PutBeforeLastString << "Cant disconnect socket since it isnt connected to anything" << std::endl << RestoreCursorPos;
        return;
    }
    asio::error_code ec;
    Socket.shutdown(Socket.shutdown_both, ec);
    if (ec) {
        std::cout << PutBeforeLastString << "Failed to shutdown socket IO with an error " << ec.value() << ':' << ec.message() << std::endl << RestoreCursorPos;
        return;
    }
    Socket.close(ec);
    if (ec) {
        std::cout << PutBeforeLastString << "Failed to close socket with an error" << ec.value() << ':' << ec.message() << std::endl << RestoreCursorPos;
        return;
    }
    Socket = asio::ip::tcp::socket(Context.get());
    std::cout << PutBeforeLastString << "Disconnected from server" << std::endl << RestoreCursorPos;
    OnDisconnect();
}
void ClientC::OnRead(size_t bytesRead) {
    for (size_t i = 0;i < bytesRead;i++) std::cout << WriteBuffer[i];
    std::cout << std::endl;
}
void ClientC::Write(const std::string_view& data) {
    if (!data.empty())
        Socket.async_write_some(asio::buffer((char*)data.data(), data.size()), [&](asio::error_code ec, size_t bytesWritten) {
            if (ec) {
                std::cout << PutBeforeLastString << "Failed to write to socket with error " << ec.value() << ' ' << ec.message()
                    << std::endl << RestoreCursorPos; return;
            }
            Write(std::string_view(data.data() + bytesWritten, data.size() - bytesWritten));
        });
}