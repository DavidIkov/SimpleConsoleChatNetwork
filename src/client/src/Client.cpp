#include"Client.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

void ClientC::_StartReading_Async() {
    Socket.async_read_some(asio::buffer((char*)WriteBuffer.data(), WriteBuffer.size()), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                OutputMacro << "Server disconnected, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
            else if (ec == asio::error::operation_aborted) {
                OutputMacro << "Client closed socket, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
            else if (ec == asio::error::connection_reset) {
                OutputMacro << "Server reseted connection, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
            else {
                OutputMacro << "Unhandled error occured while reading "
                    << ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
            return;
        }
        OnRead(bytes);
        _StartReading_Async();
        });
}
ClientC::ClientC(asio::io_context& context, std::string_view writeBuffer) :Context(context),
Socket(context), WriteBuffer(writeBuffer) { }
void ClientC::Connect(asio::ip::tcp::endpoint ep) {
    if (Socket.is_open()) {
        OutputMacro << "Cant connect socket, its already connected"
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    }
    asio::error_code ec;
    Socket.connect(ep, ec);
    if (ec) {
        Socket = asio::ip::tcp::socket(Context.get());
        OutputMacro << "Failed to connecting to endpoint "
            << ep.address().to_string() << ':' << std::to_string(ep.port())
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return;
    }
    else {
        OutputMacro << "Connected to server at "
            << ep.address().to_string() << ':' << std::to_string(ep.port())
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        OnConnect();
        _StartReading_Async();
    }
}
void ClientC::Disconnect() {
    if (!Socket.is_open()) {
        OutputMacro << "Cant disconnect socket since it isnt connected to anything"
        << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return;
    }
    asio::error_code ec;
    Socket.shutdown(Socket.shutdown_both, ec);
    if (ec) {
        OutputMacro << "Failed to shutdown socket IO with an error "
            << ec.value() << ':' << ec.message()
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return;
    }
    Socket.close(ec);
    if (ec) {
        OutputMacro << "Failed to close socket with an error"
            << ec.value() << ':' << ec.message()
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return;
    }
    Socket = asio::ip::tcp::socket(Context.get());
    OutputMacro << "Disconnected from server"
        << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    OnDisconnect();
}
void ClientC::OnRead(size_t bytesRead) {
    for (size_t i = 0;i < bytesRead;i++) OutputMacro << WriteBuffer[i];
    OutputMacro << OutputtingProcPtr->EndLine;
}
void ClientC::Write(const std::string_view& data) {
    if (!data.empty())
        Socket.async_write_some(asio::buffer((char*)data.data(), data.size()), [&](asio::error_code ec, size_t bytesWritten) {
            if (ec) {
                OutputMacro << "Failed to write to socket with error "
                    << ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                return;
            }
            Write(std::string_view(data.data() + bytesWritten, data.size() - bytesWritten));
        });
}