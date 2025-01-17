#pragma once
#include"Client.hpp"
#include<list>

class ServerC {
protected:
    asio::error_code LastErrorCode;
    asio::io_context* AsioContext;
    asio::ip::tcp::acceptor ConnectionsAcceptor;
    //first socket is not active, it is waiting for connection
    std::list<asio::ip::tcp::socket> ActiveSockets;
public:
    struct ReadBufferS{ char* Data; size_t Size; };
protected:
    ReadBufferS ReadBuffer;
private:
    void _StartReading(asio::ip::tcp::socket& socket);
    void _AcceptConnection();
public:
    ServerC(asio::io_context& asioContext, asio::ip::port_type port, ReadBufferS readBuffer);
    ServerC(const ServerC&) = delete;
    ServerC(ServerC&&) = default;
    virtual ~ServerC();
    ServerC& operator=(ServerC&&) = default;
    ServerC& operator=(const ServerC&) = delete;
    inline asio::error_code gLastErrorCode() const noexcept { return LastErrorCode; }
protected:
    //connected socket is always first in list
    inline virtual void OnConnect() { };
    inline virtual void OnDisconnect(asio::ip::tcp::socket& disconectedSocket) {};
    inline virtual void OnRead(size_t bytesRead) {};
public:
    void WriteToSocket(asio::ip::tcp::socket& socketToWrite, const ReadBufferS& data);
    void Shutdown();
};