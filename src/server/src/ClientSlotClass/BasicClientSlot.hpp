#pragma once
#include"ThreadSafety.hpp"
#include"AsioInclude.hpp"
#include<array>

class BasicClientSlotC : protected ThreadSafety_BaseC {
private:
    asio::io_context* AsioContext;
    asio::ip::tcp::socket Socket;
    std::array<char, 64> ReadBuffer;
    struct {
        bool Active = false;
        bool ClientResponded = false, Stopped = false;
    } DisconnectEvent;
    inline bool gIsDisconnecting() const { return DisconnectEvent.Active; }
public:
    inline bool gIsConnected() const { ThreadLockC TL(this); return TL && Socket.is_open() && !gIsDisconnecting(); }

protected:
    bool IsBasicDestructorLast = true;
public:
    inline BasicClientSlotC(asio::io_context& context) :AsioContext(&context), Socket(context) {};
    inline virtual ~BasicClientSlotC() { if (IsBasicDestructorLast) ThreadSafety.LockThread(); }
    void ListenForConnection(asio::ip::tcp::acceptor& acceptor);
private:
    void _StartReading();

#pragma region OnRead
private:
    struct{ void* Data = nullptr; void(*Callback)(BasicClientSlotC*, void*, const char*, size_t) = nullptr; } OnReadCallback;
public:
    inline void sOnReadCallback(void* data, decltype(OnReadCallback.Callback) callback) {
        ThreadLockC TL(this); if(TL) OnReadCallback = { data, callback };
    }
    inline decltype(OnReadCallback) const& gOnReadCallback() const { ThreadLockC TL(this); return OnReadCallback; }
protected:
    inline virtual void OnRead(const char* data, size_t len) {
        if (OnReadCallback.Callback) OnReadCallback.Callback(this, OnReadCallback.Data, data, len);
    }
#pragma endregion OnRead

#pragma region OnConnect
private:
    struct{ void* Data = nullptr; void(*Callback)(BasicClientSlotC*, void*) = nullptr; } OnConnectCallback;
public:
    inline void sOnConnectCallback(void* data, decltype(OnConnectCallback.Callback) callback) {
        ThreadLockC TL(this); if(TL) OnConnectCallback = { data, callback };
    }
    inline decltype(OnConnectCallback) const& gOnConnectCallback() const { ThreadLockC TL(this); return OnConnectCallback; }
protected:
    inline virtual void OnConnect() {
        if (OnConnectCallback.Callback) OnConnectCallback.Callback(this, OnConnectCallback.Data);
    }
#pragma endregion OnConnect

#pragma region OnConnectionAcceptError
private:
    struct{ void* Data = nullptr; void(*Callback)(BasicClientSlotC*, void*) = nullptr; } OnConnectionAcceptErrorCallback;
public:
    inline void sOnConnectionAcceptErrorCallback(void* data, decltype(OnConnectionAcceptErrorCallback.Callback) callback) {
        ThreadLockC TL(this); if(TL) OnConnectionAcceptErrorCallback = { data, callback };
    }
    inline decltype(OnConnectionAcceptErrorCallback) const& gOnConnectionAcceptErrorCallback() const { ThreadLockC TL(this); return OnConnectionAcceptErrorCallback; }
protected:
    inline virtual void OnConnectionAcceptError() {
        if (OnConnectionAcceptErrorCallback.Callback) OnConnectionAcceptErrorCallback.Callback(this, OnConnectionAcceptErrorCallback.Data);
    }
#pragma endregion OnConnectionAcceptError

#pragma region OnDisconnect
public:
    enum class DisconnectReasonE :unsigned char {
        ServerShutdown, ServerDisconnected, ClientDisconnected, ClientResetedConnection, UnknownError,
    };
private:
    struct{ void* Data = nullptr; void(*Callback)(BasicClientSlotC*, void*, DisconnectReasonE) = nullptr; } OnDisconnectCallback;
public:
    inline void sOnDisconnectCallback(void* data, decltype(OnDisconnectCallback.Callback) callback) {
        ThreadLockC TL(this); if(TL) OnDisconnectCallback = { data, callback };
    }
    inline decltype(OnDisconnectCallback) const& gOnDisconnectCallback() const { ThreadLockC TL(this); return OnDisconnectCallback; }
protected:
    inline virtual void OnDisconnect(DisconnectReasonE reason) {
        if (OnDisconnectCallback.Callback) OnDisconnectCallback.Callback(this, OnDisconnectCallback.Data, reason);
    }
#pragma endregion OnDisconnect

public:
    enum class DisconnectResultE :unsigned char {
        NotConnectedToAnything, AlreadyDisconnecting, Canceled,
        UnknownErrorButSuccessfullDisconnect, UnknownError, NoErrors
    };
public:
    DisconnectResultE Disconnect(bool gracefull = true, DisconnectReasonE reason=DisconnectReasonE::ServerDisconnected);
public:
    enum class WriteResultE :unsigned char {
        ClientIsNotActive, Canceled, UknownError, NoErrors,
    };
private:
    WriteResultE _Write(void const* arr, size_t lenInBytes);
public:
    template<typename T> WriteResultE Write(T const* arrPtr, size_t arrSize) {
        ThreadLockC TL(this);
        if (!TL) return WriteResultE::Canceled;
        return _Write(arrPtr, arrSize * sizeof(T));
    }
    template<typename T> WriteResultE Write(T&& var) { return Write(&var, 1); }

    inline asio::ip::tcp::endpoint gConnectedEndpoint() const {
        ThreadLockC TL(this); if (!TL) return {};
        return gIsConnected() ? Socket.remote_endpoint() : asio::ip::tcp::endpoint{};
    }
};