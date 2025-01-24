#include"Client.hpp"
#include"ConsoleOutFormatting.hpp"
ClientC::ClientC(asio::io_context& context) :Context(context), Socket(context) { }
void ClientC::Connect(asio::ip::tcp::endpoint ep) {
    if (Socket.is_open()) { std::cout << PutBeforeLastString << "Cant connect socket, its already connected" << std::endl << RestoreCursorPos; return; }
    asio::error_code ec;
    Socket.connect(ep, ec);
    if (ec) {
        Socket = asio::ip::tcp::socket(Context.get());
        std::cout << PutBeforeLastString << "Failed to connecting to endpoint " << ep.address().to_string() << ':' << ep.port() << std::endl << RestoreCursorPos;
        return;
    }
    else
        std::cout << PutBeforeLastString << "Connected to server at " << ep.address().to_string() << ':' << ep.port() << std::endl << RestoreCursorPos;
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
}
