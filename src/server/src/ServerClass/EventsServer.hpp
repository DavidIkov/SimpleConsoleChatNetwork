#pragma once
#include"BasicServer.hpp"
#include"NetworkEvents.hpp"
#include"ConsoleManager.hpp"
//server that is working with events from NetworkEvents.hpp
class EventsServerC : public BasicServerC {
private:
    using BasicServerC::IsBasicDestructorLast;
public:
    EventsServerC(asio::io_context& asioContext, asio::ip::port_type port):BasicServerC(asioContext, port){
        IsBasicDestructorLast = false;
    }
    using BasicServerC::BasicServerC;
protected:
    bool IsEventsDestructorLast = true;
public:
    virtual ~EventsServerC() override { if (IsEventsDestructorLast) ThreadSafety.LockThread(); }
protected:
    virtual BasicClientSlotC& ClientFactory(asio::io_context& context) override;
    virtual void SetUpCallbacksForNewClient(BasicClientSlotC& client) override;
public:
    inline virtual void OnEvent(BasicClientSlotC& client, NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) {};
};