#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
#include<list>

class BasicServerC {
protected:
    std::reference_wrapper<asio::io_context> AsioContext;
    asio::ip::tcp::acceptor ConnectionsAcceptor;
    ConsoleManagerNS::OutputNS::OutputtingProcessC* OutputtingProcPtr = nullptr;
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

    inline void sOutputtingProcPtr(ConsoleManagerNS::OutputNS::OutputtingProcessC* outputtingProcPtr)
        { OutputtingProcPtr = outputtingProcPtr; }

    //used to call sOutputtingProcPtr automatically, enabling it on construction and disabling on destruction
    class OutputtingProcPtrWrapperC {
        BasicServerC& Server;
    public:
        inline OutputtingProcPtrWrapperC(BasicServerC& server, ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) :
            Server(server) {
            Server.sOutputtingProcPtr(&outProc);
        }
        inline ~OutputtingProcPtrWrapperC() { Server.sOutputtingProcPtr(nullptr); }
    };
    
    BasicServerC(asio::io_context& asioContext, asio::ip::port_type port);
    BasicServerC(const BasicServerC&) = delete;
    BasicServerC(BasicServerC&&) = default;
    virtual ~BasicServerC();
    BasicServerC& operator=(BasicServerC&&) = default;
    BasicServerC& operator=(const BasicServerC&) = delete;
protected:
    //connected client is always first in list
    inline virtual void OnConnect() { };
    inline virtual void OnDisconnect(ClientS& disconectedClient) {};
    inline virtual void OnRead(ClientS& client, size_t bytesRead) {};
public:
    template<typename T> void WriteToClient(ClientS& client, T&& var) { Write(client, std::string(&var, sizeof(var))); }
    template<> void WriteToClient<std::string_view const&>(ClientS& client, const std::string_view& data);
    void Shutdown();
    inline std::list<ClientS> const& gClients() const { return ActiveClients; }
};