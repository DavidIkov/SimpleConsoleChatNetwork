#pragma once
#include"ConsoleManager.hpp"
#include"ClientSlotClass/BasicClientSlot.hpp"
#include<vector>

class BasicServerC: protected ThreadSafety_BaseC {
private:
    asio::ip::tcp::acceptor ConnectionsAcceptor;
protected:
    std::reference_wrapper<asio::io_context> AsioContext;
    //first client is not active, it is waiting for connection
    std::vector<std::unique_ptr<BasicClientSlotC>> Clients;
protected:
    void _RemoveClient(BasicClientSlotC& client);
public:
    enum class DisconnectResultE :unsigned char {
        NotConnectedToAnything, AlreadyDisconnecting, Canceled,
        UnknownErrorOnSocketClosureButSuccessfullDisconnect, UnknownError, NoErrors
    };
public:
    void RemoveClient(BasicClientSlotC& client) {
        ThreadLockC TL(this);
        _RemoveClient(client);
    }
protected:
    virtual BasicClientSlotC& ClientFactory();
public:
    BasicServerC(asio::io_context& asioContext, asio::ip::port_type port);
    BasicServerC(const BasicServerC&) = delete;
    BasicServerC(BasicServerC&&) = delete;
protected:
    bool IsBasicServerDestructorLast = true;
public:
    inline virtual ~BasicServerC() { if (IsBasicServerDestructorLast) ThreadSafety.Data->Mutex.lock(); }
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
};