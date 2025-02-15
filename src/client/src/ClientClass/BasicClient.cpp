#include"BasicClient.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

void BasicClientC::_StartReading_Async() {
    Socket.async_read_some(asio::buffer(SocketBuffer, sizeof(SocketBuffer)), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            //todo dosent disconnect if server reset connection
            if (ec == asio::error::eof) {
                OutputMacro << "Server disconnected, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                Socket = asio::ip::tcp::socket(Context.get());
                OnDisconnect();
            }
            else if (ec == asio::error::operation_aborted) {
                OutputMacro << "Client closed socket, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
            else if (ec == asio::error::connection_reset) {
                OutputMacro << "Server reseted connection, stopping reading"
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                Socket = asio::ip::tcp::socket(Context.get());
                OnDisconnect();
            }
            else {
                OutputMacro << "Unhandled error occured while reading "
                    << ec.value() << ' ' << ec.message()
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                Disconnect();
            }
            return;
        }
        OnRead(bytes);
        _StartReading_Async();
        });
}
BasicClientC::BasicClientC(asio::io_context& context) :Context(context), Socket(context) { }
void BasicClientC::Connect(asio::ip::tcp::endpoint ep) {
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
void BasicClientC::Disconnect() {
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
void BasicClientC::OnRead(size_t bytesRead) {
    for (size_t i = 0;i < bytesRead;i++) OutputMacro << SocketBuffer[i];
    OutputMacro << OutputtingProcPtr->EndLine;
}
void BasicClientC::_Write(void const* arr, size_t lenInBytes) {
    if (lenInBytes != 0) {
        size_t bytesOffset = 0;
        asio::error_code ec;
        while ((bytesOffset +=
            Socket.write_some(asio::buffer((char*)arr + bytesOffset, lenInBytes - bytesOffset), ec)) != lenInBytes)
            if (ec) {
                if (ec == asio::error::operation_aborted) OutputMacro << "Canceled writing to socket" <<
                    ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                else OutputMacro << "Unhandled error occured while writing to socket" <<
                    ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
    }
}