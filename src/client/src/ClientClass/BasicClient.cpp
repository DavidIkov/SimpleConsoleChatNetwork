#include"BasicClient.hpp"

BasicClientC::~BasicClientC() {
    if (IsBasicClientDestructorLast) Mutex.lock();
    *InstanceIsDestructing = true;
    CV.notify_all();
    Mutex.unlock();
}
void BasicClientC::_StartReading_Async() {
    //no need for lock guard here since this function is called only when mutex is locked
    Socket.async_read_some(asio::buffer(SocketBuffer, sizeof(SocketBuffer)), [this](asio::error_code ec, size_t bytes) {
        std::unique_lock ul(Mutex);
        if (ec) {
            if (ec == asio::error::eof) {
                if (DisconnectEvent.Active) {
                    Socket.close(ec);
                    DisconnectEvent.ErrorHappened = (bool)ec;
                    Socket = asio::ip::tcp::socket(Context.get());
                    DisconnectEvent.ServerResponded = true;
                    CV.notify_all();
                } else {
                    Socket = asio::ip::tcp::socket(Context.get());
                    OnDisconnect(DisconnectReasonE::ServerDisconnected);
                }
            }
            else if (ec == asio::error::operation_aborted) {
                if (DisconnectEvent.Active) {
                    DisconnectEvent.Stopped = true;
                    CV.notify_all();
                }
            }
            else if (ec == asio::error::connection_reset) {
                if (DisconnectEvent.Active) {
                    Socket = asio::ip::tcp::socket(Context.get());
                    DisconnectEvent.Stopped = true;
                    CV.notify_all();
                }
                else {
                    Socket = asio::ip::tcp::socket(Context.get());
                    OnDisconnect(DisconnectReasonE::ServerResetedConnection);
                }
            }
            else {
                std::shared_ptr<bool> instDestructed = InstanceIsDestructing;
                _Disconnect(false);
                if (*instDestructed) ul.release();
            }
            return;
        }
        OnRead(bytes);
        _StartReading_Async();
        });
}
BasicClientC::BasicClientC(asio::io_context& context) :Context(context), Socket(context), InstanceIsDestructing(new bool(false)) {}
auto BasicClientC::Connect(asio::ip::tcp::endpoint ep) -> ConnectResultE {
    std::lock_guard lg(Mutex);
    if (_gIsConnected()) return ConnectResultE::SocketAlreadyConnected;
    else if (_gIsConnecting()) return ConnectResultE::SocketIsInProgressOfConnecting;
    else if (_gIsDisconnecting()) return ConnectResultE::SocketIsInternallyDisconnecting;
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
auto BasicClientC::_Disconnect(bool gracefull) -> DisconnectResultE {
    if (!_gIsConnected()) return DisconnectResultE::NotConnectedToAnything;
    else if (_gIsDisconnecting()) return DisconnectResultE::AlreadyDisconnecting;
    OnDisconnect(DisconnectReasonE::ClientDisconnected);
    DisconnectEvent.Active = true;
    DisconnectEvent.ServerResponded = false, DisconnectEvent.Stopped = false, DisconnectEvent.ErrorHappened = false;
    if (gracefull) {
        std::shared_ptr<bool> destructingInst = InstanceIsDestructing;
        std::thread fullDisconWaitingTh([&] {
            std::unique_lock ul(Mutex, std::defer_lock);
            CV.wait(ul, [&] { return *destructingInst || DisconnectEvent.Stopped || DisconnectEvent.ServerResponded; });
            ul.release();
            });
        asio::error_code ec;
        Socket.shutdown(Socket.shutdown_both, ec);
        if (ec) return DisconnectResultE::UnknownError;
        fullDisconWaitingTh.join();
        if (*destructingInst) return DisconnectResultE::OperationAborted;
        DisconnectEvent.Active = false;
        if (DisconnectEvent.ErrorHappened) return DisconnectResultE::UnknownErrorOnSocketClosureButSuccessfullDisconnect;
        else if (DisconnectEvent.Stopped) return DisconnectResultE::OperationAborted;
        return DisconnectResultE::NoErrors;
    }
    else {
        asio::error_code ec;
        Socket.close(ec);
        DisconnectEvent.Active = false;
        if (ec) return DisconnectResultE::UnknownErrorOnSocketClosureButSuccessfullDisconnect;
        return DisconnectResultE::NoErrors;
    }
}
auto BasicClientC::__Write(void const* arr, size_t lenInBytes) -> WriteResultE {
    if (!_gIsConnected()) return WriteResultE::NotConnectedToAnything;
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