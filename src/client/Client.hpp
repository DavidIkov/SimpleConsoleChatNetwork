#pragma once
#include"AsioInclude.hpp"
class ClientC {
protected:
    std::reference_wrapper<asio::io_context> Context;
    asio::ip::tcp::socket Socket;
public:
    ClientC(asio::io_context& context);
    virtual ~ClientC() = default;
    virtual void Connect(asio::ip::tcp::endpoint ep);
    virtual void Disconnect();
};