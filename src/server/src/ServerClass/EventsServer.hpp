#pragma once
#include"BasicServer.hpp"
#include"NetworkEvents.hpp"
#include"ConsoleManager.hpp"
//server that is working with events from NetworkEvents.hpp
class EventsServerC : public BasicServerC {
public:
    using BasicServerC::BasicServerC;
    virtual ~EventsServerC() override = default;
private:
    using BasicServerC::WriteToClientResultE;
    using BasicServerC::WriteToClient;
    using BasicServerC::OnConnect;
    using BasicServerC::OnDisconnect;
    using BasicServerC::OnRead;

    struct{
        //in this case END_OF_ENUM will be used to mark that no event is begin received at the moment
        NetworkEventsNS::EventsTypesToServerE Type;
        NetworkEventsNS::EventTypeToServerU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(BasicClientS& client, size_t bytesLeft, char* start);//dosent lock mutex
    virtual void OnRead(BasicClientS& client, size_t bytesRead) override final {
        std::lock_guard lg(ServerMutex);
        _OnReadWithOffset(client, bytesRead, client.ReadBuffer);
    }
public:
    inline virtual void OnConnect() override final {
        OnEvent(*Clients.begin()->get(), NetworkEventsNS::EventsTypesToServerE::ClientConnected, {});
    };
    inline virtual void OnDisconnect(BasicClientS& disconectedClient, DisconnectReasonE reason) override final {
        NetworkEventsNS::EventTypeToServerU ev; ev.ClientDisconnected = { reason };
        OnEvent(disconectedClient, NetworkEventsNS::EventsTypesToServerE::ClientDisconnected, ev);
    }
    inline virtual void OnEvent(BasicClientS& client, NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) {};
    template<NetworkEventsNS::EventsTypesToClientE EventTypeEnum>
    void SendEvent(BasicClientS& client, const NetworkEventsNS::EventTypeToClientS<EventTypeEnum>& eventData) {
        //todo handle errors from WriteToClient
        WriteToClient(client, EventTypeEnum);
        WriteToClient(client, eventData);
    }
    
};