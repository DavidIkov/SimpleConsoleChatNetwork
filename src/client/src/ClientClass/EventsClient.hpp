#pragma once
#include"BasicClient.hpp"
#include"NetworkEvents.hpp"
//client that is working with events from NetworkEvents.hpp
class EventsClientC :virtual public BasicClientC {
public:
    using BasicClientC::BasicClientC;
    virtual ~EventsClientC() = default;
private:
    using BasicClientC::Write;

    struct{
        //in this case END_OF_ENUM will be used to mark that no event is begin received at the moment
        NetworkEventsNS::EventsTypesToClientE Type;
        NetworkEventsNS::EventTypeToClientU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(size_t bytesLeft, char* start);
    virtual void OnRead(size_t bytesRead) override final { _OnReadWithOffset(bytesRead, SocketBuffer); }
public:
    inline virtual void OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) {};
    template<NetworkEventsNS::EventsTypesToServerE EventTypeEnum>
    void SendEvent(const NetworkEventsNS::EventTypeToServerS<EventTypeEnum>& eventData) {
        Write(EventTypeEnum);
        Write(eventData);
    }
};