#include"BasicClientSlot.hpp"

void BasicClientSlotC::ListenForConnection(asio::ip::tcp::acceptor& acceptor) {
    acceptor.async_accept(Socket, [&](asio::error_code ec) {
        ThreadLockC TL(this);
        if (ec) {
            if (ec == asio::error::operation_aborted) {
                OnAcceptConnectionError(OnAcceptConnectionErrorE::ServerClosedAcceptor);
                return;
            } else {
                OnAcceptConnectionError(OnAcceptConnectionErrorE::UnknownError);
                return;
            }
        }
        OnConnect();
        _StartReading();
        });

}
auto BasicClientSlotC::_Disconnect(bool gracefull) -> DisconnectResultE {
    if (!_gIsConnected()) return DisconnectResultE::NotConnectedToAnything;
    else if (_gIsDisconnecting()) return DisconnectResultE::AlreadyDisconnecting;
    OnDisconnect(DisconnectReasonE::ServerDisconnected);
    DisconnectEvent.Active = true;
    DisconnectEvent.ClientResponded = false, DisconnectEvent.Stopped = false, DisconnectEvent.ErrorHappened = false;
    if (gracefull) {
        if (!*ThreadSafety.LastActive) return DisconnectResultE::Canceled;
        std::thread fullDisconWaitingTh([&] {
            ThreadSafety.LastActive->Wait([&] { return !*ThreadSafety.LastActive || DisconnectEvent.Stopped || DisconnectEvent.ClientResponded; });
            });
        asio::error_code ec;
        Socket.shutdown(Socket.shutdown_both, ec);
        if (ec) return DisconnectResultE::UnknownError;
        fullDisconWaitingTh.join();
        if (!*ThreadSafety.LastActive) return DisconnectResultE::Canceled;
        DisconnectEvent.Active = false;
        if (DisconnectEvent.ErrorHappened) return DisconnectResultE::UnknownErrorOnSocketClosureButSuccessfullDisconnect;
        else if (DisconnectEvent.Stopped) return DisconnectResultE::Canceled;
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
void BasicClientSlotC::_StartReading() {
    Socket.async_read_some(asio::buffer(ReadBuffer, sizeof(ReadBuffer)), [&](asio::error_code ec, size_t bytes) {
        ThreadLockC TL(this);
        if (ec) {
            if (ec == asio::error::eof) {
                if (DisconnectEvent.Active) {
                    Socket.close(ec);
                    DisconnectEvent.ErrorHappened = (bool)ec;
                    DisconnectEvent.ClientResponded = true;
                    TL->CV.notify_all();
                }
                else {
                    OnDisconnect(DisconnectReasonE::ServerDisconnected);
                }
            }
            else if (ec == asio::error::operation_aborted) {
                if (DisconnectEvent.Active) {
                    DisconnectEvent.Stopped = true;
                    TL->CV.notify_all();
                }
            }
            else if (ec == asio::error::connection_reset) {
                if (DisconnectEvent.Active) {
                    DisconnectEvent.Stopped = true;
                    TL->CV.notify_all();
                }
                else {
                    OnDisconnect(DisconnectReasonE::ClientResetedConnection);
                }
            }
            else {
                if (DisconnectEvent.Active) {
                    DisconnectEvent.Stopped = true;
                    TL->CV.notify_all();
                }
                else {
                    OnDisconnect(DisconnectReasonE::UnknownError);
                }
            }
            return;
        }
        OnRead(ReadBuffer.data(), bytes);
        _StartReading();
        });
}
auto BasicClientSlotC::_Write(void const* arr, size_t lenInBytes) -> WriteResultE {
    if (!gIsConnected()) return WriteResultE::ClientIsNotActive;
    if (lenInBytes != 0) {
        size_t bytesOffset = 0;
        asio::error_code ec;
        while ((bytesOffset +=
            Socket.write_some(asio::buffer((char*)arr + bytesOffset, lenInBytes - bytesOffset), ec)) != lenInBytes)
            if (ec) {
                if (ec == asio::error::operation_aborted) return WriteResultE::StoppedByServer;
                else return WriteResultE::UknownError;
            }
    }
    return WriteResultE::NoErrors;
}