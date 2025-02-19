#pragma once
#include"BasicClient.hpp"
#include"NetworkEvents.hpp"
//client that is working with events from NetworkEvents.hpp
class EventsClientC :virtual public BasicClientC {
public:
    using BasicClientC::BasicClientC;
    virtual ~EventsClientC() override = default;
private:
    using BasicClientC::SocketBuffer;
    using BasicClientC::Write;
    using BasicClientC::OnConnect;
    using BasicClientC::OnDisconnect;
    inline virtual void OnConnect() override final {
        std::lock_guard lg(ClientMutex); OnEvent(NetworkEventsNS::EventsTypesToClientE::ConnectedToServer, {});
        BasicClientC::OnConnect();
    }
    inline virtual void OnDisconnect(DisconnectReasonE reason) override final {
        std::lock_guard lg(ClientMutex);
        NetworkEventsNS::EventTypeToClientU ev; ev.DisconnectedFromServer = { reason };
        OnEvent(NetworkEventsNS::EventsTypesToClientE::DisconnectedFromServer, ev);
        BasicClientC::OnDisconnect(reason);
    };
    struct{
        NetworkEventsNS::EventsTypesToClientE Type;
        NetworkEventsNS::EventTypeToClientU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(size_t bytesLeft, char* start);
    virtual void OnRead(size_t bytesRead) override final { _OnReadWithOffset(bytesRead, SocketBuffer); }
protected:
    inline virtual void OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) {};
public:
    template<NetworkEventsNS::EventsTypesToServerE EventTypeEnum>
    void SendEvent(const NetworkEventsNS::EventTypeToServerS<EventTypeEnum>& eventData) {
        Write(EventTypeEnum);
        Write(eventData);
    }
};