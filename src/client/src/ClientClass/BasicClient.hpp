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
    mutable std::mutex Mutex;
    std::condition_variable CV;
private:
    bool ConnectedToServer = false;
    struct {
        bool Active = false;
        bool ErrorHappened = false, ServerResponded = false, Stopped = false;
    } DisconnectEvent;
    inline bool _gIsDisconnecting() const { return DisconnectEvent.Active; }
public:
    BasicClientC(asio::io_context& context);
protected:
    std::shared_ptr<bool> InstanceIsDestructing;//false
    bool IsBasicClientDestructorLast = true;
public:
    virtual ~BasicClientC();
protected:
    inline bool _gIsConnected() const { return ConnectedToServer; }
    inline bool _gIsConnecting() const { return !_gIsConnected() && Socket.is_open(); }
public:
    //be aware that this function returns false even if client is still connected to server
    //but doing "gracefull" disconnect
    inline bool gIsConnected() const { std::lock_guard lg(Mutex); return _gIsConnected(); }
    inline bool gIsConnecting() const { std::lock_guard lg(Mutex); return _gIsConnecting(); }
    enum class ConnectResultE :unsigned char {
        TimedOut, AccessDenied, AddressIsAlreadyOccupied, SocketAlreadyConnected,
        ConnectionAbortedInMiddleWay, ServerIsNotListeningAtThisPort, ServerIsOffline,
        SocketIsInProgressOfConnecting, NoEthernetConnection, NoRouteToServer,
        AbortedByClient, SocketIsInternallyDisconnecting, UnknownError, NoErrors
    };
    ConnectResultE Connect(asio::ip::tcp::endpoint ep);
    enum class DisconnectResultE :unsigned char {
        NotConnectedToAnything, AlreadyDisconnecting, OperationAborted,
        UnknownErrorOnSocketClosureButSuccessfullDisconnect, UnknownError, NoErrors
    };
private:
    DisconnectResultE _Disconnect(bool gracefull);
public:
    //gracefull disconnect includes that function will block calling thread until full disconnection
    //process is finished(aka gracefull disconnect)
    DisconnectResultE Disconnect(bool gracefull = true) {
        std::unique_lock ul(Mutex);
        std::shared_ptr<bool> destructingInst = InstanceIsDestructing;
        DisconnectResultE res = _Disconnect(gracefull);
        if (*destructingInst) ul.release();
        return res;
    }
protected:
    inline virtual void OnConnect() { ConnectedToServer = true; };
    using DisconnectReasonE = NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::DisconnectedFromServer>::DisconnectReasonE;
    //to make things clear this event fires when Disconnect function is called, so it is not waiting for internal disconnection
    inline virtual void OnDisconnect(DisconnectReasonE) { ConnectedToServer = false; };
    virtual void OnRead(size_t bytesRead) {}
public:
    enum class WriteResultE :unsigned int {
        NotConnectedToAnything, StoppedByClient, UnknownError, NoErrors,
    };
private:
    WriteResultE __Write(void const* arr, size_t lenInBytes);
protected:
    template<typename T> WriteResultE _Write(T const* arrPtr, size_t arrSize) { return __Write(arrPtr, arrSize * sizeof(T)); }
    template<typename T> WriteResultE _Write(T&& var) { return _Write(&var, 1); }
public:
    template<typename T> WriteResultE Write(T const* arrPtr, size_t arrSize) {
        std::lock_guard lg(ProcessMutex);
        return _Write(arrPtr, arrSize);
    }
    template<typename T> WriteResultE Write(T&& var) { return Write(&var, 1); }
};