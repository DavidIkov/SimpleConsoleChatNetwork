#pragma once
#include"AsioInclude.hpp"
#include<list>

class ServerC {
protected:
    std::reference_wrapper<asio::io_context> AsioContext;
    asio::ip::tcp::acceptor ConnectionsAcceptor;
public:
    struct ClientS {
        asio::ip::tcp::socket Socket;
        char ReadBuffer[64];
        inline ClientS(asio::io_context& context) :Socket(context) {};
    };
protected:
    //first client is not active, it is waiting for connection
    std::list<ClientS> ActiveClients;
private:
    void _StartReading(ClientS& client);
    void _AcceptConnection();
public:
    ServerC(asio::io_context& asioContext, asio::ip::port_type port);
    ServerC(const ServerC&) = delete;
    ServerC(ServerC&&) = default;
    virtual ~ServerC();
    ServerC& operator=(ServerC&&) = default;
    ServerC& operator=(const ServerC&) = delete;
protected:
    //connected client is always first in list
    inline virtual void OnConnect() { };
    inline virtual void OnDisconnect(ClientS& disconectedClient) {};
    inline virtual void OnRead(size_t bytesRead) {};
public:
    void WriteToSocket(asio::ip::tcp::socket& socketToWrite, const std::string_view& data);
    void Shutdown();
    inline std::list<ClientS> const& gClients() const { return ActiveClients; }
};