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
    inline bool gIsConnected() const { return Socket.is_open(); }
    void Connect(asio::ip::tcp::endpoint ep);
    void Disconnect();
protected:
    inline virtual void OnConnect() {};
    inline virtual void OnDisconnect() {};
    //by default just prints out everything received
    virtual void OnRead(size_t bytesRead);
private:
    void _Write(void const* arr, size_t lenInBytes);
public:
    template<typename T> void Write(T const* arrPtr, size_t arrSize) { _Write(arrPtr, arrSize * sizeof(T)); }
    template<typename T> void Write(T&& var) { Write(&var, 1); }
};