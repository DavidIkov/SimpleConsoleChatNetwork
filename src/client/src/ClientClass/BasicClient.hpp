#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
//basic client functionality
class BasicClientC {
protected:
    std::reference_wrapper<asio::io_context> Context;
    asio::ip::tcp::socket Socket;
    char SocketBuffer[64];
    ConsoleManagerNS::OutputNS::OutputtingProcessC* OutputtingProcPtr = nullptr;
    void _StartReading_Async();
public:
    BasicClientC(asio::io_context& context);
    virtual ~BasicClientC() = default;

    inline void sOutputtingProcPtr(ConsoleManagerNS::OutputNS::OutputtingProcessC* outputtingProcPtr)
        { OutputtingProcPtr = outputtingProcPtr; }

    //used to call sOutputtingProcPtr automatically, enabling it on construction and disabling on destruction
    class OutputtingProcPtrWrapperC {
        BasicClientC& Client;
    public:
        inline OutputtingProcPtrWrapperC(BasicClientC& client, ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) :
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
    //by default just prints out everything received
    virtual void OnRead(size_t bytesRead);
public:
    template<typename T> void Write(T&& var) { Write(std::string_view(&var, sizeof(var))); }
    //used to send data that is stored in some adress
    template<> void Write<const std::string_view&>(const std::string_view& data);
};