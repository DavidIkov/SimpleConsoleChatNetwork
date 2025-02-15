#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
#include<vector>
class BasicServerC {
public:
    struct BasicClientS {
        asio::ip::tcp::socket Socket;
        char ReadBuffer[64];
        inline BasicClientS(asio::io_context& context) :Socket(context) {};
        virtual ~BasicClientS() = default;
    };
protected:
    std::reference_wrapper<asio::io_context> AsioContext;
    asio::ip::tcp::acceptor ConnectionsAcceptor;
    ConsoleManagerNS::OutputNS::OutputtingProcessC* OutputtingProcPtr = nullptr;
    //first client is not active, it is waiting for connection
    std::vector<std::unique_ptr<BasicClientS>> Clients;
private:
    void _RemoveClient(BasicClientS& client);
    void _StartReading(BasicClientS& client);
protected:
    virtual BasicClientS& ClientFactory();
public:

    inline void sOutputtingProcPtr(ConsoleManagerNS::OutputNS::OutputtingProcessC* outputtingProcPtr)
    {
        OutputtingProcPtr = outputtingProcPtr;
    }

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

    void StartAcceptingConnections();
protected:
    //connected client is always first in list
    inline virtual void OnConnect() {};
    inline virtual void OnDisconnect(BasicClientS& disconectedClient) {};
    inline virtual void OnRead(BasicClientS& client, size_t bytesRead) {};
private:
    void _WriteToClient(BasicClientS& client, void const* arr, size_t lenInBytes);
public:
    template<typename T> void WriteToClient(BasicClientS& client, T const* arrPtr, size_t arrSize) { _WriteToClient(client, arrPtr, arrSize * sizeof(T)); }
    template<typename T> void WriteToClient(BasicClientS& client, T&& var) { WriteToClient(client, &var, 1); }
    void Shutdown();
    inline std::vector<std::unique_ptr<BasicClientS>> const& gClients() const { return Clients; }
};