#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
#include<vector>

//todo same as with BasicClient.hpp
#include"NetworkEvents.hpp"

class BasicServerC {
public:
    struct BasicClientS {
        asio::ip::tcp::socket Socket;
        inline bool gIsActive() const { return Socket.is_open(); }
        char ReadBuffer[64];
        inline BasicClientS(asio::io_context& context) :Socket(context) {};
        virtual ~BasicClientS() = default;
    };
private:
    asio::ip::tcp::acceptor ConnectionsAcceptor;
protected:
    std::reference_wrapper<asio::io_context> AsioContext;
    //first client is not active, it is waiting for connection
    std::vector<std::unique_ptr<BasicClientS>> Clients;
    std::recursive_mutex ServerMutex;
private:
    void _RemoveClient(BasicClientS& client);
    void _StartReading(BasicClientS& client);
protected:
    virtual BasicClientS& ClientFactory();
public:
    BasicServerC(asio::io_context& asioContext, asio::ip::port_type port);
    BasicServerC(const BasicServerC&) = delete;
    BasicServerC(BasicServerC&&) = delete;
    virtual ~BasicServerC();
    BasicServerC& operator=(const BasicServerC&) = delete;
    BasicServerC& operator=(BasicServerC&&) = delete;

    void Shutdown();

protected:
    enum class OnAcceptConnectionErrorE :unsigned char {
        ServerClosedAcceptor, UnknownError
    };
    inline virtual void OnAcceptConnectionError(OnAcceptConnectionErrorE) {};
public:
    void StartAcceptingConnections();
protected:
    //connected client is always first in vector
    inline virtual void OnConnect() {};
    using DisconnectReasonE = NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientDisconnected>::DisconnectReasonE;
    inline virtual void OnDisconnect(BasicClientS& disconectedClient, DisconnectReasonE) {};
    inline virtual void OnRead(BasicClientS& client, size_t bytesRead) {};
public:
    enum class WriteToClientResultE :unsigned char {
        ClientIsNotActive, StoppedByServer, UknownError, NoErrors,
    };
private:
    WriteToClientResultE _WriteToClient(BasicClientS& client, void const* arr, size_t lenInBytes);
public:
    template<typename T> WriteToClientResultE WriteToClient(BasicClientS& client, T const* arrPtr, size_t arrSize) { return _WriteToClient(client, arrPtr, arrSize * sizeof(T)); }
    template<typename T> WriteToClientResultE WriteToClient(BasicClientS& client, T&& var) { return WriteToClient(client, &var, 1); }
};