#include"Client.hpp"
#include"ConsoleManager.hpp"
void ClientC::_StartReading_Async() {
    Socket.async_read_some(asio::buffer((char*)WriteBuffer.data(), WriteBuffer.size()), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof)
                ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Server disconnected, stopping reading"
                << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
            else if (ec == asio::error::operation_aborted)
                ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Client closed socket, stopping reading"
                << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
            else if (ec == asio::error::connection_reset)
                ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Server reseted connection, stopping reading"
                << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
            else ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Unhandled error occured while reading "
                << ec.value() << ' ' << ec.message()
                << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
                return;
        }
        OnRead(bytes);
        _StartReading_Async();
        });
}
ClientC::ClientC(asio::io_context& context, std::string_view writeBuffer) :Context(context),
Socket(context), WriteBuffer(writeBuffer) { }
void ClientC::Connect(asio::ip::tcp::endpoint ep) {
    if (Socket.is_open()) ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Cant connect socket, its already connected"
        << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
    asio::error_code ec;
    Socket.connect(ep, ec);
    if (ec) {
        Socket = asio::ip::tcp::socket(Context.get());
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Failed to connecting to endpoint "
            << ep.address().to_string() << ':' << ep.port()
            << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
        return;
    }
    else {
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Connected to server at "
            << ep.address().to_string() << ':' << ep.port()
            << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
        OnConnect();
        _StartReading_Async();
    }
}
void ClientC::Disconnect() {
    if (!Socket.is_open()) {
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Cant disconnect socket since it isnt connected to anything"
        << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
        return;
    }
    asio::error_code ec;
    Socket.shutdown(Socket.shutdown_both, ec);
    if (ec) {
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Failed to shutdown socket IO with an error "
            << ec.value() << ':' << ec.message()
            << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
        return;
    }
    Socket.close(ec);
    if (ec) {
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Failed to close socket with an error"
            << ec.value() << ':' << ec.message()
            << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
        return;
    }
    Socket = asio::ip::tcp::socket(Context.get());
    ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Disconnected from server"
        << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
    OnDisconnect();
}
void ClientC::OnRead(size_t bytesRead) {
    ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC outProc;//todo add functionality to end output
    for (size_t i = 0;i < bytesRead;i++) outProc << WriteBuffer[i];
    outProc << outProc.FlushOutput;
}
void ClientC::Write(const std::string_view& data) {
    if (!data.empty())
        Socket.async_write_some(asio::buffer((char*)data.data(), data.size()), [&](asio::error_code ec, size_t bytesWritten) {
            if (ec) {
                ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC() << "Failed to write to socket with error "
                    << ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC::FlushOutput;
                return;
            }
            Write(std::string_view(data.data() + bytesWritten, data.size() - bytesWritten));
        });
}