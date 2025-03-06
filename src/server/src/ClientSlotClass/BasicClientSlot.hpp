#pragma once
#include"ThreadSafety.hpp"
#include"AsioInclude.hpp"
#include<array>

//todo same as with BasicClient.hpp
#include"NetworkEvents.hpp"

class BasicClientSlotC : protected ThreadSafety_BaseC {
private:
    asio::ip::tcp::socket Socket;
    std::array<char, 64> ReadBuffer;
    struct {
        bool Active = false;
        bool ErrorHappened = false, ClientResponded = false, Stopped = false;
    } DisconnectEvent;
    inline bool _gIsDisconnecting() const { return DisconnectEvent.Active; }
protected:
    inline bool _gIsConnected() const { return Socket.is_open() && !_gIsDisconnecting(); }
public:
    inline bool gIsConnected() const { ThreadLockC TL(this); return _gIsConnected(); }

protected:
    bool IsBasicDestructorLast = true;
public:
    inline BasicClientSlotC(asio::io_context& context) :Socket(context) {};
    inline virtual ~BasicClientSlotC() { if (IsBasicDestructorLast) ThreadSafety.Data->Mutex.lock(); }
    void ListenForConnection(asio::ip::tcp::acceptor& acceptor);
private:
    void _StartReading();

private:
    struct{ void* Data = nullptr; void(*Callback)(void*, const char*, size_t) = nullptr; } OnReadCallback;
public:
    inline void sOnReadCallback(void* data, void(*callback)(void*, const char*, size_t)) {
        ThreadLockC TL(this); OnReadCallback = { data, callback };
    }
    inline decltype(OnReadCallback) const& gOnReadCallback() const { ThreadLockC TL(this); return OnReadCallback; }
protected:
    inline virtual void OnRead(const char* data, size_t len) {
        if (OnReadCallback.Callback) OnReadCallback.Callback(OnReadCallback.Data, data, len);
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
    }

public:
    using DisconnectReasonE = NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientDisconnected>::DisconnectReasonE;
private:
    struct{ void* Data = nullptr; void(*Callback)(void*, DisconnectReasonE) = nullptr; } OnDisconnectCallback;
public:
    inline void sOnDisconnectCallback(void* data, void(*callback)(void*, DisconnectReasonE)) {
        ThreadLockC TL(this); OnDisconnectCallback = { data, callback };
    }
    inline decltype(OnDisconnectCallback) const& gOnDisconnectCallback() const { ThreadLockC TL(this); return OnDisconnectCallback; }
protected:
    inline virtual void OnDisconnect(DisconnectReasonE) {
        if (OnDisconnectCallback.Callback) OnDisconnectCallback.Callback(OnDisconnectCallback.Data);
    }

public:
    enum class DisconnectResultE :unsigned char {
        NotConnectedToAnything, AlreadyDisconnecting, Canceled,
        UnknownErrorOnSocketClosureButSuccessfullDisconnect, UnknownError, NoErrors
    };
private:
    DisconnectResultE _Disconnect(bool gracefull);
public:
    DisconnectResultE Disconnect(bool gracefull = true){
        ThreadLockC TL(this);
        return _Disconnect(gracefull);
    }

public:
    enum class WriteResultE :unsigned char {
        ClientIsNotActive, StoppedByServer, UknownError, NoErrors,
    };
private:
    WriteResultE _Write(void const* arr, size_t lenInBytes);
public:
    template<typename T> WriteResultE Write(T const* arrPtr, size_t arrSize) {
        ThreadLockC TL(&Client);
        return _Write(arrPtr, arrSize * sizeof(T));
    }
    template<typename T> WriteResultE Write(T&& var) { return Write(&var, 1); }

};