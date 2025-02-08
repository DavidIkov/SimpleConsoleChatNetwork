#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
class ClientC {
protected:
    std::reference_wrapper<asio::io_context> Context;
    asio::ip::tcp::socket Socket;
    char SocketBuffer[64];
    ConsoleManagerNS::OutputNS::OutputtingProcessC* OutputtingProcPtr = nullptr;
    void _StartReading_Async();
public:
    ClientC(asio::io_context& context);
    virtual ~ClientC() = default;

    inline void sOutputtingProcPtr(ConsoleManagerNS::OutputNS::OutputtingProcessC* outputtingProcPtr)
        { OutputtingProcPtr = outputtingProcPtr; }

    //used to call sOutputtingProcPtr automatically, enabling it on construction and disabling on destruction
    class OutputtingProcPtrWrapperC {
        ClientC& Client;
    public:
        inline OutputtingProcPtrWrapperC(ClientC& client, ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) :
            Client(client) {
            Client.sOutputtingProcPtr(&outProc);
        }
        inline ~OutputtingProcPtrWrapperC() { Client.sOutputtingProcPtr(nullptr); }
    };

    virtual void Connect(asio::ip::tcp::endpoint ep);
    virtual void Disconnect();
protected:
    inline virtual void OnConnect() {};
    inline virtual void OnDisconnect() {};
    virtual void OnRead(size_t bytesRead);
public:
    void Write(const std::string_view& data);
};