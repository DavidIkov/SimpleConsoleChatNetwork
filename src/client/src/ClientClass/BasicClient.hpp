#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"

//todo this shouldnt be here, but it is required since DisconnectReasonsE is located here,
//maybe in future move the "special" local events to some other place where it would make sence
#include"NetworkEvents.hpp"

//basic client functionality
class BasicClientC {
private:
    std::reference_wrapper<asio::io_context> Context;
    asio::ip::tcp::socket Socket;
protected:
    char SocketBuffer[64];
private:
    void _StartReading_Async();
protected:
    mutable std::recursive_mutex ClientMutex;
private:
    bool ConnectedToServer = false;
public:
    BasicClientC(asio::io_context& context);
    virtual ~BasicClientC() = default;

    inline bool gIsConnected() const { std::lock_guard lg(ClientMutex); return ConnectedToServer; }
    inline bool gIsConnecting() const { std::lock_guard lg(ClientMutex); return !gIsConnected() && Socket.is_open(); }
    enum class ConnectResultE :unsigned char {
        TimedOut, AccessDenied, AddressIsAlreadyOccupied, SocketAlreadyConnected,
        ConnectionAbortedInMiddleWay, ServerIsNotListeningAtThisPort, ServerIsOffline,
        SocketIsInProgressOfConnecting, NoEthernetConnection, NoRouteToServer,
        AbortedByClient, UnknownError, NoErrors
    };
    ConnectResultE Connect(asio::ip::tcp::endpoint ep);
    enum class DisconnectResultE :unsigned char {
        NotConnectedToAnything, UnknownError, NoErrors
    };
    DisconnectResultE Disconnect();
protected:
    inline virtual void OnConnect() { std::lock_guard lg(ClientMutex); ConnectedToServer = true; };
    using DisconnectReasonE = NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::DisconnectedFromServer>::DisconnectReasonE;
    inline virtual void OnDisconnect(DisconnectReasonE) { std::lock_guard lg(ClientMutex); ConnectedToServer = false; };
    //by default just prints out everything received
    virtual void OnRead(size_t bytesRead) {}
public:
    enum class WriteResultE :unsigned int {
        NotConnectedToAnything, StoppedByClient, UnknownError, NoErrors,
    };
private:
    WriteResultE _Write(void const* arr, size_t lenInBytes);
public:
    template<typename T> WriteResultE Write(T const* arrPtr, size_t arrSize) { std::lock_guard lg(ClientMutex); return _Write(arrPtr, arrSize * sizeof(T)); }
    template<typename T> WriteResultE Write(T&& var) { std::lock_guard lg(ClientMutex); return Write(&var, 1); }
};