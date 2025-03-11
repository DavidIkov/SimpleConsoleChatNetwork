#pragma once
#include"ConsoleManager.hpp"
#include"ClientSlotClass/BasicClientSlot.hpp"
#include<vector>

class BasicServerC: protected ThreadSafety_BaseC {
private:
    asio::ip::tcp::acceptor ConnectionsAcceptor;
    std::reference_wrapper<asio::io_context> AsioContext;
protected:
    //last client is not active, it is waiting for connection
    std::vector<std::unique_ptr<BasicClientSlotC>> Clients;
public:
    void RemoveClient(BasicClientSlotC& client);
protected:
    virtual BasicClientSlotC& ClientFactory(asio::io_context& context);
    virtual void SetUpCallbacksForNewClient(BasicClientSlotC& client);
    inline virtual void OnConnect(BasicClientSlotC& client) {}
    using DisconnectReasonE = BasicClientSlotC::DisconnectReasonE;
    inline virtual void OnDisconnect(BasicClientSlotC& client, DisconnectReasonE reason) {}
    inline virtual void OnAcceptConnectionFailure() {}
public:
    BasicServerC(asio::io_context& asioContext, asio::ip::port_type port);
    BasicServerC(const BasicServerC&) = delete;
    BasicServerC(BasicServerC&&) = delete;
protected:
    bool IsBasicDestructorLast = true;
public:
    inline virtual ~BasicServerC() { if (IsBasicDestructorLast) ThreadSafety.LockThread(); }
    BasicServerC& operator=(const BasicServerC&) = delete;
    BasicServerC& operator=(BasicServerC&&) = delete;

    void Shutdown();

public:
    void StartAcceptingConnections();
};