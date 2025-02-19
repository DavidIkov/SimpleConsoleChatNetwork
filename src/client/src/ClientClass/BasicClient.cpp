#include"BasicClient.hpp"

void BasicClientC::_StartReading_Async() {
    Socket.async_read_some(asio::buffer(SocketBuffer, sizeof(SocketBuffer)), [&](asio::error_code ec, size_t bytes) {
        std::lock_guard lg(ClientMutex);
        if (ec) {
            //todo dosent disconnect if server reset connection
            if (ec == asio::error::eof) {
                Socket = asio::ip::tcp::socket(Context.get());
                OnDisconnect(DisconnectReasonE::ServerDisconnected);
            }
            else if (ec == asio::error::operation_aborted) {
                //dont do anything since this is error is made by some other function so it handles everything
            }
            else if (ec == asio::error::connection_reset) {
                Socket = asio::ip::tcp::socket(Context.get());
                OnDisconnect(DisconnectReasonE::ServerResetedConnection);
            }
            else {
                //todo somehow handle "Unknown" error
                Disconnect();
            }
            return;
        }
        OnRead(bytes);
        _StartReading_Async();
        });
}
BasicClientC::BasicClientC(asio::io_context& context) :Context(context), Socket(context) { }
auto BasicClientC::Connect(asio::ip::tcp::endpoint ep) -> ConnectResultE {
    if (gIsConnected()) return ConnectResultE::SocketAlreadyConnected;
    else if (Socket.is_open()) return ConnectResultE::SocketIsInProgressOfConnecting;
    std::lock_guard lg(ClientMutex);
    asio::error_code ec;
    Socket.connect(ep, ec);
    if (ec) {
        Socket = asio::ip::tcp::socket(Context.get());
        if (ec == asio::error::timed_out) return ConnectResultE::TimedOut;
        if (ec == asio::error::access_denied) return ConnectResultE::AccessDenied;
        if (ec == asio::error::address_in_use) return ConnectResultE::AddressIsAlreadyOccupied;
        if (ec == asio::error::already_connected) return ConnectResultE::SocketAlreadyConnected;
        if (ec == asio::error::connection_aborted) return ConnectResultE::ConnectionAbortedInMiddleWay;
        if (ec == asio::error::connection_refused) return ConnectResultE::ServerIsNotListeningAtThisPort;
        if (ec == asio::error::host_unreachable) return ConnectResultE::ServerIsOffline;
        if (ec == asio::error::in_progress) return ConnectResultE::SocketIsInProgressOfConnecting;
        if (ec == asio::error::network_down) return ConnectResultE::NoEthernetConnection;
        if (ec == asio::error::network_unreachable) return ConnectResultE::NoRouteToServer;
        if (ec == asio::error::operation_aborted) return ConnectResultE::AbortedByClient;
        return ConnectResultE::UnknownError;
    }
    else {
        OnConnect();
        _StartReading_Async();
        return ConnectResultE::NoErrors;
    }
}
auto BasicClientC::Disconnect() -> DisconnectResultE {
    if (!gIsConnected()) return DisconnectResultE::NotConnectedToAnything;
    std::lock_guard lg(ClientMutex);
    asio::error_code ec;
    Socket.shutdown(Socket.shutdown_both, ec);
    if (ec) return DisconnectResultE::UnknownError;
    Socket.close(ec);
    if (ec) return DisconnectResultE::UnknownError;
    Socket = asio::ip::tcp::socket(Context.get());
    OnDisconnect(DisconnectReasonE::ClientDisconnected);
    return DisconnectResultE::NoErrors;
}
auto BasicClientC::_Write(void const* arr, size_t lenInBytes) -> WriteResultE {
    if (!gIsConnected()) return WriteResultE::NotConnectedToAnything;
    std::lock_guard lg(ClientMutex);
    if (lenInBytes != 0) {
        size_t bytesOffset = 0;
        asio::error_code ec;
        while ((bytesOffset +=
            Socket.write_some(asio::buffer((char*)arr + bytesOffset, lenInBytes - bytesOffset), ec)) != lenInBytes)
            if (ec) {
                if (ec == asio::error::operation_aborted) return WriteResultE::StoppedByClient;
                else return WriteResultE::UnknownError;
            }
    }
    return WriteResultE::NoErrors;
}