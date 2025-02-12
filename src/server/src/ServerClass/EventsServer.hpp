#pragma once
#include"BasicServer.hpp"
#include"NetworkEvents.hpp"
//server that is working with events from NetworkEvents.hpp
class EventsServerC :virtual public BasicServerC {
public:
    using BasicServerC::BasicServerC;
    virtual ~EventsServerC() = default;
private:
    using BasicServerC::WriteToClient;

    struct{
        //in this case END_OF_ENUM will be used to mark that no event is begin received at the moment
        NetworkEventsNS::EventsTypesToServerE Type;
        NetworkEventsNS::EventTypeToServerU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(ClientS& client, size_t bytesLeft, char* start);
    virtual void OnRead(ClientS& client, size_t bytesRead) override final { _OnReadWithOffset(client, bytesRead, client.ReadBuffer); }
public:
    inline virtual void OnConnect() override final {
        OnEvent(*ActiveClients.begin(), NetworkEventsNS::EventsTypesToServerE::ClientConnected, {});
    };
    inline virtual void OnDisconnect(ClientS& disconectedClient) override final {
        OnEvent(disconectedClient, NetworkEventsNS::EventsTypesToServerE::ClientDisconnected, {});
    }
    inline virtual void OnEvent(ClientS& client, NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) {};
    template<NetworkEventsNS::EventsTypesToServerE EventTypeEnum>
    void SendEvent(ClientS& client, const NetworkEventsNS::EventTypeToServerS<EventTypeEnum>& eventData) {
        WriteToClient(client, EventTypeEnum);
        WriteToClient(client, eventData);
    }
};