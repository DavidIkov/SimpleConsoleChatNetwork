#pragma once
#include"AsioInclude.hpp"
class ClientC {
protected:
    std::reference_wrapper<asio::io_context> Context;
    asio::ip::tcp::socket Socket;
    std::string_view WriteBuffer;
    void _StartReading_Async();
public:
    ClientC(asio::io_context& context, std::string_view writeBuffer);
    virtual ~ClientC() = default;
    virtual void Connect(asio::ip::tcp::endpoint ep);
    virtual void Disconnect();
protected:
    inline virtual void OnConnect() {};
    inline virtual void OnDisconnect() {};
    virtual void OnRead(size_t bytesRead);
public:
    void Write(const std::string_view& data);
};