#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
#include"ThreadSafety.hpp"

//todo this shouldnt be here, but it is required since DisconnectReasonsE is located here,
//maybe in future move the "special" local events to some other place where it would make sence
#include"NetworkEvents.hpp"

//basic client functionality
//thread safe, calling functions after destruction is UB, calling functions(except destructor) that might happen
//after destruction(because of waiting for mutex ownership) is not UB
class BasicClientC: protected ThreadSafety_BaseC {
private:
    std::reference_wrapper<asio::io_context> Context;
    asio::ip::tcp::socket Socket;
    std::array<char, 64> ReadBuffer;
private:
    void _StartReading_Async();
private:
    //needed since there since there is no other way to detect that client is in process of connecting to server
    bool ConnectedToServer = false;
    struct {
        bool Active = false;
        bool ErrorHappened = false, ServerResponded = false, Stopped = false;
    } DisconnectEvent;
    inline bool _gIsDisconnecting() const { return DisconnectEvent.Active; }
public:
    BasicClientC(asio::io_context& context);
protected:
    bool IsBasicClientDestructorLast = true;
public:
    virtual ~BasicClientC();
protected:
    inline bool _gIsConnected() const { return ConnectedToServer; }
    inline bool _gIsConnecting() const { return !_gIsConnected() && Socket.is_open(); }
public:
    //be aware that this function returns false even if client is still connected to server
    //but doing "gracefull" disconnect
    inline bool gIsConnected() const { ThreadLockC TL(this); return TL && _gIsConnected(); }
    inline bool gIsConnecting() const { ThreadLockC TL(this); return TL && _gIsConnecting(); }
    enum class ConnectResultE :unsigned char {
        TimedOut, AccessDenied, AddressIsAlreadyOccupied, SocketAlreadyConnected,
        ConnectionAbortedInMiddleWay, ServerIsNotListeningAtThisPort, ServerIsOffline,
        SocketIsInProgressOfConnecting, NoEthernetConnection, NoRouteToServer,
        AbortedByClient, SocketIsInternallyDisconnecting, UnknownError, NoErrors
    };
    ConnectResultE Connect(asio::ip::tcp::endpoint ep);
    enum class DisconnectResultE :unsigned char {
        NotConnectedToAnything, AlreadyDisconnecting, Canceled,
        UnknownErrorOnSocketClosureButSuccessfullDisconnect, UnknownError, NoErrors
    };
private:
    DisconnectResultE _Disconnect(bool gracefull);
public:
    //gracefull disconnect includes that function will block calling thread until full disconnection
    //process is finished(aka gracefull disconnect)
    DisconnectResultE Disconnect(bool gracefull = true) {
        ThreadLockC TL(this);
        return _Disconnect(gracefull);
    }
private:
    struct{ void* Data = nullptr; void(*Callback)(void*) = nullptr; } OnConnectCallback;
public:
    inline void sOnConnectCallback(void* data, void(*callback)(void*)) {
        ThreadLockC TL(this); OnConnectCallback = { data, callback };
    }
    inline decltype(OnConnectCallback) const& gOnConnectCallback() const { ThreadLockC TL(this); return OnConnectCallback; }
protected:
    inline virtual void OnConnect() {
        if (OnConnectCallback.Callback) OnConnectCallback.Callback(OnConnectCallback.Data);
        ConnectedToServer = true;
    };

    using DisconnectReasonE = NetworkEventsNS::DisconnectReasonE;
private:
    struct{ void* Data = nullptr; void(*Callback)(void*, DisconnectReasonE) = nullptr; } OnDisconnectCallback;
public:
    inline void sOnDisconnectCallback(void* data, void(*callback)(void*, DisconnectReasonE)) {
        ThreadLockC TL(this); OnDisconnectCallback = { data, callback };
    }
    inline decltype(OnDisconnectCallback) const& gOnDisconnectCallback() const { ThreadLockC TL(this); return OnDisconnectCallback; }
protected:
    //to make things clear this event fires when Disconnect function is called, so it is not waiting for internal disconnection
    inline virtual void OnDisconnect(DisconnectReasonE reason) {
        if (OnDisconnectCallback.Callback) OnDisconnectCallback.Callback(OnDisconnectCallback.Data, reason);
        ConnectedToServer = false;
    };

private:
    struct{ void* Data = nullptr; void(*Callback)(void*, char const*, size_t) = nullptr; } OnReadCallback;
public:
    inline void sOnReadCallback(void* data, void(*callback)(void*, char const*, size_t)) {
        ThreadLockC TL(this); OnReadCallback = { data, callback };
    }
    inline decltype(OnReadCallback) const& gOnReadCallback() const { ThreadLockC TL(this); return OnReadCallback; }
protected:
    virtual void OnRead(char const* data, size_t len) {
        if (OnReadCallback.Callback) OnReadCallback.Callback(OnReadCallback.Data, data, len);
    }
public:
    enum class WriteResultE :unsigned int {
        NotConnectedToAnything, Canceled, UnknownError, NoErrors,
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